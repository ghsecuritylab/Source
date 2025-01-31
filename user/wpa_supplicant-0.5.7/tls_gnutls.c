/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * WPA Supplicant / SSL/TLS interface functions for openssl
 * Copyright (c) 2004-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#ifdef PKCS12_FUNCS
#include <gnutls/pkcs12.h>
#endif /* PKCS12_FUNCS */

#ifdef CONFIG_GNUTLS_EXTRA
#if LIBGNUTLS_VERSION_NUMBER >= 0x010302
#define GNUTLS_IA
#include <gnutls/extra.h>
#if LIBGNUTLS_VERSION_NUMBER == 0x010302
/* This function is not included in the current gnutls/extra.h even though it
 * should be, so define it here as a workaround for the time being. */
int gnutls_ia_verify_endphase(gnutls_session_t session, char *checksum);
#endif /* LIBGNUTLS_VERSION_NUMBER == 0x010302 */
#endif /* LIBGNUTLS_VERSION_NUMBER >= 0x010302 */
#endif /* CONFIG_GNUTLS_EXTRA */

#include "common.h"
#include "tls.h"


#define TLS_RANDOM_SIZE 32
#define TLS_MASTER_SIZE 48


#if LIBGNUTLS_VERSION_NUMBER < 0x010302
/* GnuTLS 1.3.2 added functions for using master secret. Older versions require
 * use of internal structures to get the master_secret and
 * {server,client}_random.
 */
#define GNUTLS_INTERNAL_STRUCTURE_HACK
#endif /* LIBGNUTLS_VERSION_NUMBER < 0x010302 */


#ifdef GNUTLS_INTERNAL_STRUCTURE_HACK
/*
 * It looks like gnutls does not provide access to client/server_random and
 * master_key. This is somewhat unfortunate since these are needed for key
 * derivation in EAP-{TLS,TTLS,PEAP,FAST}. Workaround for now is a horrible
 * hack that copies the gnutls_session_int definition from gnutls_int.h so that
 * we can get the needed information.
 */

typedef u8 uint8;
typedef unsigned char opaque;
typedef struct {
    uint8 suite[2];
} cipher_suite_st;

typedef struct {
	gnutls_connection_end_t entity;
	gnutls_kx_algorithm_t kx_algorithm;
	gnutls_cipher_algorithm_t read_bulk_cipher_algorithm;
	gnutls_mac_algorithm_t read_mac_algorithm;
	gnutls_compression_method_t read_compression_algorithm;
	gnutls_cipher_algorithm_t write_bulk_cipher_algorithm;
	gnutls_mac_algorithm_t write_mac_algorithm;
	gnutls_compression_method_t write_compression_algorithm;
	cipher_suite_st current_cipher_suite;
	opaque master_secret[TLS_MASTER_SIZE];
	opaque client_random[TLS_RANDOM_SIZE];
	opaque server_random[TLS_RANDOM_SIZE];
	/* followed by stuff we are not interested in */
} security_parameters_st;

struct gnutls_session_int {
	security_parameters_st security_parameters;
	/* followed by things we are not interested in */
};
#endif /* LIBGNUTLS_VERSION_NUMBER < 0x010302 */

static int tls_gnutls_ref_count = 0;

struct tls_global {
	/* Data for session resumption */
	void *session_data;
	size_t session_data_size;

	int server;

	int params_set;
	gnutls_certificate_credentials_t xcred;
};

struct tls_connection {
	gnutls_session session;
	char *subject_match, *altsubject_match;
	int read_alerts, write_alerts, failed;

	u8 *pre_shared_secret;
	size_t pre_shared_secret_len;
	int established;
	int verify_peer;

	u8 *push_buf, *pull_buf, *pull_buf_offset;
	size_t push_buf_len, pull_buf_len;

	int params_set;
	gnutls_certificate_credentials_t xcred;

	int tls_ia;
	int final_phase_finished;

#ifdef GNUTLS_IA
	gnutls_ia_server_credentials_t iacred_srv;
	gnutls_ia_client_credentials_t iacred_cli;

	/* Session keys generated in the current phase for inner secret
	 * permutation before generating/verifying PhaseFinished. */
	u8 *session_keys;
	size_t session_keys_len;

	u8 inner_secret[TLS_MASTER_SIZE];
#endif /* GNUTLS_IA */
};


static void tls_log_func(int level, const char *msg)
{
	char *s, *pos;
	if (level == 6 || level == 7) {
		/* These levels seem to be mostly I/O debug and msg dumps */
		return;
	}

	s = os_strdup(msg);
	if (s == NULL)
		return;

	pos = s;
	while (*pos != '\0') {
		if (*pos == '\n') {
			*pos = '\0';
			break;
		}
		pos++;
	}
	wpa_printf(level > 3 ? MSG_MSGDUMP : MSG_DEBUG,
		   "gnutls<%d> %s", level, s);
	os_free(s);
}


extern int wpa_debug_show_keys;

