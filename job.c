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
// $Id$
//

#ifdef __FREEBSD
# include <sys/types.h>
#endif
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
#include "common.h"

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
  /* Called when we have just received a job to be registered */
  int i;
  int nframes;
  
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

  if ((wdb->job[ijob].fishmid = get_frame_shared_memory (nframes)) == -1) {
    job_init (&wdb->job[ijob]);
    semaphore_release(wdb->semid);
    log_master(L_ERROR,"Getting frame shared memory. New job could not be registered.");
    return;
  }

  if ((wdb->job[ijob].frame_info = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void *)-1) {
    job_init (&wdb->job[ijob]);
    semaphore_release(wdb->semid);
    log_master(L_ERROR,"Attaching frame shared memory. New job could not be registered.");
    return;
  }

  /* Set done frames to NONE */
  for (i=0;i<nframes;i++) {
    job_frame_info_init (&wdb->job[ijob].frame_info[i]);
  }

  wdb->job[ijob].fleft = nframes;
  wdb->job[ijob].fdone = 0;
  wdb->job[ijob].ffailed = 0;

  wdb->job[ijob].nprocs = 0;

  wdb->job[ijob].avg_frame_time = DFLTAVGFTIME;
  wdb->job[ijob].est_finish_time = time (NULL) + (DFLTAVGFTIME * nframes);

  detach_frame_shared_memory(wdb->job[ijob].frame_info);

  semaphore_release(wdb->semid);

  log_master_job (&wdb->job[ijob],L_INFO,"Registered on position %i",ijob);
}

void job_init (struct job *job)
{
  job->used = 0;
  job->frame_info = NULL;
  job->fishmid = -1;		/* -1 when not reserved */
	job->bhshmid = -1;		// -1 when not reserved 
	job->nblocked = 0;

  job->flags = 0;
}

void job_delete (struct job *job)
{
  /* This function is called by the master locked. */
  /* This functions marks for removal the frame info structure of the job */
  /* So the shared memory block is finally removed when no processes are attached to it */
	// Does the same also with the blocked hosts structure
  if (job->fishmid != -1) {
    if (shmctl (job->fishmid,IPC_RMID,NULL) == -1) {
      log_master_job(job,L_ERROR,"job_delete: shmctl (job->fishmid,IPC_RMID,NULL) [Removing frame shared memory]");
    }
    job->fishmid = -1;
  }

	if (job->bhshmid != -1) {
    if (shmctl (job->bhshmid,IPC_RMID,NULL) == -1) {
      log_master_job(job,L_ERROR,"job_delete: shmctl (job->bhshmid,IPC_RMID,NULL) [Removing blocked hosts shared memory]");
    }
    job->bhshmid = -1;
  }

  job_init (job);
}

char *job_status_string (char status)
{
  char *msg;

  switch (status) {
  case JOBSTATUS_WAITING:
    msg = "Waiting";
    break;
  case JOBSTATUS_ACTIVE:
    msg = "Active";
    break;
  case JOBSTATUS_STOPPED:
    msg = "Stopped";
    break;
  case JOBSTATUS_FINISHED:
    msg = "Finished";
    break;
  default:
    msg = "DEFAULT (ERROR)";
    fprintf (stderr,"job_status == DEFAULT\n");
  }

  return msg;
}

char *job_frame_status_string (char status)
{
  char *msg;

  switch (status) {
  case FS_WAITING:
    msg = "Waiting";
    break;
  case FS_ASSIGNED:
    msg = "Running";
    break;
  case FS_ERROR:
    msg = "Error";
    break;
  case FS_FINISHED:
    msg = "Finished";
    break;
  default:
    msg = "DEFAULT (ERROR)";
  }

  return msg;
}

uint32_t job_nframes (struct job *job)
{
	uint32_t n_step,n;
	
	n_step = (job->frame_end - job->frame_start + job->frame_step) / job->frame_step;
	n = (n_step + job->block_size - 1) / job->block_size;

	return n;
}


