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

#include "drqman.h"
#include "drqm_common.h"

// Icons
#include "info_icon.h"

/* Static functions declarations */
extern GdkPixbuf *drqman_icon;

void CreateInfoPage (GtkWidget *notebook, struct info_drqm *info) {
  /* This function receives the notebook widget to wich the new tab will append */
  GtkWidget *label;
  GtkWidget *container;
  GtkWidget *vbox;
  GtkWidget *logo;
  GtkWidget *text;
  GtkWidget *hbox;
  GtkWidget *icon;
  GdkPixbuf *info_icon_pb;
  char msg[BUFFERLEN];
  
   // fix compiler warning
  (void)info;

  container = gtk_frame_new ("About DrQueue");
  gtk_container_border_width (GTK_CONTAINER(container),2);
  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(container),vbox);

  logo = gtk_image_new_from_pixbuf (drqman_icon);
  gtk_box_pack_start (GTK_BOX(vbox),logo,FALSE,FALSE,2);

  snprintf (msg, BUFFERLEN-1, "DrQueue version %s\nby Jorge Daza and the DrQueue development team\n"
            DRQ_COMPILED_FOR
            "\nCommunity support: <a href=\"https://ssl.drqueue.org/cwebsite/drqueue_community/\">DrQueue community forums</a>\n"
            "Commercial support: <a href=\"mailto:support@drqueue.org\">support@drqueue.org</a>\n"
            "Documentation: <a href=\"https://ssl.drqueue.org/redmine/projects/drqueue/wiki/Documentation/\">Redmine wiki</a>\n"
            "Bug reports: <a href=\"https://ssl.drqueue.org/redmine/projects/drqueue/issues/\">Redmine bugtracker</a>"
            , get_version_complete());
  text = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL(text), msg);
  gtk_box_pack_start (GTK_BOX(vbox),text,FALSE,FALSE,2);


  /* Label */
  label = gtk_label_new ("Info");
  gtk_widget_show(label);
  // Image
  info_icon_pb = gdk_pixbuf_new_from_inline (1019,info_icon,0,NULL);
  icon = gtk_image_new_from_pixbuf (info_icon_pb);
  gtk_widget_show(icon);
  hbox = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),icon,TRUE,TRUE,2);
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,2);
  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, hbox);

  gtk_widget_show_all(vbox);
  gtk_widget_show(label);
  gtk_widget_show(container);
}

