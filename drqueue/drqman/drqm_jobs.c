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
//
// $Id$
//

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <gtk/gtk.h>


#include "libdrqueue.h"
#include "drqman.h"
#include "drqm_request.h"
#include "drqm_jobs.h"
#include "drqm_common.h"

// Jdd includes
#include "drqm_jobs_jdd.h"

/* Koj includes */
#include "drqm_jobs_maya.h"
#include "drqm_jobs_blender.h"
#include "drqm_jobs_bmrt.h"
#include "drqm_jobs_pixie.h"

// Icon includes
#include "job_icon.h"

/* Static functions declaration */
static GtkWidget *CreateJobsList(struct drqm_jobs_info *info);
static GtkWidget *CreateClist (GtkWidget *window);
static GtkWidget *CreateButtonRefresh (struct drqm_jobs_info *info);
static void PressedButtonRefresh (GtkWidget *b, struct drqm_jobs_info *info);
static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct drqm_jobs_info *info);
static GtkWidget *CreateMenu (struct drqm_jobs_info *info);
static int pri_cmp_clist (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
static void update_joblist (GtkWidget *widget, struct drqm_jobs_info *info);

/* NEW JOB */
static void NewJob (GtkWidget *menu_item, struct drqm_jobs_info *info);
static void CopyJob (GtkWidget *menu_item, struct drqm_jobs_info *info);
static void CopyJob_CloneInfo (struct drqm_jobs_info *info);
static GtkWidget *NewJobDialog (struct drqm_jobs_info *info);
static void dnj_psearch (GtkWidget *button, struct drqmj_dnji *info);
static void dnj_set_cmd (GtkWidget *button, struct drqmj_dnji *info);
static void dnj_cpri_changed (GtkWidget *entry, struct drqmj_dnji *info);
static void dnj_bsubmit_pressed (GtkWidget *button, struct drqmj_dnji *info);
static void dnj_bsubmitstopped_pressed (GtkWidget *button, struct drqmj_dnji *info);
static int dnj_submit (struct drqmj_dnji *info);
static void dnj_destroyed (GtkWidget *dialog, struct drqm_jobs_info *info);

/* KOJ STUFF */
/* Basic koj handling */
static GtkWidget *dnj_koj_widgets (struct drqm_jobs_info *info);
static void dnj_koj_combo_changed (GtkWidget *combo, struct drqm_jobs_info *info);

/* Limits */
static GtkWidget *dnj_limits_widgets (struct drqm_jobs_info *info);
/* Flags */
static GtkWidget *dnj_flags_widgets (struct drqm_jobs_info *info);
static void dnj_flags_cbmailnotify_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_flags_cbdifemail_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_flags_cbjobdepend_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_flags_bjobdepend_clicked (GtkWidget *bclicked, struct drqm_jobs_info *info);
static void dnj_flags_jdepend_refresh_job_list (GtkWidget *bclicked, struct drqm_jobs_info *info);
static void dnj_flags_jdepend_accept (GtkWidget *bclicked, struct drqm_jobs_info *info);

/* JOB ACTIONS */
static GtkWidget *DeleteJobDialog (struct drqm_jobs_info *info);
static void djd_bok_pressed (GtkWidget *button, struct drqm_jobs_info *info);
static void job_hstop_cb (GtkWidget *button, struct drqm_jobs_info *info);

void CreateJobsPage (GtkWidget *notebook, struct info_drqm *info)
{
  /* This function receives the notebook widget to wich the new tab will append */
  GtkWidget *label;
  GtkWidget *container;
  GtkWidget *clist;
  GtkWidget *buttonRefresh;	/* Button to refresh the jobs list */
  GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *icon;
	GdkPixbuf *job_icon_pb;

  container = gtk_frame_new ("Jobs status");
  gtk_container_border_width (GTK_CONTAINER(container),2);
  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(container),vbox);

  /* Clist */
  clist = CreateJobsList (&info->idj);
  gtk_box_pack_start(GTK_BOX(vbox),clist,TRUE,TRUE,2);
  
  /* Button refresh */
  buttonRefresh = CreateButtonRefresh (&info->idj);
  gtk_box_pack_end(GTK_BOX(vbox),buttonRefresh,FALSE,FALSE,2);


	// Label
  label = gtk_label_new ("Jobs");
	gtk_widget_show(label);
	// Image
	job_icon_pb = gdk_pixbuf_new_from_inline (735,job_icon,0,NULL);
	icon = gtk_image_new_from_pixbuf (job_icon_pb);
	gtk_widget_show(icon);
	hbox = gtk_hbox_new (FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),icon,TRUE,TRUE,2);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,2);
  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, hbox);

  /* Put the jobs on the list */
	update_joblist(GTK_WIDGET(notebook),&info->idj);

  gtk_widget_show_all(container);
}

static GtkWidget *CreateJobsList(struct drqm_jobs_info *info)
{
  GtkWidget *window;

  /* Scrolled window */
  window = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  info->clist = CreateClist(window);

  /* Create the popup menu */
  info->menu = CreateMenu(info);

  return (window);
}

static GtkWidget *CreateClist (GtkWidget *window)
{
  gchar *titles[] = { "ID","Name","Owner","Status","Processors","Left","Done","Failed","Total","Pri" };
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (10, titles);
  gtk_container_add(GTK_CONTAINER(window),clist);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_column_titles_passive(GTK_CLIST(clist));
  gtk_clist_set_column_width (GTK_CLIST(clist),0,25);
  gtk_clist_set_column_width (GTK_CLIST(clist),1,75);
  gtk_clist_set_column_width (GTK_CLIST(clist),2,75);
  gtk_clist_set_column_width (GTK_CLIST(clist),3,75);
  gtk_clist_set_column_width (GTK_CLIST(clist),4,75);
  gtk_clist_set_column_width (GTK_CLIST(clist),5,45);
  gtk_clist_set_column_width (GTK_CLIST(clist),6,45);
  gtk_clist_set_column_width (GTK_CLIST(clist),7,45);
  gtk_clist_set_column_width (GTK_CLIST(clist),8,45);

  gtk_clist_set_sort_column (GTK_CLIST(clist),9);
  gtk_clist_set_sort_type (GTK_CLIST(clist),GTK_SORT_ASCENDING);
  gtk_clist_set_compare_func (GTK_CLIST(clist),pri_cmp_clist);

  gtk_widget_show(clist);

  return (clist);
}


static GtkWidget *CreateButtonRefresh (struct drqm_jobs_info *info)
{
  GtkWidget *b;
  
  b = gtk_button_new_with_label ("Refresh");
  gtk_container_border_width (GTK_CONTAINER(b),5);
  gtk_widget_show (GTK_WIDGET(b));
  g_signal_connect(G_OBJECT(b),"clicked",G_CALLBACK(PressedButtonRefresh),info);

  return b;
}

