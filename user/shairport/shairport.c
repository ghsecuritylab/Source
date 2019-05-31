#include <fcntl.h>
#include <signal.h>
#include "socketlib.h"
#include "shairport.h"
#include "hairtunes.h"
#include <nvram/bcmnvram.h>

#ifndef TRUE
#define TRUE (-1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#ifdef I2C_TO_ALC5627
int open_mixer_fd(void);
#define RT2880_I2C_SET_DAC_MUTE		2
#define RT2880_I2C_SET_DAC_UNMUTE	4
#endif

int kCurrentLogLevel = LOG_INFO;
int bufferStartFill = -1;
extern int forkcount;
#ifdef _WIN32
#define DEVNULL "nul"
#else
#define DEVNULL "/dev/null"
#endif

static void handleClient(int pSock, char *pPassword, char *pHWADDR);
static void writeDataToClient(int pSock, struct shairbuffer *pResponse);
static int readDataFromClient(int pSock, struct shairbuffer *pClientBuffer);

static int parseMessage(struct connection *pConn,unsigned char *pIpBin, unsigned int pIpBinLen, char *pHWADDR);
static void propogateCSeq(struct connection *pConn);

static void cleanupBuffers(struct connection *pConnection);
static void cleanup(struct connection *pConnection);

static int startAvahi(const char *pHwAddr, const char *pServerName, int pPort);

static int getAvailChars(struct shairbuffer *pBuf);
static void addToShairBuffer(struct shairbuffer *pBuf, char *pNewBuf);
static void addNToShairBuffer(struct shairbuffer *pBuf, char *pNewBuf, int pNofNewBuf);

static char *getTrimmedMalloc(char *pChar, int pSize, int pEndStr, int pAddNL);
static char *getTrimmed(char *pChar, int pSize, int pEndStr, int pAddNL, char *pTrimDest);

static void slog(int pLevel, char *pFormat, ...);

static void initConnection(struct connection *pConn, struct keyring *pKeys, struct comms *pComms, int pSocket, char *pPassword);
static void closePipe(int *pPipe);
static void initBuffer(struct shairbuffer *pBuf, int pNumChars);

static void setKeys(struct keyring *pKeys, char *pIV, char* pAESKey, char *pFmtp);
static RSA *loadKey(void);


static void handle_sigchld(int signo) {
    int status;
    waitpid(-1, &status, WNOHANG);
}

int transnum(char a)
{
	if(a>=65 && a<=70)
	   return (a-55);
	else if(a>=97 && a<=102)
	   return (a-87);
	else if (a>=48 && a<=57)
	   return (a-48);
}   


