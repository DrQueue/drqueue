/* $Id: logger.h,v 1.8 2001/08/07 13:00:33 jorge Exp $ */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>

#include "task.h"
#include "job.h"
#include "computer.h"

#define L_ERROR 0
#define L_WARNING 1
#define L_INFO 2
#define L_DEBUG 3

int loglevel = L_WARNING;

void log_slave_task (struct task *task, int level, char *msg);
FILE *log_slave_open_task (struct task *task);
void log_slave_computer (int level, char *msg);
FILE *log_slave_open_computer (char *name);

void log_master (int level, char *msg);
void log_master_job (struct job *job, int level, char *msg);
void log_master_computer (struct computer *computer, int level, char *msg);
FILE *log_master_open (void);

char *log_level_str (int level);

int log_dumptask_open (struct task *t);

#endif 


