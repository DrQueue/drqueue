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

#ifndef _DRQM_JOBS_3DELIGHT_H_
#define _DRQM_JOBS_3DELIGHT_H_

#include <gtk/gtk.h>

#define KOJ_3DELIGHT_DFLT_VIEWCMD "imf_disp $DRQUEUE_RD/$DRQUEUE_IMAGE.$DRQUEUE_FRAME.iff"

struct drqmj_koji_3delight {
  GtkWidget *escene;
  GtkWidget *fsscene;       /* File selector for the scene */
  GtkWidget *eviewcmd;
  GtkWidget *escript;       /* Entry script location */
  GtkWidget *fsscript;      /* File selectot for the script directory */
  GtkWidget *efile_owner;     /* Owner of the rendered files */
};

struct drqm_jobs_info;

GtkWidget *jdd_koj_3delight_widgets (struct drqm_jobs_info *info);
GtkWidget *dnj_koj_frame_3delight (struct drqm_jobs_info *info);

#endif /* _DRQM_JOBS_3DELIGHT_H_ */
