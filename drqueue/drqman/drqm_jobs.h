/*
 * $Header: /root/cvs/drqueue/drqman/drqm_jobs.h,v 1.4 2001/07/17 15:10:12 jorge Exp $
 */

#ifndef _DRQM_JOBS_H
#define _DRQM_JOBS_H

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
GtkWidget *CreateJobsList(struct info_drqm_jobs *info);
GtkWidget *CreateClist (GtkWidget *window);
GtkWidget *CreateButtonRefresh (struct info_drqm_jobs *info);
void PressedButtonRefresh (GtkWidget *b, struct info_drqm_jobs *info);
void drqm_update_joblist (struct info_drqm_jobs *info);

#endif /* _DRQM_JOBS_H */


