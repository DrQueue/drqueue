/* $Id: drqm_request.h,v 1.8 2001/09/06 14:33:36 jorge Exp $ */

#ifndef _DRQM_REQUEST_H_
#define _DRQM_REQUEST_H_

#include "drqm_jobs.h"
#include "drqm_computers.h"

void drqm_request_joblist (struct drqm_jobs_info *info);
void clean_joblist (struct drqm_jobs_info *info);
void drqm_request_job_delete (struct drqm_jobs_info *info);
void drqm_request_job_stop (struct drqm_jobs_info *info);
void drqm_request_job_hstop (struct drqm_jobs_info *info);
void drqm_request_job_continue (struct drqm_jobs_info *info);
void drqm_request_job_frame_waiting (uint32_t jobid,uint32_t frame);
void drqm_request_job_frame_kill (uint32_t jobid,uint32_t frame);
void drqm_request_job_frame_finish (uint32_t jobid,uint32_t frame);

void drqm_request_computerlist (struct info_drqm_computers *info);
void clean_computerlist (struct info_drqm_computers *info);

#endif /* _DRQM_REQUEST_H_ */
