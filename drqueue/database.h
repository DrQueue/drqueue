/* $Id: database.h,v 1.1 2001/05/02 16:12:33 jorge Exp $ */

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "constants.h"
#include "computer.h"
#include "job.h"

struct database {
  struct job job[MAXJOBS];	/* jobs */
  struct computer computer[MAXCOMPUTERS]; /* computers */
};

#endif /* _DATABASE_H_ */