void * tls_init(const struct tls_config *conf)
{
	struct tls_global *global;

#ifdef GNUTLS_INTERNAL_STRUCTURE_HACK
	/* Because of the horrible hack to get master_secret and client/server
	 * random, we need to make sure that the gnutls version is something
	 * that is expected to have same structure definition for the session
	 * data.. */
	const char *ver;
	const char *ok_ver[] = { "1.2.3", "1.2.4", "1.2.5", "1.2.6", "1.2.9",
				 "1.3.2",
				 NULL };
	int i;
#endif /* GNUTLS_INTERNAL_STRUCTURE_HACK */

	global = os_zalloc(sizeof(*global));
	if (global == NULL)
		return NULL;

	if (tls_gnutls_ref_count == 0 && gnutls_global_init() < 0) {
		os_free(global);
		return NULL;
	}
	tls_gnutls_ref_count++;

#ifdef GNUTLS_INTERNAL_STRUCTURE_HACK
	ver = gnutls_check_version(NULL);
	if (ver == NULL) {
		tls_deinit(global);
		return NULL;
	}
	wpa_printf(MSG_DEBUG, "%s - gnutls version %s", __func__, ver);
	for (i = 0; ok_ver[i]; i++) {
		if (strcmp(ok_ver[i], ver) == 0)
			break;
	}
	if (ok_ver[i] == NULL) {
		wpa_printf(MSG_INFO, "Untested gnutls version %s - this needs "
			   "to be tested and enabled in tls_gnutls.c", ver);
		tls_deinit(global);
		return NULL;
	}
#endif /* GNUTLS_INTERNAL_STRUCTURE_HACK */

	gnutls_global_set_log_function(tls_log_func);
	if (wpa_debug_show_keys)
		gnutls_global_set_log_level(11);
	return global;
}


void tls_deinit(void *ssl_ctx)
{
	struct tls_global *global = ssl_ctx;
	if (global) {
		if (global->params_set)
			gnutls_certificate_free_credentials(global->xcred);
		os_free(global->session_data);
		os_free(global);
	}

	tls_gnutls_ref_count--;
	if (tls_gnutls_ref_count == 0)
		gnutls_global_deinit();
}


int tls_get_errors(void *ssl_ctx)
{
	return 0;
}


static ssize_t tls_pull_func(gnutls_transport_ptr ptr, void *buf,
			     size_t len)
{
	struct tls_connection *conn = (struct tls_connection *) ptr;
	u8 *end;
	if (conn->pull_buf == NULL) {
		errno = EWOULDBLOCK;
		return -1;
	}

	end = conn->pull_buf + conn->pull_buf_len;
	if ((size_t) (end - conn->pull_buf_offset) < len)
		len = end - conn->pull_buf_offset;
	os_memcpy(buf, conn->pull_buf_offset, len);
	conn->pull_buf_offset += len;
	if (conn->pull_buf_offset == end) {
		wpa_printf(MSG_DEBUG, "%s - pull_buf consumed", __func__);
		os_free(conn->pull_buf);
		conn->pull_buf = conn->pull_buf_offset = NULL;
		conn->pull_buf_len = 0;
	} else {
		wpa_printf(MSG_DEBUG, "%s - %d bytes remaining in pull_buf",
			   __func__, end - conn->pull_buf_offset);
	}
	return len;
}


static ssize_t tls_push_func(gnutls_transport_ptr ptr, const void *buf,
			     size_t len)
{
	struct tls_connection *conn = (struct tls_connection *) ptr;
	u8 *nbuf;

	nbuf = os_realloc(conn->push_buf, conn->push_buf_len + len);
	if (nbuf == NULL) {
		errno = ENOMEM;
		return -1;
	}
	os_memcpy(nbuf + conn->push_buf_len, buf, len);
	conn->push_buf = nbuf;
	conn->push_buf_len += len;

	return len;
}


static int tls_gnutls_init_session(struct tls_global *global,
				   struct tls_connection *conn)
{
	const int cert_types[2] = { GNUTLS_CRT_X509, 0 };
	const int protos[2] = { GNUTLS_TLS1, 0 };
	int ret;

	ret = gnutls_init(&conn->session,
			  global->server ? GNUTLS_SERVER : GNUTLS_CLIENT);
	if (ret < 0) {
		wpa_printf(MSG_INFO, "TLS: Failed to initialize new TLS "
			   "connection: %s", gnutls_strerror(ret));
		return -1;
	}

	ret = gnutls_set_default_priority(conn->session);
	if (ret < 0)
		goto fail;

	ret = gnutls_certificate_type_set_priority(conn->session, cert_types);
	if (ret < 0)
		goto fail;

	ret = gnutls_protocol_set_priority(conn->session, protos);
	if (ret < 0)
		goto fail;

	gnutls_transport_set_pull_function(conn->session, tls_pull_func);
	gnutls_transport_set_push_function(conn->session, tls_push_func);
	gnutls_transport_set_ptr(conn->session, (gnutls_transport_ptr) conn);

	return 0;

fail:
	wpa_printf(MSG_INFO, "TLS: Failed to setup new TLS connection: %s",
		   gnutls_strerror(ret));
	gnutls_deinit(conn->session);
	return -1;
}


struct tls_connection * tls_connection_init(void *ssl_ctx)
{
	struct tls_global *global = ssl_ctx;
	struct tls_connection *conn;
	int ret;

	conn = os_zalloc(sizeof(*conn));
	if (conn == NULL)
		return NULL;

	if (tls_gnutls_init_session(global, conn)) {
		os_free(conn);
		return NULL;
	}

	if (global->params_set) {
		ret = gnutls_credentials_set(conn->session,
					     GNUTLS_CRD_CERTIFICATE,
					     global->xcred);
		if (ret < 0) {
			wpa_printf(MSG_INFO, "Failed to configure "
				   "credentials: %s", gnutls_strerror(ret));
			os_free(conn);
			return NULL;
		}
	}

