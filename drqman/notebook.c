/*
 * $Header: /root/cvs/drqueue/drqman/notebook.c,v 1.4 2001/08/29 15:28:03 jorge Exp $
 */

#include <gtk/gtk.h>
#include "notebook.h"
#include "drqm_jobs.h"
#include "drqm_computers.h"

void CreateNotebook (GtkWidget *window,GtkWidget *vbox,struct info_drqm *info)
{
  GtkWidget *notebook;

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),GTK_POS_TOP);
  
  CreateJobsPage(notebook,info);
  CreateComputersPage(notebook,info);
  
  gtk_box_pack_end(GTK_BOX(vbox), notebook, TRUE,TRUE, 0);

  gtk_widget_show(notebook);
}