int main(int argc, char **argv)
{
  // unfortunately we can't just IGN on non-SysV systems
  struct sigaction sa;
  sa.sa_handler = handle_sigchld;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  printf("Eversion 2.1\n");
  system("mDNSResponder");
  sleep(3);

  if (sigaction(SIGCHLD, &sa, NULL) < 0) {
      perror("sigaction");
      return 1;
  }

  char tHWID[HWID_SIZE] = {0,51,52,53,54,55};
  char tHWID_Hex[HWID_SIZE * 2 + 1];
  memset(tHWID_Hex, 0, sizeof(tHWID_Hex));

  char tServerName[56] = "ShairPort";
  char tPassword[56] = "";

  struct sockaddr_in tAddrInfo;
  int  tSimLevel = 0;
  int  tUseKnownHWID = FALSE;
  int  tDaemonize = FALSE;
  int  tPort = PORT;

  char *arg;
  while ( (arg = *++argv) ) {
    if(!strcmp(arg, "-a"))
    {
       strncpy(tServerName, *++argv, 55);
       argc--;
    }
    else if(!strncmp(arg, "--apname=", 9))
    {
      strncpy(tServerName, arg+9, 55);
    }
    else if(!strcmp(arg, "-p"))
    {
      strncpy(tPassword, *++argv, 55);
      argc--;
    }
    else if(!strncmp(arg, "--password=",11 ))
    {
      strncpy(tPassword, arg+11, 55);
    }
    else if(!strcmp(arg, "-o"))
    {
      tPort = atoi(*++argv);
      argc--;
    }
    else if(!strncmp(arg, "--server_port=", 14))
    {
      tPort = atoi(arg+14);
    }
    else if(!strcmp(arg, "-b")) 
    {
      bufferStartFill = atoi(*++argv);
      argc--;
    }
    else if(!strncmp(arg, "--buffer=", 9))
    {
      bufferStartFill = atoi(arg + 9);
    }
    else if(!strcmp(arg, "-k"))
    {
      tUseKnownHWID = TRUE;
    }
    else if(!strcmp(arg, "-q") || !strncmp(arg, "--quiet", 7))
    {
      kCurrentLogLevel = 0;
    }
    else if(!strcmp(arg, "-d"))
    {
      tDaemonize = TRUE;
      kCurrentLogLevel = 0;
    }
    else if(!strcmp(arg, "-v"))
    {
      kCurrentLogLevel = LOG_DEBUG;
    }
    else if(!strcmp(arg, "-h") || !strcmp(arg, "--help"))
    {
      slog(LOG_INFO, "ShairPort version 0.05 C port - Airport Express emulator\n");
      slog(LOG_INFO, "Usage:\nshairport [OPTION...]\n\nOptions:\n");
      slog(LOG_INFO, "  -a, --apname=AirPort    Sets Airport name\n");
      slog(LOG_INFO, "  -p, --password=secret   Sets Password (not working)\n");
      slog(LOG_INFO, "  -o, --server_port=5002  Sets Port for Avahi/dns-sd/howl\n");
      slog(LOG_INFO, "  -b, --buffer=282        Sets Number of frames to buffer before beginning playback\n");
      slog(LOG_INFO, "  -d                      Daemon mode\n");
      slog(LOG_INFO, "  -q, --quiet             Supresses all output.\n");
      slog(LOG_INFO, "  -v,	                Various debugging levels\n");
      slog(LOG_INFO, "\n");
      return 0;
    }    
  }

  if ( bufferStartFill < 30 || bufferStartFill > BUFFER_FRAMES ) {
     fprintf(stderr, "buffer value must be > 30 and < %d\n", BUFFER_FRAMES);
     return(0);
  }

  if(tDaemonize)
  {
    int tPid = fork();
    if(tPid < 0)
    {
      exit(1); // Error on fork
    }
    else if(tPid > 0)
    {
      exit(0);
    }
    else
    {
      setsid();
      int tIdx = 0;
      for(tIdx = getdtablesize(); tIdx >= 0; --tIdx)
      {
        close(tIdx);
      }
      tIdx = open(DEVNULL, O_RDWR);
      dup(tIdx);
      dup(tIdx);
    }
  }

//  srandom ( time(NULL) );
  // Copy over empty 00's
  //tPrintHWID[tIdx] = tAddr[0];
  int tIdx = 0;
  char ptb[18],pta[13],ptc[2];
  memset(pta,0,sizeof(pta));
  strcpy(ptb,nvram_get("wl0_macaddr"));
 
  for(tIdx=1;tIdx<7;tIdx++)
  {
     pta[2*tIdx-2]=ptb[3*tIdx-3];
     pta[2*tIdx-1]=ptb[3*tIdx-2];
     tHWID[tIdx-1]=transnum(pta[2*tIdx-2])*16+transnum(pta[2*tIdx-1]);
  }  
  strcpy(tHWID_Hex,pta);
  //tPrintHWID[HWID_SIZE] = '\0';
	
  slog(LOG_INFO, "LogLevel: %d\n", kCurrentLogLevel);
  slog(LOG_INFO, "AirName: %s\n", tServerName);
  slog(LOG_INFO, "HWID: %.*s\n", HWID_SIZE, tHWID+1);
  slog(LOG_INFO, "HWID_Hex(%d): %s\n", strlen(tHWID_Hex), tHWID_Hex);

  if(tSimLevel >= 1)
  {
    #ifdef SIM_INCL
    sim(tSimLevel, tTestValue, tHWID);
    #endif
    return(1);
  }
  else
  {
    startAvahi(tHWID_Hex, tServerName, tPort);
    slog(LOG_DEBUG, "Starting connection server: specified server port: %d\n", tPort);
    int tServerSock = setupListenServer(&tAddrInfo, tPort);
    if(tServerSock < 0)
    {
      //freeaddrinfo(tAddrInfo);
      slog(LOG_INFO, "Error setting up server socket on port %d, try specifying a different port\n", tPort);
      exit(1);
    }
    forkcount=0;
    int tClientSock = 0;
    while(1)
    {
      slog(LOG_DEBUG, "Waiting for clients to connect\n");
      tClientSock = acceptClient(tServerSock, &tAddrInfo);
      if(tClientSock > 0)
      {
	if(forkcount>2000)
	   	forkcount=0;
	else
  		forkcount++;
        int tPid = fork();
        if(tPid == 0)
        {
          //freeaddrinfo(tAddrInfo);
          //tAddrInfo = NULL;
          slog(LOG_DEBUG, "...Accepted Client Connection..\n");
          close(tServerSock);
          handleClient(tClientSock, tPassword, tHWID);
          //close(tClientSock);
          return 0;
        }
        else
        {
          slog(LOG_DEBUG, "Child now busy handling new client\n");
          close(tClientSock);
        }
      }
      else
      {
        // failed to init server socket....try waiting a moment...
        sleep(2);
      }
    }
  }

  slog(LOG_DEBUG, "Finished, and waiting to clean everything up\n");
  sleep(1);
  //if(tAddrInfo != NULL)
  //{
   //  return ERROR;
   // freeaddrinfo(tAddrInfo);
  //}
  return 0;
}

static int findEnd(char *tReadBuf)
{
  // find \n\n, \r\n\r\n, or \r\r is found
  int tIdx = 0;
  int tLen = strlen(tReadBuf);
  for(tIdx = 0; tIdx < tLen; tIdx++)
  {
    if(tReadBuf[tIdx] == '\r')
    {
      if(tIdx + 1 < tLen)
      {
        if(tReadBuf[tIdx+1] == '\r')
        {
          return (tIdx+1);
        }
        else if(tIdx+3 < tLen)
        {
          if(tReadBuf[tIdx+1] == '\n' &&
             tReadBuf[tIdx+2] == '\r' &&
             tReadBuf[tIdx+3] == '\n')
          {
            return (tIdx+3);
          }
        }
      }
    }
    else if(tReadBuf[tIdx] == '\n')
    {
      if(tIdx + 1 < tLen && tReadBuf[tIdx+1] == '\n')
      {
        return (tIdx + 1);
      }
    }
  }
  // Found nothing
  return -1;
}

static void handleClient(int pSock, char *pPassword, char *pHWADDR)
{
  slog(LOG_DEBUG, "In Handle Client\n");
  fflush(stdout);

  socklen_t len;
  struct sockaddr_storage addr;
  #if 0
  unsigned char ipbin[INET6_ADDRSTRLEN];
  #else
  unsigned char ipbin[INET_ADDRSTRLEN];
  #endif
  unsigned int ipbinlen;
  int port;
  char ipstr[64];

  len = sizeof addr;
  getsockname(pSock, (struct sockaddr*)&addr, &len);

  // deal with both IPv4 and IPv6:
  if (addr.ss_family == AF_INET) {
      slog(LOG_DEBUG, "Constructing ipv4 address\n");
      struct sockaddr_in *s = (struct sockaddr_in *)&addr;
      port = ntohs(s->sin_port);
      inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
      memcpy(ipbin, &s->sin_addr, 4);
      ipbinlen = 4;
  }
#if 0 
  else { // AF_INET6
      slog(LOG_DEBUG, "Constructing ipv6 address\n");
      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
      port = ntohs(s->sin6_port);
      inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);

      union {
          struct sockaddr_in6 s;
          unsigned char bin[sizeof(struct sockaddr_in6)];
      } addr;
      memcpy(&addr.s, &s->sin6_addr, sizeof(struct sockaddr_in6));

      if(memcmp(&addr.bin[0], "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\xff\xff", 12) == 0)
      {
        // its ipv4...
        memcpy(ipbin, &addr.bin[12], 4);
        ipbinlen = 4;
      }
      else
      {
        memcpy(ipbin, &s->sin6_addr, 16);
        ipbinlen = 16;
      }
  }
