/* $Id: task.c,v 1.11 2002/06/20 15:28:48 jorge Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "slave.h"
#include "semaphores.h"

void task_init_all (struct task *task)
{
  int i;

  for (i=0;i < MAXTASKS; i++)
    task_init (&task[i]);
}

void task_init (struct task *task)
{
  task->used = 0;
  strcpy(task->jobname,"EMPTY");
  task->ijob = 0;
  strcpy(task->jobcmd,"NONE");
  strcpy(task->owner,"NOBODY");
  task->frame = 0;
  task->pid = 0;
  task->itask = 0;
  task->exitstatus = 0;
  task->status = 0;
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
      sdb->comp->status.task[i].itask = (uint16_t) i;
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
  printf ("Task index:\t%i\n",task->itask);
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
  default:
    st_string = "UNKNOWN";
  }

  return st_string;
}

void task_environment_set (struct task *task)
{
  static char padframe[BUFFERLEN];
  static char frame[BUFFERLEN];
  static char owner[BUFFERLEN];
  static char frame_start[BUFFERLEN];
  static char frame_end[BUFFERLEN];
  static char frame_step[BUFFERLEN];
  static char ijob[BUFFERLEN];

  /* Padded frame number */
  snprintf (padframe,BUFFERLEN,"PADFRAME=%04i",task->frame);
  putenv (padframe);
  /* Frame number */
  snprintf (frame,BUFFERLEN,"FRAME=%i",task->frame);
  putenv (frame);
  /* Owner of the job */
  snprintf (owner,BUFFERLEN-1,"OWNER=%s",task->owner);
  putenv (owner);
  /* Start, end and step frame numbers */
  snprintf (frame_start,BUFFERLEN-1,"STARTFRAME=%i",task->frame_start);
  putenv (frame_start);
  snprintf (frame_end,BUFFERLEN-1,"ENDFRAME=%i",task->frame_end);
  putenv (frame_end);
  snprintf (frame_step,BUFFERLEN-1,"STEPFRAME=%i",task->frame_step);
  putenv (frame_step);

  /* Job Index */
  snprintf (ijob,BUFFERLEN-1,"JOBID=%i",task->ijob);
  putenv (ijob);

  /* OS */
#ifdef __LINUX
  putenv ("DRQUEUE_OS=LINUX");
#else
  putenv ("DRQUEUE_OS=IRIX");
#endif

}
