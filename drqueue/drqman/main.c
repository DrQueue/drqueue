/* 
 * $Header: /root/cvs/drqueue/drqman/main.c,v 1.3 2001/08/28 21:49:27 jorge Exp $
 */

#include <gtk/gtk.h>
#include "drqman.h"
#include "notebook.h"

int main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *main_vbox;
  struct info_drqm info;
  
  gtk_init(&argc,&argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window),"DrQueue Manager");
  gtk_window_set_default_size(GTK_WINDOW(window),700,400);
  gtk_container_border_width(GTK_CONTAINER(window), 0);
  gtk_signal_connect(GTK_OBJECT(window),"delete_event",
		     GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
  gtk_widget_set_usize(window,200,200);

  main_vbox = gtk_vbox_new(FALSE,1);
  gtk_container_add(GTK_CONTAINER(window),main_vbox);

  gtk_widget_show(main_vbox);
  gtk_widget_realize(window);

  CreateNotebook (window,main_vbox,&info);

  gtk_widget_show(window);

  gtk_main();
  
  return (0);
}
