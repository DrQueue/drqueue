/* 
 * $Id: drqm_jobs_blender.h,v 1.1 2003/12/18 04:11:07 jorge Exp $
 */
#ifndef _DRQM_JOBS_BLENDER_H_
#define _DRQM_JOBS_BLENDER_H_

#include <gtk/gtk.h>

struct drqmj_koji_blender {
  GtkWidget *escene;
  GtkWidget *fsscene;						/* File selector for the scene */
  GtkWidget *eviewcmd;
  GtkWidget *escript;						/* Entry script location */
  GtkWidget *fsscript;		/* File selectot for the script directory */
};

struct drqm_jobs_info;

GtkWidget *dnj_koj_frame_blender (struct drqm_jobs_info *info);
GtkWidget *jdd_koj_blender_widgets (struct drqm_jobs_info *info);

#endif /* _DRQM_JOBS_BLENDER_H_ */
