/* $Id: database.h,v 1.11 2001/11/16 15:48:32 jorge Exp $ */

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "constants.h"
#include "job.h"
#include "computer.h"

#define DB_VERSION 3		/* Database version. This version must change when we change the job structure */
#define DB_MAGIC   0xADDEEFBE	/* magic number */

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

struct database_hdr {		/* Database header for the saved database */
  uint32_t magic;		/* Magics number */
  uint32_t version;		/* Version number for the saved database */
  uint16_t job_size;		/* Number of total (used and empty) jobs in the jobs structure */
				/* We only save the jobs, because the computers' slaves need to be restarted */
};

void database_init (struct database *wdb);
int database_save (struct database *wdb);
int database_load (struct database *wdb);

#endif /* _DATABASE_H_ */
