/* $Id: task.c,v 1.6 2001/09/16 16:36:50 jorge Exp $ */

#include <stdio.h>
#include <stdlib.h>

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

void task_report (struct task *task)
{
  printf ("Job name:\t%s\n",task->jobname);
  printf ("Job index:\t%i\n",task->ijob);
  printf ("Job command:\t%s\n",task->jobcmd);
  printf ("Frame:\t\t%i\n",task->frame);
  printf ("Task pid:\t%i\n",task->pid);
  printf ("Task status:\t%s\n",task_status_string(task->status));
}

char *task_status_string (unsigned char status)
{
  char *st_string;
  switch (status) {
  case TASKSTATUS_LOADING:
    st_string = "Loading";
    break;
  case TASKSTATUS_RUNNING:
    st_string = "Running";
    break;
  case TASKSTATUS_STOPPED:
    st_string = "Stopped";
    break;
  case TASKSTATUS_KILLFRAME:
    st_string = "Kill frame";
    break;
  default:
    st_string = "UNKNOWN";
  }

  return st_string;
}

void task_environment_set (struct task *task)
{
  char msg[BUFFERLEN];

#ifdef __LINUX
  /* Padded frame number */
  snprintf (msg,BUFFERLEN,"%04i",task->frame);
  if (setenv("PADFRAME",msg,1) == -1) {
    printf ("ERROR\n");
    kill(0,SIGINT);
  }
  /* Frame number */
  snprintf (msg,BUFFERLEN,"%i",task->frame);
  if (setenv("FRAME",msg,1) == -1) {
    printf ("ERROR\n");
    kill(0,SIGINT);
  }
  /* OS */
  if (setenv("DRQUEUE_OS","LINUX",1) == -1) {
    printf ("ERROR\n");
    kill(0,SIGINT);
  }
#else
  /* Padded frame number */
  snprintf (msg,BUFFERLEN,"PADFRAME=%04i",task->frame);
  putenv (msg);
  /* Frame number */
  snprintf (msg,BUFFERLEN,"FRAME=%i",task->frame);
  putenv (msg);
  /* OS */
  putenv ("DRQUEUE_OS=IRIX");
#endif
}
