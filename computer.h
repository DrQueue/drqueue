/* $Id: computer.h,v 1.3 2001/05/07 15:35:04 jorge Exp $ */

#ifndef _COMPUTER_H_
#define _COMPUTER_H_

#include <netinet/in.h>
#include <time.h>

#include "arch/computer_info.h"
#include "arch/computer_status.h"

struct computer {
  struct computer_hwinfo hwinfo;
  struct computer_status status;
  time_t lastconn;		/* Time of last connection to the master */
  unsigned char used;		/* If the record is being used or not */
};

int computer_index_addr (void *pwdb,struct in_addr addr);
int computer_index_name (void *pwdb,char *name);
int computer_index_free (void *pwdb);

#endif /* _COMPUTER_H_ */
