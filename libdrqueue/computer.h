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

#ifndef _COMPUTER_H_
#define _COMPUTER_H_

#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <stdint.h>

#include "pointer.h"
#include "computer_info.h"
#include "computer_status.h"

#pragma pack (push,1)

// Autoenable flags
#define AEF_ACTIVE (1<<0)

struct pool;
struct database;

struct autoenable {   /* I put autoenable on limits even */
  uint32_t last;      /* Time of the last autoenable event happened */
  uint8_t  h,m;       /* Hour and minute of wished autoenable */
  uint8_t  flags;     // Autoenable flag
};

struct computer_limits {
  uint8_t enabled;     // Computer enabled for rendering
  uint16_t nmaxcpus;    /* Maximum number of cpus running */
  uint16_t maxfreeloadcpu; /* Maximum load that a cpu can have to be considered free */
  struct autoenable autoenable;
  uint16_t npools;
  int64_t poolshmid; // Pool's shared memory id
  int64_t poolsemid; // Pool's semaphore id
  uint32_t npoolsattached;
  fptr_type (struct pool,pool);
  fptr_type (struct pool,local_pool);
}; // __attribute__((__packed__)); // fails with swig but doesn't seem necessary

struct computer {
  struct computer_hwinfo hwinfo;
  struct computer_status status;
  struct computer_limits limits;
  uint32_t  lastconn;   /* Time of last connection to the master */
  uint8_t   used;       /* Is this computer record being used or not ? */
  int64_t   semid;      // Semaphore id for the computer
};

#pragma pack(pop)

int computer_index_addr (void *pwdb,struct in_addr addr); /* I use pointers to void instead to struct database */
int computer_index_name (void *pwdb,char *name);     /* because if I did I would have to create a dependency loop */
int computer_index_free (void *pwdb);
int computer_available (struct computer *computer);
uint16_t computer_ntasks (struct computer *comp);
uint16_t computer_nrunning (struct computer *comp);
uint16_t computer_nrunning_job (struct computer *comp,uint32_t ijob);
void computer_update_assigned (struct database *wdb, uint32_t ijob, uint32_t iframe, uint32_t icomp, uint16_t itask);
void computer_init (struct computer *computer);
int computer_free (struct computer *computer);
int computer_ncomputers_masterdb (struct database *wdb);
void computer_limits_init (struct computer_limits *cl); // This one should be used instead of the following
void computer_limits_cpu_init (struct computer *comp);
int computer_index_correct_master (struct database *wdb, uint32_t icomp);
void computer_autoenable_check (struct slave_database *sdb);

int computer_lock_check (struct computer *computer);
int computer_lock (struct computer *computer);
int computer_release (struct computer *computer);

int computer_attach (struct computer *computer);
int computer_detach (struct computer *computer);

void computer_limits_cleanup_received (struct computer_limits *cl);
void computer_limits_cleanup_to_send (struct computer_limits *cl);

#endif /* _COMPUTER_H_ */
