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

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(__LINUX)
# include <stdint.h>
#elif defined(__IRIX)
# include <sys/types.h>
# include <sys/sysget.h>
#elif defined(__OSX)
# include <stdint.h>
# include <string.h>
#elif defined(__FREEBSD)
#else
# error You need to define the OS, or OS defined not supported
#endif

#include "computer_status.h"
#include "task.h"
#include "logger.h"
#include "semaphores.h"

void get_computer_status (struct computer_status *cstatus, int semid)
{
  /* Get status not only gets the load average but also */
  /* checks that every task is running and in case they're not */
  /* it sets the task record to unused (used = 0) */
  get_loadavg (cstatus->loadavg);
  check_tasks (cstatus,semid);
}

void computer_status_init (struct computer_status *cstatus)
{
  cstatus->loadavg[0]=cstatus->loadavg[1]=cstatus->loadavg[2]=0;
  cstatus->ntasks = 0;
  task_init_all (cstatus->task);
}

void check_tasks (struct computer_status *cstatus, int semid)
{
  int i;

  semaphore_lock (semid);

  cstatus->ntasks = 0;
  for (i=0;i<MAXTASKS;i++) {
    if (cstatus->task[i].used) {
      if (cstatus->task[i].status != TASKSTATUS_LOADING) {
				/* If the task is LOADING then there is no process running yet */
				if (kill(cstatus->task[i].pid,0) == 0) { /* check if task is running */
					cstatus->ntasks++;
				} else {
					/* task is registered but not running */
					log_slave_task(&cstatus->task[i],L_WARNING,"Check tasks found no task where there should have been one.");
					cstatus->task[i].used = 0;
				}
      } else {
				cstatus->ntasks++;
      }
    }
  }

  semaphore_release (semid);
}

void get_loadavg (uint16_t *loadavg)
{
#if defined(__LINUX)			/* __LINUX  */
  FILE *f_loadavg;
  float a,b,c;
  
  if ((f_loadavg = fopen("/proc/loadavg","r")) == NULL) {
    perror ("get_loadavg: fopen");
    exit (1);
  }

  fscanf (f_loadavg,"%f %f %f",&a,&b,&c);
  
  loadavg[0] = a * 100;
  loadavg[1] = b * 100;
  loadavg[2] = c * 100;

  fclose (f_loadavg);


#elif defined(__IRIX)			/* __IRIX  */
  sgt_cookie_t cookie;
  uint32_t tla[3];

  SGT_COOKIE_SET_KSYM (&cookie,KSYM_AVENRUN);
  if (sysget (SGT_KSYM,(char *)tla,sizeof(uint32_t)*3,SGT_READ,&cookie) == -1) {
    FILE *uptime;
    char buf[BUFFERLEN];
    char *fd;			/* first digit */
    float f1,f2,f3;
    
    if ((uptime = popen ("/usr/bsd/uptime","r")) == NULL) {
      fprintf (stderr,"Warning: Problems executing '/usr/bsd/uptime'\n");
      tla[0] = tla[1] = tla[2] = 0;
    }

    while (fgets (buf,BUFFERLEN,uptime) != NULL) {
      if ((fd = strstr(buf,"average:")) != NULL) {
				while (!isdigit((int)*fd))
					fd++;
				if (sscanf (fd,"%f, %f, %f",&f1,&f2,&f3) != 3) {
					log_slave_computer (L_WARNING,"Problems on get_loadavg\n");
					f1 = f2 = f3 = 0;
				}
				tla[0] = f1 * 1000;
				tla[1] = f2 * 1000;
				tla[2] = f3 * 1000;
      }
    }
		
    pclose (uptime);
  }
  loadavg[0] = (uint16_t) (tla[0]/10);
  loadavg[1] = (uint16_t) (tla[1]/10);
  loadavg[2] = (uint16_t) (tla[2]/10);



#elif defined(__OSX)

  FILE *uptime;
  char buf[BUFFERLEN];
  char *fd;			/* first digit */
  float f1,f2,f3;
  
  if ((uptime = popen ("/usr/bin/uptime","r")) == NULL) {
    fprintf (stderr,"Warning: Problems executing '/usr/bin/uptime'\n");
    f1 = f2 = f3 = 0;
  }
  
  while (fgets (buf,BUFFERLEN,uptime) != NULL) {
    if ((fd = strstr(buf,"averages:")) != NULL) {
      while (!isdigit((int)*fd))
	fd++;
      if (sscanf (fd,"%f %f %f",&f1,&f2,&f3) != 3) {
	log_slave_computer (L_WARNING,"Problems on get_loadavg\n");
	f1 = f2 = f3 = 0;
      }
    }
  }
  
  loadavg[0] = f1 * 100;
  loadavg[1] = f2 * 100;
  loadavg[2] = f3 * 100;
  
  pclose (uptime);

#elif defined(__FREEBSD)

  double fls[3];
  if (getloadavg(fls,3)<3) {
    log_slave_computer (L_WARNING,"Problems on getloadavg\n");
    fls[0]=fls[1]=fls[2]=0.0;
  }
  loadavg[0] = (uint16_t) (fls[0] * 100);
  loadavg[1] = (uint16_t) (fls[1] * 100);
  loadavg[2] = (uint16_t) (fls[2] * 100);

#else
# error You need to define the OS, or OS defined not supported
#endif 
}


void report_computer_status (struct computer_status *status)
{
  int i;

  printf ("Load Average: %i %i %i\n",status->loadavg[0],status->loadavg[1],status->loadavg[2]);
  printf ("Number of tasks running: %i\n",status->ntasks);
  for (i=0;i<MAXTASKS;i++) {
    if (status->task[i].used) {
      printf ("\nTask record:\t%i\n",i);
      printf ("------------\n");
      task_report (&status->task[i]);
    }
  }
}