static void PressedButtonRefresh (GtkWidget *b, struct drqm_jobs_info *info)
{
	update_joblist (b,info);
}

void drqm_update_joblist (struct drqm_jobs_info *info)
{
  int i;
  char **buff;
  int ncols = 10;

  buff = (char**) g_malloc((ncols + 1) * sizeof(char*));
  for (i=0;i<ncols;i++)
    buff[i] = (char*) g_malloc (BUFFERLEN);
  buff[ncols] = NULL;
  
  gtk_clist_freeze(GTK_CLIST(info->clist));
  gtk_clist_clear(GTK_CLIST(info->clist));
  for (i=0; i < info->njobs; i++) {
    snprintf (buff[0],BUFFERLEN,"%i",info->jobs[i].id);
    strncpy(buff[1],info->jobs[i].name,BUFFERLEN);
    strncpy(buff[2],info->jobs[i].owner,BUFFERLEN); 
    snprintf (buff[3],BUFFERLEN,"%s",job_status_string(info->jobs[i].status));
    snprintf (buff[4],BUFFERLEN,"%i",info->jobs[i].nprocs);
    snprintf (buff[5],BUFFERLEN,"%i",info->jobs[i].fleft);
    snprintf (buff[6],BUFFERLEN,"%i",info->jobs[i].fdone);
    snprintf (buff[7],BUFFERLEN,"%i",info->jobs[i].ffailed);
    snprintf (buff[8],BUFFERLEN,"%i",job_nframes(&info->jobs[i]));
    snprintf (buff[9],BUFFERLEN,"%i",info->jobs[i].priority);

    gtk_clist_append(GTK_CLIST(info->clist),buff);

    gtk_clist_set_row_data(GTK_CLIST(info->clist),i,(gpointer)info->jobs[i].priority);
  }

  gtk_clist_sort (GTK_CLIST(info->clist));
  gtk_clist_thaw(GTK_CLIST(info->clist));

  for(i=0;i<ncols;i++)
    g_free (buff[i]);
}

static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct drqm_jobs_info *info)
{
  int i;
  char *buf;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button != 3)
      return FALSE;
    info->selected = gtk_clist_get_selection_info(GTK_CLIST(info->clist),
																									(int)bevent->x,(int)bevent->y,
																									&info->row,&info->column);
    
    if (info->selected) {
      gtk_clist_get_text(GTK_CLIST(info->clist),info->row,0,&buf);
      info->ijob = atoi (buf);
			
      info->row = -1;
      for (i=0;i<info->njobs;i++) {
				if (info->jobs[i].id == info->ijob) {
					info->row = i; // Points from this point on, to index of the job in info.
				}
      }

      if (info->row == -1)
				return FALSE;
    }
		
    gtk_menu_popup (GTK_MENU(info->menu), NULL, NULL, NULL, NULL,
		    bevent->button, bevent->time);
    return TRUE;
  }
  return FALSE;
}

static GtkWidget *CreateMenu (struct drqm_jobs_info *info)
{
	// This function creates the popup menu on the job's page
  GtkWidget *menu;
  GtkWidget *menu_item;
  GtkTooltips *tooltips;

  tooltips = TooltipsNew ();

  menu = gtk_menu_new ();
  menu_item = gtk_menu_item_new_with_label("Details");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(JobDetails),info);
  gtk_tooltips_set_tip(tooltips,menu_item,"Open detailed information window for the selected job",NULL);

  /* Line */
  menu_item = gtk_menu_item_new();
  gtk_menu_append(GTK_MENU(menu),menu_item);

  menu_item = gtk_menu_item_new_with_label("New Job");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(NewJob),info);
  gtk_tooltips_set_tip(tooltips,menu_item,"Send a new job to the queue",NULL);

  menu_item = gtk_menu_item_new_with_label("Copy Job");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(CopyJob),info);
  gtk_tooltips_set_tip(tooltips,menu_item,"Copy current job info to newly created job",NULL);

  /* Line */
  menu_item = gtk_menu_item_new();
  gtk_menu_append(GTK_MENU(menu),menu_item);

  menu_item = gtk_menu_item_new_with_label("Stop");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(StopJob),info);
  gtk_tooltips_set_tip (tooltips,menu_item,"Set the job as 'Stopped' but let the running frames finish",NULL);

  menu_item = gtk_menu_item_new_with_label("Hard Stop");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(HStopJob),info);
  gtk_tooltips_set_tip (tooltips,menu_item,"Set the job as 'Stopped' killing all running frames",NULL);
  gtk_widget_set_name (menu_item,"warning");

  menu_item = gtk_menu_item_new_with_label("Continue");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(ContinueJob),info);
  gtk_tooltips_set_tip (tooltips,menu_item,"Set a 'Stopped' job as 'Waiting' again",NULL);

  menu_item = gtk_menu_item_new_with_label("Delete");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(DeleteJob),info);
  gtk_tooltips_set_tip (tooltips,menu_item,"Delete the job from the queue killing running frames",NULL);
  gtk_widget_set_name (menu_item,"danger");

  g_signal_connect(G_OBJECT((info->clist)),"event",G_CALLBACK(PopupMenu),info);

  gtk_widget_show_all(menu);

  return (menu);
}


static void NewJob (GtkWidget *menu_item, struct drqm_jobs_info *info)
{
  GtkWidget *dialog;
  dialog = NewJobDialog(info);
	g_signal_connect (G_OBJECT(dialog),"destroy",
										G_CALLBACK(dnj_destroyed),info);
	gtk_grab_add(dialog);
}

static void CopyJob (GtkWidget *menu_item, struct drqm_jobs_info *info)
{
  GtkWidget *dialog;

  if (!info->selected)
    return;

  dialog = NewJobDialog(info);
	CopyJob_CloneInfo (info);
	g_signal_connect (G_OBJECT(dialog),"destroy",
										G_CALLBACK(dnj_destroyed),info);
	gtk_grab_add(dialog);
}

