//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//

#ifndef _DATABASE_H_
#define _DATABASE_H_

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "constants.h"
#include "job.h"
#include "computer.h"

#pragma pack(push,1)

#define DB_VERSION 10          // Database version. This version must change when we change the job structure
#define DB_MAGIC   0xADDEEFBE  // magic number

struct load_balancing {
  struct tpol pol[MAXJOBS]; // Priority ordered list of jobs
  uint32_t next_i;          // Variables used for load balancing
  uint32_t last_priority;   //
};

struct database {
  struct computer computer[MAXCOMPUTERS]; /* computers */
  struct job job[MAXJOBS]; /* jobs */
  struct load_balancing lb; // Load balancing info
  int64_t semid;   /* semaphores id */
  int64_t shmid;   /* shared memory id */

#ifdef COMM_REPORT
  uint64_t bsent;
  uint64_t brecv;
#endif
};

struct database_hdr {  // Database header for the saved database
  uint32_t magic;      // Magic number
  uint32_t version;    // Version number for the saved database
  uint16_t job_size;   // Number of total (used and empty) jobs in the jobs structure
                       // We only store the job list because
                       // computers will have to restart and
                       // reregister
};

#pragma pack(pop)

void database_init (struct database *wdb);
int database_save (struct database *wdb);
int database_load (struct database *wdb);
int database_backup (struct database *wdb);
uint32_t database_version_id (void);

int database_job_load_frames (int sfd, struct job *job);
int database_job_save_frames (int sfd, struct job *job);
int database_job_load_blocked_hosts (int sfd, struct job *job);
int database_job_save_blocked_hosts (int sfd, struct job *job);
int database_job_load_envvars (int sfd, struct job *job);
int database_job_save_envvars (int sfd, struct job *job);

/* new but experimental */
//static int write_32b (int sfd, void *data);
//static int write_16b (int sfd, void *data);
//static int read_32b (int sfd, void *data);
//static int read_16b (int sfd, void *data);
static int dr_file_read (int fd, char *buf, uint32_t len);
static int dr_file_write (int fd, char *buf, uint32_t len);

#endif /* _DATABASE_H_ */
