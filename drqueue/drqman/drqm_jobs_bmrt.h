/* $Id: drqm_jobs_bmrt.h,v 1.1 2003/12/18 20:39:41 jorge Exp $ */

#ifndef _DRQM_JOBS_BMRT_H_
#define _DRQM_JOBS_BMRT_H_

#include <gtk/gtk.h>

struct drqmj_koji_bmrt {
  GtkWidget *escene;
  GtkWidget *fsscene;						/* File selector for the scene */
	GtkWidget *cbcrop,*ecropxmin,*ecropxmax,*ecropymin,*ecropymax;
	GtkWidget *cbsamples,*exsamples,*eysamples;
	GtkWidget *cbstats,*cbverbose,*cbbeep;
	GtkWidget *cbradiositysamples,*eradiositysamples;
	GtkWidget *cbraysamples,*eraysamples;

  GtkWidget *eviewcmd;
  GtkWidget *escript;						/* Entry script location */
  GtkWidget *fsscript;		/* File selector for the script directory */
};

struct drqm_jobs_info;

GtkWidget *jdd_koj_bmrt_widgets (struct drqm_jobs_info *info);
GtkWidget *dnj_koj_frame_bmrt (struct drqm_jobs_info *info);

#endif /* _DRQM_JOBS_BMRT_H_ */
