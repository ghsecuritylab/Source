#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <upnp.h>
#include "globals.h"
#include "config.h"
#include "pmlist.h"
#include "gatedevice.h"
#include "util.h"
#include <nvram/bcmnvram.h>

#if HAVE_LIBIPTC
#include "iptc.h"
#endif

struct portMap* pmlist_NewNode(int enabled, long int duration, char *remoteHost,
			       char *externalPort, char *internalPort,
			       char *protocol, char *internalClient, char *desc)
{
	struct portMap* temp = (struct portMap*) malloc(sizeof(struct portMap));

	temp->m_PortMappingEnabled = enabled;
	
	if (remoteHost && strlen(remoteHost) < sizeof(temp->m_RemoteHost)) strcpy(temp->m_RemoteHost, remoteHost);
		else strcpy(temp->m_RemoteHost, "");
	if (strlen(externalPort) < sizeof(temp->m_ExternalPort)) strcpy(temp->m_ExternalPort, externalPort);
		else strcpy(temp->m_ExternalPort, "");
	if (strlen(internalPort) < sizeof(temp->m_InternalPort)) strcpy(temp->m_InternalPort, internalPort);
		else strcpy(temp->m_InternalPort, "");
	if (strlen(protocol) < sizeof(temp->m_PortMappingProtocol)) strcpy(temp->m_PortMappingProtocol, protocol);
		else strcpy(temp->m_PortMappingProtocol, "");
	if (strlen(internalClient) < sizeof(temp->m_InternalClient)) strcpy(temp->m_InternalClient, internalClient);
		else strcpy(temp->m_InternalClient, "");
	if (strlen(desc) < sizeof(temp->m_PortMappingDescription)) strcpy(temp->m_PortMappingDescription, desc);
		else strcpy(temp->m_PortMappingDescription, "");
	temp->m_PortMappingLeaseDuration = duration;

	temp->next = NULL;
	temp->prev = NULL;
	
	return temp;
}
	
struct portMap* pmlist_Find(char *externalPort, char *proto, char *internalClient)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	
	do 
	{
		if ( (strcmp(temp->m_ExternalPort, externalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, proto) == 0) &&
				(strcmp(temp->m_InternalClient, internalClient) == 0) )
			return temp; // We found a match, return pointer to it
		else
			temp = temp->next;
	} while (temp != NULL);
	
	// If we made it here, we didn't find it, so return NULL
	return NULL;
}

struct portMap* pmlist_FindByIndex(int index)
{
	int i=0;
	struct portMap* temp;

	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	do
	{
		if (i == index)
			return temp;
		else
		{
			temp = temp->next;	
			i++;
		}
	} while (temp != NULL);

	return NULL;
}	

struct portMap* pmlist_FindSpecific(char *externalPort, char *protocol)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	
	do
	{
		if ( (strcmp(temp->m_ExternalPort, externalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, protocol) == 0))
			return temp;
		else
			temp = temp->next;
	} while (temp != NULL);

	return NULL;
}

int pmlist_IsEmtpy(void)
{
	if (pmlist_Head)
		return 0;
	else
		return 1;
}

int pmlist_Size(void)
{
	struct portMap* temp;
	int size = 0;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return 0;
	
	while (temp->next)
	{
		size++;
		temp = temp->next;
	}
	size++;
	return size;
}	

int pmlist_FreeList(void)
{
  struct portMap *temp, *next;

  temp = pmlist_Head;
  while(temp) {
    CancelMappingExpiration(temp->expirationEventId);
    pmlist_DeletePortMapping(temp->m_PortMappingEnabled, temp->m_PortMappingProtocol, temp->m_ExternalPort,
			     temp->m_InternalClient, temp->m_InternalPort);
    next = temp->next;
    free(temp);
    temp = next;
  }
  pmlist_Head = pmlist_Tail = NULL;
  return 1;
}
		
