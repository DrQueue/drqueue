/* $Id: semaphores.c,v 1.2 2001/05/09 10:53:08 jorge Exp $ */

#include <sys/sem.h>

void semaphore_lock (int semid)
{
  struct sembuf op;

  op.sem_num = 0;
  op.sem_op = -1;
  op.sem_flg = SEM_UNDO;
  semop(semid,&op,1);
}

void semaphore_release (int semid)
{
  struct sembuf op;

  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = 0;
  semop(semid,&op,1);
}
