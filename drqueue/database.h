/* $Id: database.h,v 1.3 2001/05/30 15:11:47 jorge Exp $ */

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "constants.h"
#include "job.h"
#include "computer.h"

struct database {
  struct job job[MAXJOBS];	/* jobs */
  struct computer computer[MAXCOMPUTERS]; /* computers */
  int semid;			/* semaphores id */
  int shmid;			/* shared memory id */
};

void database_init (struct database *wdb);

#endif /* _DATABASE_H_ */
