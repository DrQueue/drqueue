/* $Id: task.h,v 1.3 2001/05/09 10:53:08 jorge Exp $ */

#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>

#include "constants.h"

typedef enum {
  TASKSTATUS_RUNNING,
  TASKSTATUS_SLEEPING,		/* Stopped (?) */
  TASKSTATUS_FINISHFRAME,
  TASKSTATUS_KILLFRAME
} t_taskstatus;

struct t_task {
  uint8_t used;
  char jobname[MAXNAMELEN];	/* jobname */
  uint16_t jobindex;		/* index to the job in the global db */
  char jobcmd[MAXCMDLEN];	/* string the is being executed */
  char owner[MAXNAMELEN];	/* owner of the job */
  uint16_t frame;		/* current frame */
  int32_t pid;			/* pid */
  uint8_t status;		/* status */
};

void init_tasks (struct t_task *task);

#endif /* _TASK_H_ */
