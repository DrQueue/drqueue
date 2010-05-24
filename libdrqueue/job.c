//
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
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

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include "libdrqueue.h"

uint32_t job_index_free (void *pwdb) {
  /* Return the index to a free job record OR -1 if there */
  /* are no more free records */
  uint32_t index = (uint32_t) -1;
  uint32_t i;
  struct database *wdb = (struct database *)pwdb;

  for (i=0; i<MAXJOBS; i++) {
    if (wdb->job[i].used == 0) {
      index = i;
      break;
    }
  }

  return index;
}

void job_report (struct job *job) {
  printf ("JOB Report\n");
  printf ("Name:\t\t\t%s\n",job->name);
  printf ("Status:\t\t\t%s\n",job_status_string(job->status));
  printf ("Owner:\t\t\t%s\n",job->owner);
  printf ("Command:\t\t%s\n",job->cmd);
  printf ("Frame start,end:\t%i,%i\n",job->frame_start,job->frame_end);
}

void job_fix_received_invalid (struct job *job) {
  // This function will fix all those values that, if received from a
  // remote location and left unchanged, could produce error or
  // warning messages, as well as undefined behaviour.
  job->envvars.evshmid = (int64_t)-1;
  job->envvars.variables.ptr = NULL;
  job->envvars.nvariables = 0;
  job->blocked_host.ptr = NULL;
  job->bhshmid = (int64_t)-1;

  job->nblocked = 0;
  job->frame_info.ptr = NULL;
  job->fishmid = (int64_t)-1;
}

void job_init_registered (struct database *wdb,uint32_t ijob,struct job *job) {
  /* Called when we have just received a job to be registered */
  int i;
  int nframes;

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    log_auto (L_ERROR,"Job index should be valid when registering and it is not");
    return;
  }

  memcpy (&wdb->job[ijob], job, sizeof(struct job));

  wdb->job[ijob].used = 1;
  wdb->job[ijob].id = ijob;
  wdb->job[ijob].status = JOBSTATUS_WAITING;
  /* We allocate the memory for the frame_info */
  nframes = job_nframes (&wdb->job[ijob]);

  if (nframes) {
    if ((wdb->job[ijob].fishmid = (int64_t) get_frame_shared_memory (nframes)) == (int64_t)-1) {
      job_init (&wdb->job[ijob]);
      semaphore_release(wdb->semid);
      log_auto (L_ERROR,"Getting frame shared memory. New job could not be registered.");
      return;
    }

    if ((wdb->job[ijob].frame_info.ptr = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void *)-1) {
      job_init (&wdb->job[ijob]);
      semaphore_release(wdb->semid);
      log_auto (L_ERROR,"Attaching frame shared memory. New job could not be registered.");
      return;
    }

    /* Set done frames to NONE */
    for (i=0;i<nframes;i++) {
      job_frame_info_init (&wdb->job[ijob].frame_info.ptr[i]);
    }
  }

  wdb->job[ijob].fleft = nframes;
  wdb->job[ijob].fdone = 0;
  wdb->job[ijob].ffailed = 0;

  wdb->job[ijob].nprocs = 0;

  wdb->job[ijob].submit_time = time (NULL);
  wdb->job[ijob].avg_frame_time = DFLTAVGFTIME;
  wdb->job[ijob].est_finish_time = time (NULL) + (DFLTAVGFTIME * nframes);

  if (nframes)
    detach_frame_shared_memory(wdb->job[ijob].frame_info.ptr);

#ifdef __DEBUG_ENVVARS
  fprintf(stderr,"nvar wdb: %i\n",wdb->job[ijob].envvars.nvariables);
  fprintf(stderr,"nvar job: %i\n",job->envvars.nvariables);
  fprintf(stderr,"vars wdb: %p\n",wdb->job[ijob].envvars.variables);
  fprintf(stderr,"vars job: %p\n",job->envvars.variables);
  fprintf(stderr,"evsh wdb: %lli\n",wdb->job[ijob].envvars.evshmid);
  fprintf(stderr,"evsh job: %lli\n",job->envvars.evshmid);
#endif

  semaphore_release(wdb->semid);

  
  log_auto(L_INFO,"Registered on position %i",ijob);
}

void
job_shared_frame_info_init (struct job *job) {
  if (!job) {
    return;
  }
  job->frame_info.ptr = NULL;
  job->fishmid = (int64_t)-1;
}

void
job_shared_blocked_hosts_init (struct job *job) {
  job->blocked_host.ptr = NULL;
  job->bhshmid = (int64_t)-1;  // -1 when not reserved
  job->nblocked = 0;
}

void
job_init (struct job *job) {
  // Zeroed
  memset (job,0,sizeof(struct job));

  job->used = 0;

  job_shared_frame_info_init (job);
  job_shared_blocked_hosts_init (job);
  envvars_init(&job->envvars);

  job->frame_start = 1;
  job->frame_end = 1;
  job->frame_pad = 4;
  job->frame_step = 1;
  job->block_size = 1;
  job->autoRequeue = 1;
  job->status = JOBSTATUS_WAITING;
  job->priority = 500;
  job->flags = 0;

  job->koj = KOJ_GENERAL;
  snprintf (job->owner,MAXNAMELEN,"%s",getenv("USER"));

  job_limits_init (&job->limits);
}

int
job_frame_info_free (struct job *job) {
  int rv = 1;
  if (!job) {
    rv = 0;
  } else if (job->fishmid != (int64_t)-1) {
    if (shmctl ((int)job->fishmid,IPC_RMID,NULL) == -1) {
      drerrno_system = errno;
      drerrno = DRE_RMSHMEM;
      log_auto(L_ERROR,"job_delete(): could not remove frame shared memory. (%s)",strerror(drerrno_system));
      rv = 0;
    }
  }
  job_shared_frame_info_init (job);
  return rv;
}

