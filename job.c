/* $Id: job.c,v 1.30 2001/09/17 12:35:54 jorge Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

void job_init_registered (struct database *wdb,uint32_t ijob,struct job *job)
{
  /* Called we we have just received a job to be registered */
  int i;
  int nframes;
  char msg[BUFFERLEN];
  
  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    log_master (L_ERROR,"Job index should be valid when registering and it is not");
    return;
  }

  memcpy (&wdb->job[ijob], job, sizeof(struct job));
  wdb->job[ijob].used = 1;
  wdb->job[ijob].id = ijob;
  wdb->job[ijob].status = JOBSTATUS_WAITING;
  /* We allocate the memory for the frame_info */
  nframes = job_nframes (&wdb->job[ijob]);

  wdb->job[ijob].fishmid = get_frame_shared_memory (nframes);
  wdb->job[ijob].frame_info = attach_frame_shared_memory (wdb->job[ijob].fishmid);

  /* Set done frames to NONE */
  for (i=0;i<nframes;i++) {
    wdb->job[ijob].frame_info[i].status = FS_WAITING;
  }

  wdb->job[ijob].fleft = nframes;
  wdb->job[ijob].fdone = 0;
  wdb->job[ijob].ffailed = 0;

  wdb->job[ijob].nprocs = 0;

  wdb->job[ijob].avg_frame_time = DFLTAVGFTIME;
  wdb->job[ijob].est_finish_time = time (NULL) + (DFLTAVGFTIME * nframes);

  detach_frame_shared_memory(wdb->job[ijob].frame_info);

  semaphore_release(wdb->semid);

  snprintf(msg,BUFFERLEN,"Registered on position %i",ijob);
  log_master_job (&wdb->job[ijob],L_INFO,msg);
}

void job_init (struct job *job)
{
  job->used = 0;
  job->frame_info = NULL;
  job->fishmid = -1;		/* -1 when not reserved */
}

void job_delete (struct job *job)
{
  /* This function is called by the master locked. */
  /* This functions sets for removal the frame info structure of the job */
  if (job->fishmid != -1) {
    if (shmctl (job->fishmid,IPC_RMID,NULL) == -1) {
      log_master_job(job,L_ERROR,"job_delete: shmctl (job->fishmid,IPC_RMID,NULL)");
    }
    job->fishmid = -1;
  }

  job_init (job);
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
  case JOBSTATUS_FINISHED:
    strncpy (sstring,"Finished",BUFFERLEN-1);
    break;
  default:
    strncpy (sstring,"DEFAULT (?!)",BUFFERLEN-1);
    fprintf (stderr,"job_status == DEFAULT\n");
    exit (1);
  }

  return sstring;
}

char *job_frame_status_string (char status)
{
  static char sstring[BUFFERLEN];

  sstring[MAXCMDLEN-1] = 0;
  switch (status) {
  case FS_WAITING:
    strncpy (sstring,"Waiting",BUFFERLEN-1);
    break;
  case FS_ASSIGNED:
    strncpy (sstring,"Running",BUFFERLEN-1);
    break;
  case FS_ERROR:
    strncpy (sstring,"Error",BUFFERLEN-1);
    break;
  case FS_FINISHED:
    strncpy (sstring,"Finished",BUFFERLEN-1);
    break;
  default:
    strncpy (sstring,"DEFAULT (?!)",BUFFERLEN-1);
  }

  return sstring;
}

uint32_t job_nframes (struct job *job)
{
  uint32_t n;

  n = (job->frame_end - job->frame_start + 1) / job->frame_step;

  return n;
}


int job_available (struct database *wdb,uint32_t ijob, int *iframe)
{
  if (wdb->job[ijob].used == 0)
    return 0;

  if (!((wdb->job[ijob].status == JOBSTATUS_WAITING) || (wdb->job[ijob].status == JOBSTATUS_ACTIVE)))
    return 0;

  if ((*iframe = job_first_frame_available (wdb,ijob)) == -1) /* This must be the last test because it actually */
							      /* reserves one frame (sets it to assigned) */
    return 0;

  return 1;
}

