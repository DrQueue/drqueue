/* $Id: drqm_jobs_maya.h,v 1.1 2003/12/18 04:11:07 jorge Exp $ */

#ifndef _DRQM_JOBS_MAYA_H_
#define _DRQM_JOBS_MAYA_H_

#include <gtk/gtk.h>

struct drqmj_koji_maya {
  GtkWidget *escene;
  GtkWidget *fsscene;		/* File selector for the scene */
  GtkWidget *erenderdir;
  GtkWidget *fsrenderdir;	/* File selector for the output directory */
  GtkWidget *eimage;
  GtkWidget *eviewcmd;
  GtkWidget *escript;		/* Entry script location */
  GtkWidget *fsscript;		/* File selectot for the script directory */
  GtkWidget *efile_owner;	/* Owner of the rendered files */
};

struct drqm_jobs_info;

GtkWidget *jdd_koj_maya_widgets (struct drqm_jobs_info *info);
GtkWidget *dnj_koj_frame_maya (struct drqm_jobs_info *info);

#endif /* _DRQM_JOBS_MAYA_H_ */
