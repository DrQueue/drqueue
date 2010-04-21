//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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

#ifndef _TASK_H_
#define _TASK_H_

#include "constants.h"

#include <sys/types.h>
#include <stdint.h>

#define DR_EXITEDFLAG (0x0100)
#define DR_SIGNALEDFLAG (0x0200)

#define DR_WIFEXITED(stat)  ((stat)&DR_EXITEDFLAG)
#define DR_WEXITSTATUS(stat) ((stat)&0xff)
#define DR_WIFSIGNALED(stat) ((stat)&DR_SIGNALEDFLAG)
#define DR_WTERMSIG(stat)   ((stat)&0xff)

// TODO: limit time LOADING and FINISHED
typedef enum {
  TASKSTATUS_LOADING,  /* Assigned but not running yet */
  TASKSTATUS_RUNNING,
  TASKSTATUS_FINISHED  // Finished but reporting to master still
} t_taskstatus;

#pragma pack(push,1)

struct task {
  uint8_t used;
  char jobname[MAXNAMELEN]; /* jobname */
  uint32_t ijob;  /* index to the job in the global db */
  uint32_t icomp;  // Index to the computer that renders this frame
  uint16_t itask;  /* index to the task in the computer !not in the global db! */
  char jobcmd[MAXCMDLEN]; /* string that will be executed */
  char owner[MAXNAMELEN]; /* owner of the job */
  uint32_t frame;  /* current _real_ frame number (!!not index!!) */
  uint32_t frame_start,frame_end;
  uint8_t  frame_pad;
  uint32_t frame_step;
  uint32_t block_size;
  int32_t  pid;   /* pid */
  int32_t  exitstatus;  /* exit status */
  uint8_t  status;  /* status */
  uint64_t start_loading_time;
};

struct slave_database;
struct job;

#pragma pack(pop)

void task_init_all (struct task *task);
void task_init (struct task *task);
uint16_t task_available (struct slave_database *sdb);
void task_report (struct task *task);
char *task_status_string (unsigned char status);
void task_environment_set (struct task *task);
int task_set_to_job_frame (struct task *task, struct job *job, uint32_t frame);
int task_is_running (struct task *task);

#endif /* _TASK_H_ */