int job_first_frame_available (struct database *wdb,uint32_t ijob)
{
  /* This function not only returns the first frame */
  /* available but also updates the job structure when found */
  /* so the frame status goes to assigned (we still have to */
  /* set the info about the icomp,start,itask */
  /* This function is called unlocked */
  int i;
  int r = -1;
  int nframes;
  struct frame_info *fi;

  semaphore_lock(wdb->semid);
  if (!job_index_correct_master(wdb,ijob)) {
    semaphore_release(wdb->semid);
    return -1;
  }
  nframes = job_nframes (&wdb->job[ijob]);
  fi = attach_frame_shared_memory(wdb->job[ijob].fishmid);
  for (i=0;i<nframes;i++) {
    if (fi[i].status == FS_WAITING) {
      r = i;			/* return = current */
      fi[i].status = FS_ASSIGNED; /* Change the status to assigned */
      break;
    }
  }
  detach_frame_shared_memory(fi);
  semaphore_release(wdb->semid);

  return r;
}

void job_update_assigned (struct database *wdb, uint32_t ijob, int iframe, int icomp, int itask)
{
  /* LOCK BEFORE CALLING THIS FUNCTION */
  /* Here we should set all the information inside the task structure (slave) */
  /* about the assigned job (master) into the remote computer */
  /* This function is called by the master, locked */
  char msg[BUFFERLEN];

  if (!job_index_correct_master (wdb,ijob)) {
    /* Somebody could have deleted the job meanwhile */
    return;
  }

  if (!job_frame_number_correct (&wdb->job[ijob],
				 job_frame_index_to_number(&wdb->job[ijob],iframe))) {
    /* Or the data could be malicious... */
    return;
  }

  wdb->job[ijob].frame_info = attach_frame_shared_memory (wdb->job[ijob].fishmid);

  /* The status should already be FS_ASSIGNED */
  if (wdb->job[ijob].frame_info[iframe].status != FS_ASSIGNED) {
    snprintf (msg,BUFFERLEN-1,"(wdb->job[%i].frame_info[%i].status != FS_ASSIGNED)\n",ijob,iframe);
    log_master (L_ERROR,msg);
    wdb->job[ijob].frame_info[iframe].status = FS_ASSIGNED;
  }

  wdb->job[ijob].frame_info[iframe].icomp = icomp;
  wdb->job[ijob].frame_info[iframe].itask = itask;

  /* Time stuff */
  time (&wdb->job[ijob].frame_info[iframe].start_time);
  wdb->job[ijob].frame_info[iframe].end_time = wdb->job[ijob].frame_info[iframe].start_time
    + wdb->job[ijob].avg_frame_time;

  /* Exit code */
  wdb->job[ijob].frame_info[iframe].exitcode = 0;

  detach_frame_shared_memory(wdb->job[ijob].frame_info);
}

int get_frame_shared_memory (int nframes)
{
  int shmid;

  if ((shmid = shmget (IPC_PRIVATE,sizeof(struct frame_info)*nframes, IPC_EXCL|IPC_CREAT|0600)) == -1) {
    log_master (L_ERROR,"get_frame_shared_memory: shmget");
    exit (1);
  }

  return shmid;
}

void *attach_frame_shared_memory (int shmid)
{
  void *rv;			/* return value */

  if ((rv = shmat (shmid,0,0)) == (void *)-1) {
    log_master (L_ERROR,"attach_frame_shared_memory: shmat");
    perror ("shmat");
    exit (1);
  }

  return rv;
}

void detach_frame_shared_memory (struct frame_info *fishp)
{
  if (shmdt((char*)fishp) == -1) {
    log_master (L_WARNING,"Call to shmdt failed");
  }
}

int job_njobs_masterdb (struct database *wdb)
{
  int i,c=0;

  for (i=0;i<MAXJOBS;i++) {
    if (wdb->job[i].used) {
      c++;
    }
  }

  return c;
}

