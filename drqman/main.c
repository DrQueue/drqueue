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

void drqman_config_parse (char *cfg);

static struct info_drqm info;
char conf[PATH_MAX];

#define DRQMAN_CONF_FILE "/etc/drqueue/drqman.conf"

int main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *main_vbox;
  char rc_file[MAXCMDLEN];

	strncpy (conf,DRQMAN_CONF_FILE,PATH_MAX);
	drqman_config_parse(conf);
	set_default_env();  // Config files overrides environment CHANGE (?)

  if (!common_environment_check()) {
    fprintf (stderr,"Error checking the environment: %s\n",drerrno_str());
    exit (1);
  }

  gtk_init(&argc,&argv);
  snprintf(rc_file,MAXCMDLEN-1,"%s/drqman.rc",getenv("DRQUEUE_ETC"));
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

void drqman_config_parse (char *cfg)
{
	FILE *f_conf;
	char buffer[BUFFERLEN];
	char *token;
	char renv[BUFFERLEN], *penv;

	if ((f_conf = fopen (cfg,"r")) == NULL) {
		fprintf (stderr,"Could not open config file using defaults\n");
		return;
	}

	while ((fgets (buffer,BUFFERLEN-1,f_conf)) != NULL) {
		if (buffer[0] == '#') {
			continue;
		}
		token = strtok(buffer,"=\n");
		if (strcmp(token,"etc") == 0) {
			if ((token = strtok (NULL,"=\n")) != NULL) {
				fprintf (stderr,"Etc on: '%s'\n",token);
				snprintf (renv,BUFFERLEN,"DRQUEUE_ETC=%s",token);
				if ((penv = (char*) malloc (strlen (renv)+1)) == NULL) {
					fprintf (stderr,"ERROR allocating memory for DRQUEUE_ETC.\n");
					exit (1);
				}
				strncpy(penv,renv,strlen(renv)+1);
				if (putenv (penv) != 0) {
					fprintf (stderr,"ERROR seting the environment: '%s'\n",penv);
				}
			} else {
				fprintf (stderr,"Warning parsing config file. No value for etc. Using default.\n");
			}
		} else {
			fprintf (stderr,"ERROR parsing config file. Unknown token: '%s'\n",token);
			exit (1);
		}
	}
}
