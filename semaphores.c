/* $Id: semaphores.c,v 1.3 2001/06/05 12:19:45 jorge Exp $ */

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
  if (semop(semid,&op,1) == -1) {
    perror ("semaphore_lock");
    printf ("%i\n",errno);
    kill(0,SIGINT);
  }
}

void semaphore_release (int semid)
{
  struct sembuf op;

  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = 0; /*  SEM_UNDO; */
  if (semop(semid,&op,1) == -1) {
    perror ("semaphore_release");
    kill(0,SIGINT);
  }
}
