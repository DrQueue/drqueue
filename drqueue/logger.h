/* $Id: logger.h,v 1.3 2001/05/02 16:12:33 jorge Exp $ */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include "task.h"
#include "job.h"
#include "computer.h"

void log_slave_task (struct t_task *task,char *msg);
FILE *log_slave_open_task (struct t_task *task);
void log_slave_computer (char *msg);
FILE *log_slave_open_computer (char *name);

void log_master (char *msg);
void log_master_job (struct job *job,char *msg);
void log_master_computer (struct computer *computer,char *msg);
FILE *log_master_open (void);

#endif 


