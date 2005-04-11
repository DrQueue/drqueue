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
#include <sys/sem.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

void semaphore_lock (int semid)
{
  struct sembuf op;

  op.sem_num = 0;
  op.sem_op = -1;
  op.sem_flg = SEM_UNDO;

/*    fprintf (stderr,"Locking... semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid); */
  if (semop(semid,&op,1) == -1) {
    perror ("semaphore_lock");
    printf ("%i\n",errno);
    kill(0,SIGINT);
  }
/*    fprintf (stderr,"Locked !!! semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid); */
}

void semaphore_release (int semid)
{
  struct sembuf op;

/*    fprintf (stderr,"Unlocking... semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid); */
  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = SEM_UNDO; /*  SEM_UNDO; */
  if (semop(semid,&op,1) == -1) {
    perror ("semaphore_release");
    kill(0,SIGINT);
  }
/*    fprintf (stderr,"Unlocked !!! semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid); */
}