static void CopyJob_CloneInfo (struct drqm_jobs_info *info)
{
  char buf[BUFFERLEN];

  /* General */
  gtk_entry_set_text(GTK_ENTRY(info->dnj.ename),info->jobs[info->row].name);
  snprintf(buf,BUFFERLEN-1,"%i",info->jobs[info->row].frame_start);
  gtk_entry_set_text(GTK_ENTRY(info->dnj.esf),buf);
  snprintf(buf,BUFFERLEN-1,"%i",info->jobs[info->row].frame_end);
  gtk_entry_set_text(GTK_ENTRY(info->dnj.eef),buf);
  snprintf(buf,BUFFERLEN-1,"%i",info->jobs[info->row].frame_step);
  gtk_entry_set_text(GTK_ENTRY(info->dnj.estf),buf);

	/* Priority */
  if (info->jobs[info->row].priority == 100) {
		gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(info->dnj.cpri)->entry),"Highest");
  } else if (info->jobs[info->row].priority == 250) {
		gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(info->dnj.cpri)->entry),"High");
  } else if (info->jobs[info->row].priority == 500) {
		gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(info->dnj.cpri)->entry),"Normal");
  } else if (info->jobs[info->row].priority == 750) {
		gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(info->dnj.cpri)->entry),"Low");
  } else if (info->jobs[info->row].priority == 1000) {
		gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(info->dnj.cpri)->entry),"Lowest");
  } else {
		gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(info->dnj.cpri)->entry),"Custom");
		snprintf(buf,BUFFERLEN-1,"%u",info->jobs[info->row].priority);
		gtk_entry_set_text (GTK_ENTRY(info->dnj.epri),buf);
  }
	

  /* Limits */
  snprintf(buf,BUFFERLEN-1,"%i",(short int)info->jobs[info->row].limits.nmaxcpus);
  gtk_entry_set_text(GTK_ENTRY(info->dnj.limits.enmaxcpus),buf);
  snprintf(buf,BUFFERLEN-1,"%i",(short int)info->jobs[info->row].limits.nmaxcpuscomputer);
  gtk_entry_set_text(GTK_ENTRY(info->dnj.limits.enmaxcpuscomputer),buf);

  /* Flags */
	if (info->jobs[info->row].flags & JF_MAILNOTIFY) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.flags.cbmailnotify),TRUE);
		if (info->jobs[info->row].flags & JF_MNDIFEMAIL) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.flags.cbdifemail),TRUE);
			gtk_entry_set_text (GTK_ENTRY(info->dnj.flags.edifemail),
													info->jobs[info->row].email);
		}
	}
	if (info->jobs[info->row].flags & JF_JOBDEPEND) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.flags.cbjobdepend),TRUE);
		snprintf(buf,BUFFERLEN,"%i",info->jobs[info->row].dependid);
		gtk_entry_set_text (GTK_ENTRY(info->dnj.flags.ejobdepend),buf);
	}


  /* KOJ */
  switch (info->jobs[info->row].koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(info->dnj.ckoj)->entry),
											 "Maya");
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_maya.escene),
											 info->jobs[info->row].koji.maya.scene);
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_maya.erenderdir),
											 info->jobs[info->row].koji.maya.renderdir);
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_maya.eimage),
											 info->jobs[info->row].koji.maya.image);
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_maya.eviewcmd),
											 info->jobs[info->row].koji.maya.viewcmd);
    break;
  case KOJ_BLENDER:
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(info->dnj.ckoj)->entry),
											 "Blender");
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_blender.escene),
											 info->jobs[info->row].koji.blender.scene);
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_blender.eviewcmd),
											 info->jobs[info->row].koji.blender.viewcmd);
    break;
	case KOJ_BMRT:
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(info->dnj.ckoj)->entry),
											 "Bmrt");
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_bmrt.escene),
											 info->jobs[info->row].koji.bmrt.scene);
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_bmrt.eviewcmd),
											 info->jobs[info->row].koji.bmrt.viewcmd);
		/* Custom crop */
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbcrop),
																 info->jobs[info->row].koji.bmrt.custom_crop);
		snprintf(buf,BUFFERLEN-1,"%u",info->jobs[info->row].koji.bmrt.xmin);
		gtk_entry_set_text (GTK_ENTRY(info->dnj.koji_bmrt.ecropxmin),buf);
		snprintf(buf,BUFFERLEN-1,"%u",info->jobs[info->row].koji.bmrt.xmax);
		gtk_entry_set_text (GTK_ENTRY(info->dnj.koji_bmrt.ecropxmax),buf);
		snprintf(buf,BUFFERLEN-1,"%u",info->jobs[info->row].koji.bmrt.ymin);
		gtk_entry_set_text (GTK_ENTRY(info->dnj.koji_bmrt.ecropymin),buf);
		snprintf(buf,BUFFERLEN-1,"%u",info->jobs[info->row].koji.bmrt.ymax);
		gtk_entry_set_text (GTK_ENTRY(info->dnj.koji_bmrt.ecropymax),buf);
		/* Custom samples */
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbsamples),
																 info->jobs[info->row].koji.bmrt.custom_samples);
		snprintf(buf,BUFFERLEN-1,"%u",info->jobs[info->row].koji.bmrt.xsamples);
		gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_bmrt.exsamples),buf);
		snprintf(buf,BUFFERLEN-1,"%u",info->jobs[info->row].koji.bmrt.ysamples);
		gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_bmrt.eysamples),buf);
		/* Stats, verbose, beep */
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbstats),
																 info->jobs[info->row].koji.bmrt.disp_stats);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbverbose),
																 info->jobs[info->row].koji.bmrt.verbose);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbbeep),
																 info->jobs[info->row].koji.bmrt.custom_beep);
		/* Radiosity samples */
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbradiositysamples),
																 info->jobs[info->row].koji.bmrt.custom_radiosity);
		snprintf(buf,BUFFERLEN-1,"%u",info->jobs[info->row].koji.bmrt.radiosity_samples);
		gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_bmrt.eradiositysamples),buf);
		/* Custom raysamples */
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbraysamples),
																 info->jobs[info->row].koji.bmrt.custom_raysamples);
		snprintf(buf,BUFFERLEN-1,"%u",info->jobs[info->row].koji.bmrt.raysamples);
		gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_bmrt.eraysamples),buf);
  case KOJ_PIXIE:
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(info->dnj.ckoj)->entry),
											 "Pixie");
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_pixie.escene),
											 info->jobs[info->row].koji.pixie.scene);
    gtk_entry_set_text(GTK_ENTRY(info->dnj.koji_pixie.eviewcmd),
											 info->jobs[info->row].koji.pixie.viewcmd);
    break;
  }
}

