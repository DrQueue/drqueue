/* $Id: semaphores.h,v 1.2 2001/05/09 10:53:08 jorge Exp $ */

#ifndef _SEMAPHORES_H_
#define _SEMAPHORES_H_

void semaphore_lock (int semid);
void semaphore_release (int semid);

#endif /* _SEMAPHORES_H_ */
