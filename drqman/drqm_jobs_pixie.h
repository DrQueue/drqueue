/* 
 * $Id: drqm_jobs_pixie.h,v 1.1 2004/03/09 18:54:29 jorge Exp $
 */
#ifndef _DRQM_JOBS_PIXIE_H_
#define _DRQM_JOBS_PIXIE_H_

#include <gtk/gtk.h>

struct drqmj_koji_pixie {
  GtkWidget *escene;
  GtkWidget *fsscene;						/* File selector for the scene */
  GtkWidget *eviewcmd;
  GtkWidget *escript;						/* Entry script location */
  GtkWidget *fsscript;		/* File selectot for the script directory */
};

struct drqm_jobs_info;

GtkWidget *dnj_koj_frame_pixie (struct drqm_jobs_info *info);
GtkWidget *jdd_koj_pixie_widgets (struct drqm_jobs_info *info);

#endif /* _DRQM_JOBS_PIXIE_H_ */
