/* $Id: semaphores.h,v 1.1 2001/05/07 15:35:04 jorge Exp $ */

#ifndef _SEMAPHORES_H_
#define _SEMAPHORES_H_

void lock_semaphore (int semid);
void release_semaphore (int semid);

#endif /* _SEMAPHORES_H_ */
