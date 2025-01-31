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
/* pptp.c ... client shell to launch call managers, data handlers, and
 *            the pppd from the command line.
 *            C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pptp.c,v 1.1.1.1 2008/07/21 09:19:28 james26_jang Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#if defined(__FreeBSD__)
#include <libutil.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#include <util.h>
#elif defined(__APPLE__)
#include <util.h>
#else
#include <pty.h>
#endif
#ifdef USER_PPP
#include <fcntl.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/param.h>
#if defined(__APPLE__)
#include "getopt.h"
#else
#include <getopt.h>
#endif
#include <limits.h>
#ifndef N_HDLC
#include <linux/termios.h>
#endif
#include "config.h"
#include "pptp_callmgr.h"
#include "pptp_gre.h"
#include "version.h"
#include "inststr.h"
#include "util.h"
#include "pptp_quirks.h"
#include "pqueue.h"
#include "pptp_options.h"

#ifndef PPPD_BINARY
#define PPPD_BINARY "pppd"
#endif

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

int syncppp = 0;
int log_level = 0;
int disable_buffer = 0;

struct in_addr get_ip_address(char *name);
int open_callmgr(struct in_addr inetaddr, char *phonenr, int argc,char **argv,char **envp, int pty_fd, int gre_fd);
void launch_callmgr(struct in_addr inetaddr, char *phonenr, int argc,char **argv,char **envp);
int get_call_id(int sock, pid_t gre, pid_t pppd, 
		 u_int16_t *call_id, u_int16_t *peer_call_id);
void launch_pppd(char *ttydev, int argc, char **argv);

static int route_add(const struct in_addr inetaddr, struct rtentry *rt);
static int route_del(struct rtentry *rt);

/*** print usage and exit *****************************************************/
void usage(char *progname)
{
    fprintf(stderr,
            "%s\n"
            "Usage:\n"
            "  %s <hostname> [<pptp options>] [[--] <pppd options>]\n"
            "\n"
            "Or using pppd's pty option: \n"
            "  pppd pty \"%s <hostname> --nolaunchpppd <pptp options>\"\n"
            "\n"
            "Available pptp options:\n"
            "  --phone <number>	Pass <number> to remote host as phone number\n"
            "  --nolaunchpppd	Do not launch pppd, for use as a pppd pty\n"
            "  --quirks <quirk>	Work around a buggy PPTP implementation\n"
            "			Currently recognised values are BEZEQ_ISRAEL only\n"
            "  --debug		Run in foreground (for debugging with gdb)\n"
            "  --sync		Enable Synchronous HDLC (pppd must use it too)\n"
            "  --timeout <secs>	Time to wait for reordered packets (0.01 to 10 secs)\n"
	    "  --nobuffer		Disable packet buffering and reordering completely\n"
	    "  --idle-wait		Time to wait before sending echo request\n"
            "  --max-echo-wait		Time to wait before giving up on lack of reply\n"
            "  --logstring <name>	Use <name> instead of 'anon' in syslog messages\n"
            "  --localbind <addr>	Bind to specified IP address instead of wildcard\n"
            "  --loglevel <level>	Sets the debugging level (0=low, 1=default, 2=high)\n",

            version, progname, progname);
    log("%s called with wrong arguments, program not started.", progname);
    exit(1);
}

struct in_addr localbind = { INADDR_NONE };
static int signaled = 0;

/*** do nothing signal handler ************************************************/
void do_nothing(int sig)
{ 
    /* do nothing signal handler. Better than SIG_IGN. */
    signaled = sig;
}

sigjmp_buf env;

/*** signal handler ***********************************************************/
void sighandler(int sig)
{
    siglongjmp(env, 1);
}

