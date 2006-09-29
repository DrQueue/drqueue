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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
// $Id$
//

#ifndef _SLAVE_H_
#define _SLAVE_H_

#include "computer.h"
#include "job.h"
#include "constants.h"

#include <limits.h>
#include <signal.h>
#include <stdint.h>

#if defined (__CYGWIN)
#define KEY_SLAVE "%s/slave.exe"/* Key file for shared memory and semaphores */
#else
#define KEY_SLAVE "%s/slave" /* Key file for shared memory and semaphores */
#endif

/* Each slave has a slave_database global variable that is local to each */
/* process. _Except_ the pointer to the computer struct that is a shared */
/* memory one */

#define SDBF_SETMAXCPUS (1<<0) // Set if we have to change the maximum number of CPUs at start.

struct slave_database {
  struct computer *comp;
  int64_t shmid;
  int64_t semid;
  struct computer_limits limits;
  uint16_t flags;
  char conf[PATH_MAX];
}
;    /* slave database */

extern int phantom[2];

void slave_get_options (int *argc,char ***argv, int *force, struct slave_database *sdb);
void usage (void);

void set_signal_handlers (void);
void set_signal_handlers_child_listening (void); /* For the accepting process */
void set_signal_handlers_child_chandler (void); /* Once accepted the connection */
void set_signal_handlers_child_launcher (void); /* For the child that forks and then keeps waiting */
void set_signal_handlers_task_exec (void); /* For the child of the previous that actually execs (and get substituted) */

// Limits
void slave_set_limits (struct slave_database *sdb);

// Signal stuff
void clean_out (int signal, siginfo_t *info, void *data);
void sigalarm_handler (int signal, siginfo_t *info, void *data);
void sigpipe_handler (int signal, siginfo_t *info, void *data);

void slave_listening_process (struct slave_database *sdb);
void slave_consistency_process (struct slave_database *sdb);
void launch_task (struct slave_database *sdb, uint16_t itask);

int64_t get_shared_memory_slave (int force);
int64_t get_semaphores_slave (void);
void *attach_shared_memory_slave (int64_t shmid);

void zerocmd (char *cmd);

#endif
