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
// DrQueue is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//

#ifndef _DRQM_COMMON_H_
#define _DRQM_COMMON_H_

#include "libdrqueue.h"
#include <gtk/gtk.h>

#ifdef __CYGWIN
extern FILE *file_null;
//#define stderr file_null
#endif

#define TOOLTIPS_DELAY 1000

#define DRQ_COMPILED_FOR ""
#define DRQ_TITLE "DrQueue Manager"

GtkWidget *ConfirmDialog (char *text, GList *callbacks);
GtkTooltips *TooltipsNew (void);

#endif /* _DRQM_COMMON_H */

