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
/* ssl/d1_pkt.c */
/* 
 * DTLS implementation written by Nagendra Modadugu
 * (nagendra@cs.stanford.edu) for the OpenSSL project 2005.  
 */
/* ====================================================================
 * Copyright (c) 1998-2005 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include <errno.h>
#define USE_SOCKETS
#include "ssl_locl.h"
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/pqueue.h>

static int have_handshake_fragment(SSL *s, int type, unsigned char *buf, 
	int len, int peek);
static int dtls1_record_replay_check(SSL *s, DTLS1_BITMAP *bitmap,
	PQ_64BIT *seq_num);
static void dtls1_record_bitmap_update(SSL *s, DTLS1_BITMAP *bitmap);
static DTLS1_BITMAP *dtls1_get_bitmap(SSL *s, SSL3_RECORD *rr, 
    unsigned int *is_next_epoch);
#if 0
static int dtls1_record_needs_buffering(SSL *s, SSL3_RECORD *rr,
	unsigned short *priority, unsigned long *offset);
#endif
static int dtls1_buffer_record(SSL *s, record_pqueue *q,
	PQ_64BIT priority);
static int dtls1_process_record(SSL *s);
#if PQ_64BIT_IS_INTEGER
static PQ_64BIT bytes_to_long_long(unsigned char *bytes, PQ_64BIT *num);
#endif
static void dtls1_clear_timeouts(SSL *s);

/* copy buffered record into SSL structure */
static int
dtls1_copy_record(SSL *s, pitem *item)
    {
    DTLS1_RECORD_DATA *rdata;

    rdata = (DTLS1_RECORD_DATA *)item->data;
    
    if (s->s3->rbuf.buf != NULL)
        OPENSSL_free(s->s3->rbuf.buf);
    
    s->packet = rdata->packet;
    s->packet_length = rdata->packet_length;
    memcpy(&(s->s3->rbuf), &(rdata->rbuf), sizeof(SSL3_BUFFER));
    memcpy(&(s->s3->rrec), &(rdata->rrec), sizeof(SSL3_RECORD));
    
    return(1);
    }


static int
dtls1_buffer_record(SSL *s, record_pqueue *queue, PQ_64BIT priority)
{
    DTLS1_RECORD_DATA *rdata;
	pitem *item;

	rdata = OPENSSL_malloc(sizeof(DTLS1_RECORD_DATA));
	item = pitem_new(priority, rdata);
	if (rdata == NULL || item == NULL)
		{
		if (rdata != NULL) OPENSSL_free(rdata);
		if (item != NULL) pitem_free(item);
		
		SSLerr(SSL_F_DTLS1_BUFFER_RECORD, ERR_R_INTERNAL_ERROR);
		return(0);
		}
	
	rdata->packet = s->packet;
	rdata->packet_length = s->packet_length;
	memcpy(&(rdata->rbuf), &(s->s3->rbuf), sizeof(SSL3_BUFFER));
	memcpy(&(rdata->rrec), &(s->s3->rrec), sizeof(SSL3_RECORD));

	item->data = rdata;

	/* insert should not fail, since duplicates are dropped */
	if (pqueue_insert(queue->q, item) == NULL)
		{
		OPENSSL_free(rdata);
		pitem_free(item);
		return(0);
		}

	s->packet = NULL;
	s->packet_length = 0;
	memset(&(s->s3->rbuf), 0, sizeof(SSL3_BUFFER));
	memset(&(s->s3->rrec), 0, sizeof(SSL3_RECORD));
	
	if (!ssl3_setup_buffers(s))
		{
		SSLerr(SSL_F_DTLS1_BUFFER_RECORD, ERR_R_INTERNAL_ERROR);
		OPENSSL_free(rdata);
		pitem_free(item);
		return(0);
		}
	
	return(1);
    }


static int
dtls1_retrieve_buffered_record(SSL *s, record_pqueue *queue)
    {
    pitem *item;

    item = pqueue_pop(queue->q);
    if (item)
        {
        dtls1_copy_record(s, item);

        OPENSSL_free(item->data);
		pitem_free(item);

        return(1);
        }

    return(0);
    }


/* retrieve a buffered record that belongs to the new epoch, i.e., not processed 
 * yet */
#define dtls1_get_unprocessed_record(s) \
                   dtls1_retrieve_buffered_record((s), \
                   &((s)->d1->unprocessed_rcds))

/* retrieve a buffered record that belongs to the current epoch, ie, processed */
#define dtls1_get_processed_record(s) \
                   dtls1_retrieve_buffered_record((s), \
                   &((s)->d1->processed_rcds))

static int
dtls1_process_buffered_records(SSL *s)
    {
    pitem *item;
    
    item = pqueue_peek(s->d1->unprocessed_rcds.q);
    if (item)
        {
        DTLS1_RECORD_DATA *rdata;
        rdata = (DTLS1_RECORD_DATA *)item->data;
        
        /* Check if epoch is current. */
        if (s->d1->unprocessed_rcds.epoch != s->d1->r_epoch)
            return(1);  /* Nothing to do. */
        
        /* Process all the records. */
        while (pqueue_peek(s->d1->unprocessed_rcds.q))
            {
            dtls1_get_unprocessed_record(s);
            if ( ! dtls1_process_record(s))
                return(0);
            dtls1_buffer_record(s, &(s->d1->processed_rcds), 
                s->s3->rrec.seq_num);
            }
        }

    /* sync epoch numbers once all the unprocessed records 
     * have been processed */
    s->d1->processed_rcds.epoch = s->d1->r_epoch;
    s->d1->unprocessed_rcds.epoch = s->d1->r_epoch + 1;

    return(1);
    }


#if 0

static int
dtls1_get_buffered_record(SSL *s)
	{
	pitem *item;
	PQ_64BIT priority = 
		(((PQ_64BIT)s->d1->handshake_read_seq) << 32) | 
		((PQ_64BIT)s->d1->r_msg_hdr.frag_off);
	
	if ( ! SSL_in_init(s))  /* if we're not (re)negotiating, 
							   nothing buffered */
		return 0;


	item = pqueue_peek(s->d1->rcvd_records);
	if (item && item->priority == priority)
		{
		/* Check if we've received the record of interest.  It must be
		 * a handshake record, since data records as passed up without
		 * buffering */
		DTLS1_RECORD_DATA *rdata;
		item = pqueue_pop(s->d1->rcvd_records);
		rdata = (DTLS1_RECORD_DATA *)item->data;
		
		if (s->s3->rbuf.buf != NULL)
			OPENSSL_free(s->s3->rbuf.buf);
		
		s->packet = rdata->packet;
		s->packet_length = rdata->packet_length;
		memcpy(&(s->s3->rbuf), &(rdata->rbuf), sizeof(SSL3_BUFFER));
		memcpy(&(s->s3->rrec), &(rdata->rrec), sizeof(SSL3_RECORD));
		
		OPENSSL_free(item->data);
		pitem_free(item);
		
		/* s->d1->next_expected_seq_num++; */
		return(1);
		}
	
	return 0;
	}

