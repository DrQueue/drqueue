//
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
//
// DrQueue is free software; you can redistribute it and/or modify
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

#ifndef _DRQMAN_H_
#define _DRQMAN_H_

#ifdef HAVE_CONFIG_H
#include <lconfig.h>
#endif

#include "drqm_jobs.h"
#include "drqm_computers.h"
#include "drqm_globals.h"

struct info_drqm {
  struct drqm_computers_info idc; /* Computers tab */
  struct drqm_jobs_info idj; /* Jobs tab */
  GtkWidget *main_window;
};

#endif /* _DRQMAN_H */


