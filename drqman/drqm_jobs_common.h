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
//
// $Id$
//

#ifndef _DRQM_JOBS_COMMON_H_
#define _DRQM_JOBS_COMMON_H_

#include <gtk/gtk.h>

struct drqmj_sesframes {
  GtkWidget *eframe_start;
  GtkWidget *eframe_end;
  GtkWidget *eframe_step;
	GtkWidget *eblock_size;
};

struct drqmj_limits {
  GtkWidget *enmaxcpus;		/* Entries */
  GtkWidget *enmaxcpuscomputer;
  GtkWidget *lnmaxcpus;		/* Labels*/
  GtkWidget *lnmaxcpuscomputer;
	GtkWidget *lmemory;
	GtkWidget *ememory;
	GtkWidget *lpool;
	GtkWidget *epool;
  GtkWidget *cb_irix;		/* Check buttons */
  GtkWidget *cb_linux;
	GtkWidget *cb_osx;
	GtkWidget *cb_freebsd;
};

#endif
