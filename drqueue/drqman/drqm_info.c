/*
 * $Id: drqm_info.c,v 1.2 2001/11/08 11:47:41 jorge Exp $
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

#include "eeelogo.xpm"

#include "drqman.h"
#include "drqm_common.h"

/* Static functions declarations */
static GtkWidget *CreateLogoWidget (GtkWidget *widget);


void CreateInfoPage (GtkWidget *notebook, struct info_drqm *info)
{
  /* This function receives the notebook widget to wich the new tab will append */
  GtkWidget *label;
  GtkWidget *container;
  GtkWidget *vbox;
  GtkWidget *logo;

  /* Label */
  label = gtk_label_new ("Info");
  container = gtk_frame_new ("About DrQueue");
  gtk_container_border_width (GTK_CONTAINER(container),2);
  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(container),vbox);

  logo = CreateLogoWidget (info->main_window);

  gtk_box_pack_start (GTK_BOX(vbox),logo,FALSE,FALSE,2);

  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, label);

  gtk_widget_show_all(vbox);
  gtk_widget_show(label);
  gtk_widget_show(container);
}

GtkWidget *CreateLogoWidget (GtkWidget *widget)
{
  GtkWidget *toplevel;
  GdkBitmap *l_mask = NULL; /* Logo mask */
  GdkPixmap *l_data = NULL; /* Logo data */

  toplevel = gtk_widget_get_toplevel(widget);
  gtk_widget_realize(toplevel);	/* Just in case */
  l_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&l_mask,NULL,(gchar**)eeelogo_xpm);

  return gtk_pixmap_new(l_data,l_mask);
}

