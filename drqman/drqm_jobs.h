/*
 * $Header: /root/cvs/drqueue/drqman/drqm_jobs.h,v 1.9 2001/08/27 15:16:54 jorge Exp $
 */

#ifndef _DRQM_JOBS_H_
#define _DRQM_JOBS_H_

#include <gtk/gtk.h>
#include "libdrqueue.h"

struct info_dnj {		/* dialog new job */
  GtkWidget *dialog;
  GtkWidget *ename;
  GtkWidget *ecmd;
  GtkWidget *esf,*eef,*estf;	/* entry start frame, entry end frame, entry step frames */
  GtkWidget *cpri,*epri;	/* combo priority, entry priority */
  GtkWidget *fs;		/* File selector */
};

struct info_djd {		/* dialog job details */
  GtkWidget *dialog;
  GtkWidget *lname;		/* label name */
  GtkWidget *lcmd;		/* label command */
  GtkWidget *lstartend;		/* start and end frames */
  GtkWidget *lpri;		/* priority */
  GtkWidget *lfrldf;		/* frames left, done and failed */
  GtkWidget *lavgt;		/* average time per frame */
  GtkWidget *lestf;		/* estimated finish time */
  GtkWidget *clist;		/* frame info clist */
};

struct info_drqm_jobs {
  GtkWidget *clist;		/* main clist */
  gint row, column;		/* selected job */
  GtkWidget *menu;		/* Popup menu */
  int selected;			/* if a job is selected */
  int ijob;			/* index to the selected job */
  uint32_t njobs;		/* Number of jobs in the list */
  struct job *jobs;		/* The job list */
  struct info_dnj dnj;		/* dialog new job */
  struct info_djd djd;		/* dialog job details */
};

void CreateJobsPage (GtkWidget *);
void drqm_update_joblist (struct info_drqm_jobs *info);

#endif /* _DRQM_JOBS_H */


