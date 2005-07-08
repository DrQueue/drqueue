// 
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
// 
// $Id$
//

#ifndef _COMPUTER_H_
#define _COMPUTER_H_

#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>

#include "computer_info.h"
#include "computer_status.h"

// Autoenable flags
#define AEF_ACTIVE (1<<0)

struct pool {
	char name[MAXNAMELEN];
};

struct autoenable {				/* I put autoenable on limits even */
	time_t last;						/* Time of the last autoenable event happened */
	uint8_t h,m;						/* Hour and minute of wished autoenable */
	uint8_t flags;					// Autoenable flag
};

struct computer_limits {
	uint8_t enabled;					// Computer enabled for rendering
	uint16_t nmaxcpus;				/* Maximum number of cpus running */
	uint16_t maxfreeloadcpu;	/* Maximum load that a cpu can have to be considered free */
	struct autoenable autoenable;
	struct pool *pool;
	uint16_t npools;
	int poolshmid; // Pool's shared memory id
};

struct computer {
	struct computer_hwinfo hwinfo;
	struct computer_status status;
	struct computer_limits limits;
	time_t lastconn;		/* Time of last connection to the master */
	unsigned char used;		/* If the record is being used or not */
};

struct database;

int computer_index_addr (void *pwdb,struct in_addr addr); /* I use pointers to void instead to struct database */
int computer_index_name (void *pwdb,char *name);					/* because if I did I would have to create a dependency loop */
int computer_index_free (void *pwdb);
int computer_available (struct computer *computer);
int computer_ntasks (struct computer *comp);
int computer_ntasks_job (struct computer *comp,uint32_t ijob);
void computer_update_assigned (struct database *wdb,uint32_t ijob,int iframe,int icomp,int itask);
void computer_init (struct computer *computer);
int computer_free (struct computer *computer);
int computer_ncomputers_masterdb (struct database *wdb);
void computer_init_limits (struct computer *comp);
int computer_index_correct_master (struct database *wdb, uint32_t icomp);
void computer_autoenable_check (struct slave_database *sdb);

// Pools
int computer_pool_get_shared_memory (int npools);
struct pool *computer_pool_attach_shared_memory (int shmid);
void computer_pool_detach_shared_memory (struct pool *cpshp);
void computer_pool_init (struct computer_limits *cl);
int computer_pool_add (struct computer_limits *cl, char *pool);
void computer_pool_remove (struct computer_limits *cl, char *pool);
void computer_pool_list (struct computer_limits *cl);
int computer_pool_exists (struct computer_limits *cl,char *pool);
int computer_pool_free (struct computer_limits *cl);
void computer_pool_set_from_environment (struct computer_limits *cl);

#endif /* _COMPUTER_H_ */