int job_available (struct database *wdb,uint32_t ijob, int *iframe, uint32_t icomp)
{
  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    semaphore_release(wdb->semid);
    return 0;
  }

  if (!((wdb->job[ijob].status == JOBSTATUS_WAITING) 
				|| (wdb->job[ijob].status == JOBSTATUS_ACTIVE))) 
		{
			semaphore_release(wdb->semid);
			return 0;
		}

	if ((wdb->job[ijob].flags & JF_JOBDEPEND)
			&& (wdb->job[wdb->job[ijob].dependid].status != JOBSTATUS_FINISHED))
		{
			// If this job depends on another and that one hasn't finished, job is not available
			semaphore_release(wdb->semid);
			return 0;
		}
  
  if (!job_limits_passed(wdb,ijob,icomp)) {
    semaphore_release(wdb->semid);
    return 0;
  }

  if ((*iframe = job_first_frame_available (wdb,ijob,icomp)) == -1) {
    /* This must be the last test because it actually */
    /* reserves one frame (sets it to assigned) */
    /* Also point to icomp in the frame_info BUT sets itask to -1 */
    /* until it is later set properly */
    semaphore_release(wdb->semid);
    return 0;
  }
  
  semaphore_release(wdb->semid);
  return 1;

}

int job_first_frame_available (struct database *wdb,uint32_t ijob,uint32_t icomp)
{
  /* To be called LOCKED */
  /* This function not only returns the first frame */
  /* available but also updates the job structure when found */
  /* so the frame status goes to assigned (we still have to */
  /* set the info about the icomp,start,itask) */
  int i;
  int r = -1;
  int nframes;
  struct frame_info *fi;

  nframes = job_nframes (&wdb->job[ijob]);
  fi = attach_frame_shared_memory(wdb->job[ijob].fishmid);
  for (i=0;i<nframes;i++) {
    if (fi[i].status == FS_WAITING) {
      r = i;			/* return = current */
      fi[i].status = FS_ASSIGNED; /* Change the status to assigned */
      fi[i].icomp = icomp;	/* Assign the computer */
      fi[i].itask = -1;		/* Doesn't have a task yet */
      /* This is temporary and will be set correctly in job_update_info */
      wdb->job[ijob].nprocs++;	/* Add 1 to the number of running processes */
      break;
    }
  }
  detach_frame_shared_memory(fi);

  return r;
}

void job_update_assigned (struct database *wdb, uint32_t ijob, int iframe, int icomp, int itask)
{
  /* LOCK BEFORE CALLING THIS FUNCTION */
  /* Here we should set all the information inside the task structure (slave) */
  /* about the assigned job (master) into the remote computer */
  /* This function is called by the master, locked */

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
    log_master (L_ERROR,"(wdb->job[%i].frame_info[%i].status != FS_ASSIGNED)",ijob,iframe);
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

int get_blocked_host_shared_memory (int nhosts)
{
	int shmid;
					  
	if ((shmid = shmget (IPC_PRIVATE,sizeof(struct blocked_host)*nhosts, IPC_EXCL|IPC_CREAT|0600)) == -1) {
		log_master (L_ERROR,"get_blocked_host_shared_memory: shmget");
		perror ("shmget");
		return shmid;
	}

	return shmid;
}

void *attach_blocked_host_shared_memory (int shmid)
{
  void *rv;			/* return value */

  if ((rv = shmat (shmid,0,0)) == (void *)-1) {
    log_master (L_ERROR,"attach_blocked_host_shared_memory: shmat");
    perror ("shmat");
  }

  return rv;
}

void detach_blocked_host_shared_memory (struct blocked_host *bhshp)
{
  if (shmdt((char*)bhshp) == -1) {
    log_master (L_WARNING,"Call to shmdt failed");
  }
}

int get_frame_shared_memory (int nframes)
{
  int shmid;

  if ((shmid = shmget (IPC_PRIVATE,sizeof(struct frame_info)*nframes, IPC_EXCL|IPC_CREAT|0600)) == -1) {
    log_master (L_ERROR,"get_frame_shared_memory: shmget");
    perror ("shmget");
    return shmid;
  }

  return shmid;
}

void *attach_frame_shared_memory (int shmid)
{
  void *rv;			/* return value */

  if ((rv = shmat (shmid,0,0)) == (void *)-1) {
    log_master (L_ERROR,"attach_frame_shared_memory: shmat");
    perror ("shmat");
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
  /* This function set the information about running, waiting processes... */
  /* Locks, and for every frame in the job checks it's status */
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
    semaphore_release(wdb->semid);
    return;
  }

  total = job_nframes(&wdb->job[ijob]);

  fi = attach_frame_shared_memory (wdb->job[ijob].fishmid);
  wdb->job[ijob].frame_info = fi;
/*    log_master (L_DEBUG,"job_update_info: Before checking frame status"); */
  for (i=0;i<total;i++) {
/*      log_master (L_DEBUG,"job_update_info: Before checking frame %i",i); */
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
/*      avg_frame_time += SLAVEDELAY - (avg_frame_time % SLAVEDELAY); */
  }
  detach_frame_shared_memory(fi);
/*    log_master (L_DEBUG,"job_update_info: After checking frame status"); */

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
      wdb->job[ijob].status = JOBSTATUS_ACTIVE;	/* Leave it active */
    } else {
      if (fleft == 0) {
				wdb->job[ijob].status = JOBSTATUS_FINISHED;
				wdb->job[ijob].est_finish_time = time(NULL);
				if (wdb->job[ijob].flags & JF_MAILNOTIFY)
					mn_job_finished (&wdb->job[ijob]); /* Mail no	tification */
      } else {
				wdb->job[ijob].status = JOBSTATUS_WAITING;
      }
    }
    break;
  case JOBSTATUS_STOPPED:
    break;
  case JOBSTATUS_FINISHED:
    if (fleft > 0) {
      wdb->job[ijob].status = JOBSTATUS_WAITING;
    }
  }
  semaphore_release(wdb->semid);

  log_master (L_DEBUG,"Exiting job_update_info.");
}