/*** report statistics signal handler (SIGUSR1) *******************************/
void sigstats(int sig)
{
    syslog(LOG_NOTICE, "GRE statistics:\n");
#define LOG(name,value) syslog(LOG_NOTICE, name "\n", stats .value)
    LOG("rx accepted  = %d", rx_accepted);
    LOG("rx lost      = %d", rx_lost);
    LOG("rx under win = %d", rx_underwin);
    LOG("rx over  win = %d", rx_overwin);
    LOG("rx buffered  = %d", rx_buffered);
    LOG("rx OS errors = %d", rx_errors);
    LOG("rx truncated = %d", rx_truncated);
    LOG("rx invalid   = %d", rx_invalid);
    LOG("rx acks      = %d", rx_acks);
    LOG("tx sent      = %d", tx_sent);
    LOG("tx failed    = %d", tx_failed);
    LOG("tx short     = %d", tx_short);
    LOG("tx acks      = %d", tx_acks);
    LOG("tx oversize  = %d", tx_oversize);
    LOG("round trip   = %d usecs", rtt);
#undef LOG
}

/*** main *********************************************************************/
/* TODO: redesign to avoid longjmp/setjmp.  Several variables here
   have a volatile qualifier to silence warnings from gcc < 3.0.
   Remove the volatile qualifiers if longjmp/setjmp are removed.
   */
