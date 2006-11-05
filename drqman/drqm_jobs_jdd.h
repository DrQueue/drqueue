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

#ifndef _DRQM_JOBS_JDD_H_
#define _DRQM_JOBS_JDD_H_

#include "drqm_jobs_common.h"
#include "drqm_computers.h"
#include "drqm_autorefresh.h"

struct drqmj_jddi {       /* job details dialog info*/
  GtkWidget *dialog;      /* Main dialog */
  GtkWidget *lname;      /* label name */
  GtkWidget *lowner;      /* label owner */
  GtkWidget *lstatus;      /* label status */
  GtkWidget *lcmd;      /* label command */
  GtkWidget *lstartend;      /* start and end frames */
  GtkWidget *lblock_size;      /* label block_size */
  GtkWidget *lpri;      /* priority */
  GtkWidget *cpri;      /* Combo */
  GtkWidget *epri;      /* entry priority when changing */
  GtkWidget *lfrldf;      /* frames left, done and failed */
  GtkWidget *lfpad;
  GtkWidget *lbs;       /* block size */
  GtkWidget *lsubmitt;      // label for submission time
  GtkWidget *lavgt;      /* average time per frame */
  GtkWidget *lestf;      /* estimated finish time */

  // Frames
  GtkWidget *clist;      /* frame info clist */
  GtkWidget *menu;      /* Popup menu */
  GtkWidget *swindow;      // Scrolled window so get can get the adjustments later

  // Blocked hosts
  GtkWidget *menu_bh;           // Blocked hosts menu
  GtkWidget *clist_bh;          // Blocked hosts clist
  struct drqm_computers_info bhdi_computers_info;

  gint row,column;       /* selected frame */
  int selected;        /* if a frame is selected */
  struct job job;        /* The struct of the selected job */
  struct drqmj_limits limits;  /* Limits info */
  struct drqmj_sesframes sesframes; /* Info about start, end, step frames */
  struct drqm_jobs_info *oldinfo; // Pointer to the previous info, so we can update the joblist
  struct drqm_autorefresh_info ari; // Autorefresh info
};

struct idle_info {
  int fd;
  GtkWidget *text;
};

void JobDetails(GtkWidget *menu_item, struct drqm_jobs_info *info);

#endif