#endif

static int
dtls1_process_record(SSL *s)
{
    int i,al;
	int clear=0;
    int enc_err;
	SSL_SESSION *sess;
    SSL3_RECORD *rr;
	unsigned int mac_size;
	unsigned char md[EVP_MAX_MD_SIZE];


	rr= &(s->s3->rrec);
    sess = s->session;

	/* At this point, s->packet_length == SSL3_RT_HEADER_LNGTH + rr->length,
	 * and we have that many bytes in s->packet
	 */
	rr->input= &(s->packet[DTLS1_RT_HEADER_LENGTH]);

	/* ok, we can now read from 's->packet' data into 'rr'
	 * rr->input points at rr->length bytes, which
	 * need to be copied into rr->data by either
	 * the decryption or by the decompression
	 * When the data is 'copied' into the rr->data buffer,
	 * rr->input will be pointed at the new buffer */ 

	/* We now have - encrypted [ MAC [ compressed [ plain ] ] ]
	 * rr->length bytes of encrypted compressed stuff. */

	/* check is not needed I believe */
	if (rr->length > SSL3_RT_MAX_ENCRYPTED_LENGTH)
		{
		al=SSL_AD_RECORD_OVERFLOW;
		SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_ENCRYPTED_LENGTH_TOO_LONG);
		goto f_err;
		}

	/* decrypt in place in 'rr->input' */
	rr->data=rr->input;

	enc_err = s->method->ssl3_enc->enc(s,0);
	if (enc_err <= 0)
		{
		if (enc_err == 0)
			/* SSLerr() and ssl3_send_alert() have been called */
			goto err;

		/* otherwise enc_err == -1 */
		goto decryption_failed_or_bad_record_mac;
		}

#ifdef TLS_DEBUG
printf("dec %d\n",rr->length);
{ unsigned int z; for (z=0; z<rr->length; z++) printf("%02X%c",rr->data[z],((z+1)%16)?' ':'\n'); }
printf("\n");
#endif

	/* r->length is now the compressed data plus mac */
if (	(sess == NULL) ||
		(s->enc_read_ctx == NULL) ||
		(s->read_hash == NULL))
    clear=1;

	if (!clear)
		{
		mac_size=EVP_MD_size(s->read_hash);

		if (rr->length > SSL3_RT_MAX_COMPRESSED_LENGTH+mac_size)
			{
#if 0 /* OK only for stream ciphers (then rr->length is visible from ciphertext anyway) */
			al=SSL_AD_RECORD_OVERFLOW;
			SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_PRE_MAC_LENGTH_TOO_LONG);
			goto f_err;
#else
			goto decryption_failed_or_bad_record_mac;
#endif			
			}
		/* check the MAC for rr->input (it's in mac_size bytes at the tail) */
		if (rr->length < mac_size)
			{
#if 0 /* OK only for stream ciphers */
			al=SSL_AD_DECODE_ERROR;
			SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_LENGTH_TOO_SHORT);
			goto f_err;
#else
			goto decryption_failed_or_bad_record_mac;
#endif
			}
		rr->length-=mac_size;
		i=s->method->ssl3_enc->mac(s,md,0);
		if (memcmp(md,&(rr->data[rr->length]),mac_size) != 0)
			{
			goto decryption_failed_or_bad_record_mac;
			}
		}

	/* r->length is now just compressed */
	if (s->expand != NULL)
		{
		if (rr->length > SSL3_RT_MAX_COMPRESSED_LENGTH)
			{
			al=SSL_AD_RECORD_OVERFLOW;
			SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_COMPRESSED_LENGTH_TOO_LONG);
			goto f_err;
			}
		if (!ssl3_do_uncompress(s))
			{
			al=SSL_AD_DECOMPRESSION_FAILURE;
			SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_BAD_DECOMPRESSION);
			goto f_err;
			}
		}

	if (rr->length > SSL3_RT_MAX_PLAIN_LENGTH)
		{
		al=SSL_AD_RECORD_OVERFLOW;
		SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_DATA_LENGTH_TOO_LONG);
		goto f_err;
		}

	rr->off=0;
	/* So at this point the following is true
	 * ssl->s3->rrec.type 	is the type of record
	 * ssl->s3->rrec.length	== number of bytes in record
	 * ssl->s3->rrec.off	== offset to first valid byte
	 * ssl->s3->rrec.data	== where to take bytes from, increment
	 *			   after use :-).
	 */

	/* we have pulled in a full packet so zero things */
	s->packet_length=0;
    dtls1_record_bitmap_update(s, &(s->d1->bitmap));/* Mark receipt of record. */
    return(1);

decryption_failed_or_bad_record_mac:
	/* Separate 'decryption_failed' alert was introduced with TLS 1.0,
	 * SSL 3.0 only has 'bad_record_mac'.  But unless a decryption
	 * failure is directly visible from the ciphertext anyway,
	 * we should not reveal which kind of error occured -- this
	 * might become visible to an attacker (e.g. via logfile) */
	al=SSL_AD_BAD_RECORD_MAC;
	SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC);
f_err:
	ssl3_send_alert(s,SSL3_AL_FATAL,al);
err:
	return(0);
}


/* Call this to get a new input record.
 * It will return <= 0 if more data is needed, normally due to an error
 * or non-blocking IO.
 * When it finishes, one packet has been decoded and can be found in
 * ssl->s3->rrec.type    - is the type of record
 * ssl->s3->rrec.data, 	 - data
 * ssl->s3->rrec.length, - number of bytes
 */
