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

#include <gtk/gtk.h>

struct drqm_autorefresh_info {
	guint sourceid;               // id for the autorefresh source 
	GtkWidget *cbenabled;         // AutoRefresh cbutton
	GtkWidget *eseconds;          // AutoRefresh text entry
	GSourceFunc callback;         // Function to call for refresh
	gpointer data;                // Argument to pass to callback
};

// AutoRefresh
GtkWidget *CreateAutoRefreshWidgets (struct drqm_autorefresh_info *ari);
void AutoRefreshCheckButtonToggled (GtkWidget *cbutton, struct drqm_autorefresh_info *ari);
void AutoRefreshEntryChanged (GtkWidget *entry, struct drqm_autorefresh_info *ari);

