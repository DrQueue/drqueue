/* $Id: task.h,v 1.1 2001/04/25 10:45:41 jorge Exp $ */

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
  char jobname[MAXNAMELEN];	/* jobname */
  int jobindex;			/* index to the job in the global db */
  char jobcmd[MAXCMDLEN];	/* string the is being executed */
  char owner[MAXNAMELEN];	/* owner of the job */
  int frame;			/* current frame */
  int pid;			/* pid */
  t_taskstatus status;		/* status */
};

#endif /* _TASK_H_ */
