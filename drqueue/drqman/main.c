/* 
 * $Header: /root/cvs/drqueue/drqman/main.c,v 1.8 2001/11/23 11:51:41 jorge Exp $
 */

#include <stdlib.h>
#include <gtk/gtk.h>

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
  gtk_signal_connect(GTK_OBJECT(window),"delete_event",
		     GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
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
