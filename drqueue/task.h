/* $Id: task.h,v 1.2 2001/04/26 14:20:55 jorge Exp $ */

#ifndef _TASK_H_
#define _TASK_H_

#include "constants.h"

typedef enum {
  TASKSTATUS_RUNNING,
  TASKSTATUS_SLEEPING,		/* Stopped (?) */
  TASKSTATUS_FINISHFRAME,
  TASKSTATUS_KILLFRAME
} t_taskstatus;

struct t_task {
  int used;
  char jobname[MAXNAMELEN];	/* jobname */
  int jobindex;			/* index to the job in the global db */
  char jobcmd[MAXCMDLEN];	/* string the is being executed */
  char owner[MAXNAMELEN];	/* owner of the job */
  int frame;			/* current frame */
  int pid;			/* pid */
  t_taskstatus status;		/* status */
};

void init_tasks (struct t_task *task);

#endif /* _TASK_H_ */
