//
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
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
// $Header: /root/cvs/drqueue/drqman/drqm_computers.h,v 1.8 2002/02/26 15:52:05 jorge Exp $
//

#ifndef _DRQM_COMPUTERS_H
#define _DRQM_COMPUTERS_H

#include <gtk/gtk.h>
#include "libdrqueue.h"
#include "drqm_autorefresh.h"

enum {
  CDD_POOL_COL_NAME = 0,
  CDD_POOL_NUM_COLS
};

struct drqmc_cddi {      /* computer details dialog information */
  GtkWidget *dialog;
  GtkWidget *lname;       /* label name */
  GtkWidget *los;        /* OS */
  GtkWidget *lcpuinfo;     /* cpu info */
  GtkWidget *lmemory;      // Memory label
  GtkWidget *lloadavg;     /* load average */
  GtkWidget *lntasks;      /* Number of tasks running */

  struct drqm_autorefresh_info ari; // Autorefresh stuff

  struct cdd_li {        /* Computer details dialog limits info */
    GtkWidget *lenabled;    // Enabled label
    GtkWidget *lnmaxcpus;    /* Label maximum number of cpus */
    GtkWidget *enmaxcpus;    /* Entry */
    GtkWidget *lmaxfreeloadcpu; /* Label maximum free load cpu */
    GtkWidget *emaxfreeloadcpu; /* Entry */
    GtkWidget *cautoenable;   /* Autoenable checkbox */
    GtkWidget *lautoenabletime; /* Autoenable time */
    GtkWidget *eautoenabletime_h; /* Entry for the hour */
    GtkWidget *eautoenabletime_m; /* Entry for the minutes */
    GtkWidget *lpools;     // Label Pools
    GtkWidget *epool;      // Entry pool
    GtkListStore *pool_store;  // List of pools
    GtkTreeView *pool_view;
  }
  limits;

  GtkWidget *clist;       /* List of tasks */
  GtkWidget *menu;       /* Popup menu */
  gint row,column;       /* selected task */
  int selected;         /* if a task. is selected */
};

struct drqm_computers_info {
  GtkWidget *swindow;      // Scrolled window
  GtkWidget *clist;       /* main clist */
  gint row, column;
  GtkWidget *menu;       /* Popup menu */
  int selected;         /* if a computer is selected */
  uint32_t ncomputers;     /* Number of computers in the list */
  uint32_t icomp;        /* id of the selected computer */
  struct computer *computers;  /* The computer list */
  struct drqmc_cddi cdd;
  struct drqm_autorefresh_info ari; // Autorefresh infox
};

struct info_drqm;

void CreateComputersPage (GtkWidget *notebook,struct info_drqm *info);
void drqm_update_computerlist (struct drqm_computers_info *info);

void computers_column_clicked (GtkCList *clist, gint column, struct drqm_computers_info *info);
int computers_cmp_id (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int computers_cmp_enabled (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int computers_cmp_running (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int computers_cmp_name (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int computers_cmp_os (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int computers_cmp_cpus (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int computers_cmp_loadavg (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
int computers_cmp_pools (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);

#endif /* _DRQM_COMPUTERS_H */
