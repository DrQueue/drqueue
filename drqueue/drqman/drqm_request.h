/* $Id: drqm_request.h,v 1.2 2001/07/17 10:23:12 jorge Exp $ */

#ifndef _DRQM_REQUEST_H_
#define _DRQM_REQUEST_H_

#include "drqm_jobs.h"

void drqm_request_joblist (struct info_drqm_jobs *info);
void clean_joblist (struct info_drqm_jobs *info);

#endif /* _DRQM_REQUEST_H_ */
