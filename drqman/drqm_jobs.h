/*
 * $Header: /root/cvs/drqueue/drqman/drqm_jobs.h,v 1.5 2001/07/19 09:07:10 jorge Exp $
 */

#ifndef _DRQM_JOBS_H_
#define _DRQM_JOBS_H_

#include <gtk/gtk.h>
#include "libdrqueue.h"

struct info_drqm_jobs {
  GtkWidget *clist;		/* main clist */
  gint row, column;
  GtkWidget *menu;		/* Popup menu */
  uint32_t njobs;		/* Number of jobs in the list */
  struct job *jobs;		/* The job list */
};

void CreateJobsPage (GtkWidget *);
void drqm_update_joblist (struct info_drqm_jobs *info);

#endif /* _DRQM_JOBS_H */


