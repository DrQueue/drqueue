/* $Id: computer.h,v 1.2 2001/05/02 16:12:32 jorge Exp $ */

#ifndef _COMPUTER_H_
#define _COMPUTER_H_

#include <time.h>

#include "arch/computer_info.h"
#include "arch/computer_status.h"

struct computer {
  struct computer_hwinfo hwinfo;
  struct computer_status status;
  time_t lastconn;		/* Time of last connection to the master */
};

#endif /* _COMPUTER_H_ */
