#ifndef _web_ralink_h_
#define _web_ralink_h_

int ej_pap_status(int eid, webs_t wp, int argc, char_t **argv);
int ej_wl_auth_list(int eid, webs_t wp, int argc, char_t **argv);
#ifdef RUNTIME_CHECK_PAP_PASSWORD
int ej_check_pap_password(int eid, webs_t wp, int argc, char_t **argv);
#endif
#endif /* _web_ralink_h_ */
