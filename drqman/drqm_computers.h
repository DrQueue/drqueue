/*
 * $Header: /root/cvs/drqueue/drqman/drqm_computers.h,v 1.4 2001/09/07 09:16:51 jorge Exp $
 */

#ifndef _DRQM_COMPUTERS_H
#define _DRQM_COMPUTERS_H

#include <gtk/gtk.h>
#include "libdrqueue.h"

struct drqmc_cddi {		/* computer details dialog information */
  GtkWidget *dialog;
  GtkWidget *lname;		/* label name */
  GtkWidget *los;		/* OS */
  GtkWidget *lcpuinfo;		/* cpu info */
  GtkWidget *lloadavg;		/* load average */
  GtkWidget *lntasks;		/* Number of tasks running */

  GtkWidget *clist;		/* List of tasks */
  GtkWidget *menu;		/* Popup menu */
  gint row,column;		/* selected task */
  int selected;			/* if a task. is selected */
};

struct drqm_computers_info {
  GtkWidget *clist;		/* main clist */
  gint row, column;
  GtkWidget *menu;		/* Popup menu */
  int selected;			/* if a computer is selected */
  uint32_t ncomputers;		/* Number of computers in the list */
  uint32_t icomp;		/* id of the selected computer */
  struct computer *computers;	/* The computer list */
  struct drqmc_cddi cdd;
};

struct info_drqm;

void CreateComputersPage (GtkWidget *notebook,struct info_drqm *info);
void drqm_update_computerlist (struct drqm_computers_info *info);

#endif /* _DRQM_COMPUTERS_H */


