/*
 * $Header: /root/cvs/drqueue/drqman/drqm_jobs.h,v 1.1 2001/07/13 15:23:33 jorge Exp $
 */

#ifndef _DRQM_JOBS_H
#define _DRQM_JOBS_H

#include <gtk/gtk.h>

struct info_drqm_jobs {
  GtkWidget *clist;		/* main clist */
  gint row, column;
  GtkWidget *menu;		/* Popup menu */
};

void CreateJobsPage (GtkWidget *);
GtkWidget *CreateJobsList(struct info_drqm_jobs *info);
GtkWidget *CreateClist (GtkWidget *window);
GtkWidget *CreateButtonRefresh (struct info_drqm_jobs *info);
void PressedButtonRefresh (GtkWidget *b, struct info_drqm_jobs *info);

#endif /* _DRQM_JOBS_H */