/* used only by dtls1_read_bytes */
int dtls1_get_record(SSL *s)
	{
	int ssl_major,ssl_minor,al;
	int i,n;
	SSL3_RECORD *rr;
	SSL_SESSION *sess;
	unsigned char *p;
	short version;
	DTLS1_BITMAP *bitmap;
    unsigned int is_next_epoch;

	rr= &(s->s3->rrec);
	sess=s->session;

    /* The epoch may have changed.  If so, process all the
     * pending records.  This is a non-blocking operation. */
    if ( ! dtls1_process_buffered_records(s))
        return 0;

	/* if we're renegotiating, then there may be buffered records */
	if (dtls1_get_processed_record(s))
		return 1;

	/* get something from the wire */
again:
	/* check if we have the header */
	if (	(s->rstate != SSL_ST_READ_BODY) ||
		(s->packet_length < DTLS1_RT_HEADER_LENGTH)) 
		{
		n=ssl3_read_n(s, DTLS1_RT_HEADER_LENGTH, s->s3->rbuf.len, 0);
		/* read timeout is handled by dtls1_read_bytes */
		if (n <= 0) return(n); /* error or non-blocking */

		OPENSSL_assert(s->packet_length == DTLS1_RT_HEADER_LENGTH);

		s->rstate=SSL_ST_READ_BODY;

		p=s->packet;

		/* Pull apart the header into the DTLS1_RECORD */
		rr->type= *(p++);
		ssl_major= *(p++);
		ssl_minor= *(p++);
		version=(ssl_major<<8)|ssl_minor;

        /* sequence number is 64 bits, with top 2 bytes = epoch */ 
		n2s(p,rr->epoch);

		memcpy(&(s->s3->read_sequence[2]), p, 6);
		p+=6;

		n2s(p,rr->length);

		/* Lets check version */
		if (!s->first_packet)
			{
			if (version != s->version)
				{
				SSLerr(SSL_F_DTLS1_GET_RECORD,SSL_R_WRONG_VERSION_NUMBER);
				/* Send back error using their
				 * version number :-) */
				s->version=version;
				al=SSL_AD_PROTOCOL_VERSION;
				goto f_err;
				}
			}

		if ((version & 0xff00) != (DTLS1_VERSION & 0xff00))
			{
			SSLerr(SSL_F_DTLS1_GET_RECORD,SSL_R_WRONG_VERSION_NUMBER);
			goto err;
			}

		if (rr->length > SSL3_RT_MAX_ENCRYPTED_LENGTH)
			{
			al=SSL_AD_RECORD_OVERFLOW;
			SSLerr(SSL_F_DTLS1_GET_RECORD,SSL_R_PACKET_LENGTH_TOO_LONG);
			goto f_err;
			}

		/* now s->rstate == SSL_ST_READ_BODY */
		}

	/* s->rstate == SSL_ST_READ_BODY, get and decode the data */

	if (rr->length > s->packet_length-DTLS1_RT_HEADER_LENGTH)
		{
		/* now s->packet_length == DTLS1_RT_HEADER_LENGTH */
		i=rr->length;
		n=ssl3_read_n(s,i,i,1);
		if (n <= 0) return(n); /* error or non-blocking io */

		/* this packet contained a partial record, dump it */
		if ( n != i)
			{
			s->packet_length = 0;
			goto again;
			}

		/* now n == rr->length,
		 * and s->packet_length == DTLS1_RT_HEADER_LENGTH + rr->length */
		}
	s->rstate=SSL_ST_READ_HEADER; /* set state for later operations */

	/* match epochs.  NULL means the packet is dropped on the floor */
	bitmap = dtls1_get_bitmap(s, rr, &is_next_epoch);
	if ( bitmap == NULL)
        {
        s->packet_length = 0;  /* dump this record */
        goto again;   /* get another record */
		}

	/* check whether this is a repeat, or aged record */
	if ( ! dtls1_record_replay_check(s, bitmap, &(rr->seq_num)))
		{
		s->packet_length=0; /* dump this record */
		goto again;     /* get another record */
		}

	/* just read a 0 length packet */
	if (rr->length == 0) goto again;

    /* If this record is from the next epoch (either HM or ALERT), buffer it
     * since it cannot be processed at this time.
     * Records from the next epoch are marked as received even though they are 
     * not processed, so as to prevent any potential resource DoS attack */
    if (is_next_epoch)
        {
        dtls1_record_bitmap_update(s, bitmap);
        dtls1_buffer_record(s, &(s->d1->unprocessed_rcds), rr->seq_num);
        s->packet_length = 0;
        goto again;
        }

    if ( ! dtls1_process_record(s))
        return(0);

	dtls1_clear_timeouts(s);  /* done waiting */
	return(1);

f_err:
	ssl3_send_alert(s,SSL3_AL_FATAL,al);
err:
	return(0);
	}

/* Return up to 'len' payload bytes received in 'type' records.
 * 'type' is one of the following:
 *
 *   -  SSL3_RT_HANDSHAKE (when ssl3_get_message calls us)
 *   -  SSL3_RT_APPLICATION_DATA (when ssl3_read calls us)
 *   -  0 (during a shutdown, no data has to be returned)
 *
 * If we don't have stored data to work from, read a SSL/TLS record first
 * (possibly multiple records if we still don't have anything to return).
 *
 * This function must handle any surprises the peer may have for us, such as
 * Alert records (e.g. close_notify), ChangeCipherSpec records (not really
 * a surprise, but handled as if it were), or renegotiation requests.
 * Also if record payloads contain fragments too small to process, we store
 * them until there is enough for the respective protocol (the record protocol
 * may use arbitrary fragmentation and even interleaving):
 *     Change cipher spec protocol
 *             just 1 byte needed, no need for keeping anything stored
 *     Alert protocol
 *             2 bytes needed (AlertLevel, AlertDescription)
 *     Handshake protocol
 *             4 bytes needed (HandshakeType, uint24 length) -- we just have
 *             to detect unexpected Client Hello and Hello Request messages
 *             here, anything else is handled by higher layers
 *     Application data protocol
 *             none of our business
 */
int dtls1_read_bytes(SSL *s, int type, unsigned char *buf, int len, int peek)
	{
	int al,i,j,ret;
	unsigned int n;
	SSL3_RECORD *rr;
	void (*cb)(const SSL *ssl,int type2,int val)=NULL;

	if (s->s3->rbuf.buf == NULL) /* Not initialized yet */
		if (!ssl3_setup_buffers(s))
			return(-1);

    /* XXX: check what the second '&& type' is about */
	if ((type && (type != SSL3_RT_APPLICATION_DATA) && 
		(type != SSL3_RT_HANDSHAKE) && type) ||
	    (peek && (type != SSL3_RT_APPLICATION_DATA)))
		{
		SSLerr(SSL_F_DTLS1_READ_BYTES, ERR_R_INTERNAL_ERROR);
		return -1;
		}

	/* check whether there's a handshake message (client hello?) waiting */
	if ( (ret = have_handshake_fragment(s, type, buf, len, peek)))
		return ret;

	/* Now s->d1->handshake_fragment_len == 0 if type == SSL3_RT_HANDSHAKE. */

	if (!s->in_handshake && SSL_in_init(s))
		{
		/* type == SSL3_RT_APPLICATION_DATA */
		i=s->handshake_func(s);
		if (i < 0) return(i);
		if (i == 0)
			{
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_SSL_HANDSHAKE_FAILURE);
			return(-1);
			}
		}