int main(int argc, char **argv, char **envp)
{
    struct in_addr inetaddr;
    struct rtentry rt;
    volatile int callmgr_sock = -1;
    char ttydev[PATH_MAX];
    int pty_fd, tty_fd, gre_fd, rc;
    volatile pid_t parent_pid, child_pid;
    u_int16_t call_id, peer_call_id;
    char buf[128];
    int pppdargc;
    char **pppdargv;
    char phonenrbuf[65]; /* maximum length of field plus one for the trailing
                          * '\0' */
    char * volatile phonenr = NULL;
    volatile int launchpppd = 1, debug = 0;

    int disc = N_HDLC;

	//fprintf(stderr, "[pptp-client] pptp client start\n");	// tmp test
    while(1){ 
        /* structure with all recognised options for pptp */
        static struct option long_options[] = {
            {"phone", 1, 0, 0},  
            {"nolaunchpppd", 0, 0, 0},  
            {"quirks", 1, 0, 0},
            {"debug", 0, 0, 0},
            {"sync", 0, 0, 0},
            {"timeout", 1, 0, 0},
            {"logstring", 1, 0, 0},
            {"localbind", 1, 0, 0},
            {"loglevel", 1, 0, 0},
	    {"nobuffer", 0, 0, 0},
	    {"idle-wait", 1, 0, 0},
	    {"max-echo-wait", 1, 0, 0},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        int c;
        c = getopt_long (argc, argv, "", long_options, &option_index);
        if (c == -1) break;  /* no more options */
        switch (c) {
            case 0: 
                if (option_index == 0) { /* --phone specified */
                    /* copy it to a buffer, as the argv's will be overwritten
                     * by inststr() */
                    strncpy(phonenrbuf,optarg,sizeof(phonenrbuf));
                    phonenrbuf[sizeof(phonenrbuf) - 1] = '\0';
                    phonenr = phonenrbuf;
                } else if (option_index == 1) {/* --nolaunchpppd specified */
                    launchpppd = 0;
                } else if (option_index == 2) {/* --quirks specified */
                    if (set_quirk_index(find_quirk(optarg)))
                        usage(argv[0]);
                } else if (option_index == 3) {/* --debug */
                    debug = 1;
                } else if (option_index == 4) {/* --sync specified */
                    syncppp = 1;
                } else if (option_index == 5) {/* --timeout */
                    float new_packet_timeout = atof(optarg);
                    if (new_packet_timeout < 0.0099 ||
                            new_packet_timeout > 10) {
                        fprintf(stderr, "Packet timeout %s (%f) out of range: "
                                "should be between 0.01 and 10 seconds\n",
                                optarg, new_packet_timeout);
                        log("Packet timeout %s (%f) out of range: should be"
                                "between 0.01 and 10 seconds", optarg,
                                new_packet_timeout);
                        exit(2);
                    } else {
                        packet_timeout_usecs = new_packet_timeout * 1000000;
                    }
                } else if (option_index == 6) {/* --logstring */
                    log_string = strdup(optarg);
                } else if (option_index == 7) {/* --localbind */ 
                    if (inet_pton(AF_INET, optarg, (void *) &localbind) < 1) {
                        fprintf(stderr, "Local bind address %s invalid\n", 
				optarg);
                        log("Local bind address %s invalid\n", optarg);
                        exit(2);
                    }
                } else if (option_index == 8) { /* --loglevel */
                    log_level = atoi(optarg);
                    if (log_level < 0 || log_level > 2)
                        usage(argv[0]);
                } else if (option_index == 9) { /* --nobuffer */
		    disable_buffer = 1;
                } else if (option_index == 10) { /* --idle-wait */
                    int x = atoi(optarg);
                    if (x < 0) {
                        fprintf(stderr, "--idle-wait must not be negative\n");
                        log("--idle-wait must not be negative\n");
                        exit(2);
                    } else {
                        idle_wait = x;
                    }
                } else if (option_index == 11) { /* --max-echo-wait */
                    int x = atoi(optarg);
                    if (x < 0) {
                        fprintf(stderr, "--max-echo-wait must not be negative\n");
                        log("--max-echo-wait must not be negative\n");
                        exit(2);
                    } else {
                        max_echo_wait = x;
                    }
		    fprintf(stderr, "--max-echo-wait ignored, not yet implemented\n");
                }
                break;
            case '?': /* unrecognised option */
                /* fall through */
            default:
		usage(argv[0]);
        }
        if (c == -1) break;  /* no more options for pptp */
    }

	//fprintf(stderr, "[pptp-c] pptp chk 1\n");	// tmp test
    /* at least one argument is required */
    if (argc <= optind)
        usage(argv[0]);

	//fprintf(stderr, "[pptp-c] pptp chk 2\n");	// tmp test
    /* Get IP address for the hostname in argv[1] */
    inetaddr = get_ip_address(argv[optind]);
    optind++;

    /* Find the ppp options, extract phone number */
    pppdargc = argc - optind;
    pppdargv = argv + optind;
    log("The synchronous pptp option is %sactivated\n", syncppp ? "" : "NOT ");

	//fprintf(stderr, "[pptp-c] pptp chk 3\n");	// tmp test
    /* Add a route to inetaddr */
    memset(&rt, 0, sizeof(rt));
    route_add(inetaddr, &rt);

	//fprintf(stderr, "[pptp-c] pptp chk 4\n");	// tmp test
    /* Now we have the peer address, bind the GRE socket early,
       before starting pppd. This prevents the ICMP Unreachable bug
       documented in <1026868263.2855.67.camel@jander> */
    gre_fd = pptp_gre_bind(inetaddr);
    if (gre_fd < 0) {
        close(callmgr_sock);
        fatal("Cannot bind GRE socket, aborting.");
    }

	//fprintf(stderr, "[pptp-c] pptp chk 5 (launchpppd = %d)\n", launchpppd);	// tmp test
    /* Find an open pty/tty pair. */
    if(launchpppd){
        rc =  -1 /*openpty (&pty_fd, &tty_fd, ttydev, NULL, NULL)*/;
        if (rc < 0) { 
            close(callmgr_sock); 
            fatal("Could not find free pty.");
        }

        /* fork and wait. */
        signal(SIGUSR1, do_nothing); /* don't die */
        signal(SIGCHLD, do_nothing); /* don't ignore SIGCHLD */
        parent_pid = getpid();
        switch (child_pid = fork()) {
            case -1:
                fatal("Could not fork pppd process");
            case 0: /* I'm the child! */
                close (tty_fd);
                signal(SIGUSR1, SIG_DFL);
                child_pid = getpid();
                break;
            default: /* parent */
                close (pty_fd);
                /*
                 * There is still a very small race condition here.  If a signal
                 * occurs after signaled is checked but before pause is called,
                 * things will hang.
                 */
                if (!signaled) {
                    pause(); /* wait for the signal */
                }
 
                if (signaled == SIGCHLD)
                    fatal("Child process died");
 
                launch_pppd(ttydev, pppdargc, pppdargv); /* launch pppd */
                perror("Error");
                fatal("Could not launch pppd");
        }
    } else { /* ! launchpppd */
        pty_fd = tty_fd = STDIN_FILENO;
        /* close unused file descriptor, that is redirected to the pty */
        close(STDOUT_FILENO);
        child_pid = getpid();
        parent_pid = 0; /* don't kill pppd */
    }

	//fprintf(stderr, "[pptp-c] pptp chk 6\n");	// tmp test
    do {
        /*
         * Open connection to call manager (Launch call manager if necessary.)
         */
        callmgr_sock = open_callmgr(inetaddr, phonenr, argc, argv, envp,
		pty_fd, gre_fd);
        /* Exchange PIDs, get call ID */
    } while (get_call_id(callmgr_sock, parent_pid, child_pid, 
                &call_id, &peer_call_id) < 0);

	//fprintf(stderr, "[pptp-c] pptp chk 6.1 (%d)\n", launchpppd);	// tmp test
    /* Send signal to wake up pppd task */
    if (launchpppd) {
	//fprintf(stderr, "[pptp-c] kill %d SIGUSR1\n", parent_pid);	// tmp test
        kill(parent_pid, SIGUSR1);
        sleep(2);
	//fprintf(stderr, "become a daemon\n");	// tmp test
        /* become a daemon */
        if (!debug && daemon(0, 0) != 0) {
            perror("daemon");
        }
    } else {
        /* re-open stderr as /dev/null to release it */
	//fprintf(stderr, "[pptp-c] reopen stderr as null and release\n");	// tmp test
        file2fd("/dev/null", "wb", STDERR_FILENO);	// disable for tmp
    }

	//fprintf(stderr, "[pptp-c] pptp chk 7\n");	// tmp test
    snprintf(buf, sizeof(buf), "pptp: GRE-to-PPP gateway on %s", 
            ttyname(tty_fd));
    inststr(argc, argv, envp, buf);
    if (sigsetjmp(env, 1)!= 0) 
    {
	//fprintf(stderr, "[pptp-c] sigsetjmp fail,  goto shutdown\n");	// tmp test
	goto shutdown;
    }
	//fprintf(stderr, "[pptp-c] pptp chk 8\n");	// tmp test
    signal(SIGINT,  sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGKILL, sighandler);
    signal(SIGCHLD, sighandler);
    signal(SIGUSR1, sigstats);

	//fprintf(stderr, "[pptp-c] pptp chk 8.1\n");	// tmp test
    if (syncppp) {
	//fprintf(stderr, "[pptp-c] syncppp \n");	// tmp test
        if (ioctl(pty_fd, TIOCSETD, &disc) < 0) {
            fatal("Unable to set line discipline to N_HDLC");
            //fprintf(stderr, "[pptp-c] Unable to set line discipline to N_HDLC");	// tmp test
        } else {
            log("Changed pty line discipline to N_HDLC for synchronous mode");
        }
    }

	//fprintf(stderr, "[pptp-c] pptp chk 9\n");	// tmp test
    /* Do GRE copy until close. */
    pptp_gre_copy(call_id, peer_call_id, pty_fd, gre_fd);

	//fprintf(stderr, "[pptp-c] pptp chk 10 (%d)\n", launchpppd);	// tmp test
shutdown:
    /* on close, kill all. */
    if(launchpppd)
        kill(parent_pid, SIGTERM);
    close(pty_fd);
    close(callmgr_sock);
    /*route_del(&rt); -- don't delete, as otherwise it would try to use pppX in demand mode*/
	//fprintf(stderr, "[pptp-c] exit\n");	// tmp test
    exit(0);
}

/*** get the ipaddress coming from the command line ***************************/
struct in_addr get_ip_address(char *name)
{
    struct in_addr retval;
    struct hostent *host = gethostbyname(name);
    if (host == NULL) {
        if (h_errno == HOST_NOT_FOUND)
            fatal("gethostbyname '%s': HOST NOT FOUND", name);
        else if (h_errno == NO_ADDRESS)
            fatal("gethostbyname '%s': NO IP ADDRESS", name);
        else
            fatal("gethostbyname '%s': name server error", name);
    }
    if (host->h_addrtype != AF_INET)
        fatal("Host '%s' has non-internet address", name);
    memcpy(&retval.s_addr, host->h_addr, sizeof(retval.s_addr));
    return retval;
}

/*** start the call manager ***************************************************/
int open_callmgr(struct in_addr inetaddr, char *phonenr, int argc, char **argv,
        char **envp, int pty_fd, int gre_fd)
{
    /* Try to open unix domain socket to call manager. */
    struct sockaddr_un where;
    const int NUM_TRIES = 3;
    int i, fd;
    pid_t pid;
    int status;

	//fprintf(stderr, "[pptp-c] open callmgr\n");	// tmp test
    /* Open socket */
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        fatal("Could not create unix domain socket: %s", strerror(errno));
    }
    /* Make address */
    callmgr_name_unixsock(&where, inetaddr, localbind);
    for (i = 0; i < NUM_TRIES; i++) {
	//fprintf(stderr, "[pptp-c] try connect [%d / %d]\n", i, NUM_TRIES);	// tmp test
        if (connect(fd, (struct sockaddr *) &where, sizeof(where)) < 0) {
            /* couldn't connect.  We'll have to launch this guy. */

		//fprintf(stderr, "[pptp-c] could not connect, launch this guy\n");	// tmp test
            unlink (where.sun_path);	

            /* fork and launch call manager process */
            switch (pid = fork()) {
                case -1: /* failure */
                    fatal("fork() to launch call manager failed.");
                case 0: /* child */
                {
                    close (fd);
                    /* close the pty and gre in the call manager */
                    close(pty_fd);
                    close(gre_fd);
                    launch_callmgr(inetaddr, phonenr, argc, argv, envp);
                }
                default: /* parent */
                    waitpid(pid, &status, 0);
                    if (status!= 0)
                        fatal("Call manager exited with error %d", status);
                    break;
            }
            sleep(1);
        }
        else return fd;
    }
    close(fd);
    fatal("Could not launch call manager after %d tries.", i);
    fprintf(stderr, "Could not launch call manager after %d tries.", i);
    return -1;   /* make gcc happy */
}