static GtkWidget *NewJobDialog (struct drqm_jobs_info *info)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox,*hbox2;
  GtkWidget *label;
  GtkWidget *entry; 
  GtkWidget *button;
  GtkWidget *combo;
  GtkWidget *bbox;
	GtkWidget *notebook;
  GList *items = NULL;
  GtkTooltips *tooltips;

  tooltips = TooltipsNew ();

  /* Dialog */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"New Job");
  g_signal_connect_swapped(G_OBJECT(window),"destroy",G_CALLBACK(gtk_widget_destroy),
													 (GtkObject*)window);
  gtk_window_set_default_size(GTK_WINDOW(window),600,200);
  gtk_container_set_border_width (GTK_CONTAINER(window),5);
  info->dnj.dialog = window;

	// Notebook
	label = gtk_label_new ("Main info");
	notebook = gtk_notebook_new ();
	gtk_container_add (GTK_CONTAINER(window),GTK_WIDGET(notebook));


  /* Frame */
  frame = gtk_frame_new ("Give job information");
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook),GTK_WIDGET(frame),GTK_WIDGET(label));


  /* Main vbox */
  vbox = gtk_vbox_new (FALSE,2);
  info->dnj.vbox = vbox;
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  /* Label */
  label = gtk_label_new ("Information to be show here");
  gtk_label_set_pattern (GTK_LABEL(label),"________________________________");
  gtk_box_pack_start (GTK_BOX(vbox),label,FALSE,FALSE,4);
  gtk_widget_show (label);

  /* Name of the job */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  gtk_widget_show (hbox);
  label = gtk_label_new ("Name:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  gtk_widget_show(label);
  entry = gtk_entry_new_with_max_length (MAXNAMELEN-1);
  gtk_tooltips_set_tip(tooltips,entry,"Name of the job. Should be a descriptive name",NULL);
  info->dnj.ename = entry;
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  gtk_widget_show(entry);

  /* Cmd of the job */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  gtk_widget_show (hbox);
  label = gtk_label_new ("Command:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  gtk_widget_show(label);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  gtk_widget_show (hbox2);
  entry = gtk_entry_new_with_max_length (MAXCMDLEN-1);
  gtk_tooltips_set_tip(tooltips,entry,"Job script or command that will be executed",NULL);
  info->dnj.ecmd = entry;
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  gtk_widget_show(entry);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for job command",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  gtk_widget_show (button);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(dnj_psearch),&info->dnj);

  /* Start and End frames */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Start frame:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(entry),"1");
  info->dnj.esf = entry;
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("End frame:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(entry),"100");
  info->dnj.eef = entry;
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Step frames:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(entry),"1");
  info->dnj.estf = entry;
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);

	/* Block Size */
	hbox = gtk_hbox_new (TRUE,2);
	gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	label = gtk_label_new ("Block size:");
	gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
	entry = gtk_entry_new ();
  gtk_tooltips_set_tip(tooltips,entry,"Size of the block that will be rendered",NULL);
	gtk_entry_set_text (GTK_ENTRY(entry),"1");
	info->dnj.ebs = entry;
	gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Priority */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  gtk_widget_show (hbox);
  label = gtk_label_new ("Priority:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  gtk_widget_show(label);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  gtk_widget_show (hbox2);
  items = g_list_append (items,"Highest");
  items = g_list_append (items,"High");
  items = g_list_append (items,"Normal");
  items = g_list_append (items,"Low");
  items = g_list_append (items,"Lowest");
  items = g_list_append (items,"Custom");
  combo = gtk_combo_new();
  gtk_combo_set_popdown_strings (GTK_COMBO(combo),items);
  gtk_widget_show (combo);
  gtk_box_pack_start (GTK_BOX(hbox2),combo,TRUE,TRUE,0);
  gtk_entry_set_editable (GTK_ENTRY(GTK_COMBO(combo)->entry),FALSE);
  info->dnj.cpri = combo;
  entry = gtk_entry_new_with_max_length (MAXCMDLEN-1);
  info->dnj.epri = entry;
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  gtk_widget_show(entry);
  g_signal_connect (G_OBJECT(GTK_ENTRY(GTK_COMBO(combo)->entry)),
										"changed",G_CALLBACK(dnj_cpri_changed),&info->dnj);
  gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(combo)->entry),"Normal");

  /* Limits STUFF */
  frame = dnj_limits_widgets (info);
  gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,5);

  /* Flags STUFF */
  frame = dnj_flags_widgets (info);
  gtk_box_pack_start(GTK_BOX(vbox),frame,TRUE,TRUE,5);

  /* KOJ STUFF */
	label = gtk_label_new ("Kind of Job");
  frame = dnj_koj_widgets (info);
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook),GTK_WIDGET(frame),GTK_WIDGET(label));

  /* Buttons */
  /* submit */
  bbox = gtk_hbutton_box_new ();
  gtk_box_pack_end (GTK_BOX(vbox),bbox,FALSE,FALSE,5);
  gtk_widget_show (bbox);
  button = gtk_button_new_with_label ("Submit");
  gtk_tooltips_set_tip(tooltips,button,"Send the information to the queue",NULL);
  gtk_box_pack_start (GTK_BOX(bbox),button,TRUE,TRUE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(dnj_bsubmit_pressed),&info->dnj);
	
	// Submit stopped
	info->dnj.submitstopped = 0;
  button = gtk_button_new_with_label ("Submit stopped");
  gtk_tooltips_set_tip(tooltips,button,"Submit job but set as stopped",NULL);
  gtk_box_pack_start (GTK_BOX(bbox),button,TRUE,TRUE,2);
	g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(dnj_bsubmitstopped_pressed), &info->dnj);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(dnj_bsubmit_pressed),&info->dnj);


  /* cancel */
  button = gtk_button_new_with_label ("Cancel");
  gtk_tooltips_set_tip(tooltips,button,"Close without sending any information",NULL);
  gtk_box_pack_start (GTK_BOX(bbox),button,TRUE,TRUE,2);
  g_signal_connect_swapped (G_OBJECT(button),"clicked",
														G_CALLBACK(gtk_widget_destroy),
														(gpointer) window);

  gtk_widget_show_all(window);

  return window;
}

static void dnj_bsubmitstopped_pressed (GtkWidget *button, struct drqmj_dnji *info)
{
	info->submitstopped = 1;
}

static void dnj_psearch (GtkWidget *button, struct drqmj_dnji *info)
{
  GtkWidget *dialog;

  dialog = gtk_file_selection_new ("Please select a file as job command");
  info->fs = dialog;

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (dnj_set_cmd), info);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->cancel_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_widget_show (dialog);
  gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

static void dnj_set_cmd (GtkWidget *button, struct drqmj_dnji *info)
{
  gtk_entry_set_text (GTK_ENTRY(info->ecmd),gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fs)));
}

