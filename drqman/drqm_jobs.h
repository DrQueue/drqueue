/*
 * $Header: /root/cvs/drqueue/drqman/drqm_jobs.h,v 1.19 2001/09/14 16:32:01 jorge Exp $
 */

#ifndef _DRQM_JOBS_H_
#define _DRQM_JOBS_H_

#include <gtk/gtk.h>
#include "libdrqueue.h"

struct drqmj_kojid_maya {
  GtkWidget *escene;
  GtkWidget *eproject;
  GtkWidget *eimage;
  GtkWidget *eviewcmd;
};

struct drqmj_dnji {		/* dialog new job */
  GtkWidget *dialog;
  GtkWidget *ename;
  GtkWidget *ecmd;
  GtkWidget *esf,*eef,*estf;	/* entry start frame, entry end frame, entry step frames */
  GtkWidget *cpri,*epri;	/* combo priority, entry priority */
  GtkWidget *fs;		/* File selector */

  /* koj */
  GtkWidget *ckoj;		/* combo koj */
  uint16_t koj;			/* koj */
  union koj_info koji;		/* koj info */
};

struct drqmj_jddi {		/* job details dialog */
  GtkWidget *dialog;		/* Main dialog */
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

struct drqmj_msgdi {		/* Maya script generator dialog */
  GtkWidget *dialog;
  GtkWidget *escene;		/* Entry scene */
  GtkWidget *fsscene;		/* File selector for the scene */
  GtkWidget *eproject;		/* Entry project */
  GtkWidget *fsproject;		/* File selector for the project */
  GtkWidget *eimage;		/* Entry image name */
  GtkWidget *fsscript;		/* File selectot for the script directory */
  GtkWidget *escript;		/* Entry script location */
};

struct drqm_jobs_info {
  GtkWidget *clist;		/* main clist */
  gint row, column;		/* selected job */
  GtkWidget *menu;		/* Popup menu */
  int selected;			/* if a job is selected */
  int ijob;			/* index to the selected job */
  uint32_t njobs;		/* Number of jobs in the list */
  struct job *jobs;		/* The job list */
  struct drqmj_dnji dnj;	/* dialog new job */
  struct drqmj_jddi jdd;	/* job details dialog */
  struct drqmj_msgdi msgd;	/* Maya script generator dialog */
};

struct info_drqm;

void CreateJobsPage (GtkWidget *notebook, struct info_drqm *info);
void drqm_update_joblist (struct drqm_jobs_info *info);

#endif /* _DRQM_JOBS_H */








