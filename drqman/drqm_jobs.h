//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
//
// DrQueue is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// DrQueue is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
//
// $Id$
//

#ifndef _DRQM_JOBS_H_
#define _DRQM_JOBS_H_

#include <gtk/gtk.h>
#include "libdrqueue.h"
#include "drqm_jobs_common.h"
#include "drqm_jobs_jdd.h"
#include "drqm_autorefresh.h"

// Script generators
#include "drqm_jobs_maya.h"
#include "drqm_jobs_mentalray.h"
#include "drqm_jobs_blender.h"
#include "drqm_jobs_bmrt.h"
#include "drqm_jobs_mantra.h"
#include "drqm_jobs_aqsis.h"
#include "drqm_jobs_pixie.h"
#include "drqm_jobs_3delight.h"
#include "drqm_jobs_lightwave.h"
#include "drqm_jobs_aftereffects.h"
#include "drqm_jobs_shake.h"
#include "drqm_jobs_terragen.h"
#include "drqm_jobs_nuke.h"
#include "drqm_jobs_turtle.h"
#include "drqm_jobs_xsi.h"

#include <string.h>

enum {
  DNJ_FLAGS_DEPEND_COL_ID = 0,
  DNJ_FLAGS_DEPEND_COL_NAME,
  DNJ_FLAGS_DEPEND_NUM_COLS
};

struct drqmj_flags {
  GtkWidget *cbmailnotify;   /* Check button */
  GtkWidget *cbdifemail; /* Specific email for mail notification ? */
  GtkWidget *edifemail;     /* Entry for the email */
  GtkWidget *cbjobdepend;    // Depends on another job
  GtkWidget *ejobdepend;    // Which one ?
  GtkWidget *bjobdepend;    // Show a list
  GtkListStore *store;     // List of jobs
  GtkTreeView *view;
  GtkWidget *cbjobdelete;    // Delete job when finished
};

enum {
  DNJ_ENVVARS_COL_NAME = 0,
  DNJ_ENVVARS_COL_VALUE,
  DNJ_ENVVARS_NUM_COLS
};

struct drqmj_envvars {
  GtkListStore *store;
  GtkTreeView *view;
  struct envvars envvars;
  GtkWidget *ename;
  GtkWidget *evalue;
};

struct drqmj_dnji {       /* dialog new job info */
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *ename;
  GtkWidget *ecmd;
  GtkWidget *esf,*eef,*estf; /* entry start frame, entry end frame, entry step frames */
  GtkWidget *ebs,*efp;       /* entry block_size, entry frame pad */
  GtkWidget *cpri,*epri;     /* combo priority, entry priority */
  GtkWidget *fs;             /* File selector */

  /* koj */
  GtkWidget *ckoj;       /* combo koj */
  GtkWidget *fkoj;       /* frame koj */
  GtkWidget *vbkoj;      /* vbox inside the koj's tab */
  uint16_t koj;          /* koj */
  struct drqmj_koji_maya koji_maya; /* koj info for maya */
  struct drqmj_koji_mentalray koji_mentalray; /* koj info for mentalray */
  struct drqmj_koji_blender koji_blender; /* koj info for blender */
  struct drqmj_koji_bmrt koji_bmrt; /* koj info for bmrt */
  struct drqmj_koji_aqsis koji_aqsis; /* koj info for aqsis */
  struct drqmj_koji_mantra koji_mantra; /* koj info for aqsis */
  struct drqmj_koji_pixie koji_pixie; /* koj info for pixie */
  struct drqmj_koji_3delight koji_3delight;
  struct drqmj_koji_lightwave koji_lightwave;
  struct drqmj_koji_nuke koji_nuke;
  struct drqmj_koji_aftereffects koji_aftereffects;
  struct drqmj_koji_shake koji_shake;
  struct drqmj_koji_terragen koji_terragen;
  struct drqmj_koji_turtle koji_turtle;
  struct drqmj_koji_xsi koji_xsi;

  struct drqmj_limits limits;   /* limits info */
  struct drqmj_flags flags;     /* flags info */
  struct drqmj_envvars envvars; // Environment variables info

  int submitstopped;            /* Set if the job has to be stopped just after submission */
};

struct drqm_jobs_info {
  GtkWidget *clist;                  /* main clist */
  gint row, column;                  /* selected job */
  GtkWidget *menu;                   /* Popup menu */
  struct drqm_autorefresh_info ari;  // AutoRefresh info
  int selected;             /* if a job is selected */
  int ijob;                 /* index to the selected job */
  uint32_t njobs;           /* Number of jobs in the list */
  struct job *jobs;         /* The job list */
  struct drqmj_dnji dnj;    /* dialog new job */
  struct drqmj_jddi jdd;    /* job details dialog */
};


struct info_drqm;

void CreateJobsPage (GtkWidget *notebook, struct info_drqm *info);
void drqm_update_joblist (struct drqm_jobs_info *info);
void free_job_list(GtkWidget *joblist,gpointer userdata);

void DeleteJob (GtkWidget *menu_item, struct drqm_jobs_info *info);
void StopJob (GtkWidget *menu_item, struct drqm_jobs_info *info);
void HStopJob (GtkWidget *menu_item, struct drqm_jobs_info *info);
void ContinueJob (GtkWidget *menu_item, struct drqm_jobs_info *info);
void ReRunJob (GtkWidget *menu_item, struct drqm_jobs_info *info);

void jobs_column_clicked (GtkCList *clist, gint column, struct drqm_jobs_info *info);
int jobs_cmp_id (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_name (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_owner (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_status (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_processors (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_left (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_done (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_failed (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_total (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_pri (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int jobs_cmp_pool (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);

#endif /* _DRQM_JOBS_H */