static void dnj_cpri_changed (GtkWidget *entry, struct drqmj_dnji *info)
{
  if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"Highest") == 0) {
    gtk_entry_set_editable (GTK_ENTRY(info->epri),FALSE);
    gtk_entry_set_text (GTK_ENTRY(info->epri),"100");
  } else if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"High") == 0) {
    gtk_entry_set_editable (GTK_ENTRY(info->epri),FALSE);
    gtk_entry_set_text (GTK_ENTRY(info->epri),"250");
  } else if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"Normal") == 0) {
    gtk_entry_set_editable (GTK_ENTRY(info->epri),FALSE);
    gtk_entry_set_text (GTK_ENTRY(info->epri),"500");
  } else if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"Low") == 0) {
    gtk_entry_set_editable (GTK_ENTRY(info->epri),FALSE);
    gtk_entry_set_text (GTK_ENTRY(info->epri),"750");
  } else if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"Lowest") == 0) {
    gtk_entry_set_editable (GTK_ENTRY(info->epri),FALSE);
    gtk_entry_set_text (GTK_ENTRY(info->epri),"1000");
  } else {
		/* Custom */
    gtk_entry_set_editable (GTK_ENTRY(info->epri),TRUE);
    gtk_entry_set_text (GTK_ENTRY(info->epri),"500");
  }
}

static void dnj_bsubmit_pressed (GtkWidget *button, struct drqmj_dnji *info)
{
  GtkWidget *dialog, *label, *okay_button;

  if (!dnj_submit(info)) {
    dialog = gtk_dialog_new();
    gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
    label = gtk_label_new ("The information is not correct or master not available.\nCheck it and try again, please.");
    gtk_misc_set_padding (GTK_MISC(label),10,10);
    okay_button = gtk_button_new_with_label("Ok");

    gtk_signal_connect_object (GTK_OBJECT (okay_button), "clicked",
			       GTK_SIGNAL_FUNC (gtk_widget_destroy),(GtkObject*)dialog);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
		       okay_button);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                      label);

    gtk_widget_show_all (dialog);
  } else {
    gtk_widget_destroy (info->dialog);
  }
}

static int dnj_submit (struct drqmj_dnji *info)
{
  /* This is the function that actually submits the job info */
  struct job job;
  struct passwd *pw;

  strncpy(job.name,gtk_entry_get_text(GTK_ENTRY(info->ename)),MAXNAMELEN-1);
  if (strlen(job.name) == 0)
    return 0;
  strncpy(job.cmd,gtk_entry_get_text(GTK_ENTRY(info->ecmd)),MAXCMDLEN-1);
  if (strlen(job.cmd) == 0)
    return 0;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->esf)),"%u",&job.frame_start) != 1)
    return 0;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->eef)),"%u",&job.frame_end) != 1)
    return 0;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->estf)),"%u",&job.frame_step) != 1)
    return 0;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->ebs)),"%u",&job.block_size) != 1)
    return 0;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->epri)),"%u",&job.priority) != 1)
    return 0;

  if (!(pw = getpwuid(geteuid()))) {
    strncpy (job.owner,"ERROR",MAXNAMELEN-1);
  } else {
    strncpy (job.owner,pw->pw_name,MAXNAMELEN-1);
  }
  job.owner[MAXNAMELEN-1] = 0;
  job.status = JOBSTATUS_WAITING;
  job.frame_info = NULL;

  /* KOJ */
  job.koj = info->koj;
  switch (info->koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
    strncpy(job.koji.maya.scene,gtk_entry_get_text(GTK_ENTRY(info->koji_maya.escene)),BUFFERLEN-1);
    strncpy(job.koji.maya.renderdir,gtk_entry_get_text(GTK_ENTRY(info->koji_maya.erenderdir)),BUFFERLEN-1);
    strncpy(job.koji.maya.image,gtk_entry_get_text(GTK_ENTRY(info->koji_maya.eimage)),BUFFERLEN-1);
    strncpy(job.koji.maya.viewcmd,gtk_entry_get_text(GTK_ENTRY(info->koji_maya.eviewcmd)),BUFFERLEN-1);
    break;
  case KOJ_BLENDER:
    strncpy(job.koji.blender.scene,gtk_entry_get_text(GTK_ENTRY(info->koji_blender.escene)),BUFFERLEN-1);
    strncpy(job.koji.blender.viewcmd,gtk_entry_get_text(GTK_ENTRY(info->koji_blender.eviewcmd)),BUFFERLEN-1);
    break;
  case KOJ_BMRT:
    strncpy(job.koji.bmrt.scene,gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.escene)),BUFFERLEN-1);
    strncpy(job.koji.bmrt.viewcmd,gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.eviewcmd)),BUFFERLEN-1);
		/* Custom crop */
		job.koji.bmrt.custom_crop = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbcrop)->active;
		if (job.koji.bmrt.custom_crop) {
			if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.ecropxmin)),"%u",&job.koji.bmrt.xmin) != 1)
				return 0;
			if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.ecropxmax)),"%u",&job.koji.bmrt.xmax) != 1)
				return 0;
			if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.ecropymin)),"%u",&job.koji.bmrt.ymin) != 1)
				return 0;
			if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.ecropymax)),"%u",&job.koji.bmrt.ymax) != 1)
				return 0;
		}
		/* Custom samples */
		job.koji.bmrt.custom_samples = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbsamples)->active;
		if (job.koji.bmrt.custom_samples) {
			if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.exsamples)),"%u",&job.koji.bmrt.xsamples) != 1)
				return 0;
			if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.eysamples)),"%u",&job.koji.bmrt.ysamples) != 1)
				return 0;
		}
		/* Stats, verbose, beep */
		job.koji.bmrt.disp_stats = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbstats)->active;
		job.koji.bmrt.verbose = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbverbose)->active;
		job.koji.bmrt.custom_beep = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbbeep)->active;
		/* Custom radiosity */
		job.koji.bmrt.custom_radiosity = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbradiositysamples)->active;
		if (job.koji.bmrt.custom_radiosity) {
			if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.eradiositysamples)),"%u",&job.koji.bmrt.radiosity_samples) != 1)
				return 0;
		}
		/* Custom ray samples */
		job.koji.bmrt.custom_raysamples = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbraysamples)->active;
		if (job.koji.bmrt.custom_raysamples) {
			if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.eraysamples)),"%u",&job.koji.bmrt.raysamples) != 1)
				return 0;
		}
    break;
  case KOJ_PIXIE:
    strncpy(job.koji.blender.scene,gtk_entry_get_text(GTK_ENTRY(info->koji_pixie.escene)),BUFFERLEN-1);
    strncpy(job.koji.blender.viewcmd,gtk_entry_get_text(GTK_ENTRY(info->koji_pixie.eviewcmd)),BUFFERLEN-1);
    break;
  }

  /* Limits */
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->limits.enmaxcpus)),"%hu",&job.limits.nmaxcpus) != 1)
    return 0;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->limits.enmaxcpuscomputer)),"%hu",&job.limits.nmaxcpuscomputer) != 1)
    return 0;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->limits.ememory)),"%u",&job.limits.memory) != 1)
    return 0;
	// Pool
	strncpy(job.limits.pool,gtk_entry_get_text(GTK_ENTRY(info->limits.epool)),MAXNAMELEN-1);


  /* Limits OS Flags */
  job.limits.os_flags = 0;
  if (GTK_TOGGLE_BUTTON(info->limits.cb_irix)->active) {
    job.limits.os_flags |= (OSF_IRIX);
  }
  if (GTK_TOGGLE_BUTTON(info->limits.cb_linux)->active) {
    job.limits.os_flags |= (OSF_LINUX);
  }
  if (GTK_TOGGLE_BUTTON(info->limits.cb_osx)->active) {
    job.limits.os_flags |= (OSF_OSX);
  }
	if (GTK_TOGGLE_BUTTON(info->limits.cb_freebsd)->active) {
		job.limits.os_flags |= (OSF_FREEBSD);
	}

  /* Flags */
  job.flags = 0;
  if (GTK_TOGGLE_BUTTON(info->flags.cbmailnotify)->active) {
    job.flags = job.flags | (JF_MAILNOTIFY);
  }
  if (GTK_TOGGLE_BUTTON(info->flags.cbdifemail)->active) {
    job.flags = job.flags | (JF_MNDIFEMAIL);
    strncpy(job.email,gtk_entry_get_text(GTK_ENTRY(info->flags.edifemail)),MAXNAMELEN-1);
  } else {
    strncpy(job.email,job.owner,MAXNAMELEN-1);
  }
	if (GTK_TOGGLE_BUTTON(info->flags.cbjobdepend)->active) {
		job.flags |= JF_JOBDEPEND;
		job.dependid = atoi (gtk_entry_get_text(GTK_ENTRY(info->flags.ejobdepend)));
	}

  if (!register_job (&job))
    return 0;

	// job.id is set on the call to register_job
	if (info->submitstopped) {
		request_job_hstop (job.id,CLIENT);
	}

  return 1;
}

