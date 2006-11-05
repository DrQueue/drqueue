//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
//
// DrQueue is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// DrQueue is distributed in the hope that it will be useful,
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

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "libdrqueue.h"
#include "drqm_jobs.h"
#include "drqm_computers.h"
#include "drqm_request.h"
#include "logger.h"

void
drqm_request_joblist (struct drqm_jobs_info *info) {
  /* This function is called non-blocked */
  /* This function is called from inside drqman */
  struct job *tjob;
  uint32_t njobs;

  njobs = request_job_list(&tjob,CLIENT);
  
  drqm_clean_joblist (info);
  if (njobs) {
    info->njobs = njobs;
    info->jobs = tjob;
  } else {
    info->njobs = 0;
    info->jobs = NULL;
  }
}

void drqm_clean_joblist (struct drqm_jobs_info *info) {
  if (info->jobs) {
    free_job_list(NULL,info);
    free (info->jobs);
    info->jobs = NULL;
  }
}

void
drqm_request_computerlist (struct drqm_computers_info *info) {
  struct computer *tcomputer;

  drqm_clean_computerlist (info);

  info->ncomputers = request_computer_list(&tcomputer,CLIENT);
  if (info->ncomputers == -1) {
    fprintf (stderr,"Error receiving computer list");
    return;
  }
  info->computers = tcomputer;

  return;
}

void
drqm_clean_computerlist (struct drqm_computers_info *info) {
  int i;

  if (info->computers) {
    for (i=0;i<info->ncomputers;i++) {
      //   fprintf (stderr,"drqm_clean_computerlist: Freeing computer %i\n",i);
      computer_free(&info->computers[i]);
    }
    free (info->computers);
    info->computers = NULL;
    info->ncomputers = 0;
  }
}

void drqm_request_job_delete (uint32_t jobid) {
  // This function sends the request to delete the job selected from the queue
  request_job_delete (jobid,CLIENT);
}

void drqm_request_job_stop (uint32_t jobid) {
  // This function sends the request to stop the job selected from the queue
  request_job_stop (jobid,CLIENT);
}

void drqm_request_job_hstop (uint32_t jobid) {
  // This function sends the request to hard stop the job selected from the queue
  request_job_hstop (jobid,CLIENT);
}

void drqm_request_job_continue (uint32_t jobid) {
  // This function sends the request to continue the (stopped) job selected from the queue
  request_job_continue (jobid,CLIENT);
}

void drqm_request_job_rerun (uint32_t jobid) {
  request_job_rerun (jobid,CLIENT);
}

void drqm_request_job_frame_waiting (uint32_t jobid,uint32_t frame) {
  request_job_frame_waiting (jobid,frame,CLIENT);
}

void drqm_request_job_frame_kill (uint32_t jobid,uint32_t frame) {
  request_job_frame_kill (jobid,frame,CLIENT);
}

void drqm_request_job_frame_finish (uint32_t jobid,uint32_t frame) {
  request_job_frame_finish (jobid,frame,CLIENT);
}

void drqm_request_job_frame_reset_requeued (uint32_t jobid,uint32_t frame) {
  request_job_frame_reset_requeued (jobid,frame,CLIENT);
}

void drqm_request_job_frame_kill_finish (uint32_t jobid,uint32_t frame) {
  request_job_frame_kill_finish (jobid,frame,CLIENT);
}

void drqm_request_slave_limits_nmaxcpus_set (char *slave,uint32_t nmaxcpus) {
  request_slave_limits_nmaxcpus_set (slave,nmaxcpus,CLIENT);
}

void drqm_request_slave_limits_maxfreeloadcpu_set (char *slave,uint32_t maxfreeloadcpu) {
  request_slave_limits_maxfreeloadcpu_set (slave,maxfreeloadcpu,CLIENT);
}

void drqm_request_slave_limits_autoenable_set (char *slave,uint32_t h, uint32_t m, unsigned char flags) {
  request_slave_limits_autoenable_set (slave,h,m,flags,CLIENT);
}

void drqm_request_slave_task_kill (char *slave,uint16_t itask) {
  request_slave_killtask (slave,itask,CLIENT);
}

void drqm_request_job_sesupdate (uint32_t jobid,uint32_t frame_start,uint32_t frame_end,
                                 uint32_t frame_step, uint32_t block_size) {
  request_job_sesupdate (jobid,frame_start,frame_end,frame_step,block_size,CLIENT);
}

void drqm_request_job_limits_nmaxcpus_set (uint32_t jobid, uint16_t nmaxcpus) {
  request_job_limits_nmaxcpus_set (jobid,nmaxcpus,CLIENT);
}

void drqm_request_job_limits_nmaxcpuscomputer_set (uint32_t jobid, uint16_t nmaxcpuscomputer) {
  request_job_limits_nmaxcpuscomputer_set (jobid,nmaxcpuscomputer,CLIENT);
}

void drqm_request_job_priority_update (uint32_t jobid, uint32_t priority) {
  request_job_priority_update (jobid,priority,CLIENT);
}

void drqm_request_slave_limits_pool_add (char *slave,char *pool) {
  request_slave_limits_pool_add (slave,pool,CLIENT);
}

void drqm_request_slave_limits_pool_remove (char *slave,char *pool) {
  request_slave_limits_pool_remove (slave,pool,CLIENT);
}

void drqm_request_slave_limits_enabled_set (char *slave, uint8_t enabled) {
  request_slave_limits_enabled_set (slave,enabled,CLIENT);
}
