/* $Id: database.h,v 1.4 2001/06/05 12:19:45 jorge Exp $ */

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
};

void database_init (struct database *wdb);

#endif /* _DATABASE_H_ */