int
job_block_host_free (struct job *job) {
  int rv = 1;

  if (job->bhshmid != (int64_t)-1) {
    if (shmctl ((int)job->bhshmid,IPC_RMID,NULL) == -1) {
      drerrno_system = errno;
      drerrno = DRE_RMSHMEM;
      log_auto(L_ERROR,"job_delete(): could not remove blocked hosts shared memory. (%s)",strerror(drerrno_system));
      rv = 0;
    }
  }
  job_shared_blocked_hosts_init(job);
  return rv;
}

void
job_delete (struct job *job) {
  //
  // Deallocates all memory reserved for the job and initializes it.
  //
  if (!job) {
    // TODO: log it
    return;
  }
  job_frame_info_free (job);
  job_block_host_free (job);
  envvars_free(&job->envvars);
  
  job_init (job);
}

char *job_status_string (uint16_t status) {
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

char *job_frame_status_string (uint8_t status) {
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

uint32_t job_nframes (struct job *job) {
  uint32_t n_step,n = 0;

  if (job->frame_step) {
    n_step = (job->frame_end - job->frame_start + job->frame_step) / job->frame_step;
    if (job->block_size) {
      n = (n_step + job->block_size - 1) / job->block_size;
    }
  }

  return n;
}



int job_available (struct database *wdb,uint32_t ijob, uint32_t *iframe, uint32_t icomp) {
  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    semaphore_release(wdb->semid);
    return 0;
  }

  if (!((wdb->job[ijob].status == JOBSTATUS_WAITING)
        || (wdb->job[ijob].status == JOBSTATUS_ACTIVE))) {
    semaphore_release(wdb->semid);
    return 0;
  }

  if ((wdb->job[ijob].flags & JF_JOBDEPEND)
      && (wdb->job[wdb->job[ijob].dependid].status != JOBSTATUS_FINISHED)) {
    // If this job depends on another and that one hasn't finished, job is not available
    semaphore_release(wdb->semid);
    return 0;
  }

  if (!job_limits_passed(wdb,ijob,icomp)) {
    semaphore_release(wdb->semid);
    return 0;
  }

  if ((*iframe = job_first_frame_available (wdb,ijob,icomp)) == (uint32_t) -1) {
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

uint32_t job_first_frame_available (struct database *wdb,uint32_t ijob,uint32_t icomp) {
  /* To be called LOCKED */
  /* This function not only returns the first frame */
  /* available but also updates the job structure when found */
  /* so the frame status goes to assigned (we still have to */
  /* set the info about the icomp,start,itask) */
  uint32_t i;
  uint32_t r = -1;
  uint32_t nframes;
  struct frame_info *fi;

  nframes = job_nframes (&wdb->job[ijob]);
  if (nframes) {
    fi = attach_frame_shared_memory(wdb->job[ijob].fishmid);
    if (fi == (void *) -1)
      return (-1);
    for (i=0;i<nframes;i++) {
      if (fi[i].status == FS_WAITING) {
        r = i;   /* return = current */
        fi[i].status = FS_ASSIGNED; /* Change the status to assigned */
        fi[i].icomp = icomp; /* Assign the computer */
        fi[i].itask = -1;  /* Doesn't have a task yet */
        /* This is temporary and will be set correctly in job_update_info */
        wdb->job[ijob].nprocs++; /* Add 1 to the number of running processes */
        wdb->job[ijob].fleft--;
        break;
      }
    }
    detach_frame_shared_memory(fi);
  }

  return r;
}

void job_update_assigned (struct database *wdb, uint32_t ijob, uint32_t iframe, uint32_t icomp, uint16_t itask) {
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

  if ((wdb->job[ijob].frame_info.ptr = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void *) -1)
    return;

  /* The status should already be FS_ASSIGNED */
  if (wdb->job[ijob].frame_info.ptr[iframe].status != FS_ASSIGNED) {
    log_auto (L_ERROR,"(wdb->job[%i].frame_info.ptr[%i].status != FS_ASSIGNED)",ijob,iframe);
    wdb->job[ijob].frame_info.ptr[iframe].status = FS_ASSIGNED;
  }

  wdb->job[ijob].frame_info.ptr[iframe].icomp = icomp;
  wdb->job[ijob].frame_info.ptr[iframe].itask = itask;

  /* Time stuff */
  time_t ttime;
  time (&ttime);
  wdb->job[ijob].frame_info.ptr[iframe].start_time = (uint64_t) ttime;
  wdb->job[ijob].frame_info.ptr[iframe].end_time = wdb->job[ijob].frame_info.ptr[iframe].start_time
      + wdb->job[ijob].avg_frame_time;

  /* Exit code */
  wdb->job[ijob].frame_info.ptr[iframe].exitcode = 0;

  if (wdb->job[ijob].nprocs)
    wdb->job[ijob].status = JOBSTATUS_ACTIVE;

  detach_frame_shared_memory(wdb->job[ijob].frame_info.ptr);
}

int64_t
get_blocked_host_shared_memory (uint32_t nhosts) {
  int64_t shmid;

  if ((shmid = (int64_t) shmget (IPC_PRIVATE,sizeof(struct blocked_host)*nhosts, IPC_EXCL|IPC_CREAT|0600)) == (int64_t)-1) {
    drerrno = DRE_GETSHMEM;
    log_auto (L_ERROR,"get_blocked_host_shared_memory: shmget (%s)",drerrno_str());
    perror ("shmget");
    return shmid;
  }

  return shmid;
}

struct blocked_host *
attach_blocked_host_shared_memory (int64_t shmid) {
  void *rv;   /* return value */

  if ((rv = shmat ((int)shmid,0,0)) == (void *)-1) {
    drerrno = DRE_ATTACHSHMEM;
    log_auto (L_ERROR,"attach_blocked_host_shared_memory (shmat): %s",drerrno_str());
    perror ("blocked host shmat");
  }

  return (struct blocked_host *) rv;
}

void
detach_blocked_host_shared_memory (struct blocked_host *bhshp) {
  if (bhshp && shmdt((char*)bhshp) == -1) {
    drerrno = DRE_RMSHMEM;
    log_auto (L_ERROR,"Call to shmdt failed (detach_blocked_host_shared_memory): %s",drerrno_str());
    perror ("shmdt: detach_blocked_host_shared_memory");
  }
  bhshp = NULL;
}

int64_t get_frame_shared_memory (uint32_t nframes) {
  int64_t shmid;

  if ((shmid = (int64_t) shmget (IPC_PRIVATE,sizeof(struct frame_info)*nframes, IPC_EXCL|IPC_CREAT|0600)) == (int64_t)-1) {
    drerrno = DRE_GETSHMEM;
    log_auto (L_ERROR,"get_frame_shared_memory: %s (%s)",drerrno_str(),strerror(errno));
  }

  return shmid;
}

struct frame_info *attach_frame_shared_memory (int64_t shmid) {
  void *pam;   // pointer to attached memory

  if ((pam = shmat ((int)shmid,0,0)) == (void *)-1) {
    drerrno = DRE_ATTACHSHMEM;
    log_auto (L_ERROR,"attach_frame_shared_memory (shmat): %s",drerrno_str(),strerror(errno));
  }

  return (struct frame_info *) pam;
}

void detach_frame_shared_memory (struct frame_info *fishp) {
  if (shmdt((char*)fishp) == -1) {
    drerrno = DRE_RMSHMEM;
    log_auto (L_ERROR,"Call to shmdt failed: detach_frame_shared_memory: %s. (%s)", drerrno_str(),strerror(errno));
  }
}

uint32_t job_njobs_masterdb (struct database *wdb) {
  uint32_t i,c;

  for (i=0,c=0;i<MAXJOBS;i++) {
    if (wdb->job[i].used) {
      c++;
    }
  }

  return c;
}

void
job_update_info (struct database *wdb,uint32_t ijob) {
  /* This function is called by the master */
  /* It updates the number of process running */
  /* This function is called unlocked */
  /* This function set the information about running, waiting processes... */
  /* Locks, and for every frame in the job checks it's status */
  uint16_t nprocs=0;
  struct frame_info *fi;
  uint32_t fleft=0,fdone=0,ffailed=0;
  uint32_t i,total;
  uint64_t avg_frame_time = 0;      // it's bigger than the actual
                                    // job.avg_frame_time because this
                                    // one accumulates the sum of all
                                    // frame times to then divide by
                                    // the number of finished ones.
  struct job job;

  log_auto (L_DEBUG3,"job_update_info(): >Entering job_update_info.");

  ///
  semaphore_lock(wdb->semid);

  if (!job_index_correct_master (wdb,ijob)) {
    semaphore_release(wdb->semid);
    return;
  }

  job_copy (&wdb->job[ijob],&job);
  total = job_nframes(&job);
  job.frame_info.ptr = NULL;
  if (total) {
    fi = attach_frame_shared_memory (wdb->job[ijob].fishmid);
    if (fi == (void *) -1) {
      semaphore_release (wdb->semid);
      return;
    }
    job.frame_info.ptr = (struct frame_info *) malloc (sizeof(struct frame_info)*total);
    if (!job.frame_info.ptr) {
      detach_frame_shared_memory(fi);
      semaphore_release (wdb->semid);
      return;
    }
    memcpy(job.frame_info.ptr,fi,sizeof(struct frame_info)*total);
    detach_frame_shared_memory(fi);
    fi = NULL;
  }
  semaphore_release(wdb->semid);
  ///

  for (i=0;i<total;i++) {
    if (!job_check_frame_status (wdb,ijob,i,job.frame_info.ptr)) {
      log_auto (L_WARNING,"Problem in job_check_frame_status inside job_update_info");
      free (job.frame_info.ptr);
      return;
    }

    switch (job.frame_info.ptr[i].status) {
    case FS_ASSIGNED:
      nprocs++;
      break;
    case FS_WAITING:
      fleft++;
      break;
    case FS_FINISHED:
      fdone++;
      avg_frame_time += (job.frame_info.ptr[i].end_time - job.frame_info.ptr[i].start_time);
      break;
    case FS_ERROR:
      ffailed++;
      break;
    }
  }

  if (total)
    free (job.frame_info.ptr);
  
  job.frame_info.ptr = NULL;

  if (fdone) {
    avg_frame_time /= fdone;
  }

  ///
  semaphore_lock(wdb->semid);

  if (!job_index_correct_master (wdb,ijob)) {
    semaphore_release(wdb->semid);
    return;
  }

  wdb->job[ijob].nprocs  = nprocs;
  wdb->job[ijob].fleft   = fleft;
  wdb->job[ijob].fdone   = fdone;
  wdb->job[ijob].ffailed = ffailed;

  if (fdone)
    wdb->job[ijob].avg_frame_time = (uint32_t) avg_frame_time;

  /* If we do not check old_fdone and old_nprocs, the est_finish_time is being updated every time */
  /* this function is called. In this way it is only updated when it must, that is when a frame is */
  /* finished or when the number of running processors change */
  if ((nprocs) &&
      ((fdone != wdb->job[ijob].old_fdone)
       ||
       (nprocs != wdb->job[ijob].old_nprocs))) 
  {
    wdb->job[ijob].est_finish_time = time(NULL) + ((avg_frame_time * (fleft+nprocs)) / nprocs);
    wdb->job[ijob].old_fdone = fdone;
    wdb->job[ijob].old_nprocs = nprocs;
  }

  switch (wdb->job[ijob].status) {
  case JOBSTATUS_WAITING:
  case JOBSTATUS_ACTIVE:
    if (nprocs > 0) {
      wdb->job[ijob].status = JOBSTATUS_ACTIVE; /* Leave it active */
    } else {
      if (fleft == 0) {
        wdb->job[ijob].status = JOBSTATUS_FINISHED;
        wdb->job[ijob].est_finish_time = time(NULL);
        if (wdb->job[ijob].flags & JF_MAILNOTIFY)
          mn_job_finished (&wdb->job[ijob]); /* Mail no tification */
        if (wdb->job[ijob].flags & JF_JOBDELETE) {
	  log_auto (L_INFO,"Deleting job because flag is set");
          job_delete (&wdb->job[ijob]);
	}
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
  ///

  log_auto (L_DEBUG3,"job_update_info(): <Returning...");
}

int job_check_frame_status (struct database *wdb,uint32_t ijob, uint32_t iframe, struct frame_info *fi) {
  // This function checks if the running or loading (in frame_info at job) process is actually
  /* runnning or not (in task at computer) */
  t_framestatus fistatus;
  int running = 1;
  uint16_t icomp,itask;
  t_taskstatus tstatus;
  struct job *job;

  //
  semaphore_lock (wdb->semid);

  if (!job_index_correct_master (wdb,ijob)) {
    log_auto (L_WARNING,"Job index not correct (%i)",ijob);
    semaphore_release(wdb->semid);
    return 0;
  }

  if (!job_frame_number_correct(&wdb->job[ijob],job_frame_index_to_number(&wdb->job[ijob],iframe))) {
    log_auto (L_WARNING,"Frame index %i not correct for job %i",iframe,ijob);
    semaphore_release (wdb->semid);
    return 0;
  }

  job = &wdb->job[ijob];

  job->frame_info.ptr = fi;

  fistatus = (t_framestatus) job->frame_info.ptr[iframe].status;
  icomp = job->frame_info.ptr[iframe].icomp;
  itask = job->frame_info.ptr[iframe].itask;

  if (fistatus == FS_ASSIGNED) {
    if (!computer_index_correct_master(wdb,icomp)) {
      log_auto (L_WARNING,"Computer index not correct (%i)",icomp);
      running = 0;
    } else if (itask != (uint16_t)-1) {
      if (wdb->computer[icomp].status.task[itask].used == 0) {
        /* If it has a task assigned and that's not beign used */
        log_auto (L_WARNING,"Task in computer (%s:%i) is not being used",wdb->computer[icomp].hwinfo.name,icomp);
        running = 0;
      } else {
        tstatus = (t_taskstatus) wdb->computer[icomp].status.task[itask].status;

        /* check if the task status is running */
        if ((tstatus != TASKSTATUS_RUNNING) && (tstatus != TASKSTATUS_LOADING)) {
          log_auto (L_WARNING,"Task status in computer (%s:%i) is not running or loading", wdb->computer[icomp].hwinfo.name,icomp);
          running = 0;
        }

        /* check if the job is the same in index */
        if (wdb->computer[icomp].status.task[itask].ijob != ijob) {
          log_auto (L_WARNING,"Job indices between task and frame info differ");
          running = 0;
        }
        /* check if the job is the same in name */
        if (!job_index_correct_master (wdb,ijob)) {
          log_auto (L_WARNING,"Job index is not correct");
          running = 0;
        } else if (strcmp (wdb->computer[icomp].status.task[itask].jobname,job->name) != 0) {
          log_auto (L_WARNING,"Job names are different between task and job");
          running = 0;
        }
      }
    } else { // itask == -1
      /* The task is being loaded, so it hasn't yet a itask assigned */
      // TODO: check for timeout
      log_auto(L_WARNING,"job_check_frame_status(): task is being loaded (?). itask == -1.");
      running = 1;
    }
  }

  if (!running) {
    struct frame_info *fitmp;
    log_auto (L_DEBUG,"Checking iframe %i of ijob %i. icomp: %i itask: %i", iframe,ijob,icomp,itask);
    log_auto (L_WARNING,"Task registered as running not running. Requeued");
    fitmp = attach_frame_shared_memory (job->fishmid);
    if (fitmp == (void *) -1) {
      // Couldn't attach frame_info shared memory
      semaphore_release (wdb->semid);
      drerrno = DRE_ATTACHSHMEM;
      log_auto (L_ERROR,"job_check_frame_status(): error attaching frame memory. (%s)",strerror(drerrno_system));
      return 0;
    } else {
      fitmp[iframe].status = FS_WAITING;
      fitmp[iframe].start_time = 0;
      fitmp[iframe].requeued++;
      
      detach_frame_shared_memory(fitmp);
    }
  }

  semaphore_release (wdb->semid);
  ///

  return 1;
}

int priority_job_compare (const void *a,const void *b) {
  struct tpol *apt,*bpt;

  apt = (struct tpol *)a;
  bpt = (struct tpol *)b;

  if (apt->pri > bpt->pri)
    return -1;
  else if (apt->pri < bpt->pri)
    return 1;
  else if (apt->submit_time < bpt->submit_time)
    return -1;
  else if (apt->submit_time > bpt->submit_time)
    return 1;
  else if (apt->index > bpt->index)
    return 1;
  else if (apt->index < bpt->index)
    return -1;

  return 0;
}

void job_stop (struct job *job) {
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

void job_continue (struct job *job) {
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

void job_frame_waiting (struct database *wdb,uint32_t ijob, uint32_t iframe) {
  /* This function is called unlocked, it's called by the master */
  /* This function sets a frame status to FS_WAITING */
  struct frame_info *fi;

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob))
    return;

  if (!job_frame_number_correct(&wdb->job[ijob],job_frame_index_to_number(&wdb->job[ijob],iframe)))
    return;

  fi = attach_frame_shared_memory(wdb->job[ijob].fishmid);
  if (fi == (void *) -1) {
    semaphore_release (wdb->semid);
    return;
  }

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

uint32_t
job_frame_index_to_number (struct job *job,uint32_t index) {
  return (job->frame_start + (index * job->block_size * job->frame_step));
}

uint32_t job_frame_number_to_index (struct job *job,uint32_t number) {
  return ((number - job->frame_start) / (job->block_size * job->frame_step));
}

int job_frame_number_correct (struct job *job,uint32_t number) {
  if (number > job->frame_end)
    return 0;
  if (number < job->frame_start)
    return 0;
  if (((number - job->frame_start) % job->frame_step) != 0)
    return 0;

  return 1;
}

int job_index_correct_master (struct database *wdb,uint32_t ijob) {
  if (ijob > MAXJOBS)
    return 0;
  if (!wdb->job[ijob].used)
    return 0;

  return 1;
}

void job_environment_set (struct job *job, uint32_t iframe) {
  //
  // FIXME: needs to add compid and computer OS properly
  //
  uint32_t frame;
  static char scene[BUFFERLEN];
  static char renderdir[BUFFERLEN];
  static char projectdir[BUFFERLEN];
  static char configdir[BUFFERLEN];
  static char precommand[BUFFERLEN];
  static char postcommand[BUFFERLEN];
  static char scriptfile[BUFFERLEN];
  static char worldfile[BUFFERLEN];
  static char terrainfile[BUFFERLEN];
  static char image[BUFFERLEN];
  static char imageExt[BUFFERLEN];
  static char name[BUFFERLEN];
  static char project[BUFFERLEN];
  static char comp[BUFFERLEN];
  static char script[BUFFERLEN];
  static char xsipass[BUFFERLEN];

  struct task task;


  frame = job_frame_index_to_number (job,iframe);
  task_set_to_job_frame (&task,job,frame);
  task_environment_set (&task);

  /* Name of the job */
  snprintf (name,BUFFERLEN-1,"DRQUEUE_BASE=%s",job->name);
  putenv (name);

  /* OS */
#if defined(__LINUX)

  putenv ("DRQUEUE_OS=LINUX");
#elif defined(__FREEBSD)

  putenv ("DRQUEUE_OS=FREEBSD");
#elif defined(__OSX)

  putenv ("DRQUEUE_OS=OSX");
#elif defined(__CYGWIN)

  putenv ("DRQUEUE_OS=WINDOWS");
#else

  putenv ("DRQUEUE_OS=IRIX");
#endif

  switch (job->koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.maya.scene);
    putenv (scene);
    snprintf (renderdir,BUFFERLEN-1,"DRQUEUE_RD=%s",job->koji.maya.renderdir);
    putenv (renderdir);
    snprintf (projectdir,BUFFERLEN-1,"DRQUEUE_PD=%s",job->koji.maya.projectdir);
    putenv (projectdir);
    snprintf (precommand,BUFFERLEN-1,"DRQUEUE_PRE=%s",job->koji.maya.precommand);
    putenv (precommand);
    snprintf (postcommand,BUFFERLEN-1,"DRQUEUE_POST=%s",job->koji.maya.postcommand);
    putenv (postcommand);
    snprintf (image,BUFFERLEN-1,"DRQUEUE_IMAGE=%s",job->koji.maya.image);
    putenv (image);
    break;
  case KOJ_MENTALRAY:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.mentalray.scene);
    putenv (scene);
    snprintf (renderdir,BUFFERLEN-1,"DRQUEUE_RD=%s",job->koji.mentalray.renderdir);
    putenv (renderdir);
    snprintf (image,BUFFERLEN-1,"DRQUEUE_IMAGE=%s",job->koji.mentalray.image);
    putenv (image);
    break;
  case KOJ_BLENDER:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.blender.scene);
    putenv (scene);
    break;
  case KOJ_PIXIE:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.pixie.scene);
    putenv (scene);
    break;
  case KOJ_3DELIGHT:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.threedelight.scene);
    putenv (scene);
    break;
  case KOJ_LIGHTWAVE:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.lightwave.scene);
    putenv (scene);
    snprintf (projectdir,BUFFERLEN-1,"DRQUEUE_PD=%s",job->koji.lightwave.projectdir);
    putenv (projectdir);
    snprintf (configdir,BUFFERLEN-1,"DRQUEUE_CD=%s",job->koji.lightwave.configdir);
    putenv (configdir);
    break;
  case KOJ_NUKE:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.nuke.scene);
    putenv (scene);
    break;
  case KOJ_AFTEREFFECTS:
    snprintf (project,BUFFERLEN-1,"DRQUEUE_PROJECT=%s",job->koji.aftereffects.project);
    putenv (project);
    snprintf (comp,BUFFERLEN-1,"DRQUEUE_COMP=%s",job->koji.aftereffects.comp);
    putenv (comp);
    break;
  case KOJ_SHAKE:
    snprintf (script,BUFFERLEN-1,"DRQUEUE_SCRIPT=%s",job->koji.shake.script);
    putenv (script);
    break;
  case KOJ_AQSIS:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCRIPT=%s",job->koji.aqsis.scene);
    putenv (scene);
    break;
  case KOJ_MANTRA:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCRIPT=%s",job->koji.mantra.scene);
    putenv (scene);
    snprintf (renderdir,BUFFERLEN-1,"DRQUEUE_RD=%s",job->koji.mantra.renderdir);
    putenv (renderdir);
    break;
  case KOJ_TERRAGEN:
    snprintf (scriptfile,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.terragen.scriptfile);
    putenv (scriptfile);
    snprintf (worldfile,BUFFERLEN-1,"DRQUEUE_WF=%s",job->koji.terragen.worldfile);
    putenv (worldfile);
    snprintf (terrainfile,BUFFERLEN-1,"DRQUEUE_CD=%s",job->koji.terragen.terrainfile);
    putenv (terrainfile);
    break;
  case KOJ_TURTLE:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.turtle.scene);
    putenv (scene);
    snprintf (renderdir,BUFFERLEN-1,"DRQUEUE_RD=%s",job->koji.turtle.renderdir);
    putenv (renderdir);
    snprintf (projectdir,BUFFERLEN-1,"DRQUEUE_PD=%s",job->koji.turtle.projectdir);
    putenv (projectdir);
    snprintf (image,BUFFERLEN-1,"DRQUEUE_IMAGE=%s",job->koji.turtle.image);
    putenv (image);
    break;
  case KOJ_XSI:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.xsi.scene);
    putenv (scene);
    snprintf (xsipass,BUFFERLEN-1,"DRQUEUE_PASS=%s",job->koji.xsi.xsipass);
    putenv (xsipass);
    snprintf (renderdir,BUFFERLEN-1,"DRQUEUE_RD=%s",job->koji.xsi.renderdir);
    putenv (renderdir);
    snprintf (image,BUFFERLEN-1,"DRQUEUE_IMAGE=%s",job->koji.xsi.image);
    putenv (image);
    snprintf (imageExt,BUFFERLEN-1,"DRQUEUE_IMAGEEXT=%s",job->koji.xsi.imageExt);
    putenv (imageExt);
    break;
  case KOJ_LUXRENDER:
    snprintf (scene,BUFFERLEN-1,"DRQUEUE_SCENE=%s",job->koji.luxrender.scene);
    putenv (scene);
    break;
  }
}

