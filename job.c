/* $Id: job.c,v 1.1 2001/05/28 14:21:31 jorge Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "job.h"
#include "database.h"
#include "constants.h"
#include "slave.h"
#include "logger.h"

int job_index_free (void *pwdb)
{
  /* Return the index to a free job record OR -1 if there */
  /* are no more free records */
  int index = -1;
  int i;
  struct database *wdb = (struct database *)pwdb;

  for (i=0; i<MAXJOBS; i++) {
    if (wdb->job[i].used == 0) {
      index = i;
      break;
    }
  }

  return index;
}

void job_report (struct job *job)
{
  printf ("JOB Report\n");
  printf ("Name:\t\t\t%s\n",job->name);
  printf ("Status:\t\t\t%s\n",job_status_string(job->status));
  printf ("Owner:\t\t\t%s\n",job->owner);
  printf ("Command:\t\t%s\n",job->cmd);
  printf ("Frame start,end:\t%i,%i\n",job->frame_start,job->frame_end);
}

void job_init (struct job *job)
{
  int i;

  job->status = JOBSTATUS_WAITING;

  /* Set done frames to NONE */
  for (i=0;i<job_nframes(job);i++) {
    job->frame_info[i].status = FS_WAITING;
  }
}

void job_delete (struct job *job)
{
  job->used = 0;

  free (job->frame_info);
  job->frame_info = NULL;
}

char *job_status_string (char status)
{
  static char sstring[BUFFERLEN];

  sstring[MAXCMDLEN-1] = 0;
  switch (status) {
  case JOBSTATUS_WAITING:
    strncpy (sstring,"Waiting",BUFFERLEN-1);
    break;
  case JOBSTATUS_ACTIVE:
    strncpy (sstring,"Active",BUFFERLEN-1);
    break;
  case JOBSTATUS_STOPPED:
    strncpy (sstring,"Stopped",BUFFERLEN-1);
    break;
  case JOBSTATUS_HSTOPPED:
    strncpy (sstring,"Hard Stopped",BUFFERLEN-1);
    break;
  case JOBSTATUS_DELETING:
    strncpy (sstring,"Deleting",BUFFERLEN-1);
    break;
  default:
    strncpy (sstring,"DEFAULT (?!)",BUFFERLEN-1);
    fprintf (stderr,"job_status == DEFAULT\n");
    exit (1);
  }

  return sstring;
}
int job_nframes (struct job *job)
{
  int n;

  n = job->frame_start - job->frame_end;
  n = (n<0) ? -n : n;
  n++;

  return n;
}