	if (gnutls_certificate_allocate_credentials(&conn->xcred)) {
		os_free(conn);
		return NULL;
	}

	return conn;
}


void tls_connection_deinit(void *ssl_ctx, struct tls_connection *conn)
{
	if (conn == NULL)
		return;

#ifdef GNUTLS_IA
	if (conn->iacred_srv)
		gnutls_ia_free_server_credentials(conn->iacred_srv);
	if (conn->iacred_cli)
		gnutls_ia_free_client_credentials(conn->iacred_cli);
	if (conn->session_keys) {
		os_memset(conn->session_keys, 0, conn->session_keys_len);
		os_free(conn->session_keys);
	}
#endif /* GNUTLS_IA */

	gnutls_certificate_free_credentials(conn->xcred);
	gnutls_deinit(conn->session);
	os_free(conn->pre_shared_secret);
	os_free(conn->subject_match);
	os_free(conn->altsubject_match);
	os_free(conn->push_buf);
	os_free(conn->pull_buf);
	os_free(conn);
}


int tls_connection_established(void *ssl_ctx, struct tls_connection *conn)
{
	return conn ? conn->established : 0;
}


int tls_connection_shutdown(void *ssl_ctx, struct tls_connection *conn)
{
	struct tls_global *global = ssl_ctx;
	int ret;

	if (conn == NULL)
		return -1;

	/* Shutdown previous TLS connection without notifying the peer
	 * because the connection was already terminated in practice
	 * and "close notify" shutdown alert would confuse AS. */
	gnutls_bye(conn->session, GNUTLS_SHUT_RDWR);
	os_free(conn->push_buf);
	conn->push_buf = NULL;
	conn->push_buf_len = 0;
	conn->established = 0;
	conn->final_phase_finished = 0;
#ifdef GNUTLS_IA
	if (conn->session_keys) {
		os_memset(conn->session_keys, 0, conn->session_keys_len);
		os_free(conn->session_keys);
	}
	conn->session_keys_len = 0;
#endif /* GNUTLS_IA */

	gnutls_deinit(conn->session);
	if (tls_gnutls_init_session(global, conn)) {
		wpa_printf(MSG_INFO, "GnuTLS: Failed to preparare new session "
			   "for session resumption use");
		return -1;
	}

	ret = gnutls_credentials_set(conn->session, GNUTLS_CRD_CERTIFICATE,
				     conn->params_set ? conn->xcred :
				     global->xcred);
	if (ret < 0) {
		wpa_printf(MSG_INFO, "GnuTLS: Failed to configure credentials "
			   "for session resumption: %s", gnutls_strerror(ret));
		return -1;
	}

	if (global->session_data) {
		ret = gnutls_session_set_data(conn->session,
					      global->session_data,
					      global->session_data_size);
		if (ret < 0) {
			wpa_printf(MSG_INFO, "GnuTLS: Failed to set session "
				   "data: %s", gnutls_strerror(ret));
			return -1;
		}
	}

	return 0;
}


#if 0
static int tls_match_altsubject(X509 *cert, const char *match)
{
	GENERAL_NAME *gen;
	char *field, *tmp;
	void *ext;
	int i, found = 0;
	size_t len;

	ext = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);

	for (i = 0; ext && i < sk_GENERAL_NAME_num(ext); i++) {
		gen = sk_GENERAL_NAME_value(ext, i);
		switch (gen->type) {
		case GEN_EMAIL:
			field = "EMAIL";
			break;
		case GEN_DNS:
			field = "DNS";
			break;
		case GEN_URI:
			field = "URI";
			break;
		default:
			field = NULL;
			wpa_printf(MSG_DEBUG, "TLS: altSubjectName: "
				   "unsupported type=%d", gen->type);
			break;
		}

		if (!field)
			continue;

		wpa_printf(MSG_DEBUG, "TLS: altSubjectName: %s:%s",
			   field, gen->d.ia5->data);
		len = os_strlen(field) + 1 +
			strlen((char *) gen->d.ia5->data) + 1;
		tmp = os_malloc(len);
		if (tmp == NULL)
			continue;
		snprintf(tmp, len, "%s:%s", field, gen->d.ia5->data);
		if (strstr(tmp, match))
			found++;
		os_free(tmp);
	}

	return found;
}
#endif


#if 0
static int tls_verify_cb(int preverify_ok, X509_STORE_CTX *x509_ctx)
{
	char buf[256];
	X509 *err_cert;
	int err, depth;
	SSL *ssl;
	struct tls_connection *conn;
	char *match, *altmatch;

	err_cert = X509_STORE_CTX_get_current_cert(x509_ctx);
	err = X509_STORE_CTX_get_error(x509_ctx);
	depth = X509_STORE_CTX_get_error_depth(x509_ctx);
	ssl = X509_STORE_CTX_get_ex_data(x509_ctx,
					 SSL_get_ex_data_X509_STORE_CTX_idx());
	X509_NAME_oneline(X509_get_subject_name(err_cert), buf, sizeof(buf));

	conn = SSL_get_app_data(ssl);
	match = conn ? conn->subject_match : NULL;
	altmatch = conn ? conn->altsubject_match : NULL;

	if (!preverify_ok) {
		wpa_printf(MSG_WARNING, "TLS: Certificate verification failed,"
			   " error %d (%s) depth %d for '%s'", err,
			   X509_verify_cert_error_string(err), depth, buf);
	} else {
		wpa_printf(MSG_DEBUG, "TLS: tls_verify_cb - "
			   "preverify_ok=%d err=%d (%s) depth=%d buf='%s'",
			   preverify_ok, err,
			   X509_verify_cert_error_string(err), depth, buf);
		if (depth == 0 && match && strstr(buf, match) == NULL) {
			wpa_printf(MSG_WARNING, "TLS: Subject '%s' did not "
				   "match with '%s'", buf, match);
			preverify_ok = 0;
		} else if (depth == 0 && altmatch &&
			   !tls_match_altsubject(err_cert, altmatch)) {
			wpa_printf(MSG_WARNING, "TLS: altSubjectName match "
				   "'%s' not found", altmatch);
			preverify_ok = 0;
		}
	}

	return preverify_ok;
}
#endif


