/*
 * $Header: /root/cvs/drqueue/drqman/drqm_computers.h,v 1.1 2001/07/19 09:22:25 jorge Exp $
 */

#ifndef _DRQM_COMPUTERS_H
#define _DRQM_COMPUTERS_H

#include <gtk/gtk.h>
#include "libdrqueue.h"

struct info_drqm_computers {
  GtkWidget *clist;		/* main clist */
  gint row, column;
  GtkWidget *menu;		/* Popup menu */
  uint32_t ncomputers;		/* Number of computers in the list */
  struct computer *computers;		/* The computer list */
};

void CreateComputersPage (GtkWidget *);
void drqm_update_computerlist (struct info_drqm_computers *info);

#endif /* _DRQM_COMPUTERS_H */


