/* $Id: computer.c,v 1.25 2001/09/17 10:56:15 jorge Exp $ */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#include "computer.h"
#include "database.h"
#include "logger.h"
#include "semaphores.h"

int computer_index_addr (void *pwdb,struct in_addr addr)
{
  /* This function is called by the master */
  /* This functions resolves the name associated with the ip of the socket and */
  /* and finds that name in the computer list and returns it's index position */
  int index;
  struct hostent *host;
  char *dot;
  int i=0;
  char msg[BUFFERLEN];
  char *name;

  log_master (L_DEBUG,"Entering computer_index_addr");

  if ((host = gethostbyaddr ((const void *)&addr.s_addr,sizeof (struct in_addr),AF_INET)) == NULL) {
    snprintf(msg,BUFFERLEN-1,"Could not resolve name for: %s",inet_ntoa(addr));
    log_master (L_WARNING,msg);
    return -1;
  }

  if ((dot = strchr (host->h_name,'.')) != NULL) 
      *dot = '\0';
  /*      printf ("Name: %s\n",host->h_name); */
  name = host->h_name;
  while (host->h_aliases[i] != NULL) {
    /*        printf ("Alias: %s\n",host->h_aliases[i]); */
    i++;
  }
  while (*host->h_aliases != NULL) {
    /*        printf ("Alias: %s\n",*host->h_aliases); */
    host->h_aliases++;
  }

  semaphore_lock(((struct database *)pwdb)->semid);
  index = computer_index_name (pwdb,name);
  semaphore_release(((struct database *)pwdb)->semid);

  snprintf (msg,BUFFERLEN-1,"Exiting computer_index_addr. Index of computer %s is %i.",name,index);
  log_master (L_DEBUG,msg);

  return index;
}

int computer_index_name (void *pwdb,char *name)
{
  struct database *wdb = (struct database *)pwdb;
  int index = -1;
  int i;
  
  for (i=0;((i<MAXCOMPUTERS)&&(index==-1)); i++) {
    if ((strcmp(name,wdb->computer[i].hwinfo.name) == 0) && (wdb->computer[i].used))
      index = i;
  }

  return index;
}

int computer_index_free (void *pwdb)
{
  /* Return the index to a free computer record OR -1 if there */
  /* are no more free records */
  int index = -1;
  int i;
  struct database *wdb = (struct database *)pwdb;

  for (i=0; i<MAXCOMPUTERS; i++) {
    if (wdb->computer[i].used == 0) {
      index = i;
      break;
    }
  }

  return index;
}

int computer_available (struct computer *computer)
{
  int npt;			/* number of possible tasks */
  int t;
  /* At the beginning npt is the minimum of the nmaxcpus or ncpus */
  /* This means that never will be assigned more tasks than processors */
  /* This behaviour could be changed in the future */
  npt = (computer->limits.nmaxcpus < computer->hwinfo.ncpus) ? computer->limits.nmaxcpus : computer->hwinfo.ncpus;
/*    printf ("1) npt: %i\n",npt); */

  /* then npt is the minimum of npt or the number of free tasks structures */
  npt = (npt < MAXTASKS) ? npt : MAXTASKS;
/*    printf ("2) npt: %i\n",npt); */

  /* Prevent floating point exception */
  if (!computer->limits.maxfreeloadcpu)
    return 0;
  /* Care must be taken because we substract the running tasks TWO times */
  /* one because of the load, another one here. */
  /* SOLUTION: we substract maxfreeloadcpu from the load */
  /* CONS: At the beggining of a frame it is not represented on the load average */
  /*       If we substract then we are probably substracting from another task's load */
  /* Number of cpus charged based on the load */
  t = (computer->status.loadavg[0] / computer->limits.maxfreeloadcpu) - computer->status.ntasks;
  t = ( t < 0 ) ? 0 : t;
  npt -= t;
/*    printf ("3) npt: %i\n",npt); */

  /* Number of current working tasks */
  npt -= computer->status.ntasks;
/*    printf ("4) npt: %i\n",npt); */

  if (computer->status.ntasks > MAXTASKS) {
    /* This should never happen, btw */
    fprintf (stderr,"CRITICAL ERROR: the computer has exceeded the MAXTASKS limit\n");
    kill (0,SIGINT);
  }

/*    printf ("Number of possible tasks: %i\n",npt); */
    
  if (npt <= 0)
    return 0;

  return 1;
}

void computer_update_assigned (struct database *wdb,uint32_t ijob,int iframe,int icomp,int itask)
{
  /* This function should put into the computer task structure */
  /* all the information about ijob, iframe */
  struct task *task = &wdb->computer[icomp].status.task[itask];
  struct job *job = &wdb->job[ijob];
  task->used = 1;
  task->status = TASKSTATUS_LOADING; /* Not yet running */
  strncpy(task->jobname,job->name,MAXNAMELEN-1);
  task->ijob = ijob;
  strncpy(task->jobcmd,job->cmd,MAXCMDLEN-1);
  strncpy(task->owner,job->owner,MAXNAMELEN-1);
  task->frame = job_frame_index_to_number (&wdb->job[ijob],iframe);
  task->pid = 0;
  task->exitstatus = 0;
}

void computer_init (struct computer *computer)
{
  computer->used = 0;
}

int computer_ncomputers_masterdb (struct database *wdb)
{
  /* Returns the number of computers that are registered in the master database */
  int i,c=0;

  for (i=0;i<MAXCOMPUTERS;i++) {
    if (wdb->computer[i].used) {
      c++;
    }
  }

  return c;
}

int computer_ntasks (struct computer *comp)
{
  /* This function should be called locked */
  int i;
  int ntasks = 0;

  for (i=0; i < MAXTASKS; i++) {
    if (comp->status.task[i].used)
      ntasks ++;
  }

  return ntasks;
}

void computer_init_limits (struct computer *comp)
{
  comp->limits.nmaxcpus = comp->hwinfo.ncpus;
  comp->limits.maxfreeloadcpu = 80;
}