int tls_connection_set_params(void *tls_ctx, struct tls_connection *conn,
			      const struct tls_connection_params *params)
{
	int ret;

	if (conn == NULL || params == NULL)
		return -1;

	os_free(conn->subject_match);
	conn->subject_match = NULL;
	if (params->subject_match) {
		conn->subject_match = os_strdup(params->subject_match);
		if (conn->subject_match == NULL)
			return -1;
	}

	os_free(conn->altsubject_match);
	conn->altsubject_match = NULL;
	if (params->altsubject_match) {
		conn->altsubject_match = os_strdup(params->altsubject_match);
		if (conn->altsubject_match == NULL)
			return -1;
	}

	/* TODO: gnutls_certificate_set_verify_flags(xcred, flags); 
	 * to force peer validation(?) */

	if (params->ca_cert) {
		conn->verify_peer = 1;
		ret = gnutls_certificate_set_x509_trust_file(
			conn->xcred, params->ca_cert, GNUTLS_X509_FMT_PEM);
		if (ret < 0) {
			wpa_printf(MSG_DEBUG, "Failed to read CA cert '%s' "
				   "in PEM format: %s", params->ca_cert,
				   gnutls_strerror(ret));
			ret = gnutls_certificate_set_x509_trust_file(
				conn->xcred, params->ca_cert,
				GNUTLS_X509_FMT_DER);
			if (ret < 0) {
				wpa_printf(MSG_DEBUG, "Failed to read CA cert "
					   "'%s' in DER format: %s",
					   params->ca_cert,
					   gnutls_strerror(ret));
				return -1;
			}
		}
	}

	if (params->client_cert && params->private_key) {
		/* TODO: private_key_passwd? */
		ret = gnutls_certificate_set_x509_key_file(
			conn->xcred, params->client_cert, params->private_key,
			GNUTLS_X509_FMT_PEM);
		if (ret < 0) {
			wpa_printf(MSG_DEBUG, "Failed to read client cert/key "
				   "in PEM format: %s", gnutls_strerror(ret));
			ret = gnutls_certificate_set_x509_key_file(
				conn->xcred, params->client_cert,
				params->private_key, GNUTLS_X509_FMT_DER);
			if (ret < 0) {
				wpa_printf(MSG_DEBUG, "Failed to read client "
					   "cert/key in DER format: %s",
					   gnutls_strerror(ret));
				return ret;
			}
		}
	} else if (params->private_key) {
		int pkcs12_ok = 0;
#ifdef PKCS12_FUNCS
		/* Try to load in PKCS#12 format */
#if LIBGNUTLS_VERSION_NUMBER >= 0x010302
		ret = gnutls_certificate_set_x509_simple_pkcs12_file(
			conn->xcred, params->private_key, GNUTLS_X509_FMT_DER,
			params->private_key_passwd);
		if (ret != 0) {
			wpa_printf(MSG_DEBUG, "Failed to load private_key in "
				   "PKCS#12 format: %s", gnutls_strerror(ret));
			return -1;
		} else
			pkcs12_ok = 1;
#endif /* LIBGNUTLS_VERSION_NUMBER >= 0x010302 */
#endif /* PKCS12_FUNCS */

		if (!pkcs12_ok) {
			wpa_printf(MSG_DEBUG, "GnuTLS: PKCS#12 support not "
				   "included");
			return -1;
		}
	}

	conn->tls_ia = params->tls_ia;
	conn->params_set = 1;

	ret = gnutls_credentials_set(conn->session, GNUTLS_CRD_CERTIFICATE,
				     conn->xcred);
	if (ret < 0) {
		wpa_printf(MSG_INFO, "Failed to configure credentials: %s",
			   gnutls_strerror(ret));
	}

#ifdef GNUTLS_IA
	if (conn->iacred_cli)
		gnutls_ia_free_client_credentials(conn->iacred_cli);

	ret = gnutls_ia_allocate_client_credentials(&conn->iacred_cli);
	if (ret) {
		wpa_printf(MSG_DEBUG, "Failed to allocate IA credentials: %s",
			   gnutls_strerror(ret));
		return -1;
	}

	ret = gnutls_credentials_set(conn->session, GNUTLS_CRD_IA,
				     conn->iacred_cli);
	if (ret) {
		wpa_printf(MSG_DEBUG, "Failed to configure IA credentials: %s",
			   gnutls_strerror(ret));
		gnutls_ia_free_client_credentials(conn->iacred_cli);
		conn->iacred_cli = NULL;
		return -1;
	}
#endif /* GNUTLS_IE */

	return ret;
}


