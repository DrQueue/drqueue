/* $Id: task.h,v 1.17 2001/10/02 12:40:47 jorge Exp $ */

#ifndef _TASK_H_
#define _TASK_H_

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
# else
#  error You need to define the OS, or OS defined not supported
# endif
#endif

#include "constants.h"

#define DR_EXITEDFLAG (0x0100)
#define DR_SIGNALEDFLAG (0x0200)

#define DR_WIFEXITED(stat)   ((stat)&DR_EXITEDFLAG)
#define DR_WEXITSTATUS(stat) ((stat)&0xff)
#define DR_WIFSIGNALED(stat) ((stat)&DR_SIGNALEDFLAG)
#define DR_WTERMSIG(stat)    ((stat)&0xff)

typedef enum {
  TASKSTATUS_LOADING,		/* Assigned but not running yet */
  TASKSTATUS_RUNNING
} t_taskstatus;

struct task {
  uint8_t used;
  char jobname[MAXNAMELEN];	/* jobname */
  uint32_t ijob;		/* index to the job in the global db */
  uint16_t itask;		/* index to the task in the computer !not in the global db! */
  char jobcmd[MAXCMDLEN];	/* string that will be executed */
  char owner[MAXNAMELEN];	/* owner of the job */
  uint32_t frame;		/* current _real_ frame number (!!not index!!) */
  int32_t pid;			/* pid */
  int32_t exitstatus;		/* exit status */
  uint8_t status;		/* status */
};

struct slave_database;

void task_init_all (struct task *task);
void task_init (struct task *task);
int task_available (struct slave_database *sdb);
void task_report (struct task *task);
char *task_status_string (unsigned char status);
void task_environment_set (struct task *task);

#endif /* _TASK_H_ */




