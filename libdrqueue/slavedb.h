//
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
//
// DrQueue is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// DrQueue is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
// $Id: /drqueue/remote/branches/0.65.x/slave.h 1813 2007-02-25T23:33:33.078183Z jorge  $
//

#ifndef _SLAVEDB_H_
#define _SLAVEDB_H_

#include "computer.h"

#include <limits.h>
#include <stdint.h>

#define KEY_SLAVE "slave" /* Key file for shared memory and semaphores */

/* Each slave has a slave_database global variable that is local to each */
/* process. _Except_ the pointer to the computer struct that is a shared */
/* memory one */

#define SDBF_SETMAXCPUS (1<<0) // Set if we have to change the maximum number of CPUs at start.

#pragma pack(push,1)

struct slave_database {
  struct computer *comp;
  int64_t shmid;
  int64_t semid;
  struct computer_limits limits;
  uint16_t flags;
  char conf[PATH_MAX];
};                                    /* slave database */

#pragma pack(pop)

#endif
