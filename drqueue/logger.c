/* $Id: logger.c,v 1.2 2001/04/26 16:06:22 jorge Exp $ */

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "logger.h"
#include "task.h"

void log_slave_task (struct t_task *task,char *msg)
{
  FILE *f_log;
  char name[MAXNAMELEN];
  char buf[BUFFERLEN];
  time_t now;
  
  if (gethostname (name,MAXNAMELEN-1) == -1) {
    strcpy (name,"UNKNOWN");
  }

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  f_log = log_open_task (task);
  fprintf (f_log,"%10s : %10s -> Job: %10s || Owner: %10s || Frame: %4i || Msg: %s\n",buf,name,
	   task->jobname,task->owner,task->frame,msg);

  fclose (f_log);
}

FILE *log_open_task (struct t_task *task)
{
  FILE *f;
  char filename[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_ROOT")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_ROOT not set. Aborting...\n");
    exit (1);
  }

  snprintf(filename,BUFFERLEN-1,"%s/logs/%s.log",basedir,task->jobname);
  
  if ((f = fopen (filename,"a")) == NULL) {
    perror ("log_open_task: Could't open file for writing");
    exit (1);
  }

  return f;
}
