/*
 * $Header: /root/cvs/drqueue/drqman/drqm_computers.h,v 1.2 2001/08/28 21:49:26 jorge Exp $
 */

#ifndef _DRQM_COMPUTERS_H
#define _DRQM_COMPUTERS_H

#include <gtk/gtk.h>
#include "libdrqueue.h"

struct info_cdd {		/* computer details dialog */
  GtkWidget *dialog;
  GtkWidget *lname;		/* label name */

  GtkWidget *menu;		/* Popup menu */
  gint row,column;		/* selected processor */
  int selected;			/* if a processor is selected */
};

struct info_drqm_computers {
  GtkWidget *clist;		/* main clist */
  gint row, column;
  GtkWidget *menu;		/* Popup menu */
  int selected;			/* if a computer is selected */
  uint32_t ncomputers;		/* Number of computers in the list */
  uint32_t icomp;		/* id of the selected computer */
  struct computer *computers;	/* The computer list */
  struct info_cdd cdd;
};

struct info_drqm;

void CreateComputersPage (GtkWidget *notebook,struct info_drqm *info);
void drqm_update_computerlist (struct info_drqm_computers *info);

#endif /* _DRQM_COMPUTERS_H */


