/*
 * $Header: /root/cvs/drqueue/drqman/drqm_jobs.h,v 1.14 2001/09/03 16:03:44 jorge Exp $
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

struct info_jdd {		/* job details dialog */
  GtkWidget *dialog;
  GtkWidget *lname;		/* label name */
  GtkWidget *lstatus;		/* label status */
  GtkWidget *lcmd;		/* label command */
  GtkWidget *lstartend;		/* start and end frames */
  GtkWidget *lpri;		/* priority */
  GtkWidget *lfrldf;		/* frames left, done and failed */
  GtkWidget *lavgt;		/* average time per frame */
  GtkWidget *lestf;		/* estimated finish time */
  GtkWidget *clist;		/* frame info clist */
  
  GtkWidget *menu;		/* Popup menu */
  gint row,column;		/* selected frame */
  int selected;			/* if a frame is selected */
};

struct info_msgd {		/* Maya script generator dialog */
  GtkWidget *dialog;
  GtkWidget *escene;		/* Entry scene */
  GtkWidget *fsscene;		/* File selector for the scene */
  GtkWidget *eproject;		/* Entry project */
  GtkWidget *fsproject;		/* File selector for the project */
  GtkWidget *eimage;		/* Entry image name */
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
  struct info_jdd jdd;		/* job details dialog */
  struct info_msgd msgd;	/* Maya script generator dialog */
};

struct info_drqm;

void CreateJobsPage (GtkWidget *notebook, struct info_drqm *info);
void drqm_update_joblist (struct info_drqm_jobs *info);

#endif /* _DRQM_JOBS_H */








