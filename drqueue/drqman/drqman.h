/*
 * $Header: /root/cvs/drqueue/drqman/drqman.h,v 1.1 2001/08/28 21:22:10 jorge Exp $
 */

#ifndef _DRQMAN_H_
#define _DRQMAN_H_

#include "drqm_jobs.h"
#include "drqm_computers.h"

struct info_drqm {
  struct info_drqm_computers ct; /* Computers tab */
  struct info_drqm_jobs jt;	/* Jobs tab */
};

#endif /* _DRQMAN_H */