int pmlist_PushBack(struct portMap* item)
{
	int action_succeeded = 0;

	if (pmlist_Tail) // We have a list, place on the end
	{
		pmlist_Tail->next = item;
		item->prev = pmlist_Tail;
		item->next = NULL;
		pmlist_Tail = item;
		action_succeeded = 1;
	}
	else // We obviously have no list, because we have no tail :D
	{
		pmlist_Head = pmlist_Tail = pmlist_Current = item;
		item->prev = NULL;
		item->next = NULL;
 		action_succeeded = 1;
		trace(3, "appended %d %s %s %s %s %ld", item->m_PortMappingEnabled, 
				    item->m_PortMappingProtocol, item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort,
				    item->m_PortMappingLeaseDuration);
	}
	if (action_succeeded == 1)
	{
		pmlist_AddPortMapping(item->m_PortMappingEnabled, item->m_PortMappingProtocol,
				      item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort);
		return 1;
	}
	else
		return 0;
}

		
int pmlist_Delete(struct portMap* item)
{
	struct portMap *temp;
	int action_succeeded = 0;

	temp = pmlist_Find(item->m_ExternalPort, item->m_PortMappingProtocol, item->m_InternalClient);
	if (temp) // We found the item to delete
	{
	  CancelMappingExpiration(temp->expirationEventId);
		pmlist_DeletePortMapping(item->m_PortMappingEnabled, item->m_PortMappingProtocol, item->m_ExternalPort, 
				item->m_InternalClient, item->m_InternalPort);
		if (temp == pmlist_Head) // We are the head of the list
		{
			if (temp->next == NULL) // We're the only node in the list
			{
				pmlist_Head = pmlist_Tail = pmlist_Current = NULL;
				free (temp);
				action_succeeded = 1;
			}
			else // we have a next, so change head to point to it
			{
				pmlist_Head = temp->next;
				pmlist_Head->prev = NULL;
				free (temp);
				action_succeeded = 1;	
			}
		}
		else if (temp == pmlist_Tail) // We are the Tail, but not the Head so we have prev
		{
			pmlist_Tail = pmlist_Tail->prev;
			free (pmlist_Tail->next);
			pmlist_Tail->next = NULL;
			action_succeeded = 1;
		}
		else // We exist and we are between two nodes
		{
			temp->prev->next = temp->next;
			temp->next->prev = temp->prev;
			pmlist_Current = temp->next; // We put current to the right after a extraction
			free (temp);	
			action_succeeded = 1;
		}
	}
	else  // We're deleting something that's not there, so return 0
		action_succeeded = 0;

	return action_succeeded;
}

#define T_NAT		1
#define T_FORWARD	2
#define NAT_FILE "/tmp/nat_rules"
#define REDIRECT_FILE "/tmp/redirect_rules"
#define FORWARD_FILE "/tmp/filter_rules"
char *restore_nat_cmd[] = {"iptables-restore", NAT_FILE, NULL};
char *restore_forward_cmd[] = {"iptables-restore", FORWARD_FILE, NULL};

int wait_nat_rewriting()
{
        int chk_counts = 0;

        while(strcmp(nvram_safe_get("nat_rewriting"), "1") == 0)
        {
                ++chk_counts;
                sleep(1);
		printf("wait for nat rewriting...(%d)\n", chk_counts);	// tmp test
                if(chk_counts > 10)
                {
                        printf("nat rewriting too long\n");
                        break;
                }
        }
	if(chk_counts <= 10)
	{
		nvram_set("nat_rewriting", "1");
		return 1;
	}
	return -1;
}

void post_nat_rewriting()
{
	nvram_set("nat_rewriting", "0");
}

void add_to_file(FILE *fp, char *str)
{
	char tmp_buf[512];
	int counts = 0;

        memset(tmp_buf, 0, sizeof(tmp_buf));
        while(((fgets(tmp_buf, sizeof(tmp_buf), fp)) != NULL)
        && strncmp(tmp_buf, "COMMIT", 6) != 0
        ){
	        counts+=strlen(tmp_buf);
                memset(tmp_buf, 0, sizeof(tmp_buf));
        }

        rewind(fp);
        fseek(fp, counts, SEEK_SET);
        fwrite(str, strlen(str), 1, fp);
        fwrite("COMMIT\n", 7, 1, fp);
}

void del_from_file(const char *filename, FILE *fp, char *str)
{
        FILE *fp_tmp = fopen("/tmp/tmpnat", "w+");
	char tmp_buf[512];

        memset(tmp_buf, 0, sizeof(tmp_buf));
        while(((fgets(tmp_buf, sizeof(tmp_buf), fp)) != NULL)
        ){
        	if(strncmp(tmp_buf, str, strlen(str)) != 0)
                	fwrite(tmp_buf, strlen(tmp_buf), 1, fp_tmp);
                memset(tmp_buf, 0, sizeof(tmp_buf));
        }

        fclose(fp);
        fclose(fp_tmp);
        unlink(filename);
        rename("/tmp/tmpnat", filename);
}

