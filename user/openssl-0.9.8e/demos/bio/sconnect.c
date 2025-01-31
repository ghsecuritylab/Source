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
/* NOCW */
/* demos/bio/sconnect.c */

/* A minimal program to do SSL to a passed host and port.
 * It is actually using non-blocking IO but in a very simple manner
 * sconnect host:port - it does a 'GET / HTTP/1.0'
 *
 * cc -I../../include sconnect.c -L../.. -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

extern int errno;

int main(argc,argv)
int argc;
char *argv[];
	{
	char *host;
	BIO *out;
	char buf[1024*10],*p;
	SSL_CTX *ssl_ctx=NULL;
	SSL *ssl;
	BIO *ssl_bio;
	int i,len,off,ret=1;

	if (argc <= 1)
		host="localhost:4433";
	else
		host=argv[1];

#ifdef WATT32
	dbug_init();
	sock_init();
#endif

	/* Lets get nice error messages */
	SSL_load_error_strings();

	/* Setup all the global SSL stuff */
	OpenSSL_add_ssl_algorithms();
	ssl_ctx=SSL_CTX_new(SSLv23_client_method());

	/* Lets make a SSL structure */
	ssl=SSL_new(ssl_ctx);
	SSL_set_connect_state(ssl);

	/* Use it inside an SSL BIO */
	ssl_bio=BIO_new(BIO_f_ssl());
	BIO_set_ssl(ssl_bio,ssl,BIO_CLOSE);

	/* Lets use a connect BIO under the SSL BIO */
	out=BIO_new(BIO_s_connect());
	BIO_set_conn_hostname(out,host);
	BIO_set_nbio(out,1);
	out=BIO_push(ssl_bio,out);

	p="GET / HTTP/1.0\r\n\r\n";
	len=strlen(p);

	off=0;
	for (;;)
		{
		i=BIO_write(out,&(p[off]),len);
		if (i <= 0)
			{
			if (BIO_should_retry(out))
				{
				fprintf(stderr,"write DELAY\n");
				sleep(1);
				continue;
				}
			else
				{
				goto err;
				}
			}
		off+=i;
		len-=i;
		if (len <= 0) break;
		}

	for (;;)
		{
		i=BIO_read(out,buf,sizeof(buf));
		if (i == 0) break;
		if (i < 0)
			{
			if (BIO_should_retry(out))
				{
				fprintf(stderr,"read DELAY\n");
				sleep(1);
				continue;
				}
			goto err;
			}
		fwrite(buf,1,i,stdout);
		}

	ret=1;

	if (0)
		{
err:
		if (ERR_peek_error() == 0) /* system call error */
			{
			fprintf(stderr,"errno=%d ",errno);
			perror("error");
			}
		else
			ERR_print_errors_fp(stderr);
		}
	BIO_free_all(out);
	if (ssl_ctx != NULL) SSL_CTX_free(ssl_ctx);
	exit(!ret);
	return(ret);
	}

