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
// $Id$
//

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "drqman.h"
#include "notebook.h"
#include "libdrqueue.h"

#include "weasel.xpm"

GdkPixbuf *drqman_icon;
GList *icon_list;

static struct info_drqm info;
char conf[PATH_MAX];

#define DRQMAN_CONF_FILE "/etc/drqueue/drqman.conf"

#ifdef __CYGWIN
FILE *file_null;
#endif

int main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *main_vbox;
  char rc_file[MAXCMDLEN];

	//	fprintf (stderr,"drqman pid: %i\n",getpid());

	strncpy (conf,DRQMAN_CONF_FILE,PATH_MAX);
	config_parse(conf);
	set_default_env();  // Config files overrides environment CHANGE (?)

  if (!common_environment_check()) {
    fprintf (stderr,"Error checking the environment: %s\n",drerrno_str());
    exit (1);
  }

  gtk_init(&argc,&argv);
#ifdef __CYGWIN
  snprintf(rc_file,MAXCMDLEN-1,"%s/drqman-windows.rc",getenv("DRQUEUE_ETC"));
  file_null = fopen("/dev/null", "r+");
#else
  snprintf(rc_file,MAXCMDLEN-1,"%s/drqman.rc",getenv("DRQUEUE_ETC"));
#endif
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

	// The icon
	drqman_icon = gdk_pixbuf_new_from_xpm_data ((const char **)weasel_xpm);
	icon_list = g_list_append (NULL,drqman_icon);
	gtk_window_set_default_icon_list (icon_list);

  main_vbox = gtk_vbox_new(FALSE,1);
  gtk_container_add(GTK_CONTAINER(window),main_vbox);

  gtk_widget_show(main_vbox);
  gtk_widget_realize(window);

  CreateNotebook (window,main_vbox,&info);


  gtk_widget_show(window);

  gtk_main();
 
#ifdef __CYGWIN
  fclose(file_null);
#endif

  return (0);
}


