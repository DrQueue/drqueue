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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>

#include "libdrqueue.h"
#include "slave.h"

int computer_index_addr (void *pwdb,struct in_addr addr) {
  /* This function is called by the master */
  /* This functions resolves the name associated with the ip of the socket and */
  /* and finds that name in the computer list and returns it's index position */
  int index;
  struct hostent *host;
  char *dot;
  char *name;

  log_master (L_DEBUG,"Entering computer_index_addr");

  if ((host = gethostbyaddr ((const void *)&addr.s_addr,sizeof (struct in_addr),AF_INET)) == NULL) {
    log_master (L_INFO,"computer_index_addr(). Using IP address as host name because '%s' could not be resolved",inet_ntoa(addr));
    name=inet_ntoa(addr);
  } else {
    //int i=0;
    if (((dot = strchr (host->h_name,'.')) != NULL) && (dot != host->h_name)) {
      // take out whatever comes after the first '.', if it's not the
      // whole name.
      *dot = '\0';
    }
    name = host->h_name;
  }


  semaphore_lock(((struct database *)pwdb)->semid);
  index = computer_index_name (pwdb,name);
  semaphore_release(((struct database *)pwdb)->semid);

  log_master (L_DEBUG,"Exiting computer_index_addr. Index of computer %s is %i.",name,index);

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
  log_slave_computer(L_DEBUG2,"computer_available() phase 1 (limits on cpus) : npt = %i",npt);

  /* then npt is the minimum of npt or the number of free tasks structures */
  npt = (npt < MAXTASKS) ? npt : MAXTASKS;
  log_slave_computer(L_DEBUG2,"computer_available() phase 2 (<maxtasks) : npt = %i",npt);

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
  log_slave_computer(L_DEBUG2,"computer_available() phase 3 (after considering the loadavg) : npt = %i",npt);

  /* Number of current working tasks */
  npt -= computer->status.nrunning;
  log_slave_computer(L_DEBUG2,"computer_available() phase 3 (substract nrunning) : npt = %i",npt);

  if (computer->status.nrunning > MAXTASKS) {
    /* This should never happen, btw */
    log_slave_computer (L_ERROR,"The computer has exceeded the MAXTASKS limit");
    kill (0,SIGINT);
  }

  if (npt <= 0) {
    log_slave_computer(L_DEBUG2,"computer_available() returning 0 : NOT available");
    return 0;
  }

  log_slave_computer(L_DEBUG2,"computer_available() returning 1 : available");
  return 1;
}

void computer_update_assigned (struct database *wdb,uint32_t ijob,int iframe,int icomp,int itask) {
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
  computer_status_init(&computer->status);
  computer_pool_init (&computer->limits);
}

int computer_free (struct computer *computer) {
  computer->used = 0;
  computer_status_init(&computer->status);
  if (!computer_pool_free (&computer->limits)) {
    log_auto (L_ERROR,"computer_pool_free() found a problem while freeing computer pool memory. (%s) (%s)\n",drerrno_str(),strerror(errno));
  }

  return 1;
}

void computer_pool_set_from_environment (struct computer_limits *cl) {
  char *buf;
  char *pool;

  if ((buf = getenv ("DRQUEUE_POOL")) == NULL) {
    log_slave_computer (L_WARNING,"DRQUEUE_POOL not set, joining \"%s\"",DEFAULT_POOL);
    computer_pool_add (cl,DEFAULT_POOL);
  } else {
    if ((pool = strtok (buf,": ,=\r\n")) != NULL) {
      if (strlen(pool) > 0) {
        computer_pool_add (cl,pool);
        log_slave_computer (L_INFO,"Joining pool: \"%s\"",pool);
      }
      while ((pool = strtok (NULL,": ,=\n")) != NULL) {
        if (strlen(pool) > 0) {
          computer_pool_add (cl,pool);
          log_slave_computer (L_INFO,"Joining pool: \"%s\"",pool);
        }
      }
    } else {
      log_slave_computer (L_WARNING,"DRQUEUE_POOL in not properly set, joining \"%s\"",DEFAULT_POOL);
      computer_pool_add (cl,DEFAULT_POOL);
    }
  }
  
  if (cl->npools == 0) {
    // it added no pools :/
    computer_pool_add (cl,DEFAULT_POOL);
    if (cl->npools == 1) {
      log_slave_computer (L_WARNING,"Check your DRQUEUE_POOL value:'%s'. Something made it to be parsed as an empty pool list. We joined the default pool : '%s'",
                          DEFAULT_POOL);
    } else {
      log_auto (L_ERROR,"computer_pool_set_from_environment() it has been not possible to add any pool. Check your logs for shared memory problems.");
      log_slave_computer (L_ERROR,"computer_pool_set_from_environment() it has been not possible to add any pool. Check your logs for shared memory problems.");
    }
  }
}