int tls_global_set_params(void *tls_ctx,
			  const struct tls_connection_params *params)
{
	struct tls_global *global = tls_ctx;
	int ret;

	/* Currently, global parameters are only set when running in server
	 * mode. */
	global->server = 1;

	if (global->params_set) {
		gnutls_certificate_free_credentials(global->xcred);
		global->params_set = 0;
	}

	ret = gnutls_certificate_allocate_credentials(&global->xcred);
	if (ret) {
		wpa_printf(MSG_DEBUG, "Failed to allocate global credentials "
			   "%s", gnutls_strerror(ret));
		return -1;
	}

	if (params->ca_cert) {
		ret = gnutls_certificate_set_x509_trust_file(
			global->xcred, params->ca_cert, GNUTLS_X509_FMT_PEM);
		if (ret < 0) {
			wpa_printf(MSG_DEBUG, "Failed to read CA cert '%s' "
				   "in PEM format: %s", params->ca_cert,
				   gnutls_strerror(ret));
			ret = gnutls_certificate_set_x509_trust_file(
				global->xcred, params->ca_cert,
				GNUTLS_X509_FMT_DER);
			if (ret < 0) {
				wpa_printf(MSG_DEBUG, "Failed to read CA cert "
					   "'%s' in DER format: %s",
					   params->ca_cert,
					   gnutls_strerror(ret));
				goto fail;
			}
		}
	}

	if (params->client_cert && params->private_key) {
		/* TODO: private_key_passwd? */
		ret = gnutls_certificate_set_x509_key_file(
			global->xcred, params->client_cert,
			params->private_key, GNUTLS_X509_FMT_PEM);
		if (ret < 0) {
			wpa_printf(MSG_DEBUG, "Failed to read client cert/key "
				   "in PEM format: %s", gnutls_strerror(ret));
			ret = gnutls_certificate_set_x509_key_file(
				global->xcred, params->client_cert,
				params->private_key, GNUTLS_X509_FMT_DER);
			if (ret < 0) {
				wpa_printf(MSG_DEBUG, "Failed to read client "
					   "cert/key in DER format: %s",
					   gnutls_strerror(ret));
				goto fail;
			}
		}
	} else if (params->private_key) {
		int pkcs12_ok = 0;
#ifdef PKCS12_FUNCS
		/* Try to load in PKCS#12 format */
#if LIBGNUTLS_VERSION_NUMBER >= 0x010302
		ret = gnutls_certificate_set_x509_simple_pkcs12_file(
			global->xcred, params->private_key,
			GNUTLS_X509_FMT_DER, params->private_key_passwd);
		if (ret != 0) {
			wpa_printf(MSG_DEBUG, "Failed to load private_key in "
				   "PKCS#12 format: %s", gnutls_strerror(ret));
			goto fail;
		} else
			pkcs12_ok = 1;
#endif /* LIBGNUTLS_VERSION_NUMBER >= 0x010302 */
#endif /* PKCS12_FUNCS */

		if (!pkcs12_ok) {
			wpa_printf(MSG_DEBUG, "GnuTLS: PKCS#12 support not "
				   "included");
			goto fail;
		}
	}

	global->params_set = 1;

	return 0;

fail:
	gnutls_certificate_free_credentials(global->xcred);
	return -1;
}


int tls_global_set_verify(void *ssl_ctx, int check_crl)
{
	/* TODO */
	return 0;
}


int tls_connection_set_verify(void *ssl_ctx, struct tls_connection *conn,
			      int verify_peer)
{
	if (conn == NULL || conn->session == NULL)
		return -1;

	conn->verify_peer = verify_peer;
	gnutls_certificate_server_set_request(conn->session,
					      verify_peer ? GNUTLS_CERT_REQUIRE
					      : GNUTLS_CERT_REQUEST);

	return 0;
}


int tls_connection_get_keys(void *ssl_ctx, struct tls_connection *conn,
			    struct tls_keys *keys)
{
#ifdef GNUTLS_INTERNAL_STRUCTURE_HACK
	security_parameters_st *sec;
#endif /* GNUTLS_INTERNAL_STRUCTURE_HACK */

	if (conn == NULL || conn->session == NULL || keys == NULL)
		return -1;

	os_memset(keys, 0, sizeof(*keys));

#ifdef GNUTLS_INTERNAL_STRUCTURE_HACK
	sec = &conn->session->security_parameters;
	keys->master_key = sec->master_secret;
	keys->master_key_len = TLS_MASTER_SIZE;
	keys->client_random = sec->client_random;
	keys->server_random = sec->server_random;
#else /* GNUTLS_INTERNAL_STRUCTURE_HACK */
	keys->client_random = gnutls_session_get_client_random(conn->session);
	keys->server_random = gnutls_session_get_server_random(conn->session);
	/* No access to master_secret */
#endif /* GNUTLS_INTERNAL_STRUCTURE_HACK */

#ifdef GNUTLS_IA
	gnutls_ia_extract_inner_secret(conn->session, conn->inner_secret);
	keys->inner_secret = conn->inner_secret;
	keys->inner_secret_len = TLS_MASTER_SIZE;
#endif /* GNUTLS_IA */

	keys->client_random_len = TLS_RANDOM_SIZE;
	keys->server_random_len = TLS_RANDOM_SIZE;

	return 0;
}


