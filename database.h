/* $Id: database.h,v 1.2 2001/05/07 15:35:04 jorge Exp $ */

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "constants.h"
#include "computer.h"
#include "job.h"

struct database {
  struct job job[MAXJOBS];	/* jobs */
  struct computer computer[MAXCOMPUTERS]; /* computers */
  int semid;			/* semaphores id */
  int shmid;			/* shared memory id */
};

#endif /* _DATABASE_H_ */