void computer_pool_init (struct computer_limits *cl) {
  log_auto (L_DEBUG3,"computer_pool_init() : PID (%i) poolshmid (%lli) pool (%p)",getpid(),cl->poolshmid,cl->pool);
  cl->pool = NULL;
  cl->poolshmid = (int64_t)-1;
  cl->npools = 0;
}

int64_t computer_pool_get_shared_memory (int npools) {
  int64_t shmid;

  if ((shmid = (int64_t) shmget (IPC_PRIVATE,(size_t)sizeof(struct pool)*npools, IPC_EXCL|IPC_CREAT|0600)) == (int64_t)-1) {
    log_auto (L_ERROR,"computer_pool_get_shared_memory() could not allocate shared memory for %i pools: %s",npools,strerror(errno));
    drerrno = DRE_GETSHMEM;
    return shmid;
  }

  log_auto (L_DEBUG2,"PID (%i) shmid (%lli): Allocated space for %i pools", getpid(),shmid,npools);

  drerrno = DRE_NOERROR;
  return shmid;
}

struct pool *computer_pool_attach_shared_memory (struct computer_limits *cl) {
  // Returns -1 on failure
  void *rv;   /* return value */

  if (cl->pool) {
    // already attached.
    log_auto (L_WARNING,"computer_pool_attach_shared_memory() : trying to attach what seems an already attached memory segment. Proceeding anyway.");
  }

  if ((rv = shmat ((int)cl->poolshmid,0,0)) == (void *)-1) {
    log_auto (L_ERROR,"computer_pool_attach_shared_memory() : error attaching %lli poolshmid. (%s)",cl->poolshmid,strerror(errno));
    log_auto (L_ERROR,"computer_pool_attach_shared_memory() : removing poolshmid from previous error (%lli)",cl->poolshmid);
    drerrno = DRE_ATTACHSHMEM;
    computer_pool_free(cl);
  } else {
    drerrno = DRE_NOERROR;
    cl->pool = (struct pool *)rv;
    log_auto (L_DEBUG2,"computer_pool_attach_shared_memory() : successful attach (shmid='%lli',npools='%i',pool='%p')",cl->poolshmid,cl->npools,cl->pool);
  }
  
  return (struct pool *)rv;
}

int computer_pool_free (struct computer_limits *cl) {
  int rv = 1;
  log_auto (L_DEBUG3,"PID (%i): computer_pool_free (cl=%x,cl->poolshmid=%lli,cl->npools=%i)",getpid(),cl,cl->poolshmid,cl->npools);
  if (cl->poolshmid != (int64_t)-1) {
    if (shmctl (cl->poolshmid,IPC_RMID,NULL) == -1) {
      log_auto (L_ERROR,"computer_pool_free() error found while deleting shared memory poolshmid: %lli (%s)",cl->poolshmid,strerror(errno));
      drerrno = DRE_RMSHMEM;
      rv = 0;
    } else {
      log_auto (L_DEBUG3,"PID (%i): computer_pool_free () : pool shared memory successfully removed",getpid());
    }
  }

  computer_pool_init (cl);
  return rv;
}

int computer_pool_detach_shared_memory (struct computer_limits *cl) {
  int rv = 1;
  if (cl->pool) {
    if (shmdt((char*)cl->pool) == -1) {
      log_auto(L_ERROR,"computer_pool_detach_shared_memory(): %s",strerror(errno));
      rv = 0;
    } 
    cl->pool = NULL;
  } else {
    if (cl->poolshmid != -1) {
      log_auto(L_WARNING,"computer_pool_detach_shared_memory(): something tried to detach NULL with a poolshmid != -1.");
    } else {
      log_auto(L_DEBUG,"computer_pool_detach_shared_memory(): tried to detach NULL with a poolshmid == -1. Not an issue.");
    }
    rv = 0;
  }
  return rv;
}

