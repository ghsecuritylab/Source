#ifndef _web_iradio_h_
#define _web_iradio_h_

#ifdef USB_TO_CM6510
#define CM6510_FW_UPLOAD_PATH	"/tmp/cm6510.bin"
#endif
#define INI_UPLOAD_PATH		"/tmp/radio_tb.prf"

void do_download_ini_cgi(char *url, FILE *stream);
#ifdef USB_TO_CM6510
void do_upgrade_cm6510_cgi(char *url, FILE *stream);
#endif
void do_upload_ini_cgi(char *url, FILE *stream);
#ifdef URL_PLAYER
void do_url_cgi(char *url, FILE *stream);
#endif

//audio control
int ej_get_vol(int eid, webs_t wp, int argc, char_t **argv);
int ej_set_vol(int eid, webs_t wp, int argc, char_t **argv);
int ej_audio_mute(int eid, webs_t wp, int argc, char_t **argv);
int ej_set_audio_eq(int eid, webs_t wp, int argc, char_t **argv);
//add/del/play files
int ej_get_url_table(int eid, webs_t wp, int argc, char_t **argv);
int ej_get_play_num(int eid, webs_t wp, int argc, char_t **argv);
int ej_get_builtin_list(int eid, webs_t wp, int argc, char_t **argv);
int ej_get_user_list(int eid, webs_t wp, int argc, char_t **argv);
int ej_check_iradio(int eid, webs_t wp, int argc, char_t **argv);
int ej_detect_audiojack(int eid, webs_t wp, int argc, char_t **argv);
int ej_play_rs(int eid, webs_t wp, int argc, char_t **argv);
int ej_update_ralist(int eid, webs_t wp, int argc, char_t **argv);
int ej_del_ralist(int eid, webs_t wp, int argc, char_t **argv);
int ej_get_status(int eid, webs_t wp, int argc, char_t **argv);
int ej_test_audio(int eid, webs_t wp, int argc, char_t **argv);
int ej_upgrade_radio_list(int eid, webs_t wp, int argc, char_t **argv);
int ej_get_upload_table_status(int eid, webs_t wp, int argc, char_t **argv);
#endif /* _web_iradio_h_ */