void
job_copy (struct job *src, struct job *dst) {
  memcpy (dst,src,sizeof(struct job));
  job_fix_received_invalid (dst);
}

void
job_limits_init (struct job_limits *limits) {
  limits->nmaxcpus = (uint16_t)-1;           // No limit or 65535
  limits->nmaxcpuscomputer = (uint16_t)-1;   // the same
  limits->os_flags = (uint16_t) -1;          // All operating systems
  limits->memory = 0;                        // No memory limit
  snprintf(limits->pool,MAXNAMELEN,"%s",DEFAULT_POOL); // Belongs to
                                                       // the default pool
}

void
job_bswap_from_network (struct job *orig, struct job *dest) {
  // CHECK: what about the environment and frame info ?
  dest->id = ntohl (orig->id);
  dest->nprocs = ntohs (orig->nprocs);
  dest->status = ntohs (orig->status);

  /* Koj Stuff */
  dest->koj = ntohs (orig->koj);
  switch (dest->koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
    break;
  case KOJ_MENTALRAY:
  case KOJ_BLENDER:
  case KOJ_LIGHTWAVE:
  case KOJ_TERRAGEN:
  case KOJ_NUKE:
  case KOJ_AFTEREFFECTS:
  case KOJ_SHAKE:
  case KOJ_LUXRENDER:
    break;
  case KOJ_3DELIGHT:
  case KOJ_PIXIE:
  case KOJ_XSI:
    break;
  case KOJ_TURTLE:
    dest->koji.turtle.resx = ntohl (orig->koji.turtle.resx);
    dest->koji.turtle.resy = ntohl (orig->koji.turtle.resy);
    break;
  }

  dest->frame_info.ptr = NULL;
  dest->frame_start = ntohl (orig->frame_start);
  dest->frame_end = ntohl (orig->frame_end);
  dest->frame_step = ntohl (orig->frame_step);
  dest->frame_step = (orig->frame_step == 0) ? 1 : orig->frame_step; /* No 0 on step !! */
  dest->block_size = ntohl (orig->block_size);
  dest->avg_frame_time = ntohl (orig->avg_frame_time);
  dest->est_finish_time = ntohl (orig->est_finish_time);
  dest->fleft = ntohl (orig->fleft);
  dest->fdone = ntohl (orig->fdone);
  dest->ffailed = ntohl (orig->ffailed);

  dest->priority = ntohl (orig->priority);

  dest->flags = ntohl (orig->flags);

  /* Limits */
  job_limits_bswap_from_network (&orig->limits,&orig->limits);
}

