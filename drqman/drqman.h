/*
 * $Header: /root/cvs/drqueue/drqman/drqman.h,v 1.5 2001/11/08 11:47:42 jorge Exp $
 */

#ifndef _DRQMAN_H_
#define _DRQMAN_H_

#include "drqm_jobs.h"
#include "drqm_computers.h"

struct info_drqm {
  struct drqm_computers_info idc; /* Computers tab */
  struct drqm_jobs_info idj;	/* Jobs tab */
  GtkWidget *main_window;
};

#endif /* _DRQMAN_H */


