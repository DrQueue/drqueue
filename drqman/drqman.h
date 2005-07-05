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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
// 
/*
 * $Header: /root/cvs/drqueue/drqman/drqman.h,v 1.5 2001/11/08 11:47:42 jorge Exp $
 */

#ifndef _DRQMAN_H_
#define _DRQMAN_H_

#include "drqm_jobs.h"
#include "drqm_computers.h"

struct info_drqm {
	struct drqm_computers_info idc; /* Computers tab */
	struct drqm_jobs_info idj;	/* Jobs tab */
	GtkWidget *main_window;
};

#endif /* _DRQMAN_H */


