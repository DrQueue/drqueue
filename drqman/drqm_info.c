/*
 * $Id: drqm_info.c,v 1.1 2001/11/08 11:10:20 jorge Exp $
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

#include "waiting.xpm"
#include "running.xpm"
#include "finished.xpm"
#include "error.xpm"

#include "drqman.h"
#include "drqm_common.h"

void CreateInfoPage (GtkWidget *notebook, struct info_drqm *info)
{
  /* This function receives the notebook widget to wich the new tab will append */
  GtkWidget *label;
  GtkWidget *container;
  GtkWidget *vbox;

  /* Label */
  label = gtk_label_new ("Info");
  container = gtk_frame_new ("About DrQueue");
  gtk_container_border_width (GTK_CONTAINER(container),2);
  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(container),vbox);

  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, label);

  gtk_widget_show(vbox);
  gtk_widget_show(label);
  gtk_widget_show(container);
}



