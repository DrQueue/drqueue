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

#include "notebook.h"
#include "drqm_jobs.h"
#include "drqm_computers.h"
#include "drqm_info.h"

void
CreateNotebook (GtkWidget *window, GtkWidget *vbox, struct info_drqm *info) {
  GtkWidget *notebook;
  
  // fix compiler warning
  (void)window;

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),GTK_POS_TOP);

  CreateJobsPage(notebook,info);
  CreateComputersPage(notebook,info);
  CreateInfoPage(notebook,info);

  gtk_box_pack_end(GTK_BOX(vbox), notebook, TRUE,TRUE, 0);

  gtk_widget_show(notebook);
}