start:
	s->rwstate=SSL_NOTHING;

	/* s->s3->rrec.type	    - is the type of record
	 * s->s3->rrec.data,    - data
	 * s->s3->rrec.off,     - offset into 'data' for next read
	 * s->s3->rrec.length,  - number of bytes. */
	rr = &(s->s3->rrec);

	/* get new packet if necessary */
	if ((rr->length == 0) || (s->rstate == SSL_ST_READ_BODY))
		{
		ret=dtls1_get_record(s);
		if (ret <= 0) 
			{
			ret = dtls1_read_failed(s, ret);
			/* anything other than a timeout is an error */
			if (ret <= 0)  
				return(ret);
			else
				goto start;
			}
		}

	/* we now have a packet which can be read and processed */

	if (s->s3->change_cipher_spec /* set when we receive ChangeCipherSpec,
	                               * reset by ssl3_get_finished */
		&& (rr->type != SSL3_RT_HANDSHAKE))
		{
		al=SSL_AD_UNEXPECTED_MESSAGE;
		SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_DATA_BETWEEN_CCS_AND_FINISHED);
		goto err;
		}

	/* If the other end has shut down, throw anything we read away
	 * (even in 'peek' mode) */
	if (s->shutdown & SSL_RECEIVED_SHUTDOWN)
		{
		rr->length=0;
		s->rwstate=SSL_NOTHING;
		return(0);
		}


	if (type == rr->type) /* SSL3_RT_APPLICATION_DATA or SSL3_RT_HANDSHAKE */
		{
		/* make sure that we are not getting application data when we
		 * are doing a handshake for the first time */
		if (SSL_in_init(s) && (type == SSL3_RT_APPLICATION_DATA) &&
			(s->enc_read_ctx == NULL))
			{
			al=SSL_AD_UNEXPECTED_MESSAGE;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_APP_DATA_IN_HANDSHAKE);
			goto f_err;
			}

		if (len <= 0) return(len);

		if ((unsigned int)len > rr->length)
			n = rr->length;
		else
			n = (unsigned int)len;

		memcpy(buf,&(rr->data[rr->off]),n);
		if (!peek)
			{
			rr->length-=n;
			rr->off+=n;
			if (rr->length == 0)
				{
				s->rstate=SSL_ST_READ_HEADER;
				rr->off=0;
				}
			}
		return(n);
		}


	/* If we get here, then type != rr->type; if we have a handshake
	 * message, then it was unexpected (Hello Request or Client Hello). */

	/* In case of record types for which we have 'fragment' storage,
	 * fill that so that we can process the data at a fixed place.
	 */
		{
		unsigned int k, dest_maxlen = 0;
		unsigned char *dest = NULL;
		unsigned int *dest_len = NULL;

		if (rr->type == SSL3_RT_HANDSHAKE)
			{
			dest_maxlen = sizeof s->d1->handshake_fragment;
			dest = s->d1->handshake_fragment;
			dest_len = &s->d1->handshake_fragment_len;
			}
		else if (rr->type == SSL3_RT_ALERT)
			{
			dest_maxlen = sizeof(s->d1->alert_fragment);
			dest = s->d1->alert_fragment;
			dest_len = &s->d1->alert_fragment_len;
			}
                /* else it's a CCS message, or it's wrong */
                else if (rr->type != SSL3_RT_CHANGE_CIPHER_SPEC)
                        {
                          /* Not certain if this is the right error handling */
                          al=SSL_AD_UNEXPECTED_MESSAGE;
                          SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_UNEXPECTED_RECORD);
                          goto f_err;
                        }


		if (dest_maxlen > 0)
			{
            /* XDTLS:  In a pathalogical case, the Client Hello
             *  may be fragmented--don't always expect dest_maxlen bytes */
			if ( rr->length < dest_maxlen)
				{
				s->rstate=SSL_ST_READ_HEADER;
				rr->length = 0;
				goto start;
				}

			/* now move 'n' bytes: */
			for ( k = 0; k < dest_maxlen; k++)
				{
				dest[k] = rr->data[rr->off++];
				rr->length--;
				}
			*dest_len = dest_maxlen;
			}
		}

	/* s->d1->handshake_fragment_len == 12  iff  rr->type == SSL3_RT_HANDSHAKE;
	 * s->d1->alert_fragment_len == 7      iff  rr->type == SSL3_RT_ALERT.
	 * (Possibly rr is 'empty' now, i.e. rr->length may be 0.) */

	/* If we are a client, check for an incoming 'Hello Request': */
	if ((!s->server) &&
		(s->d1->handshake_fragment_len >= DTLS1_HM_HEADER_LENGTH) &&
		(s->d1->handshake_fragment[0] == SSL3_MT_HELLO_REQUEST) &&
		(s->session != NULL) && (s->session->cipher != NULL))
		{
		s->d1->handshake_fragment_len = 0;

		if ((s->d1->handshake_fragment[1] != 0) ||
			(s->d1->handshake_fragment[2] != 0) ||
			(s->d1->handshake_fragment[3] != 0))
			{
			al=SSL_AD_DECODE_ERROR;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_BAD_HELLO_REQUEST);
			goto err;
			}

		/* no need to check sequence number on HELLO REQUEST messages */

		if (s->msg_callback)
			s->msg_callback(0, s->version, SSL3_RT_HANDSHAKE, 
				s->d1->handshake_fragment, 4, s, s->msg_callback_arg);

		if (SSL_is_init_finished(s) &&
			!(s->s3->flags & SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS) &&
			!s->s3->renegotiate)
			{
			ssl3_renegotiate(s);
			if (ssl3_renegotiate_check(s))
				{
				i=s->handshake_func(s);
				if (i < 0) return(i);
				if (i == 0)
					{
					SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_SSL_HANDSHAKE_FAILURE);
					return(-1);
					}

				if (!(s->mode & SSL_MODE_AUTO_RETRY))
					{
					if (s->s3->rbuf.left == 0) /* no read-ahead left? */
						{
						BIO *bio;
						/* In the case where we try to read application data,
						 * but we trigger an SSL handshake, we return -1 with
						 * the retry option set.  Otherwise renegotiation may
						 * cause nasty problems in the blocking world */
						s->rwstate=SSL_READING;
						bio=SSL_get_rbio(s);
						BIO_clear_retry_flags(bio);
						BIO_set_retry_read(bio);
						return(-1);
						}
					}
				}
			}
		/* we either finished a handshake or ignored the request,
		 * now try again to obtain the (application) data we were asked for */
		goto start;
		}

	if (s->d1->alert_fragment_len >= DTLS1_AL_HEADER_LENGTH)
		{
		int alert_level = s->d1->alert_fragment[0];
		int alert_descr = s->d1->alert_fragment[1];

		s->d1->alert_fragment_len = 0;

		if (s->msg_callback)
			s->msg_callback(0, s->version, SSL3_RT_ALERT, 
				s->d1->alert_fragment, 2, s, s->msg_callback_arg);

		if (s->info_callback != NULL)
			cb=s->info_callback;
		else if (s->ctx->info_callback != NULL)
			cb=s->ctx->info_callback;

		if (cb != NULL)
			{
			j = (alert_level << 8) | alert_descr;
			cb(s, SSL_CB_READ_ALERT, j);
			}

		if (alert_level == 1) /* warning */
			{
			s->s3->warn_alert = alert_descr;
			if (alert_descr == SSL_AD_CLOSE_NOTIFY)
				{
				s->shutdown |= SSL_RECEIVED_SHUTDOWN;
				return(0);
				}
#if 0
            /* XXX: this is a possible improvement in the future */
			/* now check if it's a missing record */
			if (alert_descr == DTLS1_AD_MISSING_HANDSHAKE_MESSAGE)
				{
				unsigned short seq;
				unsigned int frag_off;
				unsigned char *p = &(s->d1->alert_fragment[2]);

				n2s(p, seq);
				n2l3(p, frag_off);

				dtls1_retransmit_message(s, seq, frag_off, &found);
				if ( ! found  && SSL_in_init(s))
					{
					/* fprintf( stderr,"in init = %d\n", SSL_in_init(s)); */
					/* requested a message not yet sent, 
					   send an alert ourselves */
					ssl3_send_alert(s,SSL3_AL_WARNING,
						DTLS1_AD_MISSING_HANDSHAKE_MESSAGE);
					}
				}
#endif
			}
		else if (alert_level == 2) /* fatal */
			{
			char tmp[16];

			s->rwstate=SSL_NOTHING;
			s->s3->fatal_alert = alert_descr;
			SSLerr(SSL_F_DTLS1_READ_BYTES, SSL_AD_REASON_OFFSET + alert_descr);
			BIO_snprintf(tmp,sizeof tmp,"%d",alert_descr);
			ERR_add_error_data(2,"SSL alert number ",tmp);
			s->shutdown|=SSL_RECEIVED_SHUTDOWN;
			SSL_CTX_remove_session(s->ctx,s->session);
			return(0);
			}
		else
			{
			al=SSL_AD_ILLEGAL_PARAMETER;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_UNKNOWN_ALERT_TYPE);
			goto f_err;
			}

		goto start;
		}

	if (s->shutdown & SSL_SENT_SHUTDOWN) /* but we have not received a shutdown */
		{
		s->rwstate=SSL_NOTHING;
		rr->length=0;
		return(0);
		}

	if (rr->type == SSL3_RT_CHANGE_CIPHER_SPEC)
        {
        struct ccs_header_st ccs_hdr;

		dtls1_get_ccs_header(rr->data, &ccs_hdr);

		if ( ccs_hdr.seq == s->d1->handshake_read_seq)
			{
			/* 'Change Cipher Spec' is just a single byte, so we know
			 * exactly what the record payload has to look like */
			/* XDTLS: check that epoch is consistent */
			if (	(rr->length != DTLS1_CCS_HEADER_LENGTH) || 
				(rr->off != 0) || (rr->data[0] != SSL3_MT_CCS))
				{
				i=SSL_AD_ILLEGAL_PARAMETER;
				SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_BAD_CHANGE_CIPHER_SPEC);
				goto err;
				}
			
			rr->length=0;
			
			if (s->msg_callback)
				s->msg_callback(0, s->version, SSL3_RT_CHANGE_CIPHER_SPEC, 
					rr->data, 1, s, s->msg_callback_arg);
			
			s->s3->change_cipher_spec=1;
			if (!ssl3_do_change_cipher_spec(s))
				goto err;
			
			/* do this whenever CCS is processed */
			dtls1_reset_seq_numbers(s, SSL3_CC_READ);
			
			/* handshake read seq is reset upon handshake completion */
			s->d1->handshake_read_seq++;
			
			goto start;
			}
		else
			{
			rr->length = 0;
			goto start;
			}
		}

	/* Unexpected handshake message (Client Hello, or protocol violation) */
	if ((s->d1->handshake_fragment_len >= DTLS1_HM_HEADER_LENGTH) && 
		!s->in_handshake)
		{
		struct hm_header_st msg_hdr;
		
		/* this may just be a stale retransmit */
		dtls1_get_message_header(rr->data, &msg_hdr);
		if( rr->epoch != s->d1->r_epoch)
			{
			rr->length = 0;
			goto start;
			}

		if (((s->state&SSL_ST_MASK) == SSL_ST_OK) &&
			!(s->s3->flags & SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS))
			{
#if 0 /* worked only because C operator preferences are not as expected (and
       * because this is not really needed for clients except for detecting
       * protocol violations): */
			s->state=SSL_ST_BEFORE|(s->server)
				?SSL_ST_ACCEPT
				:SSL_ST_CONNECT;
#else
			s->state = s->server ? SSL_ST_ACCEPT : SSL_ST_CONNECT;
#endif
			s->new_session=1;
			}
		i=s->handshake_func(s);
		if (i < 0) return(i);
		if (i == 0)
			{
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_SSL_HANDSHAKE_FAILURE);
			return(-1);
			}

		if (!(s->mode & SSL_MODE_AUTO_RETRY))
			{
			if (s->s3->rbuf.left == 0) /* no read-ahead left? */
				{
				BIO *bio;
				/* In the case where we try to read application data,
				 * but we trigger an SSL handshake, we return -1 with
				 * the retry option set.  Otherwise renegotiation may
				 * cause nasty problems in the blocking world */
				s->rwstate=SSL_READING;
				bio=SSL_get_rbio(s);
				BIO_clear_retry_flags(bio);
				BIO_set_retry_read(bio);
				return(-1);
				}
			}
		goto start;
		}

	switch (rr->type)
		{
	default:
#ifndef OPENSSL_NO_TLS
		/* TLS just ignores unknown message types */
		if (s->version == TLS1_VERSION)
			{
			rr->length = 0;
			goto start;
			}
#endif
		al=SSL_AD_UNEXPECTED_MESSAGE;
		SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_UNEXPECTED_RECORD);
		goto f_err;
	case SSL3_RT_CHANGE_CIPHER_SPEC:
	case SSL3_RT_ALERT:
	case SSL3_RT_HANDSHAKE:
		/* we already handled all of these, with the possible exception
		 * of SSL3_RT_HANDSHAKE when s->in_handshake is set, but that
		 * should not happen when type != rr->type */
		al=SSL_AD_UNEXPECTED_MESSAGE;
		SSLerr(SSL_F_DTLS1_READ_BYTES,ERR_R_INTERNAL_ERROR);
		goto f_err;
	case SSL3_RT_APPLICATION_DATA:
		/* At this point, we were expecting handshake data,
		 * but have application data.  If the library was
		 * running inside ssl3_read() (i.e. in_read_app_data
		 * is set) and it makes sense to read application data
		 * at this point (session renegotiation not yet started),
		 * we will indulge it.
		 */
		if (s->s3->in_read_app_data &&
			(s->s3->total_renegotiations != 0) &&
			((
				(s->state & SSL_ST_CONNECT) &&
				(s->state >= SSL3_ST_CW_CLNT_HELLO_A) &&
				(s->state <= SSL3_ST_CR_SRVR_HELLO_A)
				) || (
					(s->state & SSL_ST_ACCEPT) &&
					(s->state <= SSL3_ST_SW_HELLO_REQ_A) &&
					(s->state >= SSL3_ST_SR_CLNT_HELLO_A)
					)
				))
			{
			s->s3->in_read_app_data=2;
			return(-1);
			}
		else
			{
			al=SSL_AD_UNEXPECTED_MESSAGE;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_UNEXPECTED_RECORD);
			goto f_err;
			}
		}
	/* not reached */

