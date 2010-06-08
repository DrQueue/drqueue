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

#ifndef _COMPUTER_STATUS_H_
#define _COMPUTER_STATUS_H_

#include "constants.h"
#include "task.h"

#include <time.h>
#include <sys/types.h>
#include <stdint.h>

#pragma pack(push,1)

struct computer_status {
  uint16_t loadavg[3];  /* load average last minute, last 5, and last 15 */
  uint16_t ntasks;      // number of tasks (processes) being used
  uint16_t nrunning;    // number of tasks on execution or loading
  struct task task[MAXTASKS];
};

#pragma pack(pop)

void get_computer_status (struct computer_status *cstatus, int64_t semid);
void computer_status_init (struct computer_status *cstatus);
void check_tasks (struct computer_status *cstatus, int64_t semid);
void get_loadavg (uint16_t *loadavg);
void report_computer_status (struct computer_status *status);

#endif /* _COMPUTER_INFO_H_ */
