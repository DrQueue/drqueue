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
/* $Id$ */

#ifndef _DRQM_REQUEST_H_
#define _DRQM_REQUEST_H_

#include "drqm_jobs.h"
#include "drqm_computers.h"

/* Requests to MASTER*/
/* Jobs related functions */
void drqm_clean_joblist (struct drqm_jobs_info *info);
void drqm_request_joblist (struct drqm_jobs_info *info);
void drqm_request_job_delete (uint32_t jobid);
void drqm_request_job_stop (uint32_t jobid);
void drqm_request_job_hstop (uint32_t jobid);
void drqm_request_job_continue (uint32_t jobid);
void drqm_request_job_frame_waiting (uint32_t jobid,uint32_t frame);
void drqm_request_job_frame_kill (uint32_t jobid,uint32_t frame);
void drqm_request_job_frame_finish (uint32_t jobid,uint32_t frame);
void drqm_request_job_frame_kill_finish (uint32_t jobid,uint32_t frame);
void drqm_request_job_sesupdate (uint32_t jobid,uint32_t frame_start,uint32_t frame_end,
								                uint32_t frame_step, uint32_t block_size);
void drqm_request_job_limits_nmaxcpus_set (uint32_t jobid, uint16_t nmaxcpus);
void drqm_request_job_limits_nmaxcpuscomputer_set (uint32_t jobid, uint16_t nmaxcpuscomputer);
void drqm_request_job_priority_update (uint32_t jobid, uint32_t priority);
void drqm_request_job_frame_reset_requeued (uint32_t jobid,uint32_t frame);

/* Computer related functions */
void drqm_clean_computerlist (struct drqm_computers_info *info);
void drqm_request_computerlist (struct drqm_computers_info *info);

void drqm_request_slave_limits_nmaxcpus_set (char *slave,uint32_t nmaxcpus);
void drqm_request_slave_limits_maxfreeloadcpu_set (char *slave,uint32_t maxfreeloadcpu);
void drqm_request_slave_limits_autoenable_set (char *slave,uint32_t h, uint32_t m);
void drqm_request_slave_task_kill (char *slave,uint16_t itask);

#endif /* _DRQM_REQUEST_H_ */
