/* $Id: master.h,v 1.2 2001/05/07 15:35:04 jorge Exp $ */

#ifndef _MASTER_H_
#define _MASTER_H_

#include "database.h"

int get_shared_memory (void);
int get_semaphores (void);
void *attach_shared_memory (int shmid);

void set_signal_handlers (void);
void set_signal_handlers_child (void);
void clean_out (int signal, siginfo_t *info, void *data);
void sigalarm_handler (int signal, siginfo_t *info, void *data);
void sigpipe_handler (int signal, siginfo_t *info, void *data);
void set_alarm (void);

void handle_request (int csfd,struct database *wdb,int icomp);

#endif /* _MASTER_H_ */
