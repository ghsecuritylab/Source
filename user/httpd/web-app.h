#ifndef _web_app_h_
#define _web_app_h_

#ifdef RUNTIME_CHECK_PAP_PASSWORD
#define CHECK_PAP_PASSWORD_RESULT "/tmp/check_pap_pw_result"
#endif

void do_app_none_cgi(char *url, FILE *stream);
void do_app_auth_cgi(char *url, FILE *stream);
#endif /* _web_app_h_ */