#endif
  slog(LOG_DEBUG, "Peer IP address: %s\n", ipstr);
  slog(LOG_DEBUG, "Peer port      : %d\n", port);

  int tMoreDataNeeded = 1;
  struct keyring     tKeys;
  struct comms       tComms;
  struct connection  tConn;
  initConnection(&tConn, &tKeys, &tComms, pSock, pPassword);

  while(1)
  {
    tMoreDataNeeded = 1;

    initBuffer(&tConn.recv, 80); // Just a random, small size to seed the buffer with.
    initBuffer(&tConn.resp, 80);
    
    int tError = FALSE;
    while(1 == tMoreDataNeeded)
    {
      tError = readDataFromClient(pSock, &(tConn.recv));
      if(!tError && strlen(tConn.recv.data) > 0)
      {
        slog(LOG_DEBUG, "Finished Reading some data from client\n");
        // parse client request
        tMoreDataNeeded = parseMessage(&tConn, ipbin, ipbinlen, pHWADDR);
        if(1 == tMoreDataNeeded)
        {
          slog(LOG_DEBUG, "\n\nNeed to read more data\n");
        }
        else if(-1 == tMoreDataNeeded) // Forked process down below ended.
        {
          slog(LOG_DEBUG, "Forked Process ended...cleaning up\n");
          cleanup(&tConn);
          // pSock was already closed
          return;
        }
        // if more data needed,
      }
      else
      {
        slog(LOG_DEBUG, "Error reading from socket, closing client\n");
        // Error reading data....quit.
        cleanup(&tConn);
        return;
      }
    }
    slog(LOG_DEBUG, "Writing: %d chars to socket\n", tConn.resp.current);
    //tConn->resp.data[tConn->resp.current-1] = '\0';
    writeDataToClient(pSock, &(tConn.resp));
   // Finished reading one message...
    cleanupBuffers(&tConn);
  }
  cleanup(&tConn);
  fflush(stdout);
}

static void writeDataToClient(int pSock, struct shairbuffer *pResponse)
{
  slog(LOG_DEBUG, "\n----Beg Send Response Header----\n%.*s\n", pResponse->current, pResponse->data);
  send(pSock, pResponse->data, pResponse->current,0);
  slog(LOG_DEBUG, "----Send Response Header----\n");
}

static int readDataFromClient(int pSock, struct shairbuffer *pClientBuffer)
{
  char tReadBuf[MAX_SIZE];
  tReadBuf[0] = '\0';

  int tRetval = 1;
  int tEnd = -1;
  while(tRetval > 0 && tEnd < 0)
  {
     // Read from socket until \n\n, \r\n\r\n, or \r\r is found
      slog(LOG_DEBUG, "Waiting To Read...\n");
      fflush(stdout);
      tRetval = read(pSock, tReadBuf, MAX_SIZE);

      // if new buffer contains the end of request string, only copy partial buffer?
      tEnd = findEnd(tReadBuf);
      if(tEnd >= 0)
      {
        if(pClientBuffer->marker == 0)
        {
          pClientBuffer->marker = tEnd+1; // Marks start of content
        }
        slog(LOG_DEBUG, "Found end of http request at: %d\n", tEnd);
        fflush(stdout);        
      }
      else
      {
        tEnd = MAX_SIZE;
        slog(LOG_DEBUG, "Read %d of data so far\n%s\n", tRetval, tReadBuf);
        fflush(stdout);
      }
      if(tRetval > 0)
      {
        // Copy read data into tReceive;
        slog(LOG_DEBUG, "Read %d data, using %d of it\n", tRetval, tEnd);
        addNToShairBuffer(pClientBuffer, tReadBuf, tRetval);
        slog(LOG_DEBUG, "Finished copying data\n");
      }
      else
      {
        slog(LOG_DEBUG, "Error reading data from socket, got: %d bytes", tRetval);
        return tRetval;
      }
  }
  if(tEnd + 1 != tRetval)
  {
    slog(LOG_DEBUG, "Read more data after end of http request. %d instead of %d\n", tRetval, tEnd+1);
  }
  slog(LOG_DEBUG, "Finished Reading Data:\n%s\nEndOfData\n", pClientBuffer->data);
  fflush(stdout);
  return 0;
}

static char *getFromBuffer(char *pBufferPtr, const char *pField, int pLenAfterField, int *pReturnSize, char *pDelims)
{
  slog(LOG_DEBUG, "GettingFromBuffer: %s\n", pField);
  char* tFound = strstr(pBufferPtr, pField);
  int tSize = 0;
  if(tFound != NULL)
  {
    tFound += (strlen(pField) + pLenAfterField);
    int tIdx = 0;
    char tDelim = pDelims[tIdx];
    char *tShortest = NULL;
    char *tEnd = NULL;
    while(tDelim != '\0')
    {
      tDelim = pDelims[tIdx++]; // Ensures that \0 is also searched.
      tEnd = strchr(tFound, tDelim);
      if(tEnd != NULL && (NULL == tShortest || tEnd < tShortest))
      {
        tShortest = tEnd;
      }
    }
    
    tSize = (int) (tShortest - tFound);
    slog(LOG_DEBUG, "Found %.*s  length: %d\n", tSize, tFound, tSize);
    if(pReturnSize != NULL)
    {
      *pReturnSize = tSize;
    }
  }
  else
  {
    slog(LOG_DEBUG, "Not Found\n");
  }
  return tFound;
}

