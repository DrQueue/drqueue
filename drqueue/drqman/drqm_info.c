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
 * $Id$
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

/* #include "eeelogo.xpm" */

#include "drqman.h"
#include "drqm_common.h"

/* Static functions declarations */
/* static GtkWidget *CreateLogoWidget (GtkWidget *widget); */


void CreateInfoPage (GtkWidget *notebook, struct info_drqm *info)
{
  /* This function receives the notebook widget to wich the new tab will append */
  GtkWidget *label;
  GtkWidget *container;
  GtkWidget *vbox;
/*   GtkWidget *logo; */
  GtkWidget *text;
  char msg[BUFFERLEN];

  /* Label */
  label = gtk_label_new ("Info");
  container = gtk_frame_new ("About DrQueue");
  gtk_container_border_width (GTK_CONTAINER(container),2);
  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(container),vbox);

/*   logo = CreateLogoWidget (info->main_window); */
/*   gtk_box_pack_start (GTK_BOX(vbox),logo,FALSE,FALSE,2); */

  snprintf (msg,BUFFERLEN-1,"DrQueue Version %s\nby Jorge Daza\n"
	    "jorge@drqueue.org",VERSION);
  text = gtk_label_new (msg);
  gtk_box_pack_start (GTK_BOX(vbox),text,FALSE,FALSE,2);

  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, label);

  gtk_widget_show_all(vbox);
  gtk_widget_show(label);
  gtk_widget_show(container);
}

/* GtkWidget *CreateLogoWidget (GtkWidget *widget) */
/* { */
/*   GtkWidget *toplevel; */
/*   GdkBitmap *l_mask = NULL; /\* Logo mask *\/ */
/*   GdkPixmap *l_data = NULL; /\* Logo data *\/ */

/*   toplevel = gtk_widget_get_toplevel(widget); */
/*   gtk_widget_realize(toplevel);	/\* Just in case *\/ */
/*   l_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&l_mask,NULL,(gchar**)eeelogo_xpm); */

/*   return gtk_pixmap_new(l_data,l_mask); */
/* } */

