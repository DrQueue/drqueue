/* $Id: computer_status.h,v 1.7 2002/02/15 11:51:00 jorge Exp $ */

#ifndef _COMPUTER_STATUS_H_
#define _COMPUTER_STATUS_H_

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
# else
#  error You need to define the OS, or OS defined not supported
# endif
#endif

#include <time.h>

#include "constants.h"
#include "task.h"


struct computer_status {
  uint16_t loadavg[3];		/* load average last minute, last 5, and last 15 */
  uint16_t ntasks;		/* number of tasks (processes) being run at this time */
  struct task task[MAXTASKS];
};

void get_computer_status (struct computer_status *cstatus);
void computer_status_init (struct computer_status *cstatus);
void check_tasks (struct computer_status *cstatus);
void get_loadavg (uint16_t *loadavg);
void report_computer_status (struct computer_status *status);

#endif /* _COMPUTER_INFO_H_ */