static char *getFromHeader(char *pHeaderPtr, const char *pField, int *pReturnSize)
{
  return getFromBuffer(pHeaderPtr, pField, 2, pReturnSize, "\r\n");
}

static char *getFromContent(char *pContentPtr, const char* pField, int *pReturnSize)
{
  return getFromBuffer(pContentPtr, pField, 1, pReturnSize, "\r\n");
}

static char *getFromSetup(char *pContentPtr, const char* pField, int *pReturnSize)
{
  return getFromBuffer(pContentPtr, pField, 1, pReturnSize, ";\r\n");
}

// Handles compiling the Apple-Challenge, HWID, and Server IP Address
// Into the response the airplay client is expecting.
static int buildAppleResponse(struct connection *pConn, unsigned char *pIpBin,
                              unsigned int pIpBinLen, char *pHWID)
{
  // Find Apple-Challenge
  char *tResponse = NULL;

  int tFoundSize = 0;
  char* tFound = getFromHeader(pConn->recv.data, "Apple-Challenge", &tFoundSize);
  if(tFound != NULL)
  {
    char tTrim[tFoundSize + 2];
    getTrimmed(tFound, tFoundSize, TRUE, TRUE, tTrim);
    slog(LOG_DEBUG, "HeaderChallenge:  [%s] len: %d  sizeFound: %d\n", tTrim, strlen(tTrim), tFoundSize);
    int tChallengeDecodeSize = 16;
    char *tChallenge = decode_base64((unsigned char *)tTrim, tFoundSize, &tChallengeDecodeSize);
    slog(LOG_DEBUG, "Challenge Decode size: %d  expected 16\n", tChallengeDecodeSize);

    int tCurSize = 0;
    unsigned char tChalResp[38];

    memcpy(tChalResp, tChallenge, tChallengeDecodeSize);
    tCurSize += tChallengeDecodeSize;
    
    memcpy(tChalResp+tCurSize, pIpBin, pIpBinLen);
    tCurSize += pIpBinLen;

    memcpy(tChalResp+tCurSize, pHWID, HWID_SIZE);
    tCurSize += HWID_SIZE;

    int tPad = 32 - tCurSize;
    if (tPad > 0)
    {
      memset(tChalResp+tCurSize, 0, tPad);
      tCurSize += tPad;
    }

    char *tTmp = encode_base64((unsigned char *)tChalResp, tCurSize);
    slog(LOG_DEBUG, "Full sig: %s\n", tTmp);
    free(tTmp);

    // RSA Encrypt
    RSA *rsa = loadKey();  // Free RSA
    int tSize = RSA_size(rsa);
    unsigned char tTo[tSize];
    RSA_private_encrypt(tCurSize, (unsigned char *)tChalResp, tTo, rsa, RSA_PKCS1_PADDING);
    
    // Wrap RSA Encrypted binary in Base64 encoding
    tResponse = encode_base64(tTo, tSize);
    int tLen = strlen(tResponse);
    while(tLen > 1 && tResponse[tLen-1] == '=')
    {
      tResponse[tLen-1] = '\0';
    }
    free(tChallenge);
    RSA_free(rsa);
  }

  if(tResponse != NULL)
  {
    // Append to current response
    addToShairBuffer(&(pConn->resp), "Apple-Response: ");
    addToShairBuffer(&(pConn->resp), tResponse);
    addToShairBuffer(&(pConn->resp), "\r\n");
    free(tResponse);
    return TRUE;
  }
  return FALSE;
}

