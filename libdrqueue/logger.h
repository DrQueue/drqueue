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

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "task.h"
#include "job.h"
#include "computer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#define L_ERROR 0
#define L_WARNING 1
#define L_INFO 2
#define L_DEBUG 3
#define L_DEBUG2 4
#define L_DEBUG3 5

typedef enum {
  DRQ_LOG_TOOL_UNKNOWN,
  DRQ_LOG_TOOL_MASTER,
  DRQ_LOG_TOOL_SLAVE,
  DRQ_LOG_TOOL_SLAVE_TASK
} logtooltype;

#define LAPP_MASTER 0
#define LAPP_SLAVE  1

extern int loglevel;
extern int logonscreen;
extern logtooltype logtool;

void log_auto (int level, char *fmt, ...);
void log_auto_job (struct job *job, int level, char *fmt, ...);

void log_slave_task (struct task *task,int level,char *fmt,...);
FILE *log_slave_open_task (struct task *task);
void log_slave_computer (int level, char *fmt, ...);
FILE *log_slave_open_computer (char *name);

void log_master (int level, char *fmt, ...);
void log_master_job (struct job *job, int level, char *fmt, ...);
void log_master_computer (struct computer *computer, int level, char *fmt, ...);
FILE *log_master_open (void);

char *log_level_str (int level);

int log_dumptask_open (struct task *t);
int log_dumptask_open_ro (struct task *t);

int log_job_path_get (uint32_t jobid,char *path,int pathlen);
int log_task_filename_get (struct task *task, char *path, int pathlen);

#endif


