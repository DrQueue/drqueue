/* $Id: computer_status.h,v 1.3 2001/05/09 10:53:08 jorge Exp $ */

#ifndef _COMPUTER_STATUS_H_
#define _COMPUTER_STATUS_H_

#include <stdint.h>

#include "constants.h"
#include "task.h"

struct computer_status {
  uint16_t loadavg[3];		/* load average last minute, last 5, and last 15 */
  uint16_t numtasks;			/* number of tasks (processes) being run at this time */
  struct t_task task[MAXTASKS];
};

void get_computer_status (struct computer_status *cstatus);
void init_computer_status (struct computer_status *cstatus);
void check_tasks (struct computer_status *cstatus);
void get_loadavg (uint16_t *loadavg);
void report_computer_status (struct computer_status *status);

#endif /* _COMPUTER_INFO_H_ */