f_err:
	ssl3_send_alert(s,SSL3_AL_FATAL,al);
err:
	return(-1);
	}

int
dtls1_write_app_data_bytes(SSL *s, int type, const void *buf_, int len)
	{
	unsigned int n,tot;
	int i;

	if (SSL_in_init(s) && !s->in_handshake)
		{
		i=s->handshake_func(s);
		if (i < 0) return(i);
		if (i == 0)
			{
			SSLerr(SSL_F_DTLS1_WRITE_APP_DATA_BYTES,SSL_R_SSL_HANDSHAKE_FAILURE);
			return -1;
			}
		}

	tot = s->s3->wnum;
	n = len - tot;

	while( n)
		{
		/* dtls1_write_bytes sends one record at a time, sized according to 
		 * the currently known MTU */
		i = dtls1_write_bytes(s, type, buf_, len);
		if (i <= 0) return i;
		
		if ((i == (int)n) ||
			(type == SSL3_RT_APPLICATION_DATA &&
				(s->mode & SSL_MODE_ENABLE_PARTIAL_WRITE)))
			{
			/* next chunk of data should get another prepended empty fragment
			 * in ciphersuites with known-IV weakness: */
			s->s3->empty_fragment_done = 0;
			return tot+i;
			}

		tot += i;
		n-=i;
		}

	return tot;
	}


	/* this only happens when a client hello is received and a handshake 
	 * is started. */