void
job_bswap_to_network (struct job *orig, struct job *dest) {
  /* We make a copy coz we need to modify the values */
  memcpy (dest,orig,sizeof(struct job));

  dest->id = htonl (orig->id);
  dest->nprocs = htons (orig->nprocs);
  dest->status = htons (orig->status);

  /* Koj Stuff */
  switch (orig->koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
    break;
  case KOJ_MENTALRAY:
  case KOJ_BLENDER:
  case KOJ_LIGHTWAVE:
  case KOJ_TERRAGEN:
  case KOJ_NUKE:
  case KOJ_AFTEREFFECTS:
  case KOJ_SHAKE:
  case KOJ_LUXRENDER:
    break;
  case KOJ_3DELIGHT:
  case KOJ_PIXIE:
  case KOJ_XSI:
    break;
  case KOJ_TURTLE:
    dest->koji.turtle.resx = htonl (orig->koji.turtle.resx);
    dest->koji.turtle.resy = htonl (orig->koji.turtle.resy);
    break;
  }
  dest->koj = htons (orig->koj);

  dest->frame_info.ptr = NULL;  
  dest->frame_start = htonl (orig->frame_start);
  dest->frame_end = htonl (orig->frame_end);
  dest->frame_step = htonl (orig->frame_step);
  dest->block_size = htonl (orig->block_size);
  dest->avg_frame_time = htonl (orig->avg_frame_time);
  dest->est_finish_time = htonl (orig->est_finish_time);
  dest->fleft = htonl (orig->fleft);
  dest->fdone = htonl (orig->fdone);
  dest->ffailed = htonl (orig->ffailed);

  dest->priority = htonl (orig->priority);

  dest->flags = htonl (orig->flags);

  // Limits
  job_limits_bswap_to_network (&orig->limits,&dest->limits);

  // Filling the envvars with neutral values
  envvars_init(&dest->envvars);
  dest->envvars.nvariables = htons(orig->envvars.nvariables);
//  dest->envvars.variables.ptr = NULL;
//  dest->envvars.evshmid = (int64_t)-1; // 64bit 
}

