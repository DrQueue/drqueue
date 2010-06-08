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

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include "task.h"
#include "job.h"
#include "computer.h"

#define L_ERROR         0
#define L_WARNING       1
#define L_INFO          2
#define L_DEBUG         3
#define L_DEBUG2        4
#define L_DEBUG3        5

#define L_LEVELMASK  0x00ff
#define L_OUTMASK    0xff00

#define L_ONSCREEN    1<<8

#define MAXLOGLINELEN 16384

typedef enum {
  DRQ_LOG_TOOL_UNKNOWN,
  DRQ_LOG_TOOL_MASTER,
  DRQ_LOG_TOOL_SLAVE,
  DRQ_LOG_TOOL_SLAVE_TASK
} logtooltype;

#define LAPP_MASTER 0
#define LAPP_SLAVE  1

extern int loglevel;
extern logtooltype logtool;
extern struct job *logger_job;
extern struct task *logger_task;
extern struct computer *logger_computer;

void log_auto (int level, char *fmt, ...);

char *log_level_str (int level);
void log_level_out_set (int outlevel);
void log_level_severity_set (int severity);

int log_dumptask_open (struct task *t);
int log_dumptask_open_ro (struct task *t);

#endif


