/* $Id: task.h,v 1.5 2001/05/30 15:11:47 jorge Exp $ */

#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>

#include "constants.h"

typedef enum {
  TASKSTATUS_LOADING,		/* Assigned but not running yet */
  TASKSTATUS_RUNNING,
  TASKSTATUS_STOPPED,		/* Stopped (?) */
  TASKSTATUS_FINISHFRAME,
  TASKSTATUS_KILLFRAME
} t_taskstatus;

struct task {
  uint8_t used;
  char jobname[MAXNAMELEN];	/* jobname */
  uint16_t jobindex;		/* index to the job in the global db */
  char jobcmd[MAXCMDLEN];	/* string the is being executed */
  char owner[MAXNAMELEN];	/* owner of the job */
  uint32_t frame;		/* current frame */
  int32_t pid;			/* pid */
  uint8_t status;		/* status */
};

struct slave_database;

void init_tasks (struct task *task);
int task_available (struct slave_database *sdb);

#endif /* _TASK_H_ */
