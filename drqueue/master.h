/* $Id: master.h,v 1.3 2001/05/09 10:53:08 jorge Exp $ */

#ifndef _MASTER_H_
#define _MASTER_H_

#include "database.h"

int get_shared_memory (void);
int get_semaphores (void);
void *attach_shared_memory (int shmid);

void set_signal_handlers (void);
void set_signal_handlers_child_conn_handler (void);
void set_signal_handlers_child_cchecks (void);

void clean_out (int signal, siginfo_t *info, void *data);
void sigalarm_handler (int signal, siginfo_t *info, void *data);
void sigpipe_handler (int signal, siginfo_t *info, void *data);
void set_alarm (void);

void master_consistency_checks (struct database *wdb); /* Main consistency checks function */
void check_lastconn_times (struct database *wdb);


#endif /* _MASTER_H_ */
