/* $Id: job.c,v 1.2 2001/05/30 15:11:47 jorge Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "job.h"
#include "database.h"
#include "constants.h"
#include "slave.h"
#include "logger.h"
#include "semaphores.h"

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

void job_init_assigned (struct database *wdb,int ijob)
{
  int i;
  int nframes;
  
  semaphore_lock(wdb->semid);
  wdb->job[ijob].used = 1;
  wdb->job[ijob].status = JOBSTATUS_WAITING;

  /* We allocate the memory for the frame_info */
  nframes = job_nframes (&wdb->job[index]);
  job[index].frame_info = (struct frame_info *) malloc (sizeof (struct frame_info) * nframes);
  if (wdb->job[index].frame_info == NULL) {
    log_master ("Warning: Could not allocate memory for frame info");
    wdb->job[index].used = 0;
  } else {
    job_init_assigned (&wdb->job[index]);
  }

  /* Set done frames to NONE */
  for (i=0;i<job_nframes(job);i++) {
    job->frame_info[i].status = FS_WAITING;
  }
}

void job_delete (struct job *job)
{
  job->used = 0;

  if (job->frame_info)
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


int job_available (struct database *wdb,int ijob, int *iframe)
{
  if (wdb->job[ijob].used == 0)
    return 0;

  if (wdb->job[ijob].status != JOBSTATUS_WAITING)
    return 0;

  if ((*iframe = job_first_frame_available (wdb,ijob)) == -1)
    return 0;

  return 1;
}

int job_first_frame_available (struct database *wdb,int ijob)
{
  /* This fuction not only returns the first frame */
  /* available but also updates the job structure when found */
  /* so the frame status goes to assigned (we still have to */
  /* set the info about the icomp,start,itask */
  int i;
  int nframes = job_nframes (&wdb->job[ijob]);
  int r = -1;

  semaphore_lock(wdb->semid);
  for (i=0;i<nframes;i++) {
    if (wdb->job[ijob].frame_info[i].status == FS_WAITING) {
      r = i;			/* return = current */
      wdb->job[ijob].frame_info[i].status = FS_ASSIGNED; /* Change the status to assigned */
      break;
    }
  }
  semaphore_release(wdb->semid);

  return r;
}

void job_update_assigned (struct database *wdb, int ijob, int iframe, int icomp, int itask)
{
  semaphore_lock(wdb->semid);
  wdb->job[ijob].frame_info[i].icomp = icomp;
  wdb->job[ijob].frame_info[i].itask = itask;
}

