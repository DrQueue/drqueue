/* $Id: slave.h,v 1.4 2001/05/28 14:21:31 jorge Exp $ */

#ifndef _SLAVE_H_
#define _SLAVE_H_

#include <signal.h>

#include "computer.h"
#include "job.h"

/* Each slave has a slave_database global variable that is local to each */
/* process. _Except_ the pointer to the computer struct that is a shared */
/* memory one */

struct slave_database {
  struct computer *comp;
  struct job job;		/* Current process job */
  int shmid;
  int semid;
  int itask;			/* Index to current process task */
};				/* slave database */

void set_signal_handlers (void);
void set_signal_handlers_child_listening (void); /* For the accepting process */
void set_signal_handlers_child_chandler (void);	/* Once accepted the connection */

void clean_out (int signal, siginfo_t *info, void *data);
void sigalarm_handler (int signal, siginfo_t *info, void *data);
void sigpipe_handler (int signal, siginfo_t *info, void *data);

void slave_listening_process (struct slave_database *sdb);
void launch_task (struct slave_database *sdb);

int get_shared_memory_slave (void);
int get_semaphores_slave (void);
void *attach_shared_memory_slave (int shmid);

#endif
