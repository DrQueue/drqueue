/* $Id: logger.c,v 1.4 2001/05/30 15:11:47 jorge Exp $ */

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "logger.h"
#include "task.h"
#include "job.h"
#include "computer.h"

void log_slave_task (struct task *task,char *msg)
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

  f_log = log_slave_open_task (task);
  fprintf (f_log,"%10s : %10s -> Job: %10s || Owner: %10s || Frame: %4i || Msg: %s\n",buf,name,
	   task->jobname,task->owner,task->frame,msg);

  fclose (f_log);
}

FILE *log_slave_open_task (struct task *task)
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
    perror ("log_slave_open_task: Couldn't open file for writing");
    exit (1);
  }

  return f;
}

void log_slave_computer (char *msg)
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

  f_log = log_slave_open_computer (name);
  fprintf (f_log,"%10s : %10s -> Msg: %s\n",buf,name,msg);

  fclose (f_log);
}

FILE *log_slave_open_computer (char *name)
{
  FILE *f;
  char filename[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_ROOT")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_ROOT not set. Aborting...\n");
    exit (1);
  }

  snprintf(filename,BUFFERLEN-1,"%s/logs/%s.log",basedir,name);
  
  if ((f = fopen (filename,"a")) == NULL) {
    perror ("log_slave_open_computer: Couldn't open file for writing");
    exit (1);
  }

  return f;
}

void log_master_job (struct job *job,char *msg)
{
  FILE *f_log;
  char buf[BUFFERLEN];
  time_t now;
  
  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  f_log = log_master_open ();
  fprintf (f_log,"%10s : Job: %10s || Owner: %10s || Msg: %s\n",buf,job->name,job->owner,msg);
  fclose (f_log);
}

void log_master_computer (struct computer *computer,char *msg)
{
  FILE *f_log;
  char buf[BUFFERLEN];
  time_t now;
  
  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  f_log = log_master_open ();
  fprintf (f_log,"%10s : Computer: %10s || Msg: %s\n",buf,computer->hwinfo.name,msg);
  fclose (f_log);
}

void log_master (char *msg)
{
  FILE *f_log;
  char buf[BUFFERLEN];
  time_t now;
  
  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  f_log = log_master_open ();
  fprintf (f_log,"%10s : Msg: %s\n",buf,msg);
  fclose (f_log);
}

FILE *log_master_open (void)
{
  FILE *f;
  char filename[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_ROOT")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_ROOT not set. Aborting...\n");
    exit (1);
  }

  snprintf(filename,BUFFERLEN-1,"%s/logs/master.log",basedir);
  
  if ((f = fopen (filename,"a")) == NULL) {
    perror ("log_master_open: Couldn't open file for writing");
    exit (1);
  }

  return f;
}





