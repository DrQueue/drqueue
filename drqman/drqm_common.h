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
 * $Header: /root/cvs/drqueue/drqman/drqm_common.h,v 1.2 2001/09/24 16:10:20 jorge Exp $
 */

#ifndef _DRQM_COMMON_H_
#define _DRQM_COMMON_H_

#include <gtk/gtk.h>
#include "libdrqueue.h"

#define TOOLTIPS_DELAY 1000

GtkWidget *ConfirmDialog (char *text, GList *callbacks);
GtkTooltips *TooltipsNew (void);

#ifdef __CYGWIN
char *conv_to_posix_path(char *win32_path);
char *conv_to_win32_path(char *posix_path);
#endif

#endif /* _DRQM_COMMON_H */

