/* $Id: database.h,v 1.5 2001/09/20 10:51:33 jorge Exp $ */

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "constants.h"
#include "job.h"
#include "computer.h"

struct database {
  struct computer computer[MAXCOMPUTERS]; /* computers */
  struct job job[MAXJOBS];	/* jobs */
  int semid;			/* semaphores id */
  int shmid;			/* shared memory id */
#ifdef COMM_REPORT
  long int bsent;
  long int brecv;
#endif
};

void database_init (struct database *wdb);

#endif /* _DATABASE_H_ */
