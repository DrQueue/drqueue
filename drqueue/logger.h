/* $Id: logger.h,v 1.5 2001/05/30 15:11:47 jorge Exp $ */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>

#include "task.h"
#include "job.h"
#include "computer.h"

#define L_CURRENT 2

#define L_ERROR 0
#define L_WARNING 1
#define L_INFO 2
#define L_DEBUG 3

void log_slave_task (struct task *task,char *msg);
FILE *log_slave_open_task (struct task *task);
void log_slave_computer (char *msg);
FILE *log_slave_open_computer (char *name);

void log_master (char *msg);
void log_master_job (struct job *job,char *msg);
void log_master_computer (struct computer *computer,char *msg);
FILE *log_master_open (void);

#endif 