static void update_joblist (GtkWidget *widget, struct drqm_jobs_info *info)
{
	drqm_request_joblist (info);
	drqm_update_joblist (info);
}

static void dnj_destroyed (GtkWidget *dialog, struct drqm_jobs_info *info)
{
	update_joblist (dialog,info);
}

static int pri_cmp_clist (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  uint32_t a,b;

  a = (uint32_t) ((GtkCListRow*)ptr1)->data;
  b = (uint32_t) ((GtkCListRow*)ptr2)->data;

  if (a > b) {
    return 1;
  } else if (a == b) {
    return 0;
  } else {
    return -1;
  }

  return 0;
}

void DeleteJob (GtkWidget *menu_item, struct drqm_jobs_info *info)
{
  GtkWidget *dialog;

  if (!info->selected)
    return;

  dialog = DeleteJobDialog(info);

  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
}

static GtkWidget *DeleteJobDialog (struct drqm_jobs_info *info)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;

  /* Dialog */
  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(dialog),"You Sure?");

  /* Label */
  label = gtk_label_new ("Do you really want to delete the job?");
  gtk_misc_set_padding (GTK_MISC(label), 10, 10);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,TRUE,5);
 
  /* Buttons */
  button = gtk_button_new_with_label ("Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(djd_bok_pressed),info);
  g_signal_connect_swapped(G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),dialog);

  button = gtk_button_new_with_label ("No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  g_signal_connect_swapped(G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),dialog);
  GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);

  gtk_widget_show_all (dialog);

  return dialog;
}

static void djd_bok_pressed (GtkWidget *button, struct drqm_jobs_info *info)
{
	// README <- Crashes after deleting jobs from the jdd
	// This function is called at the time of this writing 13.10.2004
	// by two different callbacks: one from the popup menu on main 
	// job window, and another one from the job's detail dialog.
	// So we need to treat the differently. Is also commented on bug
	// number 0000001 of the bug tracker
	if (info->jdd.dialog) {
		drqm_request_job_delete (info->jdd.job.id);
		update_joblist(button,info->jdd.oldinfo);
		gtk_widget_destroy (info->jdd.dialog);
		info->jdd.dialog = NULL;
	} else {
		drqm_request_job_delete (info->jobs[info->row].id);
		update_joblist(button,info);
	}
}

void StopJob (GtkWidget *menu_item, struct drqm_jobs_info *info)
{
  if (!info->selected)
    return;

	if (info->jdd.dialog) {
		drqm_request_job_stop (info->jdd.job.id);
	} else {
		drqm_request_job_stop (info->jobs[info->row].id);
	}
  update_joblist(menu_item,info); /* updates the list */
}

void ContinueJob (GtkWidget *menu_item, struct drqm_jobs_info *info)
{
  if (!info->selected)
    return;

	if (info->jdd.dialog) {
		drqm_request_job_continue (info->jdd.job.id);
	} else {
		drqm_request_job_continue (info->jobs[info->row].id);
	}
  update_joblist(menu_item,info); /* Updates the list */
}

void HStopJob (GtkWidget *menu_item, struct drqm_jobs_info *info)
{
  GtkWidget *dialog;
  GList *cbs = NULL ;		/* callbacks, pairs (function, argument)*/

  if (!info->selected)
    return;
  
  cbs = g_list_append (cbs,job_hstop_cb);
	cbs = g_list_append (cbs,info);
  cbs = g_list_append (cbs,update_joblist);
	cbs = g_list_append (cbs,info);

  dialog = ConfirmDialog ("Do you really want to hard stop the job?\n(This will kill all current running processes)",
			  cbs);

  g_list_free (cbs);

  gtk_grab_add(dialog);
}

static void job_hstop_cb (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* job hstop in the for of a gtk signal func callback */
  if (info->jdd.dialog) {
		drqm_request_job_hstop (info->jdd.job.id);
	} else {
		drqm_request_job_hstop (info->jobs[info->row].id);
	}
}