//parseMessage(tConn->recv.data, tConn->recv.mark, &tConn->resp, ipstr, pHWADDR, tConn->keys);
static int parseMessage(struct connection *pConn, unsigned char *pIpBin, unsigned int pIpBinLen, char *pHWID)
{
  int tReturn = 0; // 0 = good, 1 = Needs More Data, -1 = close client socket.
  if(pConn->resp.data == NULL)
  {
    initBuffer(&(pConn->resp), MAX_SIZE);
  }

  char *tContent = getFromHeader(pConn->recv.data, "Content-Length", NULL);
  if(tContent != NULL)
  {
    int tContentSize = atoi(tContent);
    if(pConn->recv.marker == 0 || strlen(pConn->recv.data+pConn->recv.marker) != tContentSize)
    {
      if(kCurrentLogLevel==LOG_DEBUG)
      {
        slog(LOG_DEBUG, "Content-Length: %s value -> %d\n", tContent, tContentSize);
        if(pConn->recv.marker != 0)
        {
          slog(LOG_DEBUG, "ContentPtr has %d, but needs %d\n", 
                  strlen(pConn->recv.data+pConn->recv.marker), tContentSize);
        }
      }
      // check if value in tContent > 2nd read from client.
      return 1; // means more content-length needed
    }
  }
  else
  {
    slog(LOG_DEBUG, "No content, header only\n");
  }

  // "Creates" a new Response Header for our response message
  addToShairBuffer(&(pConn->resp), "RTSP/1.0 200 OK\r\n");

  if(kCurrentLogLevel==LOG_INFO)
  {
    int tLen = strchr(pConn->recv.data, ' ') - pConn->recv.data;
    if(tLen < 0 || tLen > 20)
    {
      tLen = 20;
    }
    slog(LOG_INFO, "********** RECV %.*s **********\n", tLen, pConn->recv.data);
  }

  if(pConn->password != NULL)
  {
    
  }

  if(buildAppleResponse(pConn, pIpBin, pIpBinLen, pHWID)) // need to free sig
  {
    slog(LOG_DEBUG, "Added AppleResponse to Apple-Challenge request\n");
  }

  // Find option, then based on option, do different actions.
  if(strncmp(pConn->recv.data, "OPTIONS", 7) == 0)
  {
    propogateCSeq(pConn);
    addToShairBuffer(&(pConn->resp),
      "Public: ANNOUNCE, SETUP, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, GET_PARAMETER, SET_PARAMETER\r\n");
  }
  else if(!strncmp(pConn->recv.data, "ANNOUNCE", 8))
  {
    char *tContent = pConn->recv.data + pConn->recv.marker;
    int tSize = 0;
    char *tHeaderVal = getFromContent(tContent, "a=aesiv", &tSize); // Not allocated memory, just pointing
    char command[40],audiod[5],pre_shairpid[5];
    FILE *fp;

    sprintf(audiod,"0");
    sprintf(pre_shairpid,"0");
       
    fp = fopen("/var/run/audiod.pid", "r"); 
    if(fp != NULL){
	 fgets(audiod,5,fp);
	 fclose(fp);
    }
    else 
 	 goto pass;

    fp = fopen("/var/run/shairport_fork.pid", "r"); 
    if(fp != NULL){
	fgets(pre_shairpid,5,fp);
	fclose(fp);
    }
    else
	goto pass;

    if(atoi(pre_shairpid)!=0)
    {

	//fprintf(stderr,"now pid=%d,kill pid=%d\n",getpid(),atoi(pre_shairpid));
	memset(command,0,sizeof(command));
	sprintf(command,"kill -SIGALRM %s",audiod);
	system(command);
	sleep(2);
    }

pass:		   	

    slog(LOG_INFO, "kill asplayer & m3player!!!!!\n");
    unlink("/tmp/iradio");
    system("killall -9 asplayer");
    system("killall m3player");

    if(tSize > 0)
    {
      int tKeySize = 0;
      char tEncodedAesIV[tSize + 2];
      getTrimmed(tHeaderVal, tSize, TRUE, TRUE, tEncodedAesIV);
      slog(LOG_DEBUG, "AESIV: [%.*s] Size: %d  Strlen: %d\n", tSize, tEncodedAesIV, tSize, strlen(tEncodedAesIV));
      char *tDecodedIV =  decode_base64((unsigned char*) tEncodedAesIV, tSize, &tSize);

      // grab the key, copy it out of the receive buffer
      tHeaderVal = getFromContent(tContent, "a=rsaaeskey", &tKeySize);
      char tEncodedAesKey[tKeySize + 2]; // +1 for nl, +1 for \0
      getTrimmed(tHeaderVal, tKeySize, TRUE, TRUE, tEncodedAesKey);
      slog(LOG_DEBUG, "AES KEY: [%s] Size: %d  Strlen: %d\n", tEncodedAesKey, tKeySize, strlen(tEncodedAesKey));
      // remove base64 coding from key
      char *tDecodedAesKey = decode_base64((unsigned char*) tEncodedAesKey,
                              tKeySize, &tKeySize);  // Need to free DecodedAesKey

      // Grab the formats
      int tFmtpSize = 0;
      char *tFmtp = getFromContent(tContent, "a=fmtp", &tFmtpSize);  // Don't need to free
      tFmtp = getTrimmedMalloc(tFmtp, tFmtpSize, TRUE, FALSE); // will need to free
      slog(LOG_DEBUG, "Format: %s\n", tFmtp);

      RSA *rsa = loadKey();
      // Decrypt the binary aes key
      char *tDecryptedKey = malloc(RSA_size(rsa) * sizeof(char)); // Need to Free Decrypted key
      //char tDecryptedKey[RSA_size(rsa)];
      if(RSA_private_decrypt(tKeySize, (unsigned char *)tDecodedAesKey, 
      (unsigned char*) tDecryptedKey, rsa, RSA_PKCS1_OAEP_PADDING) >= 0)
      {
        slog(LOG_DEBUG, "Decrypted AES key from RSA Successfully\n");
      }
      else
      {
        slog(LOG_INFO, "Error Decrypting AES key from RSA\n");
      }
      free(tDecodedAesKey);
      RSA_free(rsa);

      setKeys(pConn->keys, tDecodedIV, tDecryptedKey, tFmtp);

      propogateCSeq(pConn);
#ifdef LED_INDICATE_AUDIO_PLAY
      nvram_set("audio_playing", "1");
#endif
    }
  }
  else if(!strncmp(pConn->recv.data, "SETUP", 5))
  {
    // Setup pipes
    struct comms *tComms = pConn->hairtunes;
    if (! (pipe(tComms->in) == 0 && pipe(tComms->out) == 0))
    {
      slog(LOG_INFO, "Error setting up hairtunes communications...some things probably wont work very well.\n");
    }
    
    // Setup fork
    char tPort[8] = "6000";  // get this from dup()'d stdout of child pid
    
    FILE *fp1;
    system("rm -rf /var/run/shairport_fork.pid");
    fp1= fopen("/var/run/shairport_fork.pid", "w");
    if(fp1 != NULL){
  	fprintf(fp1, "%d", getpid());
        fclose(fp1);
    }

    int tPid = fork();
    if(tPid == 0)
    {
      int tDataport=0;
      char tCPortStr[8] = "59010";
      char tTPortStr[8] = "59012";
      int tSize = 0;

      char *tFound  =getFromSetup(pConn->recv.data, "control_port", &tSize);
      getTrimmed(tFound, tSize, 1, 0, tCPortStr);
      tFound = getFromSetup(pConn->recv.data, "timing_port", &tSize);
      getTrimmed(tFound, tSize, 1, 0, tTPortStr);

      slog(LOG_DEBUG, "converting %s and %s from str->int\n", tCPortStr, tTPortStr);
      int tControlport = atoi(tCPortStr);
      int tTimingport = atoi(tTPortStr);

      slog(LOG_DEBUG, "Got %d for CPort and %d for TPort\n", tControlport, tTimingport);
      char *tRtp = NULL;
      char *tPipe = NULL;
      char *tAoDriver = NULL;
      char *tAoDeviceName = NULL;
      char *tAoDeviceId = NULL;

      // *************************************************
      // ** Setting up Pipes, AKA no more debug/output  **
      // *************************************************
      dup2(tComms->in[0],0);   // Input to child
      closePipe(&(tComms->in[0]));
      closePipe(&(tComms->in[1]));

      dup2(tComms->out[1], 1); // Output from child
      closePipe(&(tComms->out[1]));
      closePipe(&(tComms->out[0]));

      struct keyring *tKeys = pConn->keys;
      pConn->keys = NULL;
      pConn->hairtunes = NULL;

      // Free up any recv buffers, etc..
      if(pConn->clientSocket != -1)
      {
        close(pConn->clientSocket);
        pConn->clientSocket = -1;
      }
      cleanupBuffers(pConn);
      hairtunes_init(tKeys->aeskey, tKeys->aesiv, tKeys->fmt, tControlport, tTimingport,
                      tDataport, tRtp, tPipe, tAoDriver, tAoDeviceName, tAoDeviceId,
                      bufferStartFill);

      // Quit when finished.
      slog(LOG_DEBUG, "Returned from hairtunes init....returning -1, should close out this whole side of the fork\n");
      return -1;
    }
    else if(tPid >0)
    {
      // Ensure Connection has access to the pipe.
      closePipe(&(tComms->in[0]));
      closePipe(&(tComms->out[1]));

      char tFromHairtunes[80];
      int tRead = read(tComms->out[0], tFromHairtunes, 80);
      if(tRead <= 0)
      {
        slog(LOG_INFO, "Error reading port from hairtunes function, assuming default port: %d\n", tPort);
      }
      else
      {
        int tSize = 0;
        char *tPortStr = getFromHeader(tFromHairtunes, "port", &tSize);
        if(tPortStr != NULL)
        {
          getTrimmed(tPortStr, tSize, TRUE, FALSE, tPort);
        }
        else
        {
          slog(LOG_INFO, "Read %d bytes, Error translating %s into a port\n", tRead, tFromHairtunes);
        }
      }
      //  READ Ports from here?close(pConn->hairtunes_pipes[0]);
      propogateCSeq(pConn);
      int tSize = 0;
      char *tTransport = getFromHeader(pConn->recv.data, "Transport", &tSize);
      addToShairBuffer(&(pConn->resp), "Transport: ");
      addNToShairBuffer(&(pConn->resp), tTransport, tSize);
      // Append server port:
      addToShairBuffer(&(pConn->resp), ";server_port=");
      addToShairBuffer(&(pConn->resp), tPort);
      addToShairBuffer(&(pConn->resp), "\r\nSession: DEADBEEF\r\n");
    }
    else
    {
      slog(LOG_INFO, "Error forking process....dere' be errors round here.\n");
      return -1;
    }
  }
  else if(!strncmp(pConn->recv.data, "TEARDOWN", 8))
  {
    // Be smart?  Do more finish up stuff...
    addToShairBuffer(&(pConn->resp), "Connection: close\r\n");
    propogateCSeq(pConn);
    close(pConn->hairtunes->in[1]);
    slog(LOG_DEBUG, "Tearing down connection, closing pipes\n");
    //close(pConn->hairtunes->out[0]);
    tReturn = -1;  // Close client socket, but sends an ACK/OK packet first
#ifdef LED_INDICATE_AUDIO_PLAY
    nvram_set("audio_playing", "0");
#endif
  }
  else if(!strncmp(pConn->recv.data, "FLUSH", 5))
  {
    write(pConn->hairtunes->in[1], "flush\n", 6);
    propogateCSeq(pConn);
  }
  else if(!strncmp(pConn->recv.data, "SET_PARAMETER", 13))
  {
    propogateCSeq(pConn);
    int tSize = 0;
    char *tVol = getFromHeader(pConn->recv.data, "volume", &tSize);
    slog(LOG_DEBUG, "About to write [vol: %.*s] data to hairtunes\n", tSize, tVol);
#if 0
    write(pConn->hairtunes->in[1], "vol: ", 5);
    write(pConn->hairtunes->in[1], tVol, tSize);
    write(pConn->hairtunes->in[1], "\n", 1);
#else
    int volume_raw, volume;
#if defined(I2C_TO_ALC5627)
    int volume_max = 31;
    int numid = 1;
    int fd;
#elif defined(USB_TO_CM6510)
    int volume_max = 62;
    int numid = 4;
#elif defined(XXXXX)
//TBD.
    int volume_max = 62;
    int numid = 1;
#endif

    /* volume: -144~0 (ignore -144~-30) convert to 0~30, and round off */
    volume_raw = atof(tVol) + 30.5;
    if (volume_raw < 0)
	volume_raw = 0;
    /* volume: 0~30 convert to 0~volume_max, and round off */
    volume = (volume_raw * volume_max + 15) / 30;
    doSystem("amixer -q cset numid=%d %d", numid, volume);
    if (nvram_match("audio_mute", "1")) {
#if defined(I2C_TO_ALC5627)
	if ((fd = open_mixer_fd()) >= 0) {
		ioctl(fd, RT2880_I2C_SET_DAC_UNMUTE, 0);
		close(fd);
	}
#elif defined(USB_TO_CM6510)
	doSystem("amixer -q cset numid=3 1");
#elif defined(XXXXX)
//TBD.
#endif
	nvram_set("audio_mute", "0");
    }
    nvram_set("shair_vol","1");
#endif    
    slog(LOG_DEBUG, "Finished writing data write data to hairtunes\n");
  }
  else
  {
    slog(LOG_DEBUG, "\n\nUn-Handled recv: %s\n", pConn->recv.data);
    propogateCSeq(pConn);
  }
  addToShairBuffer(&(pConn->resp), "\r\n");
  return tReturn;
}

