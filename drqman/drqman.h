/*
 * $Header: /root/cvs/drqueue/drqman/drqman.h,v 1.3 2001/09/04 23:28:53 jorge Exp $
 */

#ifndef _DRQMAN_H_
#define _DRQMAN_H_

#include "drqm_jobs.h"
#include "drqm_computers.h"

struct info_drqm {
  struct info_drqm_computers idc; /* Computers tab */
  struct drqm_jobs_info idj;	/* Jobs tab */
};

#endif /* _DRQMAN_H */