int tls_connection_prf(void *tls_ctx, struct tls_connection *conn,
		       const char *label, int server_random_first,
		       u8 *out, size_t out_len)
{
#if LIBGNUTLS_VERSION_NUMBER >= 0x010302
	if (conn == NULL || conn->session == NULL)
		return -1;

	return gnutls_prf(conn->session, os_strlen(label), label,
			  server_random_first, 0, NULL, out_len, out);
#else /* LIBGNUTLS_VERSION_NUMBER >= 0x010302 */
	return -1;
#endif /* LIBGNUTLS_VERSION_NUMBER >= 0x010302 */
}


static int tls_connection_verify_peer(struct tls_connection *conn)
{
	unsigned int status, num_certs, i;
	struct os_time now;
	const gnutls_datum_t *certs;
	gnutls_x509_crt_t cert;

	if (gnutls_certificate_verify_peers2(conn->session, &status) < 0) {
		wpa_printf(MSG_INFO, "TLS: Failed to verify peer "
			   "certificate chain");
		return -1;
	}

	if (conn->verify_peer && (status & GNUTLS_CERT_INVALID)) {
		wpa_printf(MSG_INFO, "TLS: Peer certificate not trusted");
		return -1;
	}

	if (status & GNUTLS_CERT_SIGNER_NOT_FOUND) {
		wpa_printf(MSG_INFO, "TLS: Peer certificate does not have a "
			   "known issuer");
		return -1;
	}

	if (status & GNUTLS_CERT_REVOKED) {
		wpa_printf(MSG_INFO, "TLS: Peer certificate has been revoked");
		return -1;
	}

	os_get_time(&now);

	certs = gnutls_certificate_get_peers(conn->session, &num_certs);
	if (certs == NULL) {
		wpa_printf(MSG_INFO, "TLS: No peer certificate chain "
			   "received");
		return -1;
	}

	for (i = 0; i < num_certs; i++) {
		char *buf;
		size_t len;
		if (gnutls_x509_crt_init(&cert) < 0) {
			wpa_printf(MSG_INFO, "TLS: Certificate initialization "
				   "failed");
			return -1;
		}

		if (gnutls_x509_crt_import(cert, &certs[i],
					   GNUTLS_X509_FMT_DER) < 0) {
			wpa_printf(MSG_INFO, "TLS: Could not parse peer "
				   "certificate %d/%d", i + 1, num_certs);
			gnutls_x509_crt_deinit(cert);
			return -1;
		}

		gnutls_x509_crt_get_dn(cert, NULL, &len);
		len++;
		buf = os_malloc(len + 1);
		if (buf) {
			buf[0] = buf[len] = '\0';
			gnutls_x509_crt_get_dn(cert, buf, &len);
		}
		wpa_printf(MSG_DEBUG, "TLS: Peer cert chain %d/%d: %s",
			   i + 1, num_certs, buf);

		if (i == 0) {
			/* TODO: validate subject_match and altsubject_match */
		}

		os_free(buf);

		if (gnutls_x509_crt_get_expiration_time(cert) < now.sec ||
		    gnutls_x509_crt_get_activation_time(cert) > now.sec) {
			wpa_printf(MSG_INFO, "TLS: Peer certificate %d/%d is "
				   "not valid at this time",
				   i + 1, num_certs);
			gnutls_x509_crt_deinit(cert);
			return -1;
		}

		gnutls_x509_crt_deinit(cert);
	}

	return 0;
}


u8 * tls_connection_handshake(void *ssl_ctx, struct tls_connection *conn,
			      const u8 *in_data, size_t in_len,
			      size_t *out_len, u8 **appl_data,
			      size_t *appl_data_len)
{
	struct tls_global *global = ssl_ctx;
	u8 *out_data;
	int ret;

	if (appl_data)
		*appl_data = NULL;

	if (in_data && in_len) {
		if (conn->pull_buf) {
			wpa_printf(MSG_DEBUG, "%s - %d bytes remaining in "
				   "pull_buf", __func__, conn->pull_buf_len);
			os_free(conn->pull_buf);
		}
		conn->pull_buf = os_malloc(in_len);
		if (conn->pull_buf == NULL)
			return NULL;
		os_memcpy(conn->pull_buf, in_data, in_len);
		conn->pull_buf_offset = conn->pull_buf;
		conn->pull_buf_len = in_len;
	}

	ret = gnutls_handshake(conn->session);
	if (ret < 0) {
		switch (ret) {
		case GNUTLS_E_AGAIN:
			if (global->server && conn->established &&
			    conn->push_buf == NULL) {
				/* Need to return something to trigger
				 * completion of EAP-TLS. */
				conn->push_buf = os_malloc(1);
			}
			break;
		case GNUTLS_E_FATAL_ALERT_RECEIVED:
			wpa_printf(MSG_DEBUG, "%s - received fatal '%s' alert",
				   __func__, gnutls_alert_get_name(
					   gnutls_alert_get(conn->session)));
			conn->read_alerts++;
			/* continue */
		default:
			wpa_printf(MSG_DEBUG, "%s - gnutls_handshake failed "
				   "-> %s", __func__, gnutls_strerror(ret));
			conn->failed++;
		}
	} else {
		size_t size;

		if (conn->verify_peer && tls_connection_verify_peer(conn)) {
			wpa_printf(MSG_INFO, "TLS: Peer certificate chain "
				   "failed validation");
			conn->failed++;
			return NULL;
		}

		if (conn->tls_ia && !gnutls_ia_handshake_p(conn->session)) {
			wpa_printf(MSG_INFO, "TLS: No TLS/IA negotiation");
			conn->failed++;
			return NULL;
		}

		if (conn->tls_ia)
			wpa_printf(MSG_DEBUG, "TLS: Start TLS/IA handshake");
		else {
			wpa_printf(MSG_DEBUG, "TLS: Handshake completed "
				   "successfully");
		}
		conn->established = 1;
		if (conn->push_buf == NULL) {
			/* Need to return something to get final TLS ACK. */
			conn->push_buf = os_malloc(1);
		}

		gnutls_session_get_data(conn->session, NULL, &size);
		if (global->session_data == NULL ||
		    global->session_data_size < size) {
			os_free(global->session_data);
			global->session_data = os_malloc(size);
		}
		if (global->session_data) {
			global->session_data_size = size;
			gnutls_session_get_data(conn->session,
						global->session_data,
						&global->session_data_size);
		}
	}

	out_data = conn->push_buf;
	*out_len = conn->push_buf_len;
	conn->push_buf = NULL;
	conn->push_buf_len = 0;
	return out_data;
}