// Copies CSeq value from request, and adds standard header values in.
static void propogateCSeq(struct connection *pConn) //char *pRecvBuffer, struct shairbuffer *pConn->recp.data)
{
  int tSize=0;
  char *tRecPtr = getFromHeader(pConn->recv.data, "CSeq", &tSize);
  addToShairBuffer(&(pConn->resp), "Audio-Jack-Status: connected; type=analog\r\n");
  addToShairBuffer(&(pConn->resp), "CSeq: ");
  addNToShairBuffer(&(pConn->resp), tRecPtr, tSize);
  addToShairBuffer(&(pConn->resp), "\r\n");
}

void cleanupBuffers(struct connection *pConn)
{
  if(pConn->recv.data != NULL)
  {
    free(pConn->recv.data);
    pConn->recv.data = NULL;
  }
  if(pConn->resp.data != NULL)
  {
    free(pConn->resp.data);
    pConn->resp.data = NULL;
  }
}

static void cleanup(struct connection *pConn)
{
  cleanupBuffers(pConn);
  if(pConn->hairtunes != NULL)
  {

    closePipe(&(pConn->hairtunes->in[0]));
    closePipe(&(pConn->hairtunes->in[1]));
    closePipe(&(pConn->hairtunes->out[0]));
    closePipe(&(pConn->hairtunes->out[1]));
  }
  if(pConn->keys != NULL)
  {
    if(pConn->keys->aesiv != NULL)
    {
      free(pConn->keys->aesiv);
    }
    if(pConn->keys->aeskey != NULL)
    {
      free(pConn->keys->aeskey);
    }
    if(pConn->keys->fmt != NULL)
    {
      free(pConn->keys->fmt);
    }
    pConn->keys = NULL;
  }
  if(pConn->clientSocket != -1)
  {
    close(pConn->clientSocket);
    pConn->clientSocket = -1;
  }
}

