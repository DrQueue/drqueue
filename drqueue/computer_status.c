/* $Id: computer_status.c,v 1.8 2001/07/23 10:29:23 jorge Exp $ */

#include <stdio.h>
#include <signal.h>

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
#include <sys/sysget.h>
# else
#  error You need to define the OS, or OS defined not supported
# endif
#endif

#include "computer_status.h"
#include "task.h"
#include "logger.h"

void get_computer_status (struct computer_status *cstatus)
{
  /* Get status not only gets the load average but also */
  /* checks that every task is running and in case they're not */
  /* it sets the task record to unused (used = 0) */
  get_loadavg (cstatus->loadavg);
  check_tasks (cstatus);
}

void init_computer_status (struct computer_status *cstatus)
{
  init_tasks (cstatus->task);
}

void check_tasks (struct computer_status *cstatus)
{
  int i;

  cstatus->ntasks = 0;
  for (i=0;i<MAXTASKS;i++) {
    if (cstatus->task[i].used) {
      if (cstatus->task[i].status != TASKSTATUS_LOADING) {
	/* If the task is LOADING then there is no process running yet */
	if (kill(cstatus->task[i].pid,0) == 0) { /* check if task is running */
	  cstatus->ntasks++;
	} else {
	  /* task is registered but not running */
	  cstatus->task[i].used = 0;
	}
      } else {
	cstatus->ntasks++;
      }
    }
  }
}

#ifdef __LINUX			/* __LINUX  */
void get_loadavg (uint16_t *loadavg)
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
# ifdef __IRIX
void get_loadavg (uint16_t *loadavg)
{
  sgt_cookie_t cookie;
  uint32_t tla[3];

  SGT_COOKIE_SET_KSYM (&cookie,KSYM_AVENRUN);
  sysget (SGT_KSYM,(char *)tla,sizeof(uint32_t)*3,SGT_READ,&cookie);
  loadavg[0] = (uint16_t) (tla[0]/10);
  loadavg[1] = (uint16_t) (tla[1]/10);
  loadavg[2] = (uint16_t) (tla[2]/10);
}
# else
#  error You need to define the OS, or OS defined not supported
# endif
#endif 

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






