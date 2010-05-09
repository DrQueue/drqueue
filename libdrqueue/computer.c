//
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>

#include "libdrqueue.h"
#include "computer_pool.h"
#include "computer.h"
#include "semaphore.h"

// ONGOING:
// * --- GUESS DONE task_is_running (some parts belong here)
// * 

int computer_index_addr (void *pwdb,struct in_addr addr) {
  /* This function is called by the master */
  /* This functions resolves the name associated with the ip of the socket and */
  /* and finds that name in the computer list and returns it's index position */
  int index;
  struct hostent *host;
  char *dot;
  char *name;

  log_auto (L_DEBUG,"Entering computer_index_addr");

  if ((host = gethostbyaddr ((const void *)&addr.s_addr,sizeof (struct in_addr),AF_INET)) == NULL) {
    log_auto (L_INFO,"computer_index_addr(). Using IP address as host name because '%s' could not be resolved",inet_ntoa(addr));
    name=inet_ntoa(addr);
  } else {
    //int i=0;
    if (((dot = strchr (host->h_name,'.')) != NULL) && (dot != host->h_name)) {
      // take out whatever comes after the first '.', if it's not the
      // whole name.
      *dot = '\0';
    }
    name = (char*) host->h_name;
  }


  semaphore_lock(((struct database *)pwdb)->semid);
  index = computer_index_name (pwdb,name);
  semaphore_release(((struct database *)pwdb)->semid);

  log_auto (L_DEBUG,"Exiting computer_index_addr. Index of computer %s is %i.",name,index);

  return index;
}

int computer_index_name (void *pwdb,char *name) {
  struct database *wdb = (struct database *)pwdb;
  int index = -1;
  int i;

  for (i=0;((i<MAXCOMPUTERS)&&(index==-1)); i++) {
    if ((strcmp(name,wdb->computer[i].hwinfo.name) == 0) && (wdb->computer[i].used))
      index = i;
  }

  return index;
}

