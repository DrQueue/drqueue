/* $Id: job.h,v 1.1 2001/05/02 16:12:33 jorge Exp $ */

#ifndef _JOB_H_
#define _JOB_H_

typedef enum {
  JOBSTATUS_ACTIVE,
  JOBSTATUS_STOPPED,
  JOBSTATUS_HSTOPPED,
  JOBSTATUS_DELETING
} t_jobstatus;

struct job {
  int used;
  char name[MAXNAMELEN];
  char jobcmd[MAXCMDLEN];
  char owner[MAXNAMELEN];
  t_jobstatus status;
};

#endif /* _JOB_H_ */