u8 * tls_connection_server_handshake(void *ssl_ctx,
				     struct tls_connection *conn,
				     const u8 *in_data, size_t in_len,
				     size_t *out_len)
{
	return tls_connection_handshake(ssl_ctx, conn, in_data, in_len,
					out_len);
}


int tls_connection_encrypt(void *ssl_ctx, struct tls_connection *conn,
			   const u8 *in_data, size_t in_len,
			   u8 *out_data, size_t out_len)
{
	ssize_t res;

#ifdef GNUTLS_IA
	if (conn->tls_ia)
		res = gnutls_ia_send(conn->session, (char *) in_data, in_len);
	else
#endif /* GNUTLS_IA */
	res = gnutls_record_send(conn->session, in_data, in_len);
	if (res < 0) {
		wpa_printf(MSG_INFO, "%s: Encryption failed: %s",
			   __func__, gnutls_strerror(res));
		return -1;
	}
	if (conn->push_buf == NULL)
		return -1;
	if (conn->push_buf_len < out_len)
		out_len = conn->push_buf_len;
	os_memcpy(out_data, conn->push_buf, out_len);
	os_free(conn->push_buf);
	conn->push_buf = NULL;
	conn->push_buf_len = 0;
	return out_len;
}


int tls_connection_decrypt(void *ssl_ctx, struct tls_connection *conn,
			   const u8 *in_data, size_t in_len,
			   u8 *out_data, size_t out_len)
{
	ssize_t res;

	if (conn->pull_buf) {
		wpa_printf(MSG_DEBUG, "%s - %d bytes remaining in "
			   "pull_buf", __func__, conn->pull_buf_len);
		os_free(conn->pull_buf);
	}
	conn->pull_buf = os_malloc(in_len);
	if (conn->pull_buf == NULL)
		return -1;
	os_memcpy(conn->pull_buf, in_data, in_len);
	conn->pull_buf_offset = conn->pull_buf;
	conn->pull_buf_len = in_len;

#ifdef GNUTLS_IA
	if (conn->tls_ia) {
		res = gnutls_ia_recv(conn->session, out_data, out_len);
		if (out_len >= 12 &&
		    (res == GNUTLS_E_WARNING_IA_IPHF_RECEIVED ||
		     res == GNUTLS_E_WARNING_IA_FPHF_RECEIVED)) {
			int final = res == GNUTLS_E_WARNING_IA_FPHF_RECEIVED;
			wpa_printf(MSG_DEBUG, "%s: Received %sPhaseFinished",
				   __func__, final ? "Final" : "Intermediate");

			res = gnutls_ia_permute_inner_secret(
				conn->session, conn->session_keys_len,
				conn->session_keys);
			if (conn->session_keys) {
				os_memset(conn->session_keys, 0,
					  conn->session_keys_len);
				os_free(conn->session_keys);
			}
			conn->session_keys = NULL;
			conn->session_keys_len = 0;
			if (res) {
				wpa_printf(MSG_DEBUG, "%s: Failed to permute "
					   "inner secret: %s",
					   __func__, gnutls_strerror(res));
				return -1;
			}

			res = gnutls_ia_verify_endphase(conn->session,
							out_data);
			if (res == 0) {
				wpa_printf(MSG_DEBUG, "%s: Correct endphase "
					   "checksum", __func__);
			} else {
				wpa_printf(MSG_INFO, "%s: Endphase "
					   "verification failed: %s",
					   __func__, gnutls_strerror(res));
				return -1;
			}

			if (final)
				conn->final_phase_finished = 1;

			return 0;
		}

		if (res < 0) {
			wpa_printf(MSG_DEBUG, "%s - gnutls_ia_recv failed: %d "
				   "(%s)", __func__, res,
				   gnutls_strerror(res));
		}
		return res;
	}
#endif /* GNUTLS_IA */

	res = gnutls_record_recv(conn->session, out_data, out_len);
	if (res < 0) {
		wpa_printf(MSG_DEBUG, "%s - gnutls_record_recv failed: %d "
			   "(%s)", __func__, res, gnutls_strerror(res));
	}

	return res;
}


int tls_connection_resumed(void *ssl_ctx, struct tls_connection *conn)
{
	if (conn == NULL)
		return 0;
	return gnutls_session_is_resumed(conn->session);
}