void job_check_frame_status (struct database *wdb,uint32_t ijob, uint32_t iframe)
{
  /* This function is called by the master, LOCKED */
  /* This function check if the running or loading (in frame_info at job) process is actually */
  /* runnning or not (in task at computer) */
  /* This function is called with the frame info memory ATTACHED <------- */
  t_framestatus fistatus;
  int running = 1;
  uint16_t icomp,itask;
  t_taskstatus tstatus;


  fistatus = wdb->job[ijob].frame_info[iframe].status;
  icomp = wdb->job[ijob].frame_info[iframe].icomp;
  itask = wdb->job[ijob].frame_info[iframe].itask;

  if (fistatus == FS_ASSIGNED) {
    if (!computer_index_correct_master(wdb,icomp)) {
      log_master (L_WARNING,"Index not correct.");
      running = 0;
    } else if (itask != (uint16_t)-1) {
      if (wdb->computer[icomp].status.task[itask].used == 0) {
	/* If it has a task assigned and that's not beign used */
	log_master (L_WARNING,"Task in the computer is not being used");
	running = 0;
      } else {
	tstatus = wdb->computer[icomp].status.task[itask].status;
	
	/* check if the task status is running */
	if ((tstatus != TASKSTATUS_RUNNING) && (tstatus != TASKSTATUS_LOADING)) {
	  log_master (L_WARNING,"Task status is not running or loading");
	  running = 0;
	}
	
	/* check if the job is the same in index */
	if (wdb->computer[icomp].status.task[itask].ijob != ijob) {
	  log_master (L_WARNING,"Job indices between task and frame info differ");
	  running = 0;
	}
	/* check if the job is the same in name */
	if (!job_index_correct_master (wdb,ijob)) {
	  log_master (L_WARNING,"Job index is not correct");
	  running = 0;
	} else if (strcmp (wdb->computer[icomp].status.task[itask].jobname,wdb->job[ijob].name) != 0) {
	  log_master (L_WARNING,"Job names are different between task and job");
	  running = 0;
	}
      }
    } else {
      /* The task is being loaded, so it hasn't yet a itask assigned  */
    }
  }

  if (!running) {
    log_master (L_DEBUG,"Checking iframe %i of ijob %i. icomp: %i itask: %i", iframe,ijob,icomp,itask);

    log_master_job (&wdb->job[ijob],L_WARNING,"Task registered as running not running. Requeued");
    wdb->job[ijob].frame_info[iframe].status = FS_WAITING;
    wdb->job[ijob].frame_info[iframe].start_time = 0;
    wdb->job[ijob].frame_info[iframe].requeued++;
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
    break;
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
  case JOBSTATUS_FINISHED:
    job->status = JOBSTATUS_WAITING;
    break;
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
	fi[iframe].start_time = 0;
	fi[iframe].requeued++;
  detach_frame_shared_memory(fi);

  /* This is a temporary adjust for consistency reasons */
  /* The appropiate value will be set on job_update_info */ 
  if (wdb->job[ijob].nprocs)
    wdb->job[ijob].nprocs--;

  semaphore_release(wdb->semid);
}

uint32_t job_frame_index_to_number (struct job *job,uint32_t index)
{
  return (job->frame_start + (index * job->block_size * job->frame_step)); 
}

uint32_t job_frame_number_to_index (struct job *job,uint32_t number)
{
  return ((number - job->frame_start) / (job->block_size * job->frame_step)); 
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
  static char scene[BUFFERLEN];
  static char renderdir[BUFFERLEN];
  static char image[BUFFERLEN];
  static char owner[BUFFERLEN];
  static char startframe[BUFFERLEN];
  static char endframe[BUFFERLEN];
  static char stepframe[BUFFERLEN];
	static char blocksize[BUFFERLEN];

  frame = job_frame_index_to_number (job,iframe);

  /* Padded frame number */
  /* TODO: make padding length user defined */
  snprintf (padframe,BUFFERLEN-1,"DRQUEUE_PADFRAME=%04i",frame);
  putenv (padframe);
  /* Frame number */
  snprintf (s_frame,BUFFERLEN-1,"DRQUEUE_FRAME=%i",frame);
  putenv (s_frame);
  /* Start and end frame numbers */
  snprintf (startframe,BUFFERLEN-1,"DRQUEUE_STARTFRAME=%i",job->frame_start);
  putenv (startframe);
  snprintf (endframe,BUFFERLEN-1,"DRQUEUE_ENDFRAME=%i",job->frame_end);
  putenv (endframe);
  /* Step frames */
  snprintf (stepframe,BUFFERLEN-1,"DRQUEUE_STEPFRAME=%i",job->frame_step);
  putenv (stepframe);
  /* Block size */
  snprintf (blocksize,BUFFERLEN-1,"DRQUEUE_BLOCKKSIZE=%i",job->block_size);
  putenv (blocksize);

  /* Owner of the job */
  snprintf (owner,BUFFERLEN-1,"DRQUEUE_OWNER=%s",job->owner);
  putenv (owner);

  /* OS */
#if defined(__LINUX)
  putenv ("DRQUEUE_OS=LINUX");
#elif defined(__FREEBSD)
  putenv ("DRQUEUE_OS=FREEBSD");
#elif defined(__OSX)
  putenv ("DRQUEUE_OS=OSX");
#else
  putenv ("DRQUEUE_OS=IRIX");
#endif

  switch (job->koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
    snprintf (scene,BUFFERLEN-1,"SCENE=%s",job->koji.maya.scene);
    putenv (scene);
    snprintf (renderdir,BUFFERLEN-1,"RD=%s",job->koji.maya.renderdir);
    putenv (renderdir);
    snprintf (image,BUFFERLEN-1,"IMAGE=%s",job->koji.maya.image);
    putenv (image);
  case KOJ_BLENDER:
    snprintf (scene,BUFFERLEN-1,"SCENE=%s",job->koji.blender.scene);
    break;
  case KOJ_BMRT:
    snprintf (scene,BUFFERLEN-1,"SCENE=%s",job->koji.bmrt.scene);
    break;
  }
}

void job_copy (struct job *src, struct job *dst)
{
  memcpy (dst,src,sizeof(struct job));
  dst->frame_info = NULL;
}

void job_init_limits (struct job *job)
{
  job->limits.nmaxcpus = -1;	/* No limit or 65535 */
  job->limits.nmaxcpuscomputer = -1; /* the same */
  job->limits.os_flags = -1;	/* All operating systems */
}

int job_limits_passed (struct database *wdb, uint32_t ijob, uint32_t icomp)
{
  /* This function should return 0 in case the limits are not met for the computer */
	int i;
	struct blocked_host *bh;
  
  if (wdb->job[ijob].nprocs >= wdb->job[ijob].limits.nmaxcpus)
    return 0;

  if (computer_ntasks_job(&wdb->computer[icomp],ijob) >= wdb->job[ijob].limits.nmaxcpuscomputer)
    return 0;

	// Solved bug 0000003
  if ((wdb->computer[icomp].hwinfo.os == OS_LINUX) && !(wdb->job[ijob].limits.os_flags & OSF_LINUX))
		return 0;
  if ((wdb->computer[icomp].hwinfo.os == OS_IRIX) && !(wdb->job[ijob].limits.os_flags & OSF_IRIX))
		return 0;
  if ((wdb->computer[icomp].hwinfo.os == OS_OSX) && !(wdb->job[ijob].limits.os_flags & OSF_OSX))
		return 0;
  if ((wdb->computer[icomp].hwinfo.os == OS_FREEBSD) && !(wdb->job[ijob].limits.os_flags & OSF_FREEBSD))
		return 0;

	// Memory
	if (wdb->computer[icomp].hwinfo.memory < wdb->job[ijob].limits.memory)
		return 0;

	if (wdb->job[ijob].nblocked
									&& ((bh = attach_blocked_host_shared_memory (wdb->job[ijob].bhshmid)) != (void *)-1))
	{
		for (i=0;i<wdb->job[ijob].nblocked;i++) {
			if (strcmp(wdb->computer[icomp].hwinfo.name,bh[i].name) == 0) {
				return 0;
			}
		}
	}

	if (bh != (void *)-1) {
		detach_blocked_host_shared_memory (bh);
	}

  return 1;
}

void job_frame_info_init (struct frame_info *fi)
{
  fi->status = FS_WAITING;
  fi->start_time = fi->end_time = 0;
  fi->exitcode = 0;
  fi->icomp = fi->itask = 0;
	fi->requeued = 0;
	fi->flags = 0;
}

void job_logs_remove (struct job *job)
{
  char dir[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
    /* This should never happen because we check at the begining of every program */
    return;
  }

  snprintf(dir,BUFFERLEN-1,"%s/%s",basedir,job->name);

  remove_dir(dir);
}

char *job_koj_string (struct job *job)
{
  char *msg;

  switch (job->koj) {
  case KOJ_GENERAL:
    msg = "General";
    break;
  case KOJ_MAYA:
    msg = "Maya";
    break;
  case KOJ_BLENDER:
    msg = "Blender";
    break;
	case KOJ_BMRT:
		msg = "Bmrt";
		break;
  case KOJ_PIXIE:
    msg = "Pixie";
    break;
  default:
    msg = "DEFAULT (ERROR)";
  }

  return msg;
}
	  
int job_available_no_icomp (struct database *wdb,uint32_t ijob, int *iframe)
{
	/* This function returns 1 in case there is a job available without asigning it to any computer */
  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    semaphore_release(wdb->semid);
    return 0;
  }

  if (!((wdb->job[ijob].status == JOBSTATUS_WAITING) || (wdb->job[ijob].status == JOBSTATUS_ACTIVE))) {
    semaphore_release(wdb->semid);
    return 0;
  }

	if ((wdb->job[ijob].flags & JF_JOBDEPEND)
			&& (wdb->job[wdb->job[ijob].dependid].status != JOBSTATUS_FINISHED))
		{
			// If this job depends on another and that one hasn't finished, job is not available
			semaphore_release(wdb->semid);
			return 0;
		}
  
  if ((*iframe = job_first_frame_available_no_icomp (wdb,ijob)) == -1) {
    semaphore_release(wdb->semid);
    return 0;
  }
  
  semaphore_release(wdb->semid);
  return 1;
}

int job_first_frame_available_no_icomp (struct database *wdb,uint32_t ijob)
{
  /* To be called LOCKED */
  /* This function not only returns the first frame */
  /* available without updating the job structure */
  int i;
  int r = -1;
  int nframes;
  struct frame_info *fi;

  nframes = job_nframes (&wdb->job[ijob]);
  fi = attach_frame_shared_memory(wdb->job[ijob].fishmid);
  for (i=0;i<nframes;i++) {
    if (fi[i].status == FS_WAITING) {
      r = i;										/* return = current */
      break;
    }
  }
  detach_frame_shared_memory(fi);

  return r;
}
