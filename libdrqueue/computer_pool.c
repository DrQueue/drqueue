//
// Copyright (C) 2006,2007 Jorge Daza Garcia-Blanes
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

#include "libdrqueue.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>


// PENDING:
// * attach counter
// * semaphore

int
computer_pool_lock_check (struct computer_limits *cl) {
#if defined (_NO_COMPUTER_POOL_SEMAPHORES)
  return 1;
#else
  if (!semaphore_valid(cl->poolsemid) ) {
    log_auto (L_WARNING,"computer_pool_lock_check(): semaphore not valid, creating a new one.");
    cl->poolsemid = semaphore_get();
    if (cl->poolsemid == -1) {
      log_auto (L_ERROR,"CRITICAL (EXIT): computer_pool_lock_check(): Could not create semaphore for pool list. Msg: %s",
		strerror(drerrno_system));
      exit (1);
    }
    return 0;
  }
  return 1;
#endif
}

int
computer_pool_lock (struct computer_limits *cl) {
#if defined (_NO_COMPUTER_POOL_SEMAPHORES)
  return 1;
#else
  computer_pool_lock_check (cl);
  if (!semaphore_lock (cl->poolsemid)) {
    log_auto (L_ERROR,"computer_pool_lock(): There was an error while trying to lock the computer pool structure. Msg: %s",
	      strerror(drerrno_system));
    return 0;
  }
  log_auto (L_DEBUG3,"computer_pool_lock(): computer pools locked successfully.");
  return 1;
#endif
}

int
computer_pool_release (struct computer_limits *cl) {
#if defined (_NO_COMPUTER_POOL_SEMAPHORES)
  return 1;
#else
  computer_pool_lock_check (cl);
  if (semaphore_release (cl->poolsemid) == 0) {
    log_auto (L_ERROR,"computer_pool_release(): There was an error while trying to release the computer pools structure. Msg: %s",
	      strerror(drerrno_system));
    return 0;
  }
  log_auto (L_DEBUG3,"computer_pool_release(): computer pools were released successfully.");
  return 1;
#endif
}


void
computer_pool_set_from_environment (struct computer_limits *cl) {
  char *buf;
  char *pool;

  if ((buf = getenv ("DRQUEUE_POOL")) == NULL) {
    log_auto (L_WARNING,"WARNING: Adding default pool. DRQUEUE_POOL not set, joining \"%s\"",DEFAULT_POOL);
    computer_pool_add (cl,DEFAULT_POOL);
  } else {
    // TODO: list = config_pool_list_get()
    while ((pool = strtok (buf,": ,=\n")) != NULL) {
      if (strlen(pool) > 0) {
        buf=NULL;
        computer_pool_add (cl,pool);
        log_auto (L_INFO,"Joining pool: \"%s\"",pool);
      } else {
        log_auto (L_WARNING,"DRQUEUE_POOL is not properly set, joining \"%s\"",DEFAULT_POOL);
        computer_pool_add (cl,DEFAULT_POOL);
      }
    }
  }
  
  if (cl->npools == 0) {
    // There are no pools even though at least "Default" should be there.
    log_auto (L_WARNING,"WARNING (Second): Adding default pool. DRQUEUE_POOL not set, joining \"%s\"",DEFAULT_POOL);
    computer_pool_add (cl,DEFAULT_POOL);
    if (cl->npools == 1) {
      log_auto (L_WARNING,"Check your DRQUEUE_POOL value:'%s'. Something made it to be parsed as an "
		"empty pool list. We joined the default pool : '%s'",
		buf,DEFAULT_POOL);
    } else {
      log_auto (L_ERROR,"computer_pool_set_from_environment() it has been not possible to add any pool. "
		"Check your logs for shared memory problems.");
      // TODO: clean_exit()
      kill(0,SIGINT);
    }
  }
}

