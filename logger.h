/* $Id: logger.h,v 1.2 2001/04/26 16:06:22 jorge Exp $ */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include "task.h"

void log_slave_task (struct t_task *task,char *msg);
FILE *log_open_task (struct t_task *task);

#endif 


