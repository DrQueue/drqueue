// 
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
// 
// $Id$
//

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#include "libdrqueue.h"

int computer_index_addr (void *pwdb,struct in_addr addr)
{
  /* This function is called by the master */
  /* This functions resolves the name associated with the ip of the socket and */
  /* and finds that name in the computer list and returns it's index position */
  int index;
  struct hostent *host;
  char *dot;
  int i=0;
  char *name;

  log_master (L_DEBUG,"Entering computer_index_addr");

  if ((host = gethostbyaddr ((const void *)&addr.s_addr,sizeof (struct in_addr),AF_INET)) == NULL) {
    log_master (L_WARNING,"Could not resolve name for: %s",inet_ntoa(addr));
    name=inet_ntoa(addr);
  } else {
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
  }


  semaphore_lock(((struct database *)pwdb)->semid);
  index = computer_index_name (pwdb,name);
  semaphore_release(((struct database *)pwdb)->semid);

  log_master (L_DEBUG,"Exiting computer_index_addr. Index of computer %s is %i.",name,index);

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

	if (computer->used == 0) {
		return 0;
	}

  /* At the beginning npt is the minimum of the nmaxcpus or ncpus */
  /* This means that never will be assigned more tasks than processors */
  /* This behaviour could be changed in the future */
  npt = (computer->limits.nmaxcpus < computer->hwinfo.ncpus) ? computer->limits.nmaxcpus : computer->hwinfo.ncpus;

  /* then npt is the minimum of npt or the number of free tasks structures */
  npt = (npt < MAXTASKS) ? npt : MAXTASKS;

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

  /* Number of current working tasks */
  npt -= computer->status.ntasks;

  if (computer->status.ntasks > MAXTASKS) {
    /* This should never happen, btw */
    fprintf (stderr,"CRITICAL ERROR: the computer has exceeded the MAXTASKS limit\n");
    kill (0,SIGINT);
  }
    
  if (npt <= 0)
    return 0;

  return 1;
}

void computer_update_assigned (struct database *wdb,uint32_t ijob,int iframe,int icomp,int itask)
{
  /* This function should put into the computer task structure */
  /* all the information about ijob, iframe */
  /* This function must be called _locked_ */
  struct task *task;
  struct job *job;

  if (!job_index_correct_master(wdb,ijob))
    return;

  if (!computer_index_correct_master(wdb,icomp))
    return;

  if (itask >= MAXTASKS)
    return;

  job = &wdb->job[ijob];
  task = &wdb->computer[icomp].status.task[itask];

  /* This updates the task */
  task->used = 1;
  task->status = TASKSTATUS_LOADING; /* Not yet running */
  strncpy(task->jobname,job->name,MAXNAMELEN-1);
  task->ijob = ijob;
	task->icomp = icomp;
  strncpy(task->jobcmd,job->cmd,MAXCMDLEN-1);
  strncpy(task->owner,job->owner,MAXNAMELEN-1);
  task->frame = job_frame_index_to_number (&wdb->job[ijob],iframe);
  task->frame_start = wdb->job[ijob].frame_start;
  task->frame_end = wdb->job[ijob].frame_end;
  task->frame_step = wdb->job[ijob].frame_step;
	task->block_size = wdb->job[ijob].block_size;
  task->pid = 0;
  task->exitstatus = 0;
  task->itask = (uint16_t) itask;

  /* This updates the number of running tasks */
  /* This is temporary because the computer will send us the correct number later */
  /* but we need to update this now because the computer won't send the information */
  /* until it has exited the launching loop. And we need this information for the limits */
  /* tests */
  wdb->computer[icomp].status.ntasks++;

}

void computer_init (struct computer *computer)
{
	// This function is called by the master when a computer is not longer on the list
  computer->used = 0;
  computer_status_init(&computer->status);
}

void computer_pool_init (struct computer_limits *cl)
{
	cl->pool = (char **) malloc (sizeof (char*));
	cl->pool[0] = NULL;
}

int computer_npools (struct computer_limits *cl)
{
	int i;
	int npools = 1;

	if (cl->pool) {
		for (i=0;cl->pool[i] != NULL;i++) 
			npools++;
	} else {
		computer_pool_init (cl);
	}

	return npools;
}

