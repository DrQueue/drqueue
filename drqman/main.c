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
// $Id$
//

#include <sys/unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#include "drqman.h"
#include "notebook.h"
#include "libdrqueue.h"

#include "weasel.xpm"

#include <gtk/gtk.h>

GdkPixbuf *drqman_icon;
GList *icon_list;

static struct info_drqm info;
char conf[PATH_MAX];


void drqman_get_options (int *argc,char ***argv);

int main (int argc, char *argv[]) {
  GtkWidget *window;
  GtkWidget *main_vbox;
  char rc_file[MAXCMDLEN];

  gtk_init(&argc,&argv);
  
  // fprintf (stderr,"drqman pid: %i\n",getpid());
  drqman_get_options(&argc,&argv);
  set_default_env(); // Config files overrides environment
  config_parse_tool("drqman");

  if (!common_environment_check()) {
    fprintf (stderr,"Error checking the environment: %s\n",drerrno_str());
    exit (1);
  }
  
  snprintf(rc_file,MAXCMDLEN-1,"%s%cdrqman.rc",getenv("DRQUEUE_ETC"),
                   DIR_SEPARATOR_CHAR);

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

  return (0);
}

void drqman_get_options (int *argc,char ***argv) {
  int opt;

  while ((opt = getopt (*argc,*argv,"l:o")) != -1) {
    switch (opt) {
    case 'l':
      log_level_severity_set (atoi(optarg));
      printf ("Logging level set to: %i (%s)\n",loglevel,log_level_str(loglevel));
      break;
    case 'o':
      log_level_out_set (L_ONSCREEN);
      printf ("Logging on screen.\n");
      break;
    case 'v':
      show_version (*argv);
      exit (0);
      break;
    case '?':
    case 'h':
      //usage();
      exit (0);
    }
  }
}
