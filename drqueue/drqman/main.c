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
 * $Header: /root/cvs/drqueue/drqman/main.c,v 1.8 2001/11/23 11:51:41 jorge Exp $
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "drqman.h"
#include "notebook.h"
#include "libdrqueue.h"

static struct info_drqm info;

int main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *main_vbox;
  char rc_file[MAXCMDLEN];

  if (!common_environment_check()) {
    fprintf (stderr,"Error checking the environment: %s\n",drerrno_str());
    exit (1);
  }

  gtk_init(&argc,&argv);
  snprintf(rc_file,MAXCMDLEN-1,"%s/etc/drqman.rc",getenv("DRQUEUE_ROOT"));
  gtk_rc_parse(rc_file);

  /* Init for button boxes */
  gtk_hbutton_box_set_layout_default (GTK_BUTTONBOX_SPREAD);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window),"DrQueue Manager");
	gtk_window_set_default_size(GTK_WINDOW(window),700,400);
  gtk_container_border_width(GTK_CONTAINER(window), 0);
  g_signal_connect(GTK_OBJECT(window),"delete_event",
									 G_CALLBACK(gtk_main_quit), NULL);
  gtk_widget_set_usize(window,200,200);
  info.main_window = window;

  main_vbox = gtk_vbox_new(FALSE,1);
  gtk_container_add(GTK_CONTAINER(window),main_vbox);

  gtk_widget_show(main_vbox);
  gtk_widget_realize(window);

  CreateNotebook (window,main_vbox,&info);

  gtk_widget_show(window);

  gtk_main();
  
  return (0);
}