void job_update_info (struct database *wdb,uint32_t ijob)
{
  /* This function is called by the master */
  /* It updates the number of process running */
  /* This function is called unlocked */
  int i,nprocs=0;
  struct frame_info *fi;
  int fleft=0,fdone=0,ffailed=0;
  int total;
  time_t avg_frame_time = 0;
  static int old_fdone = 0;	/* Old frames done to update or not the estimated finish time */
  static int old_nprocs = 0;	/* Same that old_fdone */


  log_master (L_DEBUG,"Entering job_update_info.");

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master (wdb,ijob)) {
    /* Somebody could have deleted the job meanwhile */
    return;
  }

  total = job_nframes(&wdb->job[ijob]);

  fi = attach_frame_shared_memory (wdb->job[ijob].fishmid);
  wdb->job[ijob].frame_info = fi;
  for (i=0;i<total;i++) {
    job_check_frame_status (wdb,ijob,i);
    switch (fi[i].status) {
    case FS_ASSIGNED:
      nprocs++;
      break;
    case FS_WAITING:
      fleft++;
      break;
    case FS_FINISHED:
      fdone++;
      avg_frame_time += (fi[i].end_time - fi[i].start_time);
      break;
    case FS_ERROR:
      ffailed++;
      break;
    }
  }
  if (fdone) {
    avg_frame_time /= fdone;
    avg_frame_time += SLAVEDELAY - (avg_frame_time % SLAVEDELAY);
  }
  detach_frame_shared_memory(fi);

  wdb->job[ijob].nprocs = nprocs;
  wdb->job[ijob].fleft = fleft;
  wdb->job[ijob].fdone = fdone;
  wdb->job[ijob].ffailed = ffailed;
  if (fdone)
    wdb->job[ijob].avg_frame_time = avg_frame_time;

  /* If we do not check old_fdone and old_nprocs, the est_finish_time is being updated every time */
  /* this function is called. In this way it is only updated when it must, that is when a frame is */
  /* finished or when the number of running processors change */
  if ((nprocs) && ((fdone != old_fdone) || (nprocs != old_nprocs))) {
    wdb->job[ijob].est_finish_time = time(NULL) + ((avg_frame_time * (fleft+nprocs)) / nprocs);
    old_fdone = fdone;
    old_nprocs = nprocs;
  }

  switch (wdb->job[ijob].status) {
  case JOBSTATUS_WAITING:
  case JOBSTATUS_ACTIVE:
    if (nprocs > 0) {
      wdb->job[ijob].status = JOBSTATUS_ACTIVE;
    } else {
      if (fleft == 0) {
	wdb->job[ijob].status = JOBSTATUS_FINISHED;
	wdb->job[ijob].est_finish_time = time(NULL);
      } else {
	wdb->job[ijob].status = JOBSTATUS_WAITING;
      }
    }
    break;
  case JOBSTATUS_STOPPED:
  }
  semaphore_release(wdb->semid);
}

void job_check_frame_status (struct database *wdb,uint32_t ijob, uint32_t iframe)
{
  /* This function is called by the master, locked */
  /* This function check if the running or loading (in frame_info at job) process is actually */
  /* runnning or not (in task at computer) */
  /* This function is called with the frame info memory ATTACHED <------- */
  t_framestatus fistatus;
  int running = 1;
  uint16_t icomp,itask;
  t_taskstatus tstatus;

/*    char msg[BUFFERLEN]; */

/*    log_master (L_DEBUG,"Entering job_check_frame_status."); */

/*    snprintf (msg,BUFFERLEN-1,"Checking iframe %i of ijob %i",iframe,ijob); */
/*    log_master (L_DEBUG,msg); */

  fistatus = wdb->job[ijob].frame_info[iframe].status;
  icomp = wdb->job[ijob].frame_info[iframe].icomp;
  itask = wdb->job[ijob].frame_info[iframe].itask;

/*    snprintf (msg,BUFFERLEN-1,"Checking iframe %i of ijob %i. icomp: %i itask: %i", */
/*  	    iframe,ijob,icomp,itask); */
/*    log_master (L_DEBUG,msg); */

  if (fistatus == FS_ASSIGNED) {
    if (wdb->computer[icomp].used == 0) {
      running = 0;
    } else if (wdb->computer[icomp].status.task[itask].used == 0) {
      running = 0;
    } else {
      tstatus = wdb->computer[icomp].status.task[itask].status;
      
      /* check if the task status is running */
      if ((tstatus != TASKSTATUS_RUNNING) && (tstatus != TASKSTATUS_LOADING))
	running = 0;
      /* check if the job is the same in index */
      if (wdb->computer[icomp].status.task[itask].ijob != ijob)
	running = 0;
      /* check if the job is the same in name */
      if (strcmp (wdb->computer[icomp].status.task[itask].jobname,wdb->job[ijob].name) != 0)
	running = 0;
    }
  }

  if (!running) {
    log_master_job (&wdb->job[ijob],L_WARNING,"Task registered as running not running. Requeued");
    wdb->job[ijob].frame_info[iframe].status = FS_WAITING;
  }

/*    log_master (L_DEBUG,"Exiting job_check_frame_status."); */
}