int job_limits_passed (struct database *wdb, uint32_t ijob, uint32_t icomp) {
  /* This function should return 0 in case the limits are not met for the computer */
  uint32_t i;
  struct blocked_host *bh ;

  if (wdb->job[ijob].nprocs >= wdb->job[ijob].limits.nmaxcpus)
    return 0;

  if (computer_nrunning_job(&wdb->computer[icomp],ijob) >= wdb->job[ijob].limits.nmaxcpuscomputer)
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
  if ((wdb->computer[icomp].hwinfo.os == OS_CYGWIN) && !(wdb->job[ijob].limits.os_flags & OSF_CYGWIN))
    return 0;

  // Memory
  if (wdb->computer[icomp].hwinfo.memory < wdb->job[ijob].limits.memory)
    return 0;

  // Blocked hosts
  if (wdb->job[ijob].nblocked) {
    if ((bh = attach_blocked_host_shared_memory (wdb->job[ijob].bhshmid)) == (void *)-1) {
      // This should never happen though...
      return 0;
    }

    for (i=0;i<wdb->job[ijob].nblocked;i++) {
      if (strcmp(wdb->computer[icomp].hwinfo.name,bh[i].name) == 0) {
        return 0;
      }
    }

    if (bh != (void *)-1) {
      detach_blocked_host_shared_memory (bh);
    }
  }

  // Pools
  if (!computer_pool_exists(&wdb->computer[icomp].limits,wdb->job[ijob].limits.pool)) {
    return 0;
  }

  return 1;
}

