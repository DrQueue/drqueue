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
/* 
 * $Id$
 */
#ifndef _DRQM_JOBS_BLENDER_H_
#define _DRQM_JOBS_BLENDER_H_

#include <gtk/gtk.h>

#define KOJ_BLENDER_DFLT_VIEWCMD "display image.$DRQUEUE_FRAME.jpg"

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
