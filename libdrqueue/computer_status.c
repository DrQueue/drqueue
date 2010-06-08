//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
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
# include <stdint.h>
#elif defined(__CYGWIN)
#else
# error You need to define the OS, or OS defined not supported
#endif

#include "computer_status.h"
#include "task.h"
#include "logger.h"
#include "semaphores.h"

void get_computer_status (struct computer_status *cstatus, int64_t semid) {
  /* Get status not only gets the load average but also */
  /* checks that every task is running and in case they're not */
  /* it sets the task record to unused (used = 0) */
#ifdef __CYGWIN
  if (cstatus->ntasks > 0)
    cstatus->loadavg[0] = 1;
  else
    cstatus->loadavg[0] = 0;
#else

  get_loadavg (cstatus->loadavg);
#endif

  check_tasks (cstatus,semid);
}

void computer_status_init (struct computer_status *cstatus) {
  cstatus->loadavg[0]=cstatus->loadavg[1]=cstatus->loadavg[2]=0;
  cstatus->ntasks = 0;
  cstatus->nrunning = 0;
  task_init_all (cstatus->task);
}

void check_tasks (struct computer_status *cstatus, int64_t semid) {
  int i;

  semaphore_lock (semid);

  cstatus->ntasks = 0;
  for (i=0;i<MAXTASKS;i++) {
    if (cstatus->task[i].used) {
      if (cstatus->task[i].status == TASKSTATUS_RUNNING) {
        /* If the task is LOADING then there is no process running yet */
        if (kill(cstatus->task[i].pid,0) == 0) { /* check if task is running */
          cstatus->ntasks++;
        } else {
          /* task is registered but not running */
          log_auto(L_WARNING,"Check tasks found no task where there should have been one.");
          cstatus->task[i].used = 0;
        }
      } else {
        // TODO: LOADING or FINISHED ?
        cstatus->ntasks++;
      }
    }
  }

  semaphore_release (semid);
}

void get_loadavg (uint16_t *loadavg) {
#if defined(__LINUX)   /* __LINUX */
  FILE *f_loadavg;
  float a,b,c;

  if ((f_loadavg = fopen("/proc/loadavg","r")) == NULL) {
    perror ("get_loadavg: fopen");
    exit (1);
  }

  fscanf (f_loadavg,"%f %f %f",&a,&b,&c);

  loadavg[0] = (uint16_t) (a * 100);
  loadavg[1] = (uint16_t) (b * 100);
  loadavg[2] = (uint16_t) (c * 100);

  fclose (f_loadavg);


#elif defined(__IRIX)   /* __IRIX  */

  sgt_cookie_t cookie;
  uint32_t tla[3];

  SGT_COOKIE_SET_KSYM (&cookie,KSYM_AVENRUN);
  if (sysget (SGT_KSYM,(char *)tla,sizeof(uint32_t)*3,SGT_READ,&cookie) == -1) {
    FILE *uptime;
    char buf[BUFFERLEN];
    char *fd;   /* first digit */
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
          log_auto (L_WARNING,"Problems on get_loadavg\n");
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

#elif defined(__OSX) || defined(__FREEBSD)

  double fls[3];
  if (getloadavg(fls,3)<3) {
    log_auto (L_WARNING,"Problems on getloadavg\n");
    fls[0]=fls[1]=fls[2]=0.0;
  }
  loadavg[0] = (uint16_t) (fls[0] * 100);
  loadavg[1] = (uint16_t) (fls[1] * 100);
  loadavg[2] = (uint16_t) (fls[2] * 100);

#elif defined(__CYGWIN)   /* __CYGWIN  */

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

#else
# error You need to define the OS, or OS defined not supported
#endif
}


void report_computer_status (struct computer_status *status) {
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