void job_frame_info_init (struct frame_info *fi) {
  fi->status = FS_WAITING;
  fi->start_time = fi->end_time = 0;
  fi->exitcode = 0;
  fi->icomp = fi->itask = 0;
  fi->requeued = 0;
  fi->flags = 0;
}

void
job_limits_bswap_to_network (struct job_limits *orig, struct job_limits *dest) {
  dest->nmaxcpus         = htons (orig->nmaxcpus);
  dest->nmaxcpuscomputer = htons (orig->nmaxcpuscomputer);
  dest->os_flags         = htons (orig->os_flags);
  dest->memory           = htonl (orig->memory);
}

void
job_limits_bswap_from_network (struct job_limits *orig, struct job_limits *dest) {
  dest->nmaxcpus         = ntohs (orig->nmaxcpus);
  dest->nmaxcpuscomputer = ntohs (orig->nmaxcpuscomputer);
  dest->os_flags         = ntohs (orig->os_flags);
  dest->memory           = ntohl (orig->memory);
}

void job_logs_remove (struct job *job) {
  char dir[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
    /* This should never happen because we check at the begining of every program */
    return;
  }

  snprintf(dir,BUFFERLEN-1,"%s/%s",basedir,job->name);

  remove_dir(dir);
}

char *job_koj_string (struct job *job) {
  char *msg;

  switch (job->koj) {
  case KOJ_GENERAL:
    msg = "General";
    break;
  case KOJ_MAYA:
    msg = "Maya";
    break;
  case KOJ_MENTALRAY:
    msg = "Mental Ray";
    break;
  case KOJ_BLENDER:
    msg = "Blender";
    break;
  case KOJ_PIXIE:
    msg = "Pixie";
    break;
  case KOJ_3DELIGHT:
    msg = "3delight";
    break;
  case KOJ_LIGHTWAVE:
    msg = "Lightwave";
    break;
  case KOJ_NUKE:
    msg = "Nuke";
    break;
  case KOJ_TERRAGEN:
    msg = "Terragen";
    break;
  case KOJ_AFTEREFFECTS:
    msg = "After Effects";
    break;
  case KOJ_SHAKE:
    msg = "Shake";
    break;
  case KOJ_TURTLE:
    msg = "Turtle";
    break;
  case KOJ_MANTRA:
    msg = "Mantra/Houdini";
    break;
  case KOJ_XSI:
    msg = "XSI";
    break;
  case KOJ_LUXRENDER:
    msg = "Luxrender";
    break;
  default:
    msg = "DEFAULT (ERROR)";
  }

  return msg;
}

