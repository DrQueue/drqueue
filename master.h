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
/* $Id$ */

#ifndef _MASTER_H_
#define _MASTER_H_

#include "database.h"

#define KEY_MASTER "%s/master"	/* Key for shared memory and semaphores */

#define MASTER_CONF_FILE "/etc/drqueue/master.conf"

extern int phantom[2];

void master_get_options (int *argc,char ***argv, int *force);
void usage (void);
void master_config_parse (char *cfg);

int get_shared_memory (int force);
int get_semaphores (int force);
void *attach_shared_memory (int shmid);

void set_signal_handlers (void);
void set_signal_handlers_child_conn_handler (void);
void set_signal_handlers_child_cchecks (void);

void clean_out (int signal, siginfo_t *info, void *data);
void sigalarm_handler (int signal, siginfo_t *info, void *data);
void sigpipe_handler (int signal, siginfo_t *info, void *data);
void sigsegv_handler (int signal, siginfo_t *info, void *data);
void set_alarm (void);

void master_consistency_checks (struct database *wdb); /* Main consistency checks function */
void check_lastconn_times (struct database *wdb);


#endif /* _MASTER_H_ */