static int
have_handshake_fragment(SSL *s, int type, unsigned char *buf, 
	int len, int peek)
	{
	
	if ((type == SSL3_RT_HANDSHAKE) && (s->d1->handshake_fragment_len > 0))
		/* (partially) satisfy request from storage */
		{
		unsigned char *src = s->d1->handshake_fragment;
		unsigned char *dst = buf;
		unsigned int k,n;
		
		/* peek == 0 */
		n = 0;
		while ((len > 0) && (s->d1->handshake_fragment_len > 0))
			{
			*dst++ = *src++;
			len--; s->d1->handshake_fragment_len--;
			n++;
			}
		/* move any remaining fragment bytes: */
		for (k = 0; k < s->d1->handshake_fragment_len; k++)
			s->d1->handshake_fragment[k] = *src++;
		return n;
		}
	
	return 0;
	}




/* Call this to write data in records of type 'type'
 * It will return <= 0 if not all data has been sent or non-blocking IO.
 */
int dtls1_write_bytes(SSL *s, int type, const void *buf_, int len)
	{
	const unsigned char *buf=buf_;
	unsigned int tot,n,nw;
	int i;
	unsigned int mtu;

	s->rwstate=SSL_NOTHING;
	tot=s->s3->wnum;

	n=(len-tot);

	/* handshake layer figures out MTU for itself, but data records
	 * are also sent through this interface, so need to figure out MTU */
#if 0
	mtu = BIO_ctrl(SSL_get_wbio(s), BIO_CTRL_DGRAM_GET_MTU, 0, NULL);
	mtu += DTLS1_HM_HEADER_LENGTH;  /* HM already inserted */
#endif
	mtu = s->d1->mtu;

	if (mtu > SSL3_RT_MAX_PLAIN_LENGTH)
		mtu = SSL3_RT_MAX_PLAIN_LENGTH;

	if (n > mtu)
		nw=mtu;
	else
		nw=n;
	
	i=do_dtls1_write(s, type, &(buf[tot]), nw, 0);
	if (i <= 0)
		{
		s->s3->wnum=tot;
		return i;
		}

	if ( (int)s->s3->wnum + i == len)
		s->s3->wnum = 0;
	else 
		s->s3->wnum += i;

	return tot + i;
	}

int do_dtls1_write(SSL *s, int type, const unsigned char *buf, unsigned int len, int create_empty_fragment)
	{
	unsigned char *p,*pseq;
	int i,mac_size,clear=0;
	int prefix_len = 0;
	SSL3_RECORD *wr;
	SSL3_BUFFER *wb;
	SSL_SESSION *sess;
	int bs;

	/* first check if there is a SSL3_BUFFER still being written
	 * out.  This will happen with non blocking IO */
	if (s->s3->wbuf.left != 0)
		{
		OPENSSL_assert(0); /* XDTLS:  want to see if we ever get here */
		return(ssl3_write_pending(s,type,buf,len));
		}

	/* If we have an alert to send, lets send it */
	if (s->s3->alert_dispatch)
		{
		i=s->method->ssl_dispatch_alert(s);
		if (i <= 0)
			return(i);
		/* if it went, fall through and send more stuff */
		}

	if (len == 0 && !create_empty_fragment)
		return 0;

	wr= &(s->s3->wrec);
	wb= &(s->s3->wbuf);
	sess=s->session;

	if (	(sess == NULL) ||
		(s->enc_write_ctx == NULL) ||
		(s->write_hash == NULL))
		clear=1;

	if (clear)
		mac_size=0;
	else
		mac_size=EVP_MD_size(s->write_hash);

	/* DTLS implements explicit IV, so no need for empty fragments */
#if 0
	/* 'create_empty_fragment' is true only when this function calls itself */
	if (!clear && !create_empty_fragment && !s->s3->empty_fragment_done
		&& SSL_version(s) != DTLS1_VERSION)
		{
		/* countermeasure against known-IV weakness in CBC ciphersuites
		 * (see http://www.openssl.org/~bodo/tls-cbc.txt) 
		 */

		if (s->s3->need_empty_fragments && type == SSL3_RT_APPLICATION_DATA)
			{
			/* recursive function call with 'create_empty_fragment' set;
			 * this prepares and buffers the data for an empty fragment
			 * (these 'prefix_len' bytes are sent out later
			 * together with the actual payload) */
			prefix_len = s->method->do_ssl_write(s, type, buf, 0, 1);
			if (prefix_len <= 0)
				goto err;

			if (s->s3->wbuf.len < (size_t)prefix_len + SSL3_RT_MAX_PACKET_SIZE)
				{
				/* insufficient space */
				SSLerr(SSL_F_DO_DTLS1_WRITE, ERR_R_INTERNAL_ERROR);
				goto err;
				}
			}
		
		s->s3->empty_fragment_done = 1;
		}
#endif

	p = wb->buf + prefix_len;

	/* write the header */

	*(p++)=type&0xff;
	wr->type=type;

	*(p++)=(s->version>>8);
	*(p++)=s->version&0xff;

	/* field where we are to write out packet epoch, seq num and len */
	pseq=p; 
	p+=10;

	/* lets setup the record stuff. */

	/* Make space for the explicit IV in case of CBC.
	 * (this is a bit of a boundary violation, but what the heck).
	 */
	if ( s->enc_write_ctx && 
		(EVP_CIPHER_mode( s->enc_write_ctx->cipher ) & EVP_CIPH_CBC_MODE))
		bs = EVP_CIPHER_block_size(s->enc_write_ctx->cipher);
	else
		bs = 0;

	wr->data=p + bs;  /* make room for IV in case of CBC */
	wr->length=(int)len;
	wr->input=(unsigned char *)buf;

	/* we now 'read' from wr->input, wr->length bytes into
	 * wr->data */

	/* first we compress */
	if (s->compress != NULL)
		{
		if (!ssl3_do_compress(s))
			{
			SSLerr(SSL_F_DO_DTLS1_WRITE,SSL_R_COMPRESSION_FAILURE);
			goto err;
			}
		}
	else
		{
		memcpy(wr->data,wr->input,wr->length);
		wr->input=wr->data;
		}

	/* we should still have the output to wr->data and the input
	 * from wr->input.  Length should be wr->length.
	 * wr->data still points in the wb->buf */

	if (mac_size != 0)
		{
		s->method->ssl3_enc->mac(s,&(p[wr->length + bs]),1);
		wr->length+=mac_size;
		}

	/* this is true regardless of mac size */
	wr->input=p;
	wr->data=p;


	/* ssl3_enc can only have an error on read */
	wr->length += bs;  /* bs != 0 in case of CBC.  The enc fn provides
						* the randomness */ 
	s->method->ssl3_enc->enc(s,1);

	/* record length after mac and block padding */
/*	if (type == SSL3_RT_APPLICATION_DATA ||
	(type == SSL3_RT_ALERT && ! SSL_in_init(s))) */
	
	/* there's only one epoch between handshake and app data */
	
	s2n(s->d1->w_epoch, pseq);

	/* XDTLS: ?? */
/*	else
	s2n(s->d1->handshake_epoch, pseq); */

	memcpy(pseq, &(s->s3->write_sequence[2]), 6);
	pseq+=6;
	s2n(wr->length,pseq);

	/* we should now have
	 * wr->data pointing to the encrypted data, which is
	 * wr->length long */
	wr->type=type; /* not needed but helps for debugging */
	wr->length+=DTLS1_RT_HEADER_LENGTH;

#if 0  /* this is now done at the message layer */
	/* buffer the record, making it easy to handle retransmits */
	if ( type == SSL3_RT_HANDSHAKE || type == SSL3_RT_CHANGE_CIPHER_SPEC)
		dtls1_buffer_record(s, wr->data, wr->length, 
			*((PQ_64BIT *)&(s->s3->write_sequence[0])));
#endif

	ssl3_record_sequence_update(&(s->s3->write_sequence[0]));

	if (create_empty_fragment)
		{
		/* we are in a recursive call;
		 * just return the length, don't write out anything here
		 */
		return wr->length;
		}

	/* now let's set up wb */
	wb->left = prefix_len + wr->length;
	wb->offset = 0;

	/* memorize arguments so that ssl3_write_pending can detect bad write retries later */
	s->s3->wpend_tot=len;
	s->s3->wpend_buf=buf;
	s->s3->wpend_type=type;
	s->s3->wpend_ret=len;

	/* we now just need to write the buffer */
	return ssl3_write_pending(s,type,buf,len);
err:
	return -1;
	}