/*** call the call manager main ***********************************************/
void launch_callmgr(struct in_addr inetaddr, char *phonenr, int argc,
        char**argv,char**envp) 
{
      char *my_argv[3] = { argv[0], inet_ntoa(inetaddr), phonenr };
      char buf[128];

	//fprintf(stderr, "[pptp-c] launch callmgr\n");	// tmp test
      snprintf(buf, sizeof(buf), "pptp: call manager for %s", my_argv[1]);
      inststr(argc, argv, envp, buf);
      exit(callmgr_main(3, my_argv, envp));
}

/*** exchange data with the call manager  *************************************/
/* XXX need better error checking XXX */
int get_call_id(int sock, pid_t gre, pid_t pppd, 
		 u_int16_t *call_id, u_int16_t *peer_call_id)
{
    u_int16_t m_call_id, m_peer_call_id;
    /* write pid's to socket */
    /* don't bother with network byte order, because pid's are meaningless
     * outside the local host.
     */
    int rc;

	//fprintf(stderr, "[pptp-c] get call id\n");	// tmp test
    rc = write(sock, &gre, sizeof(gre));
    if (rc != sizeof(gre))
    {
	//fprintf(stderr, "[pptp-c] (get call id) fail 1\n");	// tmp test
        return -1;
    }
    rc = write(sock, &pppd, sizeof(pppd));
    if (rc != sizeof(pppd))
    {
	//fprintf(stderr, "[pptp-c] (get call id) fail 2\n");	// tmp test
        return -1;
    }
    rc = read(sock,  &m_call_id, sizeof(m_call_id));
    if (rc != sizeof(m_call_id))
    {
	//fprintf(stderr, "[pptp-c] (get call id) fail 3\n");	// tmp test
        return -1;
    }
    rc = read(sock,  &m_peer_call_id, sizeof(m_peer_call_id));
    if (rc != sizeof(m_peer_call_id))
    {
	//fprintf(stderr, "[pptp-c] (get call id) fail 4\n");	// tmp test
        return -1;
    }
    /*
     * XXX FIXME ... DO ERROR CHECKING & TIME-OUTS XXX
     * (Rhialto: I am assuming for now that timeouts are not relevant
     * here, because the read and write calls would return -1 (fail) when
     * the peer goes away during the process. We know it is (or was)
     * running because the connect() call succeeded.)
     * (James: on the other hand, if the route to the peer goes away, we
     * wouldn't get told by read() or write() for quite some time.)
     */
    *call_id = m_call_id;
    *peer_call_id = m_peer_call_id;
    return 0;
}