int computer_pool_add (struct computer_limits *cl, char *poolname) {
  struct pool *opool = (struct pool *)-1;
  struct pool *npool;
  int64_t npoolshmid;
  struct computer_limits new_cl;

  log_auto (L_DEBUG2,"computer_pool_add (cl=%p,cl->poolshmid=%lli) : %s",cl,cl->poolshmid,poolname);

  if (computer_pool_exists (cl,poolname)) {
    // It is already on the list
    log_auto (L_DEBUG,"computer_pool_add() : pool '%s' already exists on the list",poolname);
    return 1;
  }

  if (cl->npools && ((opool = (struct pool *)computer_pool_attach_shared_memory(cl)) == (void *) -1)) {
    log_auto (L_ERROR,"computer_pool_add() : Could not attach old shared memory (cl:%p shmid: %lli)",cl,cl->poolshmid);
    return 0;
  }

  computer_pool_init(&new_cl);
  new_cl.npools = cl->npools+1;
  if ((npoolshmid = computer_pool_get_shared_memory(new_cl.npools)) == (int64_t)-1) {
    log_auto (L_ERROR,"computer_pool_add() : Could not get new shared memory (npools = %i)",new_cl.npools);
    computer_pool_detach_shared_memory(cl);
    return 0;
  }
  
  new_cl.poolshmid = npoolshmid;
  if ((npool = (struct pool *)computer_pool_attach_shared_memory(&new_cl)) == (void *) -1) {
    log_auto (L_ERROR,"computer_pool_add() : Could not attach new shared memory (shmid: %lli,npools)",new_cl.poolshmid,new_cl.npools);
    computer_pool_free(&new_cl);
    computer_pool_detach_shared_memory(cl);
    return 0;
  }
  
  if ((cl->npools) && (cl->pool != (void*) -1)) {
    memcpy (npool,opool,sizeof (struct pool) * cl->npools);
    log_auto(L_DEBUG2,"computer_pool_add() : copied %i pools from old list",cl->npools);
  } else {
    log_auto(L_DEBUG2,"computer_pool_add() : no pools to copy (npools=%i,pool=%p)",cl->npools,cl->pool);
  }

  strncpy (new_cl.pool[cl->npools].name,poolname,MAXNAMELEN);
  log_auto(L_DEBUG2,"computer_pool_add() : copied pool name '%s' to the end of the list.",new_cl.pool[cl->npools].name);

  // remove old list
  computer_pool_detach_shared_memory (cl);
  computer_pool_free (cl);

  // fprintf(stderr,"New number of pools: %i\n",cl->npools);
  cl->poolshmid = new_cl.poolshmid;
  cl->pool = new_cl.pool;
  cl->npools = new_cl.npools;

  computer_pool_detach_shared_memory(cl);
  return 1;
}

int computer_pool_remove (struct computer_limits *cl, char *pool) {
  struct pool *opool = (struct pool *)-1;
  struct pool *npool;
  int64_t npoolshmid;
  int i,j;
  struct computer_limits new_cl;

  if (!computer_pool_exists (cl,pool)) {
    log_auto(L_WARNING,"computer_pool_remove() : pool '%s' not on list",pool);
    return 0;
  }

  if (!cl->npools) {
    log_auto(L_WARNING,"computer_pool_remove() : pool was empty when tried to remove poolname '%s'",pool);
    return 0;
  }

  if (cl->npools && ((opool = (struct pool *) computer_pool_attach_shared_memory(cl)) == (void *) -1)) {
    log_auto(L_ERROR,"computer_pool_remove() : could not attach old shared memory.");
    return 0;
  }

  computer_pool_init (&new_cl);
  new_cl.npools = cl->npools-1;
  if ((npoolshmid = computer_pool_get_shared_memory(new_cl.npools)) == (int64_t)-1) {
    log_auto(L_ERROR,"computer_pool_remove() : could not allocate shared memory for %i pools",new_cl.npools);
    computer_pool_detach_shared_memory (cl);
    return 0;
  }

  new_cl.poolshmid = npoolshmid;
  if ((npool = (struct pool *) computer_pool_attach_shared_memory(&new_cl)) == (void *) -1) {
    log_auto(L_WARNING,"computer_pool_remove() : could not attach new shared memory");
    computer_pool_detach_shared_memory (cl);
    computer_pool_free(&new_cl);
    return 0;
  }

  for (i=0,j=0;i<cl->npools;i++) {
    if (strncmp(cl->pool[i].name,pool,strlen(pool)+1) == 0) {
      continue;
    }
    memcpy(&new_cl.pool[j],&cl->pool[i],sizeof(struct pool));
    j++;
  }

  computer_pool_detach_shared_memory (cl);
  computer_pool_free(cl);
  cl->poolshmid = new_cl.poolshmid;
  cl->npools = new_cl.npools;
  cl->pool = new_cl.pool;
  computer_pool_detach_shared_memory (cl);

  return 1;
}

