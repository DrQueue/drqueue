/* $Id: computer_status.h,v 1.1 2001/04/25 10:45:41 jorge Exp $ */

#ifndef _COMPUTER_STATUS_H_
#define _COMPUTER_STATUS_H_

#include "constants.h"
#include "task.h"

struct computer_status {
  int loadavg[3];		/* load average last minute, last 5, and last 15 */
  int numtasks;			/* number of tasks (processes) being run at this time */
  struct t_task task[MAXTASKS];
};

#endif /* _COMPUTER_INFO_H_ */
