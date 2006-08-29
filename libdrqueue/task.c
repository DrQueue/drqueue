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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "slave.h"
#include "semaphores.h"
#include "logger.h"
#include "request.h"
#include "drerrno.h"


void task_init_all (struct task *task) {
  int i;
  for (i=0;i < MAXTASKS; i++)
    task_init (&task[i]);
}

void task_init (struct task *task) {
  task->used = 0;
  strcpy(task->jobname,"EMPTY");
  task->ijob = 0;
  task->icomp = 0;
  strcpy(task->jobcmd,"NONE");
  strcpy(task->owner,"NOBODY");
  task->frame = 0;
  task->frame_pad = 4;

  task->pid = 0;
  task->itask = 0;
  task->exitstatus = 0;
  task->status = 0;
}

int task_available (struct slave_database *sdb) {
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

void task_report (struct task *task) {
  printf ("Job name:\t%s\n",task->jobname);
  printf ("Job index:\t%i\n",task->ijob);
  printf ("Job command:\t%s\n",task->jobcmd);
  printf ("Frame:\t\t%i\n",task->frame);
  printf ("Task pid:\t%i\n",task->pid);
  printf ("Task index:\t%i\n",task->itask);
  printf ("Task status:\t%s\n",task_status_string(task->status));
}

char *task_status_string (unsigned char status) {
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

void task_environment_set (struct task *task) {
  static char padformat[BUFFERLEN];
  static char padframe[BUFFERLEN];
  static char padframes[BUFFERLEN];
  static char frame[BUFFERLEN];
  static char owner[BUFFERLEN];
  static char frame_start[BUFFERLEN];
  static char frame_end[BUFFERLEN];
  static char frame_step[BUFFERLEN];
  static char block_size[BUFFERLEN];
  static char ijob[BUFFERLEN];
  static char icomp[BUFFERLEN];

  /* Padded frame number */
  /* TODO: make padding length user defined */
  snprintf (padformat,BUFFERLEN-1,"DRQUEUE_PADFRAME=%%0%uu",task->frame_pad);
  snprintf (padframe,BUFFERLEN-1,padformat,task->frame);
  putenv (padframe);

  //create a variable with a space delimited padded frames list
  int i;
  int block_end = task->frame+task->block_size;
  if (block_end > task->frame_end + 1) {
    block_end = task->frame_end + 1;
  }
  snprintf (padformat,BUFFERLEN-1,"DRQUEUE_PADFRAMES=%%0%uu",task->frame_pad);
  snprintf (padframes,BUFFERLEN-1,padformat,task->frame);
  for (i=task->frame+1; i<block_end; i++) {
    snprintf (padformat,BUFFERLEN-1,"%s %%0%uu",padframes,task->frame_pad);
    snprintf (padframes,BUFFERLEN-1,padformat,i);
  }
  //snprintf (padformat,BUFFERLEN-1,"%s\"",padframes);
  putenv (padframes);

  /* Frame number */
  snprintf (frame,BUFFERLEN-1,"DRQUEUE_FRAME=%u",task->frame);
  putenv (frame);
  /* Owner of the job */
  snprintf (owner,BUFFERLEN-1,"DRQUEUE_OWNER=%s",task->owner);
  putenv (owner);
  /* Start, end and step frame numbers */
  snprintf (frame_start,BUFFERLEN-1,"DRQUEUE_STARTFRAME=%u",task->frame_start);
  putenv (frame_start);
  snprintf (frame_end,BUFFERLEN-1,"DRQUEUE_ENDFRAME=%u",task->frame_end);
  putenv (frame_end);
  snprintf (frame_step,BUFFERLEN-1,"DRQUEUE_STEPFRAME=%u",task->frame_step);
  putenv (frame_step);
  /* Block size */
  snprintf (block_size,BUFFERLEN-1,"DRQUEUE_BLOCKSIZE=%u",task->block_size);
  putenv (block_size);

  /* Job Index */
  snprintf (ijob,BUFFERLEN-1,"DRQUEUE_JOBID=%i",task->ijob);
  putenv (ijob);
  // Computer Index
  snprintf (icomp,BUFFERLEN-1,"DRQUEUE_COMPID=%i",task->icomp);
  putenv (icomp);

  /* OS */
#if defined(__LINUX)

  putenv ("DRQUEUE_OS=LINUX");
#elif defined(__FREEBSD)

  putenv ("DRQUEUE_OS=FREEBSD");
#elif defined(__OSX)

  putenv ("DRQUEUE_OS=OSX");
#elif defined(__CYGWIN)

  putenv ("DRQUEUE_OS=WINDOWS");
#else

  putenv ("DRQUEUE_OS=IRIX");
#endif


  // Job specific environment variables
  struct envvars envvars;
  envvars_init(&envvars);
  if (!request_job_envvars (task->ijob,&envvars,SLAVE_LAUNCHER)) {
    log_slave_task (task,L_WARNING,"Could not receive job environment variables");
    return;
  }
  log_slave_task (task,L_DEBUG,"Received %i environment variables",envvars.nvariables);
  char *buffer;
  if (!envvars_attach(&envvars)) {
    log_slave_task (task,L_WARNING,"Custom environment variables could not be attached. (%s)",drerrno_str());
  } else {
    for (i = 0; i < envvars.nvariables; i++) {
      buffer = (char *) malloc (BUFFERLEN);
      snprintf (buffer,BUFFERLEN,"%s=%s",envvars.variables[i].name,envvars.variables[i].value);
      log_slave_task (task,L_DEBUG,"Putting \"%s\" in the environment",buffer);
      putenv (buffer);
    }
    envvars_detach(&envvars);
  }
  envvars_empty(&envvars);
}