int job_available_no_icomp (struct database *wdb,uint32_t ijob, uint32_t *iframe) {
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
      && (wdb->job[wdb->job[ijob].dependid].status != JOBSTATUS_FINISHED)) {
    // If this job depends on another and that one hasn't finished, job is not available
    semaphore_release(wdb->semid);
    return 0;
  }

  if ((*iframe = job_first_frame_available_no_icomp (wdb,ijob)) == (uint32_t)-1) {
    semaphore_release(wdb->semid);
    return 0;
  }

  semaphore_release(wdb->semid);
  return 1;
}

uint32_t job_first_frame_available_no_icomp (struct database *wdb,uint32_t ijob) {
  /* To be called LOCKED */
  /* This function not only returns the first frame */
  /* available without updating the job structure */
  uint32_t i;
  uint32_t r = (uint32_t) -1;
  uint32_t nframes;
  struct frame_info *fi;

  nframes = job_nframes (&wdb->job[ijob]);
  if (nframes) {
    fi = attach_frame_shared_memory(wdb->job[ijob].fishmid);
    if (fi == (void *) -1)
      return -1;
    for (i=0;i<nframes;i++) {
      if (fi[i].status == FS_WAITING) {
        r = i;          /* return = current */
        break;
      }
    }
    detach_frame_shared_memory(fi);
  }

  return r;
}