int tls_connection_set_master_key(void *ssl_ctx, struct tls_connection *conn,
				  const u8 *key, size_t key_len)
{
	/* TODO */
	return -1;
}


int tls_connection_set_cipher_list(void *tls_ctx, struct tls_connection *conn,
				   u8 *ciphers)
{
	/* TODO */
	return -1;
}


int tls_get_cipher(void *ssl_ctx, struct tls_connection *conn,
		   char *buf, size_t buflen)
{
	/* TODO */
	buf[0] = '\0';
	return 0;
}


int tls_connection_enable_workaround(void *ssl_ctx,
				     struct tls_connection *conn)
{
	/* TODO: set SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS */
	return 0;
}


int tls_connection_client_hello_ext(void *ssl_ctx, struct tls_connection *conn,
				    int ext_type, const u8 *data,
				    size_t data_len)
{
	/* TODO */
	return -1;
}


int tls_connection_get_failed(void *ssl_ctx, struct tls_connection *conn)
{
	if (conn == NULL)
		return -1;
	return conn->failed;
}


int tls_connection_get_read_alerts(void *ssl_ctx, struct tls_connection *conn)
{
	if (conn == NULL)
		return -1;
	return conn->read_alerts;
}


int tls_connection_get_write_alerts(void *ssl_ctx, struct tls_connection *conn)
{
	if (conn == NULL)
		return -1;
	return conn->write_alerts;
}


int tls_connection_get_keyblock_size(void *tls_ctx,
				     struct tls_connection *conn)
{
	/* TODO */
	return -1;
}


unsigned int tls_capabilities(void *tls_ctx)
{
	unsigned int capa = 0;

#ifdef GNUTLS_IA
	capa |= TLS_CAPABILITY_IA;
#endif /* GNUTLS_IA */

	return capa;
}


int tls_connection_set_ia(void *tls_ctx, struct tls_connection *conn,
			  int tls_ia)
{
#ifdef GNUTLS_IA
	int ret;

	if (conn == NULL)
		return -1;

	conn->tls_ia = tls_ia;
	if (!tls_ia)
		return 0;

	ret = gnutls_ia_allocate_server_credentials(&conn->iacred_srv);
	if (ret) {
		wpa_printf(MSG_DEBUG, "Failed to allocate IA credentials: %s",
			   gnutls_strerror(ret));
		return -1;
	}

	ret = gnutls_credentials_set(conn->session, GNUTLS_CRD_IA,
				     conn->iacred_srv);
	if (ret) {
		wpa_printf(MSG_DEBUG, "Failed to configure IA credentials: %s",
			   gnutls_strerror(ret));
		gnutls_ia_free_server_credentials(conn->iacred_srv);
		conn->iacred_srv = NULL;
		return -1;
	}

	return 0;
#else /* GNUTLS_IA */
	return -1;
#endif /* GNUTLS_IA */
}


int tls_connection_ia_send_phase_finished(void *tls_ctx,
					  struct tls_connection *conn,
					  int final,
					  u8 *out_data, size_t out_len)
{
#ifdef GNUTLS_IA
	int ret;

	if (conn == NULL || conn->session == NULL || !conn->tls_ia)
		return -1;

	ret = gnutls_ia_permute_inner_secret(conn->session,
					     conn->session_keys_len,
					     conn->session_keys);
	if (conn->session_keys) {
		os_memset(conn->session_keys, 0, conn->session_keys_len);
		os_free(conn->session_keys);
	}
	conn->session_keys = NULL;
	conn->session_keys_len = 0;
	if (ret) {
		wpa_printf(MSG_DEBUG, "%s: Failed to permute inner secret: %s",
			   __func__, gnutls_strerror(ret));
		return -1;
	}

	ret = gnutls_ia_endphase_send(conn->session, final);
	if (ret) {
		wpa_printf(MSG_DEBUG, "%s: Failed to send endphase: %s",
			   __func__, gnutls_strerror(ret));
		return -1;
	}

	if (conn->push_buf == NULL)
		return -1;
	if (conn->push_buf_len < out_len)
		out_len = conn->push_buf_len;
	os_memcpy(out_data, conn->push_buf, out_len);
	os_free(conn->push_buf);
	conn->push_buf = NULL;
	conn->push_buf_len = 0;
	return out_len;
#else /* GNUTLS_IA */
	return -1;
#endif /* GNUTLS_IA */
}


int tls_connection_ia_final_phase_finished(void *tls_ctx,
					   struct tls_connection *conn)
{
	if (conn == NULL)
		return -1;

	return conn->final_phase_finished;
}


int tls_connection_ia_permute_inner_secret(void *tls_ctx,
					   struct tls_connection *conn,
					   const u8 *key, size_t key_len)
{
#ifdef GNUTLS_IA
	if (conn == NULL || !conn->tls_ia)
		return -1;

	if (conn->session_keys) {
		os_memset(conn->session_keys, 0, conn->session_keys_len);
		os_free(conn->session_keys);
	}
	conn->session_keys_len = 0;

	if (key) {
		conn->session_keys = os_malloc(key_len);
		if (conn->session_keys == NULL)
			return -1;
		os_memcpy(conn->session_keys, key, key_len);
		conn->session_keys_len = key_len;
	} else {
		conn->session_keys = NULL;
		conn->session_keys_len = 0;
	}

	return 0;
#else /* GNUTLS_IA */
	return -1;
#endif /* GNUTLS_IA */
}