static int dtls1_record_replay_check(SSL *s, DTLS1_BITMAP *bitmap,
	PQ_64BIT *seq_num)
	{
#if PQ_64BIT_IS_INTEGER
	PQ_64BIT mask = 0x0000000000000001L;
#endif
	PQ_64BIT rcd_num, tmp;

	pq_64bit_init(&rcd_num);
	pq_64bit_init(&tmp);

	/* this is the sequence number for the record just read */
	pq_64bit_bin2num(&rcd_num, s->s3->read_sequence, 8);

	
	if (pq_64bit_gt(&rcd_num, &(bitmap->max_seq_num)) ||
		pq_64bit_eq(&rcd_num, &(bitmap->max_seq_num)))
		{
		pq_64bit_assign(seq_num, &rcd_num);
		pq_64bit_free(&rcd_num);
		pq_64bit_free(&tmp);
		return 1;  /* this record is new */
		}

	pq_64bit_sub(&tmp, &(bitmap->max_seq_num), &rcd_num);

	if ( pq_64bit_get_word(&tmp) > bitmap->length)
		{
		pq_64bit_free(&rcd_num);
		pq_64bit_free(&tmp);
		return 0;  /* stale, outside the window */
		}

#if PQ_64BIT_IS_BIGNUM
	{
	int offset;
	pq_64bit_sub(&tmp, &(bitmap->max_seq_num), &rcd_num);
	pq_64bit_sub_word(&tmp, 1);
	offset = pq_64bit_get_word(&tmp);
	if ( pq_64bit_is_bit_set(&(bitmap->map), offset))
		{
		pq_64bit_free(&rcd_num);
		pq_64bit_free(&tmp);
		return 0;
		}
	}
#else
	mask <<= (bitmap->max_seq_num - rcd_num - 1);
	if (bitmap->map & mask)
		return 0; /* record previously received */
#endif
	
	pq_64bit_assign(seq_num, &rcd_num);
	pq_64bit_free(&rcd_num);
	pq_64bit_free(&tmp);
	return 1;
	}


static void dtls1_record_bitmap_update(SSL *s, DTLS1_BITMAP *bitmap)
	{
	unsigned int shift;
	PQ_64BIT rcd_num;
	PQ_64BIT tmp;
	PQ_64BIT_CTX *ctx;

	pq_64bit_init(&rcd_num);
	pq_64bit_init(&tmp);

	pq_64bit_bin2num(&rcd_num, s->s3->read_sequence, 8);

	/* unfortunate code complexity due to 64-bit manipulation support
	 * on 32-bit machines */
	if ( pq_64bit_gt(&rcd_num, &(bitmap->max_seq_num)) ||
		pq_64bit_eq(&rcd_num, &(bitmap->max_seq_num)))
		{
		pq_64bit_sub(&tmp, &rcd_num, &(bitmap->max_seq_num));
		pq_64bit_add_word(&tmp, 1);

		shift = (unsigned int)pq_64bit_get_word(&tmp);

		pq_64bit_lshift(&(tmp), &(bitmap->map), shift);
		pq_64bit_assign(&(bitmap->map), &tmp);

		pq_64bit_set_bit(&(bitmap->map), 0);
		pq_64bit_add_word(&rcd_num, 1);
		pq_64bit_assign(&(bitmap->max_seq_num), &rcd_num);

		pq_64bit_assign_word(&tmp, 1);
		pq_64bit_lshift(&tmp, &tmp, bitmap->length);
		ctx = pq_64bit_ctx_new(&ctx);
		pq_64bit_mod(&(bitmap->map), &(bitmap->map), &tmp, ctx);
		pq_64bit_ctx_free(ctx);
		}
	else
		{
		pq_64bit_sub(&tmp, &(bitmap->max_seq_num), &rcd_num);
		pq_64bit_sub_word(&tmp, 1);
		shift = (unsigned int)pq_64bit_get_word(&tmp);

		pq_64bit_set_bit(&(bitmap->map), shift);
		}

	pq_64bit_free(&rcd_num);
	pq_64bit_free(&tmp);
	}