/*** execvp pppd **************************************************************/
void launch_pppd(char *ttydev, int argc, char **argv)
{
    char *new_argv[argc + 4];/* XXX if not using GCC, hard code a limit here. */
    int i = 0, j;
	//fprintf(stderr, "[pptp-c] launch pppd\n");	// tmp test
    new_argv[i++] = PPPD_BINARY;
#ifdef USER_PPP
    new_argv[i++] = "-direct";
    /* ppp expects to have stdin connected to ttydev */
    if ((j = open(ttydev, O_RDWR)) == -1)
        fatal("Cannot open %s: %s", ttydev, strerror(errno));
    if (dup2(j, 0) == -1)
        fatal("dup2 failed: %s", strerror(errno));
    close(j);
#else
    new_argv[i++] = ttydev;
    new_argv[i++] = "38400";
#endif
    for (j = 0; j < argc; j++)
        new_argv[i++] = argv[j];
    new_argv[i] = NULL;
    execvp(new_argv[0], new_argv);
}

/*** route manipulation *******************************************************/

static int
route_ctrl(int ctrl, struct rtentry *rt)
{
	int s;

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ||	ioctl(s, ctrl, rt) < 0)
	        warn("route_ctrl: %s", strerror(errno));
	else errno = 0;

	close(s);
	return errno;
}

