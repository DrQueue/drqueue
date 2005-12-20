// 
// Copyright (C) 2001,2002,2003,2004,2005 Jorge Daza Garcia-Blanes
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
// 
//
// $Id: drqm_jobs_aqsis.h 1330 2005-07-05 03:50:01Z jorge $
//

#ifndef _DRQM_JOBS_MANTRA_H_
#define _DRQM_JOBS_MANTRA_H_

#include <gtk/gtk.h>

#define KOJ_MANTRA_DFLT_VIEWCMD "display $DRQUEUE_RD/$DRQUEUE_IMAGE.$DRQUEUE_FRAME.tif"

struct drqmj_koji_mantra {
	GtkWidget *escene;
	GtkWidget *fsscene;						 /* File selector for the scene */
	GtkWidget *cbWH,*eWidth,*eHeight;
	GtkWidget *erenderdir;
	GtkWidget *fsrenderdir;
	GtkWidget *cbbucket,*ebucket; /*,*ecropxmax,*ecropymin,*ecropymax;*/
	GtkWidget *cblod,*elod;
	GtkWidget *cbvaryaa,*evaryaa;
	GtkWidget *cbsamples,*exsamples,*eysamples;
	//GtkWidget *cbstats,*cbverbose,*cbbeep;	
	GtkWidget *eviewcmd;
	GtkWidget *escript;						 /* Entry script location */
	GtkWidget *fsscript;					 /* File selectot for the script directory */
	GtkWidget *efile_owner;				 /* Owner of the rendered files */
	GtkWidget *cbraytrace;
	GtkWidget *cbaaoff;
	GtkWidget *cbbd, *ebd;
	GtkWidget *cbzDepth, *ezDepth;
	GtkWidget *cbCracks, *eCracks;
	GtkWidget *cbQuality, *eQuality;
	GtkWidget *cbQFiner, *eQFiner;
	GtkWidget *cbType, *eType;
	GtkWidget *cbSMultiplier, *eSMultiplier;
	GtkWidget *cbMPCache, *eMPCache;
	GtkWidget *cbMCache, *eMCache;
	GtkWidget *cbSMPolygon, *eSMPolygon;
	//GtkWidget *cbVerbose, *eVerbose;
};

struct drqm_jobs_info;
//struct drqbod_dnji;

GtkWidget *jdd_koj_mantra_widgets (struct drqm_jobs_info *info);
GtkWidget *dnj_koj_frame_mantra (struct drqm_jobs_info *info);

#endif /* _DRQM_JOBS_AQSIS_H_ */
