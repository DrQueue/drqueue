/* $Id: semaphores.c,v 1.4 2001/07/06 14:10:47 jorge Exp $ */

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

  fprintf (stderr,"Locking... semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid);

  if (semop(semid,&op,1) == -1) {
    perror ("semaphore_lock");
    printf ("%i\n",errno);
    kill(0,SIGINT);
  }
  fprintf (stderr,"Locked !!! semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid);
}

void semaphore_release (int semid)
{
  struct sembuf op;

  fprintf (stderr,"Unlocking... semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid);
  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = 0; /*  SEM_UNDO; */
  if (semop(semid,&op,1) == -1) {
    perror ("semaphore_release");
    kill(0,SIGINT);
  }
  fprintf (stderr,"Unlocked !!! semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid);
}