void add_rule_by_restore(char *rule, int type)
{
        FILE *nat_fp;
        FILE *redirect_fp;
        FILE *forward_fp;

	//if(wait_nat_rewriting() < 0)
	//	return;

	if(type == T_NAT)
	{
        	nat_fp = fopen(NAT_FILE, "r+");
        	if(nat_fp == NULL){
                	fprintf(stderr, "*** Can't make the file of the nat rules! ***\n");
                	return;
        	}
		add_to_file(nat_fp, rule);
        	fclose(nat_fp);

        	redirect_fp = fopen(REDIRECT_FILE, "r+");
        	if(redirect_fp == NULL){
                	fprintf(stderr, "*** Can't make the file of the redirect rules! ***\n");
                	return;
        	}

		add_to_file(redirect_fp, rule);
        	fclose(redirect_fp);
	}
	else if(type == T_FORWARD)
	{
                forward_fp = fopen(FORWARD_FILE, "r+");
                if(forward_fp == NULL){
                        fprintf(stderr, "*** Can't make the file of the forward rules! ***\n");
                        return;
                }
                add_to_file(forward_fp, rule);
                fclose(forward_fp);
	}
	else
		printf("invalid iptables type to add\n");
	
	//post_nat_rewriting();
}

void del_rule_by_restore(char *rule, int type)
{
        FILE *nat_fp;
        FILE *redirect_fp;
        FILE *forward_fp;
	int chk_counts = 0;

	//if(wait_nat_rewriting() < 0)
	//	return;

	if(type == T_NAT)
	{
        	nat_fp = fopen(NAT_FILE, "r+");
        	if(nat_fp == NULL){
                	fprintf(stderr, "*** Can't make the file of the nat rules! ***\n");
                	return;
        	}
        	del_from_file(NAT_FILE, nat_fp, rule);

        	redirect_fp = fopen(REDIRECT_FILE, "r+");
        	if(redirect_fp == NULL){
                	fprintf(stderr, "*** Can't make the file of the redirect rules! ***\n");
                	return;
        	}
        	del_from_file(REDIRECT_FILE, redirect_fp, rule);
	}
	else if(type == T_FORWARD)
	{
                forward_fp = fopen(FORWARD_FILE, "r+");
                if(forward_fp == NULL){
                        fprintf(stderr, "*** Can't make the file of the forward rules! ***\n");
                        return;
                }
                del_from_file(FORWARD_FILE, forward_fp, rule);
	}
	else
		printf("invalid iptables type to delete\n");

	//post_nat_rewriting();
}

