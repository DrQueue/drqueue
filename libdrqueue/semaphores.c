//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "drerrno.h"
#include "logger.h"

// PENDING:
// * get semaphores with ftok keys 
// * testing (does the validity check work ?) 

int
semaphore_lock (int64_t semid) {
  struct sembuf op;

  op.sem_num = 0;
  op.sem_op = -1;
  op.sem_flg = SEM_UNDO;

  /*  fprintf (stderr,"Locking... semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid); */
  if (semop((int)semid,&op,1) == -1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"semaphore_release(): error releasing semaphore %ji. (Msg: %s)",semid,strerror(drerrno_system));
    return 0;
  }
  /*  fprintf (stderr,"Locked !!! semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid); */
  return 1;
}

int
semaphore_release (int64_t semid) {
  struct sembuf op;

  if (semid == -1) {
    log_auto (L_DEBUG,"semaphore_release(): trying to release an invalid semaphore (-1).");
    return 1;
  }

  /*  fprintf (stderr,"Unlocking... semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid); */
  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = SEM_UNDO; /*  SEM_UNDO; */
  if (semop((int)semid,&op,1) == -1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"semaphore_release(): error releasing semaphore %ji. (Msg: %s)",semid,strerror(drerrno_system));
    return 0;
  }
  /*  fprintf (stderr,"Unlocked !!! semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid); */
  return 1;
}

int
semaphore_valid (int64_t semid) {
  if (semid == -1)
    return 0;

  if (semctl ((int)semid,0,GETVAL) == -1) {
    drerrno_system = errno;
    log_auto (L_DEBUG,"semaphore_valid(): not valid '%ji'. (Msg: %s)",semid,strerror(drerrno_system));
    return 0;
  }
  return 1;
}

int64_t
semaphore_get (void) {
  int64_t semid;
  struct sembuf op;
  if ((semid = (int64_t)semget (IPC_PRIVATE,1,IPC_CREAT|0600)) == (int64_t)-1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"semaphore_get(): getting semaphore. Msg: %s",strerror(drerrno_system));
  }
  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = 0;
  if (semop((int)semid,&op,1) == -1) {
    log_auto (L_ERROR,"semaphore_get(): error calling semop to finish semaphore setup. Msg: %s",strerror(errno));
    return -1;
  }

  return semid;
}

int
semaphore_remove (int64_t semid) {
  if (semctl ((int)semid,0,IPC_RMID,NULL) == -1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"semaphore_remove(): there was an error while removing a semaphore. Msg: %s",
	      strerror(drerrno_system));
    return 0;
  }
  return 1;
}
