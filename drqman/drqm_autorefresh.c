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

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <gtk/gtk.h>

#include "drqm_autorefresh.h"

GtkWidget *CreateAutoRefreshWidgets (struct drqm_autorefresh_info *ari)
{
	GtkWidget *hbox;
  GtkWidget *cbutton;
	GtkWidget *entry;
	GtkWidget *label;
  
	hbox = gtk_hbox_new (FALSE,2);
  cbutton = gtk_check_button_new_with_label ("AutoRefresh every");
	g_signal_connect(G_OBJECT(cbutton),"clicked",G_CALLBACK(AutoRefreshCheckButtonToggled),ari);
	gtk_box_pack_start(GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
	ari->cbenabled = cbutton;
	entry = gtk_entry_new ();
	// g_signal_connect(G_OBJECT(entry),"changed",G_CALLBACK(AutoRefreshEntryChanged),ari);
	gtk_entry_set_text (GTK_ENTRY(entry),"5");
	gtk_widget_set_size_request (GTK_WIDGET(entry),30,20);
	ari->eseconds = entry;
	gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,2);
	label = gtk_label_new ("seconds");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);

  return hbox;
}

void AutoRefreshCheckButtonToggled (GtkWidget *cbutton, struct drqm_autorefresh_info *ari)
{
	int delay;

	if (GTK_TOGGLE_BUTTON(ari->cbenabled)->active) {
		gtk_entry_set_editable(GTK_ENTRY(ari->eseconds),FALSE);
	  delay = atoi(gtk_entry_get_text(GTK_ENTRY(ari->eseconds)));
		if (delay <= 0) {
			delay = 5;
			gtk_entry_set_text(GTK_ENTRY(ari->eseconds),"5");
		}
		ari->sourceid = g_timeout_add(delay * 1000,ari->callback,ari->data);
  } else {
		g_source_remove(ari->sourceid);
		gtk_entry_set_editable(GTK_ENTRY(ari->eseconds),TRUE);
  }
}

void AutoRefreshEntryChanged (GtkWidget *entry, struct drqm_autorefresh_info *ari)
{
	// Not necessary at the moment
}
