#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <nvram/bcmnvram.h>
#include "iboxcom.h"
#include "lp.h"

#define SRV_PORT 9999
#define PRINT(fmt, args...) printf(stderr, fmt, ## args)
#define TRUE     1

//#define WLANCFG "/etc/thttpd/www/cgi-bin/wlan.cfg"
#define WLANCFG "/etc/linuxigd/WLANConfig11b"
#define PRODUCTCFG "/etc/linuxigd/general.log"
#define LAN_DEV "br0"


DWORD RECV(int sockfd , PBYTE pRcvbuf , DWORD dwlen , struct sockaddr *from, int *fromlen , DWORD timeout);
int waitsock(int sockfd , int sec , int usec);
int closesocket(int sockfd);
void readPrnID(char *prninfo);
void readParPrnID(char  *prninfo);
void readUsbPrnID(char  *prninfo);
int check_par_usb_prn(void);
int  set_pid(int pid);	   //deliver process id to driver
void sig_usr1(int sig);    //signal handler to handle signal send from driver
void sig_usr2(int sig);

int timeup=0;

/* global variables for interface settings */
/* add by James Yeh 2002/12/23             */
#define MAX_INTERFACE   5
int     g_intfCount = 0;
char*   g_intf[MAX_INTERFACE];

#define MAX_GETID_RETRY 10
int		g_retry = 0;

char ssid_g[32];
char netmask_g[32];
char productid_g[32];
char firmver_g[16];
unsigned char mac[6] = { 0x00, 0x0c, 0x6e, 0xbd, 0xf3, 0xc5};


void sig_do_nothing(int sig)
{
}

void load_sysparam(void)
{
	char macstr[32];
	char macdigit[3];

	strncpy(ssid_g, nvram_safe_get("wl_ssid"), sizeof(ssid_g));
	strncpy(netmask_g, nvram_safe_get("lan_netmask"), sizeof(netmask_g));
	strncpy(productid_g, nvram_safe_get("fac_model_name"), sizeof(productid_g));
	strncpy(firmver_g, nvram_safe_get("firmver"), sizeof(firmver_g));

	strcpy(macstr, nvram_safe_get("wl0_macaddr"));
	//printf("mac: %d\n", strlen(macstr));
	if (strlen(macstr)!=0) ether_atoe(macstr, mac);
}