void computer_pool_list (struct computer_limits *cl) {
  int i;
  struct pool *pool;

  if (cl->poolshmid != (int64_t)-1) {
    pool = (struct pool *) computer_pool_attach_shared_memory (cl);
    if (pool != (void*)-1) {
      for (i = 0; i < cl->npools; i++) {
        if (i == 0) {
          printf ("List of pools: \n");
        }
        printf (" \t%i - %s\n",i,pool[i].name);
      }
      computer_pool_detach_shared_memory (cl);
      if (i == 0) {
        printf ("WARNING: Empty list of pools.\n");
      }
    } else {
      fprintf (stderr,"ERROR: computer_pool_list() pool shared memory could not be attached. (%s)\n",drerrno_str());
    }
  }
}

int computer_pool_exists (struct computer_limits *cl,char *poolname) {
  int i;
  struct pool *npool;
  int found = 0;

  if (!cl->npools)
    return found;

  if ((npool = (struct pool *) computer_pool_attach_shared_memory (cl)) == (void *)-1) {
    return found;
  }

  for (i=0;i<cl->npools;i++) {
    if (strncmp (npool[i].name,poolname,strlen(poolname)+1) == 0) {
      found = 1;
      break;
    }
  }

  computer_pool_detach_shared_memory (cl);
  return found;
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

int computer_ntasks (struct computer *comp) {
  /* This function returns the number of running tasks */
  /* This function should be called locked */
  int i;
  int ntasks = 0;

  for (i=0; i < MAXTASKS; i++) {
    if (comp->status.task[i].used) {
      ntasks ++;
    }
  }

  return ntasks;
}

int computer_nrunning (struct computer *comp) {
  /* This function returns the number of running tasks */
  /* This function should be called locked */
  int i;
  int nrunning = 0;
  for (i=0; i < MAXTASKS; i++) {
    if (comp->status.task[i].used && comp->status.task[i].status != TASKSTATUS_FINISHED) {
      nrunning++;
    }      
  }
  return nrunning;
}

void computer_init_limits (struct computer *comp) {
  memset (&comp->limits,0,sizeof(struct computer_limits));
  comp->limits.enabled = 1;
  comp->limits.nmaxcpus = comp->hwinfo.ncpus;
  comp->limits.maxfreeloadcpu = MAXLOADAVG;
  comp->limits.autoenable.h = AE_HOUR; /* At AE_HOUR:AE_MIN autoenable by default */
  comp->limits.autoenable.m = AE_MIN;
  comp->limits.autoenable.last = 0; /* Last autoenable on Epoch */
  comp->limits.autoenable.flags = 0; // No flags set, autoenable disabled
  computer_pool_init (&comp->limits);
}

int computer_index_correct_master (struct database *wdb, uint32_t icomp) {
  if (icomp > MAXCOMPUTERS)
    return 0;
  if (!wdb->computer[icomp].used)
    return 0;
  return 1;
}

int computer_nrunning_job (struct computer *comp,uint32_t ijob) {
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

void computer_autoenable_check (struct slave_database *sdb) {
  /* This function will check if it's the time for auto enable */
  /* If so, it will change the number of available processors to be the maximum */
  time_t now;
  struct tm *tm_now;
  struct computer_limits limits;

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

      log_slave_computer (L_INFO,"Autoenabled at %i:%02i",tm_now->tm_hour,tm_now->tm_min);

      update_computer_limits (&limits);
    }
  }
}

int computer_attach (struct computer *computer) {
  struct pool *pool;

  if (computer->limits.npools) {
    if ((pool = (struct pool *) computer_pool_attach_shared_memory(&computer->limits)) == (void*)-1) {
      // list is deleted by same attach on failure
      return 0;
    }
    
    pool = (struct pool *) malloc (sizeof (struct pool) * computer->limits.npools);
    memcpy (pool,computer->limits.pool,sizeof (struct pool) * computer->limits.npools);
    computer->limits.local_pool = pool;
    computer_pool_detach_shared_memory (&computer->limits);
  }

  return 1;
}

int computer_detach (struct computer *computer) {
  if (computer->limits.local_pool) {
    free (computer->limits.local_pool);
    computer->limits.local_pool = NULL;
  }

  return 1;
}
