/*
 * $Header: /root/cvs/drqueue/drqman/notebook.c,v 1.1 2001/07/13 15:23:33 jorge Exp $
 */

#include <gtk/gtk.h>
#include "notebook.h"
#include "drqm_jobs.h"

void CreateNotebook (GtkWidget *window,GtkWidget *vbox)
{
  GtkWidget *notebook;

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),GTK_POS_TOP);
  
  CreateJobsPage(notebook);
  
  gtk_box_pack_end(GTK_BOX(vbox), notebook, TRUE,TRUE, 0);

  gtk_widget_show(notebook);
}