void computer_pool_add (struct computer_limits *cl, char *pool)
{
	int npools;
	char **new_pool;

	if (computer_pool_exists(cl,pool))
		return;

	npools = computer_npools (cl);

	new_pool = (char **) realloc (cl->pool,sizeof (char*) * (npools+1));
	new_pool[npools] = NULL;
	new_pool[npools-1] = (char *) malloc (strlen(pool)+1);
	strncpy (new_pool[npools-1],pool,strlen(pool)+1);
	cl->pool = new_pool;
}

void computer_pool_remove (struct computer_limits *cl, char *pool)
{
	int i,j;
	int npools;
	char **new_pool;

	npools = computer_npools (cl);

	new_pool = (char **) realloc (cl->pool,sizeof (char*) * (npools-1));
	for (i=0,j=0;cl->pool[i] != NULL; i++) {
		if (strncmp(cl->pool[i],pool,strlen(pool)+1) == 0) {
			continue;
		} else {
			new_pool[j] = cl->pool[i];
			j++;
		}
	}

	new_pool[j] = NULL;
}

void computer_pool_list (struct computer_limits *cl)
{
	int i;

	fprintf (stderr,"Pools:\n");
	if (cl->pool) 
		for (i=0;cl->pool[i] != NULL; i++)
			fprintf (stderr,"\t* %s\n",cl->pool[i]);
	else
		computer_pool_init (cl);
}

int computer_pool_exists (struct computer_limits *cl,char *pool)
{
	int i;

	if (cl->pool) {
		for (i=0;cl->pool[i] != NULL; i++) {
			if (strncmp(cl->pool[i],pool,strlen(pool)+1) == 0) {
				return 1;
			}
		}
	} else {
		computer_pool_init (cl);
	}

	return 0;
}

void computer_pool_free (struct computer_limits *cl)
{
	int i;

	if (cl->pool) {
		for (i=0;cl->pool[i]!=NULL;i++)
			free (cl->pool[i]);
		free (cl->pool);
		cl->pool = NULL;
	}
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
  /* This function returns the number of running tasks */
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
  comp->limits.maxfreeloadcpu = MAXLOADAVG;
  comp->limits.autoenable.h = AE_HOUR; /* At AE_HOUR:AE_MIN autoenable by default */
  comp->limits.autoenable.m = AE_MIN;
  comp->limits.autoenable.last = 0; /* Last autoenable on Epoch */
	comp->limits.autoenable.flags = 0; // No flags set, autoenable disabled
}

int computer_index_correct_master (struct database *wdb, uint32_t icomp)
{
  if (icomp > MAXCOMPUTERS)
    return 0;
  if (!wdb->computer[icomp].used)
    return 0;
  return 1;
}

int computer_ntasks_job (struct computer *comp,uint32_t ijob)
{
  /* This function returns the number of tasks that are running the specified */
  /* ijob in the given computer */
  /* This function should be called locked */
  int n = 0;
  int c;

  for (c=0;c<MAXTASKS;c++) {
    if ((comp->status.task[c].used)
				&& (comp->status.task[c].ijob == ijob)) {
      n++;
    }
  }

  return n;
}

void computer_autoenable_check (struct slave_database *sdb)
{
  /* This function will check if it's the time for auto enable */
  /* If so, it will change the number of available processors to be the maximum */
  time_t now;
  struct tm *tm_now;
  struct computer_limits limits;

  time (&now);

  if ((sdb->comp->limits.autoenable.flags & AEF_ACTIVE) 
			&& ((now - sdb->comp->limits.autoenable.last) > AE_DELAY))
	{
    /* If more time than AE_DELAY has passed since the last autoenable */
    tm_now = localtime (&now);
    if ((sdb->comp->limits.autoenable.h == tm_now->tm_hour)
				&& (sdb->comp->limits.autoenable.m == tm_now->tm_min)
				&& (sdb->comp->limits.nmaxcpus == 0)) /* Only if the computer is completely disabled (?) */
      {
				/* Time for autoenable */
				semaphore_lock (sdb->semid);
				
				sdb->comp->limits.autoenable.last = now;
				sdb->comp->limits.nmaxcpus = sdb->comp->hwinfo.ncpus;
				
				limits = sdb->comp->limits;
				
				semaphore_release (sdb->semid);
				
				log_slave_computer (L_INFO,"Autoenabled %i processor%s",limits.nmaxcpus,(limits.nmaxcpus > 1) ? "s" : "");
				
				update_computer_limits (&limits);
      }
  }
}