static GtkWidget *dnj_koj_widgets (struct drqm_jobs_info *info)
{
  GtkWidget *frame;
  GtkWidget *vbox, *hbox, *hbox2;
  GtkWidget *label, *combo;
  GList *items = NULL;
  GtkTooltips *tooltips;

  tooltips = TooltipsNew ();

  frame = gtk_frame_new ("Kind of job");
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox);
	info->dnj.vbkoj = vbox;

  /* Kind of job selector */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Kind of job:");
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,2);
  items = g_list_append (items,"General");
  items = g_list_append (items,"Maya");
	items = g_list_append (items,"Blender");
	items = g_list_append (items,"Bmrt");
	items = g_list_append (items,"Pixie");
  combo = gtk_combo_new();
  gtk_tooltips_set_tip(tooltips,GTK_COMBO(combo)->entry,"Selector for the kind of job",NULL);
  gtk_combo_set_popdown_strings (GTK_COMBO(combo),items);
  g_list_free (items);
  gtk_box_pack_start (GTK_BOX(hbox2),combo,TRUE,TRUE,2);
  gtk_entry_set_editable (GTK_ENTRY(GTK_COMBO(combo)->entry),FALSE);
  info->dnj.ckoj = combo;
  info->dnj.koj = 0;
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry),"General");
  g_signal_connect (G_OBJECT(GTK_ENTRY(GTK_COMBO(combo)->entry)),
										"changed",G_CALLBACK(dnj_koj_combo_changed),info);

  return (frame);
}

static void dnj_koj_combo_changed (GtkWidget *entry, struct drqm_jobs_info *info)
{
  int new_koj = -1;

  if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"General") == 0) {
    new_koj = KOJ_GENERAL;
  } else if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"Maya") == 0) {
    new_koj = KOJ_MAYA;
  } else if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"Blender") == 0) {
    new_koj = KOJ_BLENDER;
  } else if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"Bmrt") == 0) {
    new_koj = KOJ_BMRT;
  } else if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"Pixie") == 0) {
    new_koj = KOJ_PIXIE;
  } else {
/*     fprintf (stderr,"dnj_koj_combo_changed: koj not listed!\n"); */
/* 		fprintf (stderr,"entry: %s\n",gtk_entry_get_text(GTK_ENTRY(entry))); */
    return;
  }

  if (new_koj != info->dnj.koj) {
    if (info->dnj.fkoj) {
      gtk_widget_destroy (info->dnj.fkoj);
      info->dnj.fkoj = NULL;
    }
    info->dnj.koj = (uint16_t) new_koj;
    switch (info->dnj.koj) {
    case KOJ_GENERAL:
      break;
    case KOJ_MAYA:
      info->dnj.fkoj = dnj_koj_frame_maya (info);
      gtk_box_pack_start(GTK_BOX(info->dnj.vbkoj),info->dnj.fkoj,TRUE,TRUE,2);
      break;
    case KOJ_BLENDER:
      info->dnj.fkoj = dnj_koj_frame_blender (info);
      gtk_box_pack_start(GTK_BOX(info->dnj.vbkoj),info->dnj.fkoj,TRUE,TRUE,2);
      break;
		case KOJ_BMRT:
			info->dnj.fkoj = dnj_koj_frame_bmrt (info);
      gtk_box_pack_start(GTK_BOX(info->dnj.vbkoj),info->dnj.fkoj,TRUE,TRUE,2);
      break;
    case KOJ_PIXIE:
      info->dnj.fkoj = dnj_koj_frame_pixie (info);
      gtk_box_pack_start(GTK_BOX(info->dnj.vbkoj),info->dnj.fkoj,TRUE,TRUE,2);
      break;
    }
  }
}

static GtkWidget *dnj_limits_widgets (struct drqm_jobs_info *info)
{
  GtkWidget *frame;
  GtkWidget *vbox, *hbox;
  GtkWidget *label, *entry;
  GtkWidget *frame2, *cbutton;
  GtkTooltips *tooltips;

  tooltips = TooltipsNew ();

  frame = gtk_frame_new ("Limits");
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Maximum number of cpus:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(entry),"-1");
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  gtk_tooltips_set_tip (tooltips,entry,"Maximum number of cpus that can be running this job "
			"at the same time. Globally. "
			"For the maximum type -1",NULL);
  info->dnj.limits.enmaxcpus = entry;

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Maximum number of cpus on one computer:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(entry),"-1");
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  gtk_tooltips_set_tip (tooltips,entry,"Maximum number of cpus that can be running this job "
			"in the same computer at the same time. "
			"For the maximum type -1",NULL);
  info->dnj.limits.enmaxcpuscomputer = entry;

	// Memory stuff
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Minimum ammount of memory in Mbytes:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(entry),"0");
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  gtk_tooltips_set_tip (tooltips,entry,"Minimum ammount of memory that this jobs requires."
												"To cancel this limit use 0.",NULL);
  info->dnj.limits.ememory = entry;

	// Pools
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Render in pool:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY(entry),DEFAULT_POOL);
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  gtk_tooltips_set_tip (tooltips,entry,"Computer pool in which the job will be rendered.",NULL);
  info->dnj.limits.epool = entry;

	// OS Stuff
  frame2 = gtk_frame_new ("Operating Systems");
  gtk_box_pack_start (GTK_BOX(vbox),frame2,FALSE,FALSE,2);
  hbox = gtk_hbox_new (TRUE,2);
  gtk_container_add (GTK_CONTAINER(frame2),hbox);
  cbutton = gtk_check_button_new_with_label ("Irix");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  info->dnj.limits.cb_irix = cbutton;
  gtk_tooltips_set_tip (tooltips,cbutton,"If set this job will try to be executed on Irix "
			"computers. If not set it won't.", NULL);

  cbutton = gtk_check_button_new_with_label ("Linux");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  info->dnj.limits.cb_linux = cbutton;
  gtk_tooltips_set_tip (tooltips,cbutton,"If set this job will try to be executed on Linux "
			"computers. If not set it won't.", NULL);
  
	cbutton = gtk_check_button_new_with_label ("OS X");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  info->dnj.limits.cb_osx = cbutton;
  gtk_tooltips_set_tip (tooltips,cbutton,"If set this job will try to be executed on OS X "
			"computers. If not set it won't.", NULL);
	
  cbutton = gtk_check_button_new_with_label ("FreeBSD");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  info->dnj.limits.cb_freebsd = cbutton;
  gtk_tooltips_set_tip (tooltips,cbutton,"If set this job will try to be executed on FreeBSD "
			"computers. If not set it won't.", NULL);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.limits.cb_irix),TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.limits.cb_linux),TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.limits.cb_osx),TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.limits.cb_freebsd),TRUE);

  return (frame);
}

