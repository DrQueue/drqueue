/*
 * $Id: drqm_jobs.c,v 1.3 2001/07/17 10:21:51 jorge Exp $
 */

#include "drqm_request.h"
#include "drqm_jobs.h"

static struct info_drqm_jobs info;

void CreateJobsPage (GtkWidget *notebook)
{
  GtkWidget *label;
  GtkWidget *container;
  GtkWidget *clist;
  GtkWidget *buttonRefresh;	/* Button to refresh the jobs list */
  GtkWidget *vbox;

  /* Label */
  label = gtk_label_new ("Jobs");
  container = gtk_frame_new ("Jobs status");
  gtk_container_border_width (GTK_CONTAINER(container),2);
  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(container),vbox);

  /* Clist */
  clist = CreateJobsList (&info);
  gtk_box_pack_start(GTK_BOX(vbox),clist,TRUE,TRUE,2);
  
  /* Button refresh */
  buttonRefresh = CreateButtonRefresh (&info);
  gtk_box_pack_end(GTK_BOX(vbox),buttonRefresh,FALSE,FALSE,2);

  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, label);

  gtk_widget_show(clist);
  gtk_widget_show(vbox);
  gtk_widget_show(label);
  gtk_widget_show(container);
}

GtkWidget *CreateJobsList(struct info_drqm_jobs *info)
{
  GtkWidget *window;
  GtkWidget *clist;

  /* Scrolled window */
  window = gtk_scrolled_window_new(NULL,NULL);
  clist = CreateClist(window);
  info->clist = clist;

  return (window);
}

GtkWidget *CreateClist (GtkWidget *window)
{
  gchar *titles[] = { "Name","Owner","Status" };
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (3, titles);
  gtk_container_add(GTK_CONTAINER(window),clist);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_column_titles_passive(GTK_CLIST(clist));
  gtk_clist_set_column_width (GTK_CLIST(clist),0,325);
  gtk_widget_show(clist);

  return (clist);
}


GtkWidget *CreateButtonRefresh (struct info_drqm_jobs *info)
{
  GtkWidget *b;
  
  b = gtk_button_new_with_label ("Refresh");
  gtk_container_border_width (GTK_CONTAINER(b),5);
  gtk_widget_show (GTK_WIDGET(b));
  gtk_signal_connect(GTK_OBJECT(b),"clicked",GTK_SIGNAL_FUNC(PressedButtonRefresh),info);

  return b;
}

void PressedButtonRefresh (GtkWidget *b, struct info_drqm_jobs *info)
{
  drqm_request_joblist (info);
}

