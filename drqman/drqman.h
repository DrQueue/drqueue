/*
 * $Header: /root/cvs/drqueue/drqman/drqman.h,v 1.2 2001/08/29 15:01:27 jorge Exp $
 */

#ifndef _DRQMAN_H_
#define _DRQMAN_H_

#include "drqm_jobs.h"
#include "drqm_computers.h"

struct info_drqm {
  struct info_drqm_computers idc; /* Computers tab */
  struct info_drqm_jobs idj;	/* Jobs tab */
};

#endif /* _DRQMAN_H */