static int startAvahi(const char *pHWStr, const char *pServerName, int pPort)
{
  int tMaxServerName = 25; // Something reasonable?  iPad showed 21, iphone 25
  int tPid = fork();
  if(tPid == 0)
  {
    char tName[100 + HWID_SIZE + 3];
    if(strlen(pServerName) > tMaxServerName)
    {
      slog(LOG_INFO," we see you like long server names, "
              "so we put a strncat in our command so we don't buffer overflow, while you listen to your flow.\n"
              "We just used the first %d characters.  Pick something shorter if you want\n", tMaxServerName);
    }
    
    tName[0] = '\0';
    char tPort[SERVLEN];
    sprintf(tPort, "%d", pPort);
    strcat(tName, pHWStr);
    strcat(tName, "@");
    strncat(tName, pServerName, tMaxServerName);
    slog(LOG_DEBUG, "mDNSPublish Name: %s\n", tName);
    
    execlp("mDNSPublish", "mDNSPublish", tName,
         "_raop._tcp", tPort, "tp=UDP","sm=false","sv=false","ek=1","et=0,1",
         "cn=0,1","ch=2","ss=16","sr=44100","pw=false","vn=3","txtvers=1", NULL);
    if(errno == -1) {
            perror("error");
    }

    slog(LOG_INFO, "Bad error... couldn't find or failed to run: avahi-publish-service OR dns-sd OR mDNSPublish\n");
    exit(1);
  }
  else
  {
    slog(LOG_DEBUG, "mDNSPublish started on PID: %d\n", tPid);
  }
  return tPid;
}

static int getAvailChars(struct shairbuffer *pBuf)
{
  return (pBuf->maxsize / sizeof(char)) - pBuf->current;
}

static void addToShairBuffer(struct shairbuffer *pBuf, char *pNewBuf)
{
  addNToShairBuffer(pBuf, pNewBuf, strlen(pNewBuf));
}

static void addNToShairBuffer(struct shairbuffer *pBuf, char *pNewBuf, int pNofNewBuf)
{
  int tAvailChars = getAvailChars(pBuf);
  if(pNofNewBuf > tAvailChars)
  {
    int tNewSize = pBuf->maxsize * 2 + MAX_SIZE + sizeof(char);
    char *tTmpBuf = malloc(tNewSize);

    tTmpBuf[0] = '\0';
    memset(tTmpBuf, 0, tNewSize/sizeof(char));
    memcpy(tTmpBuf, pBuf->data, pBuf->current);
    free(pBuf->data);

    pBuf->maxsize = tNewSize;
    pBuf->data = tTmpBuf;
  }
  memcpy(pBuf->data + pBuf->current, pNewBuf, pNofNewBuf);
  pBuf->current += pNofNewBuf;
  if(getAvailChars(pBuf) > 1)
  {
    pBuf->data[pBuf->current] = '\0';
  }
}

static char *getTrimmedMalloc(char *pChar, int pSize, int pEndStr, int pAddNL)
{
  int tAdditionalSize = 0;
  if(pEndStr)
    tAdditionalSize++;
  if(pAddNL)
    tAdditionalSize++;
  char *tTrimDest = malloc(sizeof(char) * (pSize + tAdditionalSize));
  return getTrimmed(pChar, pSize, pEndStr, pAddNL, tTrimDest);
}

// Must free returned ptr
static char *getTrimmed(char *pChar, int pSize, int pEndStr, int pAddNL, char *pTrimDest)
{
  int tSize = pSize;
  if(pEndStr)
  {
    tSize++;
  }
  if(pAddNL)
  {
    tSize++;
  }
  
  memset(pTrimDest, 0, tSize);
  memcpy(pTrimDest, pChar, pSize);
  if(pAddNL)
  {
    pTrimDest[pSize] = '\n';
  }
  if(pEndStr)
  {
    pTrimDest[tSize-1] = '\0';
  }
  return pTrimDest;
}

static void slog(int pLevel, char *pFormat, ...)
{
  if(pLevel<=kCurrentLogLevel)
  {
    va_list argp;
    va_start(argp, pFormat);
    vprintf(pFormat, argp);
    va_end(argp);
  }
}