void
computer_pool_init (struct computer_limits *cl) {
  cl->pool.ptr = NULL;
  cl->local_pool.ptr = NULL;
  cl->poolshmid = -1;
  cl->poolsemid = -1;
  computer_pool_lock_check(cl);
  cl->npools = 0;
  cl->npoolsattached = 0;
}

int64_t
computer_pool_get_shared_memory (uint16_t npools) {
  int64_t shmid;

  if ((shmid = shmget (IPC_PRIVATE,(size_t)sizeof(struct pool)*npools, IPC_EXCL|IPC_CREAT|0600)) == (int64_t)-1) {
    drerrno_system = errno;
    drerrno = DRE_GETSHMEM;
    log_auto (L_ERROR,"computer_pool_get_shared_memory() could not allocate shared memory for %u pools: %s",
	      npools,strerror(drerrno_system));
    return shmid;
  }

  log_auto (L_DEBUG,"computer_pool_get_shared_memory(): Succesfully allocated space for %u pools.  (poolshmid: %ji)",npools,shmid);
  drerrno = DRE_NOERROR;
  return shmid;
}

struct pool *
computer_pool_attach_shared_memory (struct computer_limits *cl) {
  // Returns -1 on failure

  struct pool *pool; // return value

  log_auto (L_DEBUG3,"computer_pool_attach_shared_memory(): >Entering...");
  drerrno = DRE_NOERROR;

  if ((pool = (struct pool*)shmat ((int)cl->poolshmid,0,0)) == (void *)-1) {
    drerrno_system = errno;
    drerrno = DRE_ATTACHSHMEM;
    log_auto (L_ERROR,"computer_pool_attach_shared_memory() : error attaching pool shared memory. (%s)",strerror(drerrno_system));
    log_auto (L_ERROR,"computer_pool_attach_shared_memory() : removing poolshmid from previous error (%ji)",cl->poolshmid);
    computer_pool_init(cl);
  } else {
    cl->pool.ptr = pool;
    log_auto (L_DEBUG,"computer_pool_attach_shared_memory() : successful attach, pool pointer = %p",cl->pool);
  }
  
  log_auto (L_DEBUG3,"computer_pool_attach_shared_memory(): >Exiting...");
  return pool;
}

int
computer_pool_free (struct computer_limits *cl) {
  int rv = 1;
  //log_auto (L_DEBUG3,"computer_pool_free(): Entering...");
  //log_auto (L_DEBUG3,"computer_pool_free(): cl->poolshmid=%ji,cl->npools=%u,cl->pool=%p",cl->poolshmid,cl->npools,cl->pool);

  if (!cl) {
    rv = 0;
    return rv;
  }

  computer_pool_lock (cl);
  computer_pool_detach_shared_memory(cl);
  if (cl->poolshmid != -1) {
    log_auto (L_DEBUG3,"computer_pool_free(): previous to shmctl(IPC_RMID)");
    if (shmctl ((int)cl->poolshmid,IPC_RMID,NULL) == -1) {
      drerrno_system = errno;
      drerrno = DRE_RMSHMEM;
      log_auto (L_WARNING,"computer_pool_free() error found while deleting shared memory poolshmid. (Msg: %s)",strerror(drerrno_system));
      rv = 0;
    } else {
      log_auto (L_DEBUG,"computer_pool_free () : pool shared memory successfully removed.");
    }
  }
  computer_pool_release (cl);
  computer_pool_init (cl);

  //log_auto (L_DEBUG3,"computer_pool_free(): Exiting...");
  return rv;
}

