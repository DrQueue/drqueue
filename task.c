/* $Id: task.c,v 1.2 2001/05/30 15:11:47 jorge Exp $ */

#include "task.h"
#include "slave.h"
#include "semaphores.h"

void init_tasks (struct task *task)
{
  int i;

  for (i=0;i < MAXTASKS; i++)
    task[i].used = 0;
}

int task_available (struct slave_database *sdb)
{
  int i;
  int r = -1;

  semaphore_lock(sdb->semid);
  for (i=0;i<MAXTASKS;i++) {
    if (sdb->comp->status.task[i].used == 0) {
      r = i;
      sdb->comp->status.task[i].used = 1;
      sdb->comp->status.task[i].status = TASKSTATUS_LOADING;
      break;
    }
  }
  semaphore_release(sdb->semid);

  return r;
}