int priority_job_compare (const void *a,const void *b)
{
  struct tpol *apt,*bpt;
  
  apt = (struct tpol *)a;
  bpt = (struct tpol *)b;

  if (apt->pri > bpt->pri)
    return 1;
  else if (apt->pri < bpt->pri)
    return -1;
  else if (apt->index > bpt->index)
    return 1;
  else if (apt->index < bpt->index)
    return -1;

  return 0;
}

void job_stop (struct job *job)
{
  /* This function is called locked */
  switch (job->status) {
  case JOBSTATUS_WAITING:
  case JOBSTATUS_ACTIVE:
    job->status = JOBSTATUS_STOPPED;
    break;
  case JOBSTATUS_STOPPED:
  case JOBSTATUS_FINISHED:
  }
}

void job_continue (struct job *job)
{
  /* This function is called locked */
  switch (job->status) {
  case JOBSTATUS_WAITING:
  case JOBSTATUS_ACTIVE:
    break;
  case JOBSTATUS_STOPPED:
    job->status = JOBSTATUS_WAITING;
    break;
  case JOBSTATUS_FINISHED:
  }
}

void job_frame_waiting (struct database *wdb,uint32_t ijob, int iframe)
{
  /* This function is called unlocked, it's called by the master */
  /* This function sets a frame status to FS_WAITING */
  struct frame_info *fi;

  semaphore_lock(wdb->semid);
  
  if (!job_index_correct_master(wdb,ijob))
    return;

  if (!job_frame_number_correct(&wdb->job[ijob],job_frame_index_to_number(&wdb->job[ijob],iframe)))
    return;

  fi = attach_frame_shared_memory(wdb->job[ijob].fishmid);
  fi[iframe].status = FS_WAITING;
  detach_frame_shared_memory(fi);
  semaphore_release(wdb->semid);
}

uint32_t job_frame_index_to_number (struct job *job,uint32_t index)
{
  return (job->frame_start + (index * job->frame_step)); 
}

uint32_t job_frame_number_to_index (struct job *job,uint32_t number)
{
  return ((number - job->frame_start) / job->frame_step); 
}

int job_frame_number_correct (struct job *job,uint32_t number)
{
  if (number > job->frame_end)
    return 0;
  if (number < job->frame_start)
    return 0;
  if (((number - job->frame_start) % job->frame_step) != 0)
    return 0;

  return 1;
}

int job_index_correct_master (struct database *wdb,uint32_t ijob)
{
  if (ijob > MAXJOBS)
    return 0;
  if (!wdb->job[ijob].used)
    return 0;

  return 1;
}

void job_environment_set (struct job *job, uint32_t iframe)
{
  uint32_t frame;
  static char padframe[BUFFERLEN];
  static char s_frame[BUFFERLEN];
  static char s_iframe[BUFFERLEN];
  static char scene[BUFFERLEN];
  static char project[BUFFERLEN];
  static char image[BUFFERLEN];

  frame = job_frame_index_to_number (job,iframe);

  /* Padded frame number */
  snprintf (padframe,BUFFERLEN-1,"PADFRAME=%04i",frame);
  putenv (padframe);
  /* Frame number */
  snprintf (s_frame,BUFFERLEN-1,"FRAME=%i",frame);
  putenv (s_frame);
  /* Frame index */
  snprintf (s_iframe,BUFFERLEN-1,"IFRAME=%i",iframe);
  putenv (s_iframe);
  /* OS */
#ifdef __LINUX
  putenv ("DRQUEUE_OS=LINUX");
#else
  putenv ("DRQUEUE_OS=IRIX");
#endif

  switch (job->koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
    snprintf (scene,BUFFERLEN-1,"SCENE=%s",job->koji.maya.scene);
    putenv (scene);
    snprintf (project,BUFFERLEN-1,"PROJECT=%s",job->koji.maya.project);
    putenv (project);
    snprintf (image,BUFFERLEN-1,"IMAGE=%s",job->koji.maya.image);
    putenv (image);
    break;
  }
}