int
computer_pool_detach_shared_memory (struct computer_limits *cl) {
  int rv = 1;
  
  if (!cl) {
    return 0;
  }

  log_auto (L_DEBUG2,"computer_pool_detach_shared_memory() : > Entering...");

  if (cl->pool.ptr) {
    if (shmdt((void*)cl->pool.ptr) == -1) {
      drerrno_system = errno;
      drerrno = DRE_DTSHMEM;
      log_auto(L_ERROR,"computer_pool_detach_shared_memory(): error detaching shared memory: %s",strerror(drerrno_system));
      rv = 0;
    }
    cl->pool.ptr = NULL;
  } else {
    if (cl->poolshmid != (int64_t) -1) {
      log_auto(L_DEBUG2,"computer_pool_detach_shared_memory(): already detached, can't do it again.");
    } else {
      log_auto(L_DEBUG2,"computer_pool_detach_shared_memory(): tried to detach NULL with a poolshmid == -1. Not an issue.");
      cl->pool.ptr = NULL;
    }
    rv = 0;
  }

  log_auto (L_DEBUG2,"computer_pool_detach_shared_memory() : < Exiting...");
  return rv;
}

int computer_pool_add (struct computer_limits *cl, char *poolname) {
  struct pool *opool = (struct pool *)-1;
  struct pool *npool = (struct pool *)-1;
  int64_t npoolshmid;
  struct computer_limits new_cl,old_cl;

  log_auto (L_DEBUG2,"computer_pool_add (%p) : %s",cl,poolname);
  
  computer_pool_lock(cl);

  if (computer_pool_exists (cl,poolname)) {
    // It is already on the list
    computer_pool_release(cl);
    log_auto (L_DEBUG,"computer_pool_add(): pool '%s' already exists on the list",poolname);
    return 1;
  }

  if (cl->poolshmid != -1) {
    if (cl->npools && ((opool = (struct pool *)computer_pool_attach_shared_memory(cl)) == (struct pool *) -1)) {
      computer_pool_release(cl);
      log_auto (L_ERROR,"computer_pool_add(): Could not attach old shared memory (cl:%p shmid: %ji)",cl,cl->poolshmid);
      return 0;
    }
  }

  log_auto (L_DEBUG3,"computer_pool_add(): proceed to init new pools structure...");
  computer_pool_init(&new_cl);
  new_cl.npools = cl->npools+1;
  if ((npoolshmid = computer_pool_get_shared_memory(new_cl.npools)) == (int64_t)-1) {
    computer_pool_release(cl);
    log_auto (L_ERROR,"computer_pool_add() : Could not get new shared memory (npools = %i)",new_cl.npools);
    computer_pool_detach_shared_memory(cl);
    return 0;
  }

  log_auto (L_DEBUG3,"computer_pool_add() : successfully allocated new shared memory segment for %i pool(s)",new_cl.npools);  
  new_cl.poolshmid = npoolshmid;
  if ((npool = (struct pool *)computer_pool_attach_shared_memory(&new_cl)) == (void *) -1) {
    computer_pool_release(cl);
    log_auto (L_ERROR,"computer_pool_add() : Could not attach new shared memory (shmid: %ji,npools=%i)",new_cl.poolshmid,new_cl.npools);
    computer_pool_free(&new_cl);
    computer_pool_detach_shared_memory(cl);
    return 0;
  }
  
  new_cl.pool.ptr = npool;
  if ((cl->npools) && (cl->pool.ptr != (void*) -1)) {
    memcpy (npool,opool,sizeof (struct pool) * cl->npools);
    log_auto(L_DEBUG2,"computer_pool_add() : copied %i pools from old list",cl->npools);
  } else {
    log_auto(L_DEBUG2,"computer_pool_add() : no pools to copy from old pool list (npools=%i,pool=%p)",cl->npools,cl->pool.ptr);
  }

  strncpy (new_cl.pool.ptr[cl->npools].name,poolname,MAXNAMELEN);
  log_auto(L_DEBUG2,"computer_pool_add() : copied pool name '%s' to the end of the new list.",new_cl.pool.ptr[cl->npools].name);


  computer_pool_copy (cl,&old_cl);           // Store a copy of the old one
  computer_pool_release(cl);
  computer_pool_copy (&new_cl,cl);           // Substitute the old one with the new one.
  computer_pool_detach_shared_memory(cl);    // Detack the new one
  computer_pool_free (&old_cl);

  return 1;
}