int
job_block_host_add_by_name (struct job *job, char *name) {
  struct blocked_host *obh=NULL;
  struct blocked_host *nbh=NULL;
  int64_t nbhshmid;
  struct blocked_host single;
  int i;
  
  if (job->nblocked) {
    if ((obh = attach_blocked_host_shared_memory (job->bhshmid)) == (void *)-1) {
      return 0;
    }

    // TODO: block_host_exists
    // Search for coincidence
    for (i = 0; i < job->nblocked; i++) {
      if (strcmp (obh[i].name,name) == 0) {
        // Host already on the list of blocked hosts
        return 1;
      }
    }
  }

  if ((nbhshmid = get_blocked_host_shared_memory (job->nblocked+1)) == (int64_t)-1) {
    return 0;
  }

  if ((nbh = attach_blocked_host_shared_memory (nbhshmid)) == (void *)-1) {
    return 0;
  }

  if (job->nblocked) {
    memcpy (nbh,obh,sizeof(struct blocked_host)*job->nblocked);
    // Once copied we can remove the previous list
    detach_blocked_host_shared_memory (obh);
    if (shmctl ((int)job->bhshmid,IPC_RMID,NULL) == -1) {
      // TODO: log error
      // ...
    }
  }

  job->bhshmid = nbhshmid;

  snprintf(single.name,MAXNAMELEN,"%s",name);
  // Add to the end of the block list, we use the old nblocked value
  memcpy (&nbh[job->nblocked],&single,sizeof(single));
  job->nblocked++;

  return 1;
}

int
job_block_host_shared_memory_remove (int64_t shmid) {
  if (shmctl ((int)shmid,IPC_RMID,NULL) == -1) {
    drerrno_system = errno;
    drerrno = DRE_RMSHMEM;
    return 0;
  }
  return 1;
}

int
job_block_host_exists_by_name (struct job *job, char *name) {
  int i;
  struct blocked_host *obh,*tbh;
  int exists = 0;

  if (!job->nblocked) {
    return exists;
  }

  if ((obh = attach_blocked_host_shared_memory (job->bhshmid)) == (void *)-1) {
    // TODO
    return exists;
  }
  
  tbh = obh;
  for (i=0; i<job->nblocked; i++) {
    if (strcmp(tbh->name,name) == 0) {
      exists = 1;
      break;
    }
    tbh++;
  }

  return exists;
}

int
job_block_host_remove_by_name (struct job *job, char *name) {
  struct blocked_host *obh,*nbh;
  struct blocked_host *tnbh;
  int64_t nbhshmid;
  int i;

  if (!job_block_host_exists_by_name (job,name)) {
    return 1;
  }

  if (job->nblocked) {
    if ((obh = attach_blocked_host_shared_memory (job->bhshmid)) == (void *)-1) {
      // TODO
      return 0;
    }
    if ((nbhshmid = get_blocked_host_shared_memory (sizeof(struct blocked_host)*(job->nblocked-1))) == (int64_t)-1) {
      // TODO
      detach_blocked_host_shared_memory(obh);
      return 0;
    }
    if ((nbh = attach_blocked_host_shared_memory (nbhshmid)) == (void *)-1) {
      job_block_host_shared_memory_remove(nbhshmid);
      return 0;
    }
    tnbh = nbh;
    for (i=0; i < job->nblocked; i++) {
      if (strcmp(obh[i].name,name) != 0) {
        memcpy ((void*)tnbh,(void*)&obh[i],sizeof(*obh));
        tnbh++;
      } else {
        log_auto(L_INFO,"Deleted host %s from block list.",obh[i].name);
      }
    }
    // Once copied all but the removed host we can detach and remove the old shared memory
    detach_blocked_host_shared_memory (obh);
    detach_blocked_host_shared_memory (nbh);
    if (!job_block_host_shared_memory_remove(job->bhshmid)) {
      log_auto(L_WARNING,"job_block_host_remove_by_name(): could not remove previous blocked host list. Could be a memory leak."
	       " (%s)",strerror(drerrno_system));
    }
    job->bhshmid = nbhshmid;
    job->nblocked--;
    if (job->nblocked == 0) {
      job_block_host_shared_memory_remove(nbhshmid);
      job->bhshmid = -1;
      job->blocked_host.ptr = NULL;
    }
  }

  return 1;
}
