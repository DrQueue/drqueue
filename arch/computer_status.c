/* $Id: computer_status.c,v 1.2 2001/04/26 16:06:22 jorge Exp $ */

#include <stdio.h>
#include <signal.h>

#include "computer_status.h"
#include "task.h"
#include "logger.h"

void get_computerstatus (struct computer_status *cstatus)
{
  /* Get status not only gets the load average but also */
  /* checks that every task is running and in case they're not */
  /* it sets the task record to unused (used = 0) */
  get_loadavg (cstatus->loadavg);
  check_tasks (cstatus);
}

void init_computerstatus (struct computer_status *cstatus)
{
  init_tasks (cstatus->task);
}

void check_tasks (struct computer_status *cstatus)
{
  int i;

  cstatus->numtasks = 0;
  for (i=0;i<MAXTASKS;i++) {
    if (cstatus->task[i].used) {
      if (kill(cstatus->task[i].pid,0) == 0) { /* check if task is running */
	cstatus->numtasks++;
      } else {
	/* task is registered but not running */
	cstatus->task[i].used = 0;
      }
    }
  }
}

#ifdef __LINUX			/* __LINUX  */

void get_loadavg (int *loadavg)
{
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
}

#else
#error You need to define the OS, or OS defined not supported
#endif 