int main(int argc , char* argv[])
{
    int                 sockfd , clisockfd;
    unsigned int        clilen;
    int                 childpid;
    struct sockaddr_in  serv_addr,cli_addr;
    int                 broadcast = 1;
    int                 count = 0;
    int ret, unit;
    struct timeval  tv;

    if(argc < 2)
    {
        printf("not enough parameters for infosvr\n");
        printf("infosvr netif...\n");
        exit(0);    
    }


    // Write PID file
    if (!set_pid(getpid())) // Add by Joey
    {
	printf("can not set process id\n");
	exit(0);
    }


    // Load system related static parameter
    load_sysparam();

#ifdef WCLIENT
    // Signal for other tools
    signal(SIGUSR1, sig_usr1);
    signal(SIGUSR2, sig_usr2);


    // Start state transaction
    sta_info_init();
#endif

#ifdef PRNINFO //JYWeng: 20030325 for WL500g/WL500b
    // Signal for reading Printer Information
    signal(SIGALRM, sig_usr1); // Add by Joey
    signal(SIGUSR1 , sig_usr1);
    alarm(2);		       // Get ID for the first time 2 seconds later
#endif
/*    
    signal(SIGUSR1, sig_do_nothing);
*/
    signal(SIGUSR2, sig_do_nothing);

    g_intfCount = argc - 1;

    while(count < g_intfCount)
    {
        g_intf[count]  = argv[count+1];
        count ++;
    }
    

    if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0 )
    {
        PRINT("can't open datagram socket\n");
        perror("socket:");
        exit(0);
    }
    
    if(setsockopt(sockfd , SOL_SOCKET , SO_BROADCAST ,(char *)&broadcast,sizeof(broadcast)) == -1)
    {
        perror("setsockopt:");
    }

    bzero((char *)&cli_addr , sizeof(cli_addr));
    cli_addr.sin_family        = AF_INET;
    cli_addr.sin_addr.s_addr   = inet_addr("255.255.255.255");
    cli_addr.sin_port          = htons(SRV_PORT);
    
    bzero((char *)&serv_addr , sizeof(serv_addr));
    serv_addr.sin_family        = AF_INET;
    serv_addr.sin_addr.s_addr   = htonl(INADDR_ANY);
    serv_addr.sin_port          = htons(SRV_PORT);
    
    if(bind(sockfd,(struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0 )
    {
        PRINT("can't bind\n");
        perror("ERR:");
        exit(0);
    }
    
    while(TRUE)
    {
        int     len,res;
        fd_set  fdvar;
        char    databuf[1024];
        
        FD_ZERO(&fdvar);
        FD_SET(sockfd, &fdvar);
       
 
    	tv.tv_sec  = 2;
    	tv.tv_usec = 0; 
        res = select( sockfd + 1 , &fdvar , NULL , NULL , &tv);
    
        if(res == 1)
        {
	    //printf("got packet\n");
            processReq(sockfd);
        }
#ifdef WCLIENT
	sta_status_check();
#endif
    }	
}

int processReq(int sockfd)
{
    IBOX_COMM_PKT_HDR*  phdr;
    int                 iLen , iRes , iCount , iRcv;
    int                 fromlen;
    char                *hdr;
    char                pdubuf[INFO_PDU_LENGTH];
    struct sockaddr_in  from_addr;
    
    memset(pdubuf,0,sizeof(pdubuf));
    
    //PRINT("Enter processReq\n");
    
    if( waitsock(sockfd , 1 , 0) == 0)
    {
        return -1;
    }
    
    fromlen = sizeof(from_addr);
    
    //Receive the complete PDU
    iRcv = RECV(sockfd , pdubuf , INFO_PDU_LENGTH , (struct sockaddr *)&from_addr , &fromlen  , 1);
    
    if(iRcv != INFO_PDU_LENGTH)
    {
        closesocket(sockfd);
        return(-1);
    }
    
    hdr = pdubuf;
    
    processPacket(sockfd, hdr);	
    //closesocket(sockfd);
}

/************************************/
/***  Receive the socket          ***/
/***  with a timeout value        ***/
/************************************/
DWORD RECV(int sockfd , PBYTE pRcvbuf , DWORD dwlen , struct sockaddr *from, int *fromlen , DWORD timeout)
{

    if( waitsock(sockfd , timeout , 0) == 0)
    {
        //timeout
        PRINT("RECV timeout %d\n",timeout);
        return -1;
    }

    return recvfrom(sockfd, pRcvbuf , dwlen , 0 , from , fromlen );
}


int waitsock(int sockfd , int sec , int usec)
{
    struct timeval  tv;
    fd_set          fdvar;
    int             res;
    
    FD_ZERO(&fdvar);
    FD_SET(sockfd, &fdvar);
    
    tv.tv_sec  = sec;
    tv.tv_usec = usec; 
    
    res = select( sockfd + 1 , &fdvar , NULL , NULL , &tv);
    
    return res;
}

int closesocket(int sockfd)
{
    /* Clear the UDP socket */
    char dummy[1024];
    int  iLen , res;

    //PRINT("closesocket ... \n");
        
    res = waitsock(sockfd , 1 , 0);
    
    //PRINT("waitsock res %d\n",res);
    
    if(res == 1)
    {
        iLen  = recvfrom(sockfd, dummy , sizeof(dummy) , 0,NULL,0 );    
    }
    else
    {
        iLen = 0;
    }
    
    //PRINT("iLen %d\n",iLen);
    
    if(iLen == sizeof(dummy))
    {
        res = waitsock(sockfd , 1 , 0);
    }
    else
    {
        return 0;
    }
    
    
    
    while(res == 1)
    {
        iLen  = recvfrom(sockfd, dummy , sizeof(dummy) , 0,NULL,0 );    
        //PRINT("iLen = %d\n",res);
        res = waitsock(sockfd , 0 , 100);
    }
    
    return 0;
    
}


void readPrnID(char *prninfo)
{
    char    *token;
    
    if(check_par_usb_prn() == TRUE) //uese USB when return TRUE
    {
    	readUsbPrnID(prninfo);
    }
    else
    {
    	readParPrnID(prninfo);
    }
}

void deCR(char *str)
{
	int i, len;

	len = strlen(str);
	for (i=0; i<len; i++)
	{
		if (*(str+i) == '\r' || *(str+i) == '\n', *(str+i) == '"')
		{
			*(str+i) = 0;
			break;
		}
	}
}

void readUsbPrnID(char  *prninfo)
{
    char                                mfr[32];
    char                                model[64];
    int                                 fd;
    struct parport_splink_device_info   prn_info;


    PRINT("readUsbPrnID\n");
    
    if( (fd=open("/dev/usb/lp0", O_RDWR)) < 0 ) //Someone is opening the usb lp0
    {
        FILE *fp;
        
        fp=fopen("/proc/usblp/usblpid","r");

        if( fp != NULL)
        {
            ///
            char *tokenEntry;
            char *tokenField;
            char *manufac;
            char *mod;
            char buf[1024];
            char *token;

            memset(mfr , 0 , sizeof(mfr));
            memset(model , 0 , sizeof(model));
                
            while ( fgets(buf, sizeof(buf), fp) != NULL )  
            {
                if(buf[0] == '\n')
                {
                    PRINT("skip empty line\n");
                    continue;
                }
    
                if(strncmp(buf , "Manufacturer=" , strlen("Manufacturer=")) == 0)
                {
                    token= buf + strlen("Manufacturer=");
                    PRINT("Manufacturer token %s",token);
    
                    memccpy(mfr , token , '\n' , 31);
					deCR(mfr);	 
                }
                
                if(strncmp(buf , "Model=" , strlen("Model="))   == 0)
                {
                    token= buf + strlen("Model=");
                    PRINT("Model token %s",token);
    
                    memccpy(model   , token , '\n' , 63); 
					deCR(model);
                }
                
            }

            fclose(fp);
        
            sprintf( prninfo , "%s %s", mfr , model );
            
            PRINT("PRINTER MODEL %s\n",prninfo);
        }
        else
        {
            sprintf( prninfo , "  " );
            PRINT("open failed\n");
        }
        
    }
    else
    {
    
        if( ioctl(fd, LPGETID, &prn_info) <0 )
        {
            PRINT("ioctl error\n");
            perror("ioctl");
        }
        else
        {
            int  i = 0;
        
            //PRINT("manufacturer: %s\n",prn_info.mfr);
            //PRINT("model: %s\n",prn_info.model);
            //PRINT("class: %s\n",prn_info.class_name);
            //PRINT("description: %s\n",prn_info.description);

            memccpy(mfr , prn_info.mfr , 1 , 32);
            memccpy(model , prn_info.model , 1 , 32);
        
            sprintf( prninfo , "%s %s", mfr , model );
            
            PRINT("prninfo %s",prninfo);
        
        }

        close(fd);
    }
}


void readParPrnID(char  *prninfo)
{
    char                                mfr[32];
    char                                model[64];
    int                                 fd;
    struct parport_splink_device_info   prn_info;


    PRINT("readParPrnID\n");
    
    if( (fd=open("/dev/lp0", O_RDWR)) < 0 ) //Someone is opening the lp0
    {
        FILE *fp;
        
        fp=fopen("/proc/sys/dev/parport/parport0/devices/lp/deviceid","r");

        if( fp != NULL)
        {
            ///
            char *tokenEntry;
            char *tokenField;
            char *manufac;
            char *mod;
            char buf[1024];
            char *token;

            memset(mfr , 0 , sizeof(mfr));
            memset(model , 0 , sizeof(model));
                
            while ( fgets(buf, sizeof(buf), fp) != NULL )  
            {
                if(buf[0] == '\n')
                {
                    PRINT("skip empty line\n");
                    continue;
                }
    
                if(strncmp(buf , "Manufacturer: " , strlen("Manufacturer: "))   == 0)
                {
                    token= buf + strlen("Manufacturer: ");
                    PRINT("Manufacturer token %s",token);
    
                    memccpy(mfr , token , '\n' , 31); 
					deCR(mfr);
                }
                
                if(strncmp(buf , "Model: " , strlen("Model: ")) == 0)
                {
                    token= buf + strlen("Model: ");
                    PRINT("Model token %s",token);
    
                    memccpy(model   , token , '\n' , 63); 
					deCR(model);
                }
                
            }

            fclose(fp);
        
            sprintf( prninfo , "%s %s", mfr , model );
            
            PRINT("prninfo %s\n",prninfo);
        }
        else
        {
            sprintf( prninfo , "  " );
            PRINT("open failed\n");
        }
        
    }
    else
    {
    
        if( ioctl(fd, LPGETID, &prn_info) <0 )
        {
            //PRINT("ioctl error\n");
        }
        else
        {
            int  i = 0;
        
            //PRINT("manufacturer: %s\n",prn_info.mfr);
            //PRINT("model: %s\n",prn_info.model);
            //PRINT("class: %s\n",prn_info.class_name);
            //PRINT("description: %s\n",prn_info.description);

            memccpy(mfr , prn_info.mfr , 1 , 32);
            memccpy(model , prn_info.model , 1 , 32);
        
            sprintf( prninfo , "%s %s", mfr , model );
        
        }

        close(fd);
    }
}

#ifdef PRNINFO
#define ONLINE_STRING "On-Line"
#define OFFLINE_STRING "Off-Line"

void readPrnStatus(void)
{
    FILE    *fp;
    char    buf[64];
    char    *ukeyword="PRINTER_USER=\"";
    char    *skeyword="PRINTER_STATUS=\"";
    char    *token;
    char    user[32];
    char    status[32];
    
    fp = fopen("/var/state/printstatus.txt", "r");

    memset(user, 0, sizeof(user));
    memset(status, 0, sizeof(status));	

    if (fp!=NULL)
    { 
            while ( fgets(buf, sizeof(buf), fp) != NULL )  
            {
                if(buf[0] == '\n')
                {
                    PRINT("skip empty line\n");
                    continue;
                }
    
                if(strncmp(buf , ukeyword , strlen(ukeyword)) == 0)
                {
                    token= buf + strlen(ukeyword);
                    PRINT("User token %s",token);
    
		    deCR(token);
		    strcpy(user, token);
                }                
                else if(strncmp(buf , skeyword, strlen(skeyword)) == 0)
                {
                    token= buf + strlen(skeyword);
                    PRINT("Status token %s",token);
   
		    deCR(token);
		    strcpy(status, token);
                }                
            }
	    fclose(fp);					
    }
    else strcpy(status, ONLINE_STRING);	
    nvram_set("printer_status_t", status);
    nvram_set("printer_user_t", user);
    	
    return;
}

/* update current status of printer */
void sig_usr1(int sig)
{
    char    prninfo[256], status[32];
    int isUsb;
  
    //fp=fopen("/etc/linuxigd/printer.log","w");
    
    memset( prninfo , 0 , sizeof(prninfo) );
     

    if(check_par_usb_prn() == TRUE) //uese USB when return TRUE
    {
	    //fputs( "INTERFACE=USB\n" , fp );
	    nvram_set("printer_ifname", "usb");
	    readUsbPrnID(prninfo);
	    isUsb = 1;
    }
    else
    {
	    // Goto this step, then printer must be turn on 
	    //fputs( "INTERFACE=PAR\n" , fp );
	    nvram_set("printer_ifname", "par");
    	    readParPrnID(prninfo);
	    isUsb = 0;
    }
    
    if (strcmp(prninfo, "  ")==0 || strlen(prninfo)==0)
    {
		//fprintf(fp, "MODEL=\"\"\n");
		nvram_set("printer_model_t", "");

		if (!isUsb)  
		{
			/* Retry, only when paraport is plugged and no device id is returned */
			printf("Try again 2 seconds later\n");
			g_retry++;

			if (g_retry<MAX_GETID_RETRY) alarm(6);
			else g_retry = 0;			
		}
    }
    else
    {
		//fprintf(fp, "MODEL=\"%s\"\n", prninfo);
		nvram_set("printer_model_t", prninfo);
    }

    /* update status of printer */
    if (nvram_invmatch("printer_model_t", ""))
    {
		readPrnStatus();
    }
    else 
    {	
		nvram_set("printer_status_t", "");
		nvram_set("printer_user_t", "");
    }	
    PRINT("Enter sig_usr1\n"); 
}
#endif

#ifdef WCLIENT
extern int wl_scan_disabled;
void sig_usr1(int sig)
{
    //if (wl_scan_disabled) wl_scan_disabled=0;
    //else wl_scan_disabled=1;
    //sta_status_report(0, 1);
    check_tools();
}

void sig_alrm(int sig)
{
    timeup = 1;
}

// Write current status of wireless
void sig_usr2(int sig)
{
    sta_status_report(NULL, 1);
}
#endif
/* to check use usb or parport printer */
/* return TRUE when using USB		   */
/* James Yeh 2002/12/25 02:13	       */

int check_par_usb_prn()
{
    char    buf[1024];
    char    *token;
    FILE    *fp;
   
#ifdef NO_PARALLEL
    return TRUE;
#else 
    fp=fopen("/proc/sys/dev/parport/parport0/devices/lp/deviceid","r");

    if( fp != NULL)
    {
        while ( fgets(buf, sizeof(buf), fp) != NULL )  
        {

            PRINT("lp:%s\n", buf);

            if(buf[0] == '\n')
            {
                continue;
            }
    
            if(strncmp(buf , "status: " , strlen("status: "))   == 0)
            {
                token= buf + strlen("status: ");
                
                PRINT("token[0] %d\n",token[0]);
                
		        fclose(fp);
        
				
                if(token[0] == '0')
                {
                    return(TRUE);
                }
                else
                {
                    return(0);
                }
                break;
            }
            
        }
        
        fclose(fp);
    }
#endif
    
}


/* Deliver pid to driver */
/* Add by Joey           */
int set_pid(int pid)
{
	FILE *fp;
	int	fd, ret=0;
	struct print_buffer pb;	

#ifndef NO_PARALLEL
	fd = open("/dev/lp0", O_RDWR);

	if (fd<0) return 0;

	if ((pb.buf = malloc(12)) == NULL) return 0;	
	
	sprintf(pb.buf, "%d", pid);

    	if (ioctl(fd, LPSETID, &pb)<0) 
	{
	    free(pb.buf);
		return 0;
	}
	free(pb.buf);

	close(fd); 		
#endif

	fp = fopen("/var/run/infosvr.pid", "w");
	fprintf(fp, "%d", pid);
	fclose(fp);
	
	return 1;
}

void sendInfo(int sockfd, char *pdubuf)
{
    struct sockaddr_in                  cli;
    IBOX_COMM_PKT_HDR                   hdr;
    int count=0, err;

    cli.sin_family    = AF_INET;
    cli.sin_addr.s_addr = inet_addr("255.255.255.255");;
    cli.sin_port = htons(SRV_PORT);

    while(count < g_intfCount)
    {
        //PRINT("g_intf[%d] %s\n",count,g_intf[count]);   
        err = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, g_intf[count] , IFNAMSIZ);
        if(err != 0)
        {
            printf("%s\n",g_intf[count]);
            perror("setsockopt:");
        }
    
        if(sendto(sockfd , pdubuf , INFO_PDU_LENGTH , 0 ,(struct sockaddr *) &cli, sizeof(cli)) == -1)
        {
            perror("sendto:");
        }    
        count ++;
    }   

    err = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, "" , IFNAMSIZ); 

    if(err != 0)
    {
        perror("setsockopt reset:");
    }
}
