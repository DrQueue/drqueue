/* $Id: slave.h,v 1.2 2001/05/07 15:35:04 jorge Exp $ */

#ifndef _SLAVE_H_
#define _SLAVE_H_

#include "computer.h"

void set_signal_handlers (void);
void clean_out (int signal, siginfo_t *info, void *data);

void register_slave (struct computer *computer);

#endif