int computer_index_free (void *pwdb) {
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

int computer_available (struct computer *computer) {
  int npt;   /* number of possible tasks */
  int t;

  if (computer->used == 0) {
    return 0;
  }

  if (!computer->limits.enabled) {
    return 0;
  }

  /* At the beginning npt is the minimum of the nmaxcpus or ncpus */
  /* This means that never will be assigned more tasks than processors */
  /* This behaviour could be changed in the future */
  npt = (computer->limits.nmaxcpus < computer->hwinfo.ncpus) ? computer->limits.nmaxcpus : computer->hwinfo.ncpus;
  log_auto(L_DEBUG2,"computer_available() phase 1 (limits on cpus) : npt = %i",npt);

  /* then npt is the minimum of npt or the number of free tasks structures */
  npt = (npt < MAXTASKS) ? npt : MAXTASKS;
  log_auto(L_DEBUG2,"computer_available() phase 2 (<maxtasks) : npt = %i",npt);

  /* Care must be taken because we substract the running tasks TWO times */
  /* one because of the load, another one here. */
  /* SOLUTION: we substract maxfreeloadcpu from the load */
  /* CONS: At the beggining of a frame it is not represented on the load average */
  /*    If we substract then we are probably substracting from another task's load */
  /* Number of cpus charged based on the load */
  if (computer->limits.maxfreeloadcpu > 0) {
    t = (computer->status.loadavg[0] / computer->limits.maxfreeloadcpu) - computer->status.nrunning;
    t = ( t < 0 ) ? 0 : t;
  } else {
    t = npt;
  }
  npt -= t;
  log_auto(L_DEBUG2,"computer_available() phase 3 (after considering the loadavg) : npt = %i",npt);

  /* Number of current working tasks */
  npt -= computer->status.nrunning;
  log_auto(L_DEBUG2,"computer_available() phase 3 (substract nrunning) : npt = %i",npt);

  if (computer->status.nrunning > MAXTASKS) {
    /* This should never happen, btw */
    log_auto (L_ERROR,"The computer has exceeded the MAXTASKS limit");
    kill (0,SIGINT);
  }

  if (npt <= 0) {
    log_auto(L_DEBUG2,"computer_available() returning 0 : NOT available");
    return 0;
  }

  log_auto(L_DEBUG2,"computer_available() returning 1 : available");
  return 1;
}

void computer_update_assigned (struct database *wdb,uint32_t ijob,uint32_t iframe,uint32_t icomp,uint16_t itask) {
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
  task->frame_pad = wdb->job[ijob].frame_pad;
  task->pid = 0;
  task->exitstatus = 0;
  task->itask = (uint16_t) itask;

  /* This updates the number of running tasks */
  /* This is temporary because the computer will send us the correct number later */
  /* but we need to update this now because the computer won't send the information */
  /* until it has exited the launching loop. And we need this information for the limits */
  /* tests */
  wdb->computer[icomp].status.ntasks++;
  wdb->computer[icomp].status.nrunning++;
}

void computer_init (struct computer *computer) {
  // Sets all computer values to the initial valid defaults.
  // It does not free any allocated memory, be sure to use it after
  // having freed all of it.
  //
  // But also be advised that functions that free their allocated
  // space, also init the related values afterwards.
  //
  computer->used = 0;
  computer_lock_check(computer);
  computer_limits_init(&computer->limits);
  computer_status_init(&computer->status);
}

int computer_free (struct computer *computer) {
  if (!computer) {
    return 0;
  }
  if (!computer_pool_free (&computer->limits)) {
    log_auto (L_ERROR,"computer_pool_free() found a problem while freeing computer pool memory. (%s) (%s)\n",
	      drerrno_str(),strerror(drerrno_system));
  }
  computer_init(computer);
  return 1;
}


int computer_ncomputers_masterdb (struct database *wdb) {
  /* Returns the number of computers that are registered in the master database */
  int i,c=0;

  for (i=0;i<MAXCOMPUTERS;i++) {
    if (wdb->computer[i].used) {
      c++;
    }
  }

  return c;
}

uint16_t
computer_ntasks (struct computer *comp) {
  /* This function returns the number of running tasks */
  /* This function should be called locked */
  int i;
  uint16_t ntasks = 0;

  for (i=0; i < MAXTASKS; i++) {
    if (comp->status.task[i].used) {
      ntasks++;
    }
  }

  return ntasks;
}

uint16_t
computer_nrunning (struct computer *comp) {
  /* This function returns the number of running tasks */
  /* This function should be called locked */
  int i;
  uint16_t nrunning = 0;
  
  computer_lock (comp);
  for (i=0; i < MAXTASKS; i++) {
    // Old: if (comp->status.task[i].used && comp->status.task[i].status != TASKSTATUS_FINISHED) {
    if (task_is_running(&comp->status.task[i])) {
      nrunning++;
    }
  }
  computer_release (comp);

  return nrunning;
}

void
computer_limits_cleanup_received (struct computer_limits *cl) {
  // This function should initialize only those values that could only mean something remotely and not locally
  // namely: shared memory pointers, pointers of any other type, identifiers...
  cl->pool.ptr = NULL;
  cl->local_pool.ptr = NULL;
  cl->poolshmid = (int64_t)-1;
  cl->poolsemid = (int64_t)-1;
  cl->npoolsattached = 0;
}

void
computer_limits_cleanup_to_send (struct computer_limits *cl) {
  computer_limits_cleanup_received (cl);
}

void
computer_limits_init (struct computer_limits *cl) {
  memset (cl,0,sizeof(struct computer_limits));
  cl->enabled = 1;
  cl->nmaxcpus = MAXTASKS;
  cl->maxfreeloadcpu = MAXLOADAVG;
  cl->autoenable.h = AE_HOUR; /* At AE_HOUR:AE_MIN autoenable by default */
  cl->autoenable.m = AE_MIN;
  cl->autoenable.last = 0; /* Last autoenable on Epoch */
  cl->autoenable.flags = 0; // No flags set, autoenable disabled
  computer_pool_init (cl);
}

void computer_limits_cpu_init (struct computer *comp) {
  comp->limits.nmaxcpus = comp->hwinfo.ncpus;
  comp->limits.maxfreeloadcpu = MAXLOADAVG * comp->hwinfo.ncpus;
} 

int computer_index_correct_master (struct database *wdb, uint32_t icomp) {
  if (icomp > MAXCOMPUTERS)
    return 0;
  if (!wdb->computer[icomp].used)
    return 0;
  return 1;
}

uint16_t
computer_nrunning_job (struct computer *comp,uint32_t ijob) {
  /* This function returns the number of tasks that are running the specified */
  /* ijob in the given computer */
  /* This function should be called locked */
  int n = 0;
  int c;

  for (c=0;c<MAXTASKS;c++) {
    if ((comp->status.task[c].used)
        && (comp->status.task[c].ijob == ijob)
        && (comp->status.task[c].status != TASKSTATUS_FINISHED)) {
      n++;
    }
  }

  return n;
}

void
computer_autoenable_check (struct slave_database *sdb) {
  /* This function will check if it's the time for auto enable */
  /* If so, it will change the number of available processors to be the maximum */
  time_t now;
  struct tm *tm_now;
  struct computer_limits limits;

  if (sdb->comp->limits.enabled) {
    // Already enabled, why bother ?
    return;
  }

  log_auto (L_DEBUG3,"computer_autoenable_check(): >Entering...");
  time (&now);
  if ((sdb->comp->limits.autoenable.flags & AEF_ACTIVE)
      && ((now - sdb->comp->limits.autoenable.last) > AE_DELAY)) {
    /* If more time than AE_DELAY has passed since the last autoenable */
    tm_now = localtime (&now);
    if ((sdb->comp->limits.autoenable.h == tm_now->tm_hour)
        && (sdb->comp->limits.autoenable.m == tm_now->tm_min)
        && (sdb->comp->limits.enabled == 0)) /* Only if the computer is completely disabled (?) */
    {
      /* Time for autoenable */
      semaphore_lock (sdb->semid);

      sdb->comp->limits.autoenable.last = now;
      sdb->comp->limits.enabled = 1;

      limits = sdb->comp->limits;

      semaphore_release (sdb->semid);

      log_auto (L_INFO,"Slave autoenabled at %i:%02i",tm_now->tm_hour,tm_now->tm_min);

      update_computer_limits (&limits);
    }
  }
  log_auto (L_DEBUG2,"computer_autoenable_check(): Exiting...");
}

int
computer_lock_check (struct computer *computer) {
#if defined (_NO_COMPUTER_SEMAPHORES)
  return 1;
#else
  if (!semaphore_valid(computer->semid)) {
    log_auto (L_WARNING,"computer_lock(): semaphore not valid, creating a new one. Computer Id: %u",computer->hwinfo.id);
    computer->semid = semaphore_get();
    if (computer->semid == -1) {
      log_auto (L_ERROR,"CRITICAL: computer_lock(): Could not create semaphore for computer. Msg: %s",strerror(drerrno_system));
      exit (1);
    }
    return 0;
  }
  return 1;
#endif
}

int
computer_lock (struct computer *computer) {
#if defined (_NO_COMPUTER_SEMAPHORES)
  return 1;
#else
  computer_lock_check (computer);
  if (!semaphore_lock (computer->semid)) {
    log_auto (L_ERROR,"computer_lock(): There was an error while trying to lock the computer structure. Msg: %s",
	      strerror(drerrno_system));
    return 0;
  }
  log_auto (L_DEBUG3,"computer_lock(): computer locked successfully. (Comp Id: %u)",computer->hwinfo.id);
  return 1;
#endif
}

int
computer_release (struct computer *computer) {
#if defined (_NO_COMPUTER_SEMAPHORES)
  return 1;
#else
  computer_lock_check (computer);
  if (!semaphore_release (computer->semid)) {
    log_auto (L_ERROR,"computer_release(): There was an error while trying to release the computer structure. Msg: %s",
	      strerror(drerrno_system));
    return 0;
  }
  log_auto (L_DEBUG3,"computer_release(): computer released successfully. (Comp Id: %u)",computer->hwinfo.id);
  return 1;
#endif
}

int computer_attach (struct computer *computer) {
  // This function attachs shared memory, copies it to a local pointer and detachs.
  struct pool *pool;

  log_auto (L_DEBUG2,"computer_attach(): Entering...");
  computer_lock(computer);
  if (computer->limits.npools) {
    if ((pool = (struct pool *) computer_pool_attach_shared_memory(&computer->limits)) == (struct pool *)-1) {
      // list is deleted by same attach on failure
      log_auto (L_ERROR,"computer_attach(): error attaching pool shared memory. (Msg: %)",strerror(drerrno_system));
      log_auto (L_ERROR,"computer_attach(): Exiting on error...");
      computer_release(computer);
      return 0;
    }
    
    pool = (struct pool *) malloc (sizeof (struct pool) * computer->limits.npools);
    memcpy ((void*)pool,(void*)computer->limits.pool.ptr,sizeof(struct pool) * computer->limits.npools);
    computer->limits.local_pool.ptr = pool;
    computer_pool_detach_shared_memory (&computer->limits);
  }
  computer_release(computer);

  log_auto (L_DEBUG2,"computer_attach(): Exiting...");
  return 1;
}

int computer_detach (struct computer *computer) {
  // This function frees allocated memory for local pools.
  log_auto (L_DEBUG2,"computer_detach(): Entering...");
  if (computer->limits.local_pool.ptr) {
    free (computer->limits.local_pool.ptr);
    computer->limits.local_pool.ptr = NULL;
  }
  computer_release(computer);
  log_auto (L_DEBUG2,"computer_detach(): Exiting...");
  return 1;
}
