/* $Id: drqm_request.h,v 1.4 2001/08/06 12:35:06 jorge Exp $ */

#ifndef _DRQM_REQUEST_H_
#define _DRQM_REQUEST_H_

#include "drqm_jobs.h"
#include "drqm_computers.h"

void drqm_request_joblist (struct info_drqm_jobs *info);
void clean_joblist (struct info_drqm_jobs *info);
void drqm_request_job_delete (struct info_drqm_jobs *info);


void drqm_request_computerlist (struct info_drqm_computers *info);
void clean_computerlist (struct info_drqm_computers *info);

#endif /* _DRQM_REQUEST_H_ */