GtkWidget *dnj_flags_widgets (struct drqm_jobs_info *info)
{
  GtkWidget *frame;
  GtkWidget *vbox, *hbox;
  GtkWidget *cbutton,*entry;
  GtkTooltips *tooltips;
	GtkWidget *button;
  struct passwd *pw;

  tooltips = TooltipsNew ();

  frame = gtk_frame_new ("Flags");
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,TRUE,FALSE,2);

  cbutton = gtk_check_button_new_with_label ("Mail notification");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  gtk_tooltips_set_tip(tooltips,cbutton,"When set DrQueue will send emails to the owner of the job to "
		       "notify about important events.",NULL);
  gtk_signal_connect (GTK_OBJECT(cbutton),"toggled",GTK_SIGNAL_FUNC(dnj_flags_cbmailnotify_toggled),info);
  info->dnj.flags.cbmailnotify = cbutton;

  cbutton = gtk_check_button_new_with_label ("Specific email");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  gtk_tooltips_set_tip(tooltips,cbutton,"Mail notifications will be sent to this email instead of to "
		       "the regular user.",NULL);
  gtk_signal_connect (GTK_OBJECT(cbutton),"toggled",GTK_SIGNAL_FUNC(dnj_flags_cbdifemail_toggled),info);
  info->dnj.flags.cbdifemail = cbutton;

  entry = gtk_entry_new_with_max_length (MAXNAMELEN-1);
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  gtk_tooltips_set_tip(tooltips,entry,"Write here the email where you would like mail notifications "
		       "to be sent.",NULL);
  info->dnj.flags.edifemail = entry;

  if (!(pw = getpwuid(geteuid()))) {
    gtk_entry_set_text(GTK_ENTRY(entry),"ERROR");
  } else {
    gtk_entry_set_text(GTK_ENTRY(entry),pw->pw_name);
  }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.flags.cbmailnotify),FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.flags.cbdifemail),FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.cbdifemail),FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.edifemail),FALSE);

  
	// Job dependencies
	hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,TRUE,FALSE,2);

  cbutton = gtk_check_button_new_with_label ("Job depends on (jobid):");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  gtk_tooltips_set_tip(tooltips,cbutton,"When set DrQueue won't render any frame until it's related is finished",NULL);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_flags_cbjobdepend_toggled),info);
  info->dnj.flags.cbjobdepend = cbutton;

  entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  gtk_tooltips_set_tip(tooltips,entry,"Write here the job index to depend on",NULL);
  info->dnj.flags.ejobdepend = entry;

	button = gtk_button_new_with_label ("List jobs");
	gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
	gtk_tooltips_set_tip(tooltips,button,"Show a list of jobs",NULL);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(dnj_flags_bjobdepend_clicked),info);
	info->dnj.flags.bjobdepend = button;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.flags.cbjobdepend),FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.ejobdepend),FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.bjobdepend),FALSE);

  return (frame);
}

void dnj_flags_jdepend_refresh_job_list (GtkWidget *bclicked, struct drqm_jobs_info *info)
{
	GtkListStore *store = info->dnj.flags.store;
	GtkTreeIter iter;
	int i;

	gtk_list_store_clear (GTK_LIST_STORE(store));
	update_joblist(bclicked,info);
	for (i=0; i < info->njobs; i++) {
		gtk_list_store_append (store,&iter);
		gtk_list_store_set (store, &iter,
												DNJ_FLAGS_DEPEND_COL_ID, info->jobs[i].id,
												DNJ_FLAGS_DEPEND_COL_NAME, info->jobs[i].name,
												-1);
	}
}

void dnj_flags_bjobdepend_clicked (GtkWidget *bclicked, struct drqm_jobs_info *info)
{
	GtkWidget *dialog;
	GtkWidget *swindow;
	GtkWidget *button;

	// TreeView stuff
	GtkCellRenderer *renderer;
	GtkTreeModel *model;
	GtkWidget *view;
	// Store
	GtkListStore *store;

	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW(dialog),"List of jobs");
	gtk_window_set_default_size (GTK_WINDOW(dialog),300,200);

	swindow = gtk_scrolled_window_new (NULL,NULL);
	// Scrolled window
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swindow),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),swindow,TRUE,TRUE,2);
	// Refresh button
	button = gtk_button_new_with_label("Refresh");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),button,FALSE,FALSE,2);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(dnj_flags_jdepend_refresh_job_list),info);


	// The view
	view = gtk_tree_view_new();
	info->dnj.flags.view = GTK_TREE_VIEW (view);
	gtk_container_add(GTK_CONTAINER(swindow),view);
	


	// Column 1
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(view),
																							 -1,
																							 "Job Id",
																							 renderer,
																							 "text",DNJ_FLAGS_DEPEND_COL_ID,
																							 NULL);
	// Column 2
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(view),
																							 -1,
																							 "Name",
																							 renderer,
																							 "text",DNJ_FLAGS_DEPEND_COL_NAME,
																							 NULL);
	
	// Store & TreeView
	store = gtk_list_store_new (DNJ_FLAGS_DEPEND_NUM_COLS,G_TYPE_UINT, G_TYPE_STRING);
	info->dnj.flags.store = store;
	model = GTK_TREE_MODEL (store);
	gtk_tree_view_set_model (GTK_TREE_VIEW(view),model);
	g_object_unref (model);

	// Accept
	button = gtk_button_new_with_label ("Accept");
	gtk_box_pack_end(GTK_BOX(GTK_DIALOG(dialog)->action_area),button,TRUE,TRUE,2);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(dnj_flags_jdepend_accept),info);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),dialog);
	// Cancel
	button = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button,TRUE,TRUE,2);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),dialog);
	
	dnj_flags_jdepend_refresh_job_list(button,info);

	gtk_widget_show_all (dialog);

	gtk_grab_add (dialog);
}

void dnj_flags_jdepend_accept (GtkWidget *bclicked, struct drqm_jobs_info *info)
{
	GtkTreeSelection *selection = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(info->dnj.flags.view));

	if (gtk_tree_selection_get_selected (selection,&model,&iter)) {
		uint32_t jobid;
		char buf[BUFFERLEN];

		gtk_tree_model_get (GTK_TREE_MODEL(model),&iter,DNJ_FLAGS_DEPEND_COL_ID,&jobid,-1);
		snprintf(buf,BUFFERLEN,"%u",jobid);
		gtk_entry_set_text (GTK_ENTRY(info->dnj.flags.ejobdepend),buf);
	}
}

void dnj_flags_cbjobdepend_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info)
{
  if (GTK_TOGGLE_BUTTON(info->dnj.flags.cbjobdepend)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.ejobdepend),TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.bjobdepend),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.ejobdepend),FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.bjobdepend),FALSE);
  }
}


void dnj_flags_cbdifemail_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info)
{
  if (GTK_TOGGLE_BUTTON(info->dnj.flags.cbdifemail)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.edifemail),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.edifemail),FALSE);
  }
}

void dnj_flags_cbmailnotify_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info)
{
  if (GTK_TOGGLE_BUTTON(info->dnj.flags.cbmailnotify)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.cbdifemail),TRUE);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->dnj.flags.cbdifemail),FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.flags.cbdifemail),FALSE);
  }
}

