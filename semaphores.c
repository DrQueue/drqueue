/* $Id: semaphores.c,v 1.1 2001/05/07 15:35:04 jorge Exp $ */

#include <sys/sem.h>

void lock_semaphore (int semid)
{
  struct sembuf op;

  op.sem_num = 0;
  op.sem_op = -1;
  op.sem_flg = SEM_UNDO;
  semop(semid,&op,1);
}

void release_semaphore (int semid)
{
  struct sembuf op;

  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = 0;
  semop(semid,&op,1);
}
