/* $Id: master.h,v 1.1 2001/05/02 16:12:33 jorge Exp $ */

#ifndef _MASTER_H_
#define _MASTER_H_

int get_shared_memory (void);
int get_semaphores (void);
void *attach_shared_memory (int shmid);

void set_signal_handlers (void);
void set_signal_handlers_child (void);
void clean_out (int signal, siginfo_t *info, void *data);

int get_socket (void);

#endif /* _MASTER_H_ */
