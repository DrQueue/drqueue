/* $Id: slave.h,v 1.1 2001/05/02 16:12:33 jorge Exp $ */

#ifndef _SLAVE_H_
#define _SLAVE_H_

void set_signal_handlers (void);
void clean_out (int signal, siginfo_t *info, void *data);

#endif