int pmlist_AddPortMapping (int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
    if (enabled)
    {
#if HAVE_LIBIPTC
	char *buffer = malloc(strlen(internalClient) + strlen(internalPort) + 2);
	if (buffer == NULL) {
		fprintf(stderr, "failed to malloc memory\n");
		return 0;
	}

	strcpy(buffer, internalClient);
	strcat(buffer, ":");
	strcat(buffer, internalPort);

	if (g_vars.forwardRules)
		iptc_add_rule("filter", g_vars.forwardChainName, protocol, NULL, NULL, NULL, internalClient, NULL, internalPort, "ACCEPT", NULL, FALSE);

	iptc_add_rule("nat", g_vars.preroutingChainName, protocol, g_vars.extInterfaceName, NULL, NULL, NULL, NULL, externalPort, "DNAT", buffer, TRUE);
	free(buffer);
#else
	char command[COMMAND_LEN];
	int status;
	
#ifdef USE_IPTABLES
	{
	  char dest[DEST_LEN];
	  char *args[] = {"iptables", "-t", "nat", "-I", g_vars.preroutingChainName, "-i", g_vars.extInterfaceName, "-p", protocol, "--dport", externalPort, "-j", "DNAT", "--to", dest, NULL};

	  snprintf(dest, DEST_LEN, "%s:%s", internalClient, internalPort);
	  snprintf(command, COMMAND_LEN, "%s -t nat -I %s -i %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.iptables, g_vars.preroutingChainName, g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);
	  trace(3, "%s", command);
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
	}
#else
	{
		printf("\n## not use iptables ##\n");	// tmp test
	  memset(command, 0, COMMAND_LEN);
	  snprintf(command, COMMAND_LEN, "-I %s -i %s -p %s --dport %s -j DNAT --to %s:%s\n", g_vars.preroutingChainName, g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);

	  add_rule_by_restore(command, T_NAT);
	  _eval(restore_nat_cmd, NULL, 0, NULL);
	}
#endif

	if (g_vars.forwardRules)
#ifdef USE_IPTABLES
	{
	  char *args[] = {"iptables", "-A", g_vars.forwardChainName, "-p", protocol, "-d", internalClient, "--dport", internalPort, "-j", "ACCEPT", NULL};
	  
	  snprintf(command, COMMAND_LEN, "%s -A %s -p %s -d %s --dport %s -j ACCEPT", g_vars.iptables,g_vars.forwardChainName, protocol, internalClient, internalPort);
	  trace(3, "%s", command);
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
	}
#else
	{
		printf("\n##2 not use iptables ##\n");	// tmp test
	  memset(command, 0, COMMAND_LEN);
	  snprintf(command, COMMAND_LEN, "-A %s -p %s -d %s --dport %s -j ACCEPT\n", g_vars.forwardChainName, protocol, internalClient, internalPort);
	  add_rule_by_restore(command, T_FORWARD);
	  _eval(restore_forward_cmd, NULL, 0, NULL);
	}
#endif

#endif
    }
    return 1;
}

int pmlist_DeletePortMapping(int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
    if (enabled)
    {
#if HAVE_LIBIPTC
	char *buffer = malloc(strlen(internalClient) + strlen(internalPort) + 2);
	strcpy(buffer, internalClient);
	strcat(buffer, ":");
	strcat(buffer, internalPort);

	if (g_vars.forwardRules)
	    iptc_delete_rule("filter", g_vars.forwardChainName, protocol, NULL, NULL, NULL, internalClient, NULL, internalPort, "ACCEPT", NULL);

	iptc_delete_rule("nat", g_vars.preroutingChainName, protocol, g_vars.extInterfaceName, NULL, NULL, NULL, NULL, externalPort, "DNAT", buffer);
	free(buffer);
#else
	char command[COMMAND_LEN];
	int status;
	
#ifdef USE_IPTABLES
	{
	  char dest[DEST_LEN];
	  char *args[] = {"iptables", "-t", "nat", "-D", g_vars.preroutingChainName, "-i", g_vars.extInterfaceName, "-p", protocol, "--dport", externalPort, "-j", "DNAT", "--to", dest, NULL};

	  snprintf(dest, DEST_LEN, "%s:%s", internalClient, internalPort);
	  snprintf(command, COMMAND_LEN, "%s -t nat -D %s -i %s -p %s --dport %s -j DNAT --to %s:%s",
		  g_vars.iptables, g_vars.preroutingChainName, g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);
	  trace(3, "%s", command);
	  
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
	}
#else
	{
	  memset(command, 0, COMMAND_LEN);
	  snprintf(command, COMMAND_LEN, "-I %s -i %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.preroutingChainName, g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);
	  del_rule_by_restore(command, T_NAT);
	  _eval(restore_nat_cmd, NULL, 0, NULL);
	}
#endif

	if (g_vars.forwardRules)
#ifdef USE_IPTABLES
	{
	  char *args[] = {"iptables", "-D", g_vars.forwardChainName, "-p", protocol, "-d", internalClient, "--dport", internalPort, "-j", "ACCEPT", NULL};
	  
	  snprintf(command, COMMAND_LEN, "%s -D %s -p %s -d %s --dport %s -j ACCEPT", g_vars.iptables, g_vars.forwardChainName, protocol, internalClient, internalPort);
	  trace(3, "%s", command);
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
	}
#else
	{
	  memset(command, 0, COMMAND_LEN);
	  snprintf(command, COMMAND_LEN, "-A %s -p %s -d %s --dport %s -j ACCEPT", g_vars.forwardChainName, protocol, internalClient, internalPort);
	  del_rule_by_restore(command, T_FORWARD);
	  _eval(restore_forward_cmd, NULL, 0, NULL);
	}
#endif

#endif
    }
    return 1;
}
