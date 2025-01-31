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
typedef int cl_engine_init(void);
typedef int cl_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
			const BIGNUM *m, BN_CTX *cgx);
typedef int cl_mod_exp_crt(BIGNUM *r, BIGNUM *a, const BIGNUM *p,
		const BIGNUM *q, const BIGNUM *dmp1, const BIGNUM *dmq1,
		const BIGNUM *iqmp, BN_CTX *ctx);
typedef int cl_rsa_mod_exp(BIGNUM *r0, const BIGNUM *I, RSA *rsa);
typedef int cl_rsa_pub_enc(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding);
typedef int cl_rsa_pub_dec(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding);
typedef int cl_rsa_priv_enc(int flen, const unsigned char *from, 
		unsigned char *to, RSA *rsa, int padding);
typedef int cl_rsa_priv_dec(int flen, const unsigned char *from, 
		unsigned char *to, RSA *rsa, int padding);		
typedef int cl_rand_bytes(unsigned char *buf, int num);
typedef DSA_SIG *cl_dsa_sign(const unsigned char *dgst, int dlen, DSA *dsa);
typedef int cl_dsa_verify(const unsigned char *dgst, int dgst_len,
				DSA_SIG *sig, DSA *dsa);


static const char *CLUSTER_LABS_LIB_NAME = "cluster_labs";
static const char *CLUSTER_LABS_F1	 = "hw_engine_init";
static const char *CLUSTER_LABS_F2	 = "hw_mod_exp";
static const char *CLUSTER_LABS_F3	 = "hw_mod_exp_crt";
static const char *CLUSTER_LABS_F4	 = "hw_rsa_mod_exp";
static const char *CLUSTER_LABS_F5	 = "hw_rsa_priv_enc";
static const char *CLUSTER_LABS_F6	 = "hw_rsa_priv_dec";
static const char *CLUSTER_LABS_F7	 = "hw_rsa_pub_enc";
static const char *CLUSTER_LABS_F8	 = "hw_rsa_pub_dec";
static const char *CLUSTER_LABS_F20	 = "hw_rand_bytes";
static const char *CLUSTER_LABS_F30	 = "hw_dsa_sign";
static const char *CLUSTER_LABS_F31	 = "hw_dsa_verify";