static void initConnection(struct connection *pConn, struct keyring *pKeys,
                    struct comms *pComms, int pSocket, char *pPassword)
{
  pConn->hairtunes = pComms;
  if(pKeys != NULL)
  {
    pConn->keys = pKeys;
    pConn->keys->aesiv = NULL;
    pConn->keys->aeskey = NULL;
    pConn->keys->fmt = NULL;
  }
  pConn->recv.data = NULL;  // Pre-init buffer expected to be NULL
  pConn->resp.data = NULL;  // Pre-init buffer expected to be NULL
  pConn->clientSocket = pSocket;
  if(strlen(pPassword) >0)
  {
    pConn->password = pPassword;
  }
  else
  {
    pConn->password = NULL;
  }
}

static void closePipe(int *pPipe)
{
  if(*pPipe != -1)
  {
    close(*pPipe);
    *pPipe = -1;
  }
}

static void initBuffer(struct shairbuffer *pBuf, int pNumChars)
{
  if(pBuf->data != NULL)
  {
    slog(LOG_DEBUG, "Hrm, buffer wasn't cleaned up....trying to free\n");
    free(pBuf->data);
    slog(LOG_DEBUG, "Free didn't seem to seg fault....huzzah\n");
  }
  pBuf->current = 0;
  pBuf->marker = 0;
  pBuf->maxsize = sizeof(char) * pNumChars;
  pBuf->data = malloc(pBuf->maxsize);
  memset(pBuf->data, 0, pBuf->maxsize);
}

static void setKeys(struct keyring *pKeys, char *pIV, char* pAESKey, char *pFmtp)
{
  if(pKeys->aesiv != NULL)
  {
    free(pKeys->aesiv);
  }
  if(pKeys->aeskey != NULL)
  {
    free(pKeys->aeskey);
  }
  if(pKeys->fmt != NULL)
  {
    free(pKeys->fmt);
  }
  pKeys->aesiv = pIV;
  pKeys->aeskey = pAESKey;
  pKeys->fmt = pFmtp;
}

#define AIRPORT_PRIVATE_KEY \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpQIBAAKCAQEA59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUt\n" \
"wC5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDRKSKv6kDqnw4U\n" \
"wPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuBOitnZ/bDzPHrTOZz0Dew0uowxf\n" \
"/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJQ+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/\n" \
"UAaHqn9JdsBWLUEpVviYnhimNVvYFZeCXg/IdTQ+x4IRdiXNv5hEewIDAQABAoIBAQDl8Axy9XfW\n" \
"BLmkzkEiqoSwF0PsmVrPzH9KsnwLGH+QZlvjWd8SWYGN7u1507HvhF5N3drJoVU3O14nDY4TFQAa\n" \
"LlJ9VM35AApXaLyY1ERrN7u9ALKd2LUwYhM7Km539O4yUFYikE2nIPscEsA5ltpxOgUGCY7b7ez5\n" \
"NtD6nL1ZKauw7aNXmVAvmJTcuPxWmoktF3gDJKK2wxZuNGcJE0uFQEG4Z3BrWP7yoNuSK3dii2jm\n" \
"lpPHr0O/KnPQtzI3eguhe0TwUem/eYSdyzMyVx/YpwkzwtYL3sR5k0o9rKQLtvLzfAqdBxBurciz\n" \
"aaA/L0HIgAmOit1GJA2saMxTVPNhAoGBAPfgv1oeZxgxmotiCcMXFEQEWflzhWYTsXrhUIuz5jFu\n" \
"a39GLS99ZEErhLdrwj8rDDViRVJ5skOp9zFvlYAHs0xh92ji1E7V/ysnKBfsMrPkk5KSKPrnjndM\n" \
"oPdevWnVkgJ5jxFuNgxkOLMuG9i53B4yMvDTCRiIPMQ++N2iLDaRAoGBAO9v//mU8eVkQaoANf0Z\n" \
"oMjW8CN4xwWA2cSEIHkd9AfFkftuv8oyLDCG3ZAf0vrhrrtkrfa7ef+AUb69DNggq4mHQAYBp7L+\n" \
"k5DKzJrKuO0r+R0YbY9pZD1+/g9dVt91d6LQNepUE/yY2PP5CNoFmjedpLHMOPFdVgqDzDFxU8hL\n" \
"AoGBANDrr7xAJbqBjHVwIzQ4To9pb4BNeqDndk5Qe7fT3+/H1njGaC0/rXE0Qb7q5ySgnsCb3DvA\n" \
"cJyRM9SJ7OKlGt0FMSdJD5KG0XPIpAVNwgpXXH5MDJg09KHeh0kXo+QA6viFBi21y340NonnEfdf\n" \
"54PX4ZGS/Xac1UK+pLkBB+zRAoGAf0AY3H3qKS2lMEI4bzEFoHeK3G895pDaK3TFBVmD7fV0Zhov\n" \
"17fegFPMwOII8MisYm9ZfT2Z0s5Ro3s5rkt+nvLAdfC/PYPKzTLalpGSwomSNYJcB9HNMlmhkGzc\n" \
"1JnLYT4iyUyx6pcZBmCd8bD0iwY/FzcgNDaUmbX9+XDvRA0CgYEAkE7pIPlE71qvfJQgoA9em0gI\n" \
"LAuE4Pu13aKiJnfft7hIjbK+5kyb3TysZvoyDnb3HOKvInK7vXbKuU4ISgxB2bB3HcYzQMGsz1qJ\n" \
"2gG0N5hvJpzwwhbhXqFKA4zaaSrw622wDniAK5MlIE0tIAKKP4yxNGjoD2QYjhBGuhvkWKY=\n" \
"-----END RSA PRIVATE KEY-----"

static RSA *loadKey(void)
{
  BIO *tBio = BIO_new_mem_buf(AIRPORT_PRIVATE_KEY, -1);
  RSA *rsa = PEM_read_bio_RSAPrivateKey(tBio, NULL, NULL, NULL); //NULL, NULL, NULL);
  BIO_free(tBio);
  slog(LOG_DEBUG, "RSA Key: %d\n", RSA_check_key(rsa));
  return rsa;
}
