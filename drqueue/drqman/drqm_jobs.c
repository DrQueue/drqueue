/*
 * $Id: drqm_jobs.c,v 1.4 2001/07/17 15:10:11 jorge Exp $
 */

#include <string.h>

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
  gchar *titles[] = { "ID","Name","Owner","Status","Processors" };
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (5, titles);
  gtk_container_add(GTK_CONTAINER(window),clist);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_column_titles_passive(GTK_CLIST(clist));
  gtk_clist_set_column_width (GTK_CLIST(clist),0,25);
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
  drqm_update_joblist (info);
}

void drqm_update_joblist (struct info_drqm_jobs *info)
{
  int i;
  char **buff;
  
  buff = (char**) g_malloc(6 * sizeof(char*));
  buff[0] = (char*) g_malloc (BUFFERLEN);
  buff[1] = (char*) g_malloc (BUFFERLEN);
  buff[2] = (char*) g_malloc (BUFFERLEN);
  buff[3] = (char*) g_malloc (BUFFERLEN);
  buff[4] = (char*) g_malloc (BUFFERLEN);
  buff[5] = NULL;
  
  gtk_clist_freeze(GTK_CLIST(info->clist));
  gtk_clist_clear(GTK_CLIST(info->clist));
  for (i=0; i < info->njobs; i++) {
    snprintf (buff[0],BUFFERLEN,"%i",info->jobs[i].id);
    strncpy(buff[1],info->jobs[i].name,BUFFERLEN);
    strncpy(buff[2],info->jobs[i].owner,BUFFERLEN); 
    snprintf (buff[3],BUFFERLEN,"%s",job_status_string(info->jobs[i].status));
    snprintf (buff[4],BUFFERLEN,"%i",info->jobs[i].nprocs);
    gtk_clist_append(GTK_CLIST(info->clist),buff);
  }
  gtk_clist_thaw(GTK_CLIST(info->clist));
}