int dtls1_dispatch_alert(SSL *s)
	{
	int i,j;
	void (*cb)(const SSL *ssl,int type,int val)=NULL;
	unsigned char buf[2 + 2 + 3]; /* alert level + alert desc + message seq +frag_off */
	unsigned char *ptr = &buf[0];

	s->s3->alert_dispatch=0;

	memset(buf, 0x00, sizeof(buf));
	*ptr++ = s->s3->send_alert[0];
	*ptr++ = s->s3->send_alert[1];

	if (s->s3->send_alert[1] == DTLS1_AD_MISSING_HANDSHAKE_MESSAGE)
		{	
		s2n(s->d1->handshake_read_seq, ptr);
#if 0
		if ( s->d1->r_msg_hdr.frag_off == 0)  /* waiting for a new msg */

		else
			s2n(s->d1->r_msg_hdr.seq, ptr); /* partial msg read */
#endif

#if 0
		fprintf(stderr, "s->d1->handshake_read_seq = %d, s->d1->r_msg_hdr.seq = %d\n",s->d1->handshake_read_seq,s->d1->r_msg_hdr.seq);
#endif
		l2n3(s->d1->r_msg_hdr.frag_off, ptr);
		}

	i = do_dtls1_write(s, SSL3_RT_ALERT, &buf[0], sizeof(buf), 0);
	if (i <= 0)
		{
		s->s3->alert_dispatch=1;
		/* fprintf( stderr, "not done with alert\n" ); */
		}
	else
		{
		if ( s->s3->send_alert[0] == SSL3_AL_FATAL ||
			s->s3->send_alert[1] == DTLS1_AD_MISSING_HANDSHAKE_MESSAGE)
			(void)BIO_flush(s->wbio);

		if (s->msg_callback)
			s->msg_callback(1, s->version, SSL3_RT_ALERT, s->s3->send_alert, 
				2, s, s->msg_callback_arg);

		if (s->info_callback != NULL)
			cb=s->info_callback;
		else if (s->ctx->info_callback != NULL)
			cb=s->ctx->info_callback;

		if (cb != NULL)
			{
			j=(s->s3->send_alert[0]<<8)|s->s3->send_alert[1];
			cb(s,SSL_CB_WRITE_ALERT,j);
			}
		}
	return(i);
	}


static DTLS1_BITMAP *
dtls1_get_bitmap(SSL *s, SSL3_RECORD *rr, unsigned int *is_next_epoch)
    {
    
    *is_next_epoch = 0;

    /* In current epoch, accept HM, CCS, DATA, & ALERT */
    if (rr->epoch == s->d1->r_epoch)
        return &s->d1->bitmap;

    /* Only HM and ALERT messages can be from the next epoch */
    else if (rr->epoch == (unsigned long)(s->d1->r_epoch + 1) &&
        (rr->type == SSL3_RT_HANDSHAKE ||
            rr->type == SSL3_RT_ALERT))
        {
        *is_next_epoch = 1;
        return &s->d1->next_bitmap;
        }

    return NULL;
    }

#if 0
static int
dtls1_record_needs_buffering(SSL *s, SSL3_RECORD *rr, unsigned short *priority,
	unsigned long *offset)
	{

	/* alerts are passed up immediately */
	if ( rr->type == SSL3_RT_APPLICATION_DATA ||
		rr->type == SSL3_RT_ALERT)
		return 0;

	/* Only need to buffer if a handshake is underway.
	 * (this implies that Hello Request and Client Hello are passed up
	 * immediately) */
	if ( SSL_in_init(s))
		{
		unsigned char *data = rr->data;
		/* need to extract the HM/CCS sequence number here */
		if ( rr->type == SSL3_RT_HANDSHAKE ||
			rr->type == SSL3_RT_CHANGE_CIPHER_SPEC)
			{
			unsigned short seq_num;
			struct hm_header_st msg_hdr;
			struct ccs_header_st ccs_hdr;

			if ( rr->type == SSL3_RT_HANDSHAKE)
				{
				dtls1_get_message_header(data, &msg_hdr);
				seq_num = msg_hdr.seq;
				*offset = msg_hdr.frag_off;
				}
			else
				{
				dtls1_get_ccs_header(data, &ccs_hdr);
				seq_num = ccs_hdr.seq;
				*offset = 0;
				}
				
			/* this is either a record we're waiting for, or a
			 * retransmit of something we happened to previously 
			 * receive (higher layers will drop the repeat silently */
			if ( seq_num < s->d1->handshake_read_seq)
				return 0;
			if (rr->type == SSL3_RT_HANDSHAKE && 
				seq_num == s->d1->handshake_read_seq &&
				msg_hdr.frag_off < s->d1->r_msg_hdr.frag_off)
				return 0;
			else if ( seq_num == s->d1->handshake_read_seq &&
				(rr->type == SSL3_RT_CHANGE_CIPHER_SPEC ||
					msg_hdr.frag_off == s->d1->r_msg_hdr.frag_off))
				return 0;
			else
				{
				*priority = seq_num;
				return 1;
				}
			}
		else /* unknown record type */
			return 0;
		}

	return 0;
	}
#endif

void
dtls1_reset_seq_numbers(SSL *s, int rw)
	{
	unsigned char *seq;
	unsigned int seq_bytes = sizeof(s->s3->read_sequence);

	if ( rw & SSL3_CC_READ)
		{
		seq = s->s3->read_sequence;
		s->d1->r_epoch++;

		pq_64bit_assign(&(s->d1->bitmap.map), &(s->d1->next_bitmap.map));
		s->d1->bitmap.length = s->d1->next_bitmap.length;
		pq_64bit_assign(&(s->d1->bitmap.max_seq_num), 
			&(s->d1->next_bitmap.max_seq_num));

		pq_64bit_free(&(s->d1->next_bitmap.map));
		pq_64bit_free(&(s->d1->next_bitmap.max_seq_num));
		memset(&(s->d1->next_bitmap), 0x00, sizeof(DTLS1_BITMAP));
		pq_64bit_init(&(s->d1->next_bitmap.map));
		pq_64bit_init(&(s->d1->next_bitmap.max_seq_num));
		}
	else
		{
		seq = s->s3->write_sequence;
		s->d1->w_epoch++;
		}

	memset(seq, 0x00, seq_bytes);
	}

#if PQ_64BIT_IS_INTEGER
static PQ_64BIT
bytes_to_long_long(unsigned char *bytes, PQ_64BIT *num)
       {
       PQ_64BIT _num;

       _num = (((PQ_64BIT)bytes[0]) << 56) |
               (((PQ_64BIT)bytes[1]) << 48) |
               (((PQ_64BIT)bytes[2]) << 40) |
               (((PQ_64BIT)bytes[3]) << 32) |
               (((PQ_64BIT)bytes[4]) << 24) |
               (((PQ_64BIT)bytes[5]) << 16) |
               (((PQ_64BIT)bytes[6]) <<  8) |
               (((PQ_64BIT)bytes[7])      );

	   *num = _num ;
       return _num;
       }
#endif


static void
dtls1_clear_timeouts(SSL *s)
	{
	memset(&(s->d1->timeout), 0x00, sizeof(struct dtls1_timeout_st));
	}
