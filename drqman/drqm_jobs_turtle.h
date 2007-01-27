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
/* $Id$ */

#ifndef _DRQM_JOBS_TURTLE_H_
#define _DRQM_JOBS_TURTLE_H_

#include <gtk/gtk.h>

#ifdef __CYGWIN
#define KOJ_TURTLE_DFLT_VIEWCMD "tif"
#else
#define KOJ_TURTLE_DFLT_VIEWCMD "fcheck $DRQUEUE_RD/$DRQUEUE_IMAGE.$DRQUEUE_PADFRAME.sgi"
#endif

struct drqmj_koji_turtle {
  GtkWidget *escene;
  GtkWidget *fsscene;  /* File selector for the scene */
  GtkWidget *erenderdir;
  GtkWidget *fsrenderdir; /* File selector for the output directory */
  GtkWidget *eprojectdir;
  GtkWidget *fsprojectdir;
  GtkWidget *eimage;
  GtkWidget *eviewcmd;
  GtkWidget *escript;  /* Entry script location */
  GtkWidget *fsscript;  /* File selectot for the script directory */
  GtkWidget *efile_owner; /* Owner of the rendered files */
  GtkWidget *ecamera;
  GtkWidget *eresx;
  GtkWidget *eresy;
  GtkWidget *cbusemaya70;
};

struct drqm_jobs_info;

GtkWidget *jdd_koj_turtle_widgets (struct drqm_jobs_info *info);
GtkWidget *dnj_koj_frame_turtle (struct drqm_jobs_info *info);

#endif /* _DRQM_JOBS_TURTLE_H_ */
