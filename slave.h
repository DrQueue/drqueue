/* $Id: slave.h,v 1.3 2001/05/09 10:53:08 jorge Exp $ */

#ifndef _SLAVE_H_
#define _SLAVE_H_

#include "computer.h"

void set_signal_handlers (void);
void clean_out (int signal, siginfo_t *info, void *data);

int get_shared_memory_slave (void);
int get_semaphores_slave (void);
void *attach_shared_memory_slave (int shmid);


#endif
