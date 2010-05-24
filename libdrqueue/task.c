//
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
// Copyright (C) 2010 Andreas Schroeder
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "libdrqueue.h"


void task_init_all (struct task *task) {
  int i;
  for (i=0;i < MAXTASKS; i++)
    task_init (&task[i]);
}

void task_init (struct task *task) {
  if (!task) {
    return;
  }
  memset(task,0,sizeof(*task));
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
  task->start_loading_time = 0;
}

uint16_t
task_available (struct slave_database *sdb) {
  int i;
  uint16_t r = -1;

  semaphore_lock(sdb->semid);
  for (i=0;i<MAXTASKS;i++) {
    if (sdb->comp->status.task[i].used == 0) {
      r = i;
      sdb->comp->status.task[i].used = 1;
      sdb->comp->status.task[i].itask = (uint16_t) i;
      sdb->comp->status.task[i].status = TASKSTATUS_LOADING;
      sdb->comp->status.task[i].start_loading_time = time(NULL);
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
  case TASKSTATUS_FINISHED:
    st_string = "Finished";
    break;
  default:
    st_string = "UNKNOWN";
  }

  return st_string;
}

int
task_is_running (struct task *task) {
  int running = 1;
  
  if (!task) {
    log_auto (L_ERROR,"task_is_running(): received NULL pointer as task.");
    return 0;
  }
  if (!task->used) {
    return 0;
  } else if (task->status == TASKSTATUS_FINISHED) {
    return 0;
  } else {
    // task->used == 1 && task->status != TASKSTATUS_FINISHED
    switch (task->status) {
    case TASKSTATUS_RUNNING:
    case TASKSTATUS_LOADING:
      break;
    default:
      logger_task = task;
      log_auto (L_WARNING,"task_is_running(): unknown task status %u (Str: %s)",
		task->status,task_status_string(task->used));
      logger_task = NULL;      
    }
  }
  
  return running;
}

void
task_environment_set (struct task *task) {
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
  static char jobname[BUFFERLEN];

  /* Padded frame number */
  /* TODO: make padding length user defined */
  snprintf (padframe,BUFFERLEN,"DRQUEUE_PADFRAME=%0*i",task->frame_pad,task->frame);
  putenv (padframe);

  //create a variable with a space delimited padded frames list
  uint32_t i;
  uint32_t block_end = task->frame+task->block_size;
  if (block_end > task->frame_end + 1) {
    block_end = task->frame_end + 1;
  }

  snprintf (padformat,BUFFERLEN,"DRQUEUE_PADFRAMES=%0*i",task->frame_pad,task->frame);
  snprintf (padframes,BUFFERLEN,"%s",padformat);
  for (i=task->frame+1; i<block_end; i++) {
    snprintf (padformat,BUFFERLEN,"%s %0*i",padframes,task->frame_pad,i);
    snprintf (padframes,BUFFERLEN,"%s",padformat);
  }
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

  // Job name
  snprintf (jobname,BUFFERLEN-1,"DRQUEUE_JOBNAME=%s",task->jobname);
  putenv (jobname);
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
    log_auto (L_WARNING,"Could not receive job environment variables");
    return;
  }
  log_auto (L_DEBUG,"Received %u environment variables",envvars.nvariables);
  char *buffer;
  if (!envvars_attach(&envvars)) {
    if (envvars.nvariables > 0) {
      log_auto (L_WARNING,"Custom environment variables could not be attached. There should be %i available. (%s)",
		envvars.nvariables,strerror(drerrno_system));
    }
  } else {
    for (i = 0; i < envvars.nvariables; i++) {
      buffer = (char *) malloc (BUFFERLEN);
      snprintf (buffer,BUFFERLEN,"%s=%s",envvars.variables.ptr[i].name,envvars.variables.ptr[i].value);
      log_auto (L_DEBUG,"Putting \"%s\" in the environment",buffer);
      putenv (buffer);
    }
    envvars_detach(&envvars);
  }
  envvars_free(&envvars);
}

int
task_set_to_job_frame (struct task *task, struct job *job, uint32_t frame) {
  if (!task) {
    return 0;
  }
  if (!job) {
    return 0;
  }
  task_init(task);
  strncpy(task->jobname,job->name,MAXNAMELEN-1);
  task->ijob = job->id;
  strncpy(task->jobcmd,job->cmd,MAXCMDLEN-1);
  strncpy(task->owner,job->owner,MAXCMDLEN-1);
  task->frame = frame;
  task->frame_start = job->frame_start;
  task->frame_end = job->frame_end;
  task->frame_step = job->frame_step;
  task->frame_pad = job->frame_pad;
  task->block_size= job->block_size;
  return 1;
}