static int
route_del(struct rtentry *rt)
{
	if (rt->rt_dev) {
		route_ctrl(SIOCDELRT, rt);
		free(rt->rt_dev), rt->rt_dev = NULL;
	}
	
	return 0;
}

static int
route_add(const struct in_addr inetaddr, struct rtentry *rt)
{
	char buf[256], dev[64];
	int metric, flags;
	u_int32_t dest, mask;
	
	FILE *f = fopen("/proc/net/route", "r");
	if (f == NULL) {
	        warn("/proc/net/route: %s", strerror(errno));
		return -1;
	}

	while (fgets(buf, sizeof(buf), f)) 
	{
		if (sscanf(buf, "%63s %x %x %X %*s %*s %d %x", dev, &dest,
		    	&sin_addr(&rt->rt_gateway).s_addr, &flags, &metric, &mask) != 6)
			continue;
		if ((flags & (RTF_UP | RTF_GATEWAY)) == (RTF_UP | RTF_GATEWAY) && (inetaddr.s_addr & mask) == dest)
		{
			rt->rt_metric = metric;
			rt->rt_gateway.sa_family = AF_INET;
			break;
		}
	}
	
	fclose(f);

	/* check for no route */
	if (rt->rt_gateway.sa_family != AF_INET) 
	{
	        log("route_add: no route to host");
		return -1;
	}

	/* check for existing route to this host, 
	add if missing based on the existing routes */
	if (mask == INADDR_BROADCAST) {
	        log("route_add: not adding existing route");
		return -1;
	}

	sin_addr(&rt->rt_dst) = inetaddr;
	rt->rt_dst.sa_family = AF_INET;

	sin_addr(&rt->rt_genmask).s_addr = INADDR_BROADCAST;
	rt->rt_genmask.sa_family = AF_INET;

	rt->rt_flags = RTF_UP | RTF_HOST;
	if (sin_addr(&rt->rt_gateway).s_addr)
		rt->rt_flags |= RTF_GATEWAY;

	rt->rt_metric++;
	rt->rt_dev = strdup(dev);

	if (!rt->rt_dev)
	{
	        warn("route_add: no memory");
		return -1;
	}
	
	if (!route_ctrl(SIOCADDRT, rt))
		return 0;

	free(rt->rt_dev), rt->rt_dev = NULL;

	return -1;
}
