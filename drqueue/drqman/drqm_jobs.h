/*
 * $Header: /root/cvs/drqueue/drqman/drqm_jobs.h,v 1.6 2001/08/02 10:20:24 jorge Exp $
 */

#ifndef _DRQM_JOBS_H_
#define _DRQM_JOBS_H_

#include <gtk/gtk.h>
#include "libdrqueue.h"

struct info_dnj {		/* dialog new job */
  GtkWidget *dialog;
  GtkWidget *ename;
  GtkWidget *ecmd;
  GtkWidget *esf,*eef;		/* entry start frame, entry end frame */
  GtkWidget *cpri,*epri;	/* combo priority, entry priority */
  GtkWidget *fs;		/* File selector */
};

struct info_drqm_jobs {
  GtkWidget *clist;		/* main clist */
  gint row, column;
  GtkWidget *menu;		/* Popup menu */
  uint32_t njobs;		/* Number of jobs in the list */
  struct job *jobs;		/* The job list */
  struct info_dnj dnj;		/* dialog new job */
};

void CreateJobsPage (GtkWidget *);
void drqm_update_joblist (struct info_drqm_jobs *info);

#endif /* _DRQM_JOBS_H */


