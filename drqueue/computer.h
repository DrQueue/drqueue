/* $Id: computer.h,v 1.12 2001/09/25 15:50:44 jorge Exp $ */

#ifndef _COMPUTER_H_
#define _COMPUTER_H_

#include <netinet/in.h>
#include <time.h>

#include "computer_info.h"
#include "computer_status.h"

struct computer_limits {
  uint16_t nmaxcpus;		/* Maximum number of cpus running */
  uint16_t maxfreeloadcpu;	/* Maximum load that a cpu can have to be considered free */
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
int computer_index_name (void *pwdb,char *name);          /* because if I did I would have to create a dependency loop */
int computer_index_free (void *pwdb);
int computer_available (struct computer *computer);
int computer_ntasks (struct computer *comp);
int computer_ntasks_job (struct computer *comp,uint32_t ijob);
void computer_update_assigned (struct database *wdb,uint32_t ijob,int iframe,int icomp,int itask);
void computer_init (struct computer *computer);
int computer_ncomputers_masterdb (struct database *wdb);
void computer_init_limits (struct computer *comp);
int computer_index_correct_master (struct database *wdb, uint32_t icomp);


#endif /* _COMPUTER_H_ */