void
computer_pool_copy (struct computer_limits *cl_src, struct computer_limits *cl_dst) {
  cl_dst->npoolsattached = cl_src->npoolsattached;
  cl_dst->poolshmid = cl_src->poolshmid;
  cl_dst->poolsemid = cl_src->poolsemid;
  cl_dst->local_pool.ptr = cl_src->local_pool.ptr;
  cl_dst->pool.ptr = cl_src->pool.ptr;
  cl_dst->npools = cl_src->npools;
}

int
computer_pool_remove (struct computer_limits *cl, char *poolname) {
  //
  // Removes from the list the first pool with name "poolname"
  //
  struct pool *opool = (struct pool *)-1;
  struct pool *npool;
  int64_t npoolshmid;
  int i,j;
  struct computer_limits new_cl;
  
  //computer_pool_semaphore_lock(cl);
  if (!computer_pool_exists (cl,poolname)) {
    //computer_pool_semaphore_unlock(cl);
    log_auto(L_WARNING,"computer_pool_remove() : pool '%s' not on list",poolname);
    return 0;
  }

  if (!cl->npools) {
    log_auto(L_WARNING,"computer_pool_remove() : pool was empty when tried to remove poolname '%s'",poolname);
    return 0;
  }

  if (cl->npools && ((opool = (struct pool *) computer_pool_attach_shared_memory(cl)) == (void *) -1)) {
    log_auto(L_ERROR,"computer_pool_remove() : could not attach old shared memory.");
    return 0;
  }

  computer_pool_init (&new_cl);
  new_cl.npools = cl->npools-1;
  if ((npoolshmid = (int64_t) computer_pool_get_shared_memory(new_cl.npools)) == (int64_t)-1) {
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
    if (strncmp(cl->pool.ptr[i].name,poolname,strlen(poolname)+1) == 0) {
      continue;
    }
    memcpy(&new_cl.pool.ptr[j],&cl->pool.ptr[i],sizeof(struct pool));
    j++;
  }

  computer_pool_detach_shared_memory (cl);
  computer_pool_free(cl);
  cl->poolshmid = new_cl.poolshmid;
  cl->npools = new_cl.npools;
  cl->pool.ptr = new_cl.pool.ptr;
  computer_pool_detach_shared_memory (cl); // now it points to new_cl

  return 1;
}

void
computer_pool_list (struct computer_limits *cl) {
  uint16_t i;
  struct pool *pool;

  if (!cl) {
    return;
  }

  if (!cl->npools)
    return;

  if (cl->poolshmid != (int64_t)-1) {
    computer_pool_lock(cl);
    pool = (struct pool *) computer_pool_attach_shared_memory (cl);
    if (pool != (struct pool *)-1) {
      for (i = 0; i < cl->npools; i++) {
        if (i == 0) {
          log_auto (L_INFO,"List of pools follows:");
        }
        log_auto (L_INFO,"Pool number: %i -- Pool name: '%s'",i,pool[i].name);
      }
      computer_pool_detach_shared_memory (cl);
      if (i == 0) {
        log_auto (L_WARNING,"computer_pool_list(): ---> _Empty_ list of pools.");
      }
    } else {
      log_auto (L_ERROR,"computer_pool_list() : pool shared memory could not be attached. (%s)",strerror(drerrno_system));
    }
    computer_pool_release(cl);
  }
}


int computer_pool_exists (struct computer_limits *cl,char *poolname) {
  int i;
  struct pool *npool;
  int found = 0;

  if ((!cl->npools) || (cl->poolshmid == -1))
    return found;

  computer_pool_lock(cl);
  if ((npool = (struct pool *) computer_pool_attach_shared_memory (cl)) == (void*)-1) {
    // could not attach
    computer_pool_release(cl);
    return found;
  }

  for (i=0;i<cl->npools;i++) {
    if (strncmp (npool[i].name,poolname,strlen(poolname)+1) == 0) {
      found = 1;
      break;
    }
  }

  computer_pool_detach_shared_memory (cl);
  computer_pool_release(cl);

  return found;
}
