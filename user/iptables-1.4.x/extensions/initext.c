
extern void libxt_standard_init(void);
extern void libxt_state_init(void);
extern void libxt_tcp_init(void);
extern void libxt_udp_init(void);
extern void libxt_TCPMSS_init(void);
extern void libxt_time_init(void);
extern void libxt_multiport_init(void);
extern void libxt_string_init(void);
extern void libxt_limit_init(void);
void init_extensions(void);
void init_extensions(void)
{
 libxt_standard_init();
 libxt_state_init();
 libxt_tcp_init();
 libxt_udp_init();
 libxt_TCPMSS_init();
 libxt_time_init();
 libxt_multiport_init();
 libxt_string_init();
 libxt_limit_init();
}
