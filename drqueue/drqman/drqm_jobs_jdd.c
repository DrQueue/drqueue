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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Graphics
#include "waiting.xpm"
#include "running.xpm"
#include "finished.xpm"
#include "error.xpm"

// drqm
#include "drqm_common.h"
#include "drqm_request.h"
#include "drqm_jobs.h"

// Jdd static declarations
static GtkWidget *JobDetailsDialog (struct drqm_jobs_info *info);
static void jdd_destroy (GtkWidget *w, struct drqm_jobs_info *info);
static int jdd_update (GtkWidget *w, struct drqm_jobs_info *info);
// Frame's clist
static GtkWidget *CreateFrameInfoClist (void);
static GtkWidget *CreateMenuFrames (struct drqm_jobs_info *info);
static gint PopupMenuFrames (GtkWidget *clist, GdkEvent *event, struct drqm_jobs_info *info);
static void SeeFrameLog (GtkWidget *w, struct drqm_jobs_info *info);
static GtkWidget *SeeFrameLogDialog (struct drqm_jobs_info *info);
static void jdd_requeue_frames (GtkWidget *button,struct drqm_jobs_info *info_dj);
static void jdd_kill_frames_confirm (GtkWidget *button, struct drqm_jobs_info *info_dj);
static void jdd_kill_frames (GtkWidget *button,struct drqm_jobs_info *info_dj);
static void jdd_finish_frames (GtkWidget *button,struct drqm_jobs_info *info_dj);
static void jdd_kill_finish_frames_confirm (GtkWidget *button, struct drqm_jobs_info *info_dj);
static void jdd_kill_finish_frames (GtkWidget *button,struct drqm_jobs_info *info_dj);
static void jdd_frames_reset_requeued (GtkWidget *button,struct drqm_jobs_info *info);
static void jdd_sesframes_bcp (GtkWidget *button, struct drqm_jobs_info *info);
static GtkWidget *jdd_sesframes_change_dialog (struct drqm_jobs_info *info);
static void jdd_sesframes_cd_bsumbit_pressed (GtkWidget *button, struct drqm_jobs_info *info);
static void jdd_priority_bcp (GtkWidget *button, struct drqm_jobs_info *info);
static GtkWidget *jdd_priority_change_dialog (struct drqm_jobs_info *info);
static void jdd_pcd_cpri_changed (GtkWidget *entry, struct drqmj_jddi *info);
static void jdd_pcd_bsumbit_pressed (GtkWidget *button, struct drqm_jobs_info *info);
static void jdd_table_pack (GtkWidget *table, GtkWidget *label1, GtkWidget *label2, GtkWidget *button, int row);
static void jdd_framelist_column_clicked (GtkCList *clist, gint column, struct drqm_jobs_info *info);
static int jdd_framelist_cmp_frame (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
static int jdd_framelist_cmp_exitcode (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
static int jdd_framelist_cmp_status (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
static int jdd_framelist_cmp_icomp (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
static int jdd_framelist_cmp_start_time (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
static int jdd_framelist_cmp_end_time (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
static int jdd_framelist_cmp_requeued (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);
static gboolean show_log (gpointer data);

// Blocked hosts
static GtkWidget *CreateBlockedHostsClist (void);
static int jdd_update_blocked_hosts (GtkWidget *w, struct drqm_jobs_info *info);
static gint PopupMenuBlockedHosts (GtkWidget *clist, GdkEvent *event, struct drqm_jobs_info *info);
static void jdd_delete_blocked_host (GtkWidget *w, struct drqm_jobs_info *info);
static void jdd_add_blocked_host_bp (GtkWidget *button, struct drqm_jobs_info *info);
static GtkWidget *jdd_add_blocked_host_dialog (struct drqm_jobs_info *info);
static GtkWidget *jdd_add_blocked_host_dialog_clist (struct drqm_jobs_info *info);


/* Limits */
static GtkWidget *jdd_limits_widgets (struct drqm_jobs_info *info);
static void jdd_limits_nmaxcpus_bcp (GtkWidget *button, struct drqm_jobs_info *info);
static GtkWidget *jdd_nmc_dialog (struct drqm_jobs_info *info);
static void jdd_nmcd_bsumbit_pressed (GtkWidget *button, struct drqm_jobs_info *info);
static void jdd_limits_nmaxcpuscomputer_bcp (GtkWidget *button, struct drqm_jobs_info *info);
static GtkWidget *jdd_nmcc_dialog (struct drqm_jobs_info *info);
static void jdd_nmccd_bsumbit_pressed (GtkWidget *button, struct drqm_jobs_info *info);
static void jdd_limits_lmemory_bcp (GtkWidget *bclicked, struct drqm_jobs_info *info);
static void jdd_limits_lmemory_bcp_bokp (GtkWidget *bclicked, struct drqm_jobs_info *info);
// Pool
static void jdd_limits_lpool_bcp (GtkWidget *bclicked, struct drqm_jobs_info *info);
static void jdd_limits_lpool_bcp_bokp (GtkWidget *bclicked, struct drqm_jobs_info *info);
/* Flags */
static GtkWidget *jdd_flags_widgets (struct drqm_jobs_info *info);
/* KOJ */
static GtkWidget *jdd_koj_widgets (struct drqm_jobs_info *info);
/* Koj viewers */
static void jdd_maya_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info);
static void jdd_mentalray_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info);
static void jdd_blender_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info);
static void jdd_bmrt_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info);
static void jdd_pixie_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info);


struct row_data {
  uint32_t frame;
  struct drqm_jobs_info *info;
};


void JobDetails(GtkWidget *menu_item, struct drqm_jobs_info *info)
{
  GtkWidget *dialog;

  if (!info->selected)
    return;

  dialog = JobDetailsDialog(info);
}

static GtkWidget *CreateBlockedHostsClist (void)
{
	gchar *titles[] = { "Pos","Name" };
	GtkWidget *clist;

	clist = gtk_clist_new_with_titles (2,titles);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);

	gtk_widget_show(clist);

	return clist;
}

static GtkWidget *CreateMenuBlockedHosts (struct drqm_jobs_info *info)
{
	GtkWidget *menu;
	GtkWidget *menu_item;

	menu = gtk_menu_new();
	menu_item = gtk_menu_item_new_with_label("Add");
	gtk_menu_append(GTK_MENU(menu),menu_item);
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(jdd_add_blocked_host_bp),info);

	menu_item = gtk_menu_item_new_with_label("Delete");
	gtk_menu_append(GTK_MENU(menu),menu_item);
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(jdd_delete_blocked_host),info);
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(jdd_update),info);

  g_signal_connect(G_OBJECT(info->jdd.clist_bh),"event",G_CALLBACK(PopupMenuBlockedHosts),info);

  gtk_widget_show_all(menu);

	return menu;
}

static GtkWidget *CreateMenuFrames (struct drqm_jobs_info *info)
{
  GtkWidget *menu;
  GtkWidget *menu_item;
  GtkTooltips *tooltips;

  tooltips = TooltipsNew ();

  menu = gtk_menu_new ();
  menu_item = gtk_menu_item_new_with_label("Set Waiting (requeue finished)");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_requeue_frames),info);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_update),info);
  gtk_tooltips_set_tip(tooltips,menu_item,"This option will requeue al selected frames that are "
		       "currently finished. Those finished frames will be rendered again.\n"
		       "This option has no effect on frames that are not finished.",NULL);
  
  menu_item = gtk_menu_item_new_with_label("Kill + Wait (requeue running)");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_kill_frames_confirm),info);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_update),info);
  gtk_tooltips_set_tip(tooltips,menu_item,"This option will kill and requeue al selected frames that are "
		       "currently running. Those running frames will start rendering again.\n"
		       "This option has no effect on frames that are not running.",NULL);
  gtk_widget_set_name (menu_item,"warning");

  menu_item = gtk_menu_item_new_with_label("Set Finished (skip waiting)");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_finish_frames),info);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_update),info);
  gtk_tooltips_set_tip(tooltips,menu_item,"This option will set as finished those selected frames "
		       "that are currently waiting. So those frames won't start rendering and will be skipped.\n"
		       "This option has no effect on frames that are not waiting.", NULL);

  menu_item = gtk_menu_item_new_with_label("Kill + Finished (skip running)");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_kill_finish_frames_confirm),info);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_update),info);
  gtk_tooltips_set_tip(tooltips,menu_item,"This option will kill and set as finished those selected frames "
		       "that are currently running. So the render will stop and won't be requeued again (unless "
		       "manually requeued).\n"
		       "This option has no effect on frames that are not running.", NULL);
  gtk_widget_set_name (menu_item,"warning");

  /* Separation bar */
  menu_item = gtk_menu_item_new ();
  gtk_menu_append(GTK_MENU(menu),menu_item);
	
	// Reset requeued counter.
  menu_item = gtk_menu_item_new_with_label("Reset requeued counter");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_frames_reset_requeued),info);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_update),info);
  gtk_tooltips_set_tip(tooltips,menu_item,"This option will kill and set as finished those selected frames "
		       "that are currently running. So the render will stop and won't be requeued again (unless "
		       "manually requeued).\n"
		       "This option has no effect on frames that are not running.", NULL);
  gtk_widget_set_name (menu_item,"warning");
		
  /* Separation bar */
  menu_item = gtk_menu_item_new ();
  gtk_menu_append(GTK_MENU(menu),menu_item);

  menu_item = gtk_menu_item_new_with_label("Watch frame Log");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(SeeFrameLog),info);

  switch (info->jdd.job.koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
    menu_item = gtk_menu_item_new ();
    gtk_menu_append(GTK_MENU(menu),menu_item);
    menu_item = gtk_menu_item_new_with_label("Watch image");
    gtk_menu_append(GTK_MENU(menu),menu_item);
    gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_maya_viewcmd_exec),info);
    break;
  case KOJ_MENTALRAY:
    menu_item = gtk_menu_item_new ();
    gtk_menu_append(GTK_MENU(menu),menu_item);
    menu_item = gtk_menu_item_new_with_label("Watch image");
    gtk_menu_append(GTK_MENU(menu),menu_item);
    gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_mentalray_viewcmd_exec),info);
    break;
  case KOJ_BLENDER:
    menu_item = gtk_menu_item_new ();
    gtk_menu_append(GTK_MENU(menu),menu_item);
    menu_item = gtk_menu_item_new_with_label("Watch image");
    gtk_menu_append(GTK_MENU(menu),menu_item);
    gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_blender_viewcmd_exec),info);
    break;
  case KOJ_BMRT:
    menu_item = gtk_menu_item_new ();
    gtk_menu_append(GTK_MENU(menu),menu_item);
    menu_item = gtk_menu_item_new_with_label("Watch image");
    gtk_menu_append(GTK_MENU(menu),menu_item);
    gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_bmrt_viewcmd_exec),info);
    break;
  case KOJ_PIXIE:
    menu_item = gtk_menu_item_new ();
    gtk_menu_append(GTK_MENU(menu),menu_item);
    menu_item = gtk_menu_item_new_with_label("Watch image");
    gtk_menu_append(GTK_MENU(menu),menu_item);
    gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(jdd_pixie_viewcmd_exec),info);
    break;
  }

  gtk_signal_connect(GTK_OBJECT((info->jdd.clist)),"event",GTK_SIGNAL_FUNC(PopupMenuFrames),info);

  gtk_widget_show_all(menu);

  return (menu);
}

static int jdd_update_blocked_hosts (GtkWidget *w, struct drqm_jobs_info *info)
{
	int ncols = 2;
	int i;
	char **buff;

	if (!request_job_list_blocked_host(info->jdd.job.id, &info->jdd.job.blocked_host, &info->jdd.job.nblocked, CLIENT)) {
		fprintf (stderr,"Error request_job_list_blocked_host\n");
		if (info->jdd.job.blocked_host) {
			free (info->jdd.job.blocked_host);
		}
		return 0;
	}

  buff = (char**) g_malloc((ncols + 1) * sizeof(char*));
  for (i=0;i<ncols;i++)
    buff[i] = (char*) g_malloc (BUFFERLEN);
  buff[ncols] = NULL;
  
  gtk_clist_freeze(GTK_CLIST(info->jdd.clist_bh));
  gtk_clist_clear(GTK_CLIST(info->jdd.clist_bh));

  for (i=0; i < info->jdd.job.nblocked; i++) {
    snprintf (buff[0],BUFFERLEN-1,"%i",i);
    snprintf (buff[1],BUFFERLEN,"%s",info->jdd.job.blocked_host[i].name);
		
		gtk_clist_append (GTK_CLIST(info->jdd.clist_bh),buff);

		gtk_clist_set_row_data (GTK_CLIST(info->jdd.clist_bh),i,(gpointer)i);
  }

  gtk_clist_thaw(GTK_CLIST(info->jdd.clist_bh));

	free (info->jdd.job.blocked_host);

	return 1;
}

static int jdd_update (GtkWidget *w, struct drqm_jobs_info *info)
{
  /* This function depends on info->row properly set (like most) */
  int nframes;
  struct frame_info *fi = NULL;
  char msg[BUFFERLEN];
  char *buf;
  char **buff;			/* for the clist stuff */
  int ncols = 8;
  int i;
  GtkWidget *toplevel;
	// To keep the state of the scrolled window
	GtkAdjustment *adj;
	gdouble vadj_value,hadj_value;

  static GdkBitmap *w_mask = NULL;
  static GdkPixmap *w_data = NULL;
  static GdkBitmap *r_mask = NULL;
  static GdkPixmap *r_data = NULL;
  static GdkBitmap *f_mask = NULL;
  static GdkPixmap *f_data = NULL;
  static GdkBitmap *e_mask = NULL;
  static GdkPixmap *e_data = NULL;

	jdd_update_blocked_hosts (w,info);

  if (!request_job_xfer(info->jdd.job.id,&info->jdd.job,CLIENT)) {
    if (drerrno == DRE_NOTREGISTERED) {
      gtk_object_destroy (GTK_OBJECT(info->jdd.dialog));
    } else {
      fprintf (stderr,"Error request job xfer: %s\n",drerrno_str());
    }
    return 0;
  }

  nframes = job_nframes (&info->jdd.job);

  if (!info->jdd.job.frame_info) {
    if (!(fi = g_malloc(sizeof (struct frame_info) * nframes))) {
      fprintf (stderr,"Error allocating memory for frame information\n");
      return 0;
    }

    if (!request_job_xferfi (info->jdd.job.id,fi,nframes,CLIENT)) {
      fprintf (stderr,"Error request job frame info xfer: %s\n",drerrno_str());
      g_free (fi);
      return 0;
    }

    info->jdd.job.frame_info = fi;
  }

  gtk_label_set_text (GTK_LABEL(info->jdd.lname),info->jdd.job.name);
  gtk_label_set_text (GTK_LABEL(info->jdd.lowner),info->jdd.job.owner);
  gtk_label_set_text (GTK_LABEL(info->jdd.lcmd),info->jdd.job.cmd);
  gtk_label_set_text (GTK_LABEL(info->jdd.lstatus),job_status_string(info->jdd.job.status));

  snprintf(msg,BUFFERLEN-1,"From %u to %u every %u",
	   info->jdd.job.frame_start,
	   info->jdd.job.frame_end,
	   info->jdd.job.frame_step);
  gtk_label_set_text (GTK_LABEL(info->jdd.lstartend),msg);

	snprintf(msg,BUFFERLEN-1,"%u",info->jdd.job.block_size);
	gtk_label_set_text (GTK_LABEL(info->jdd.lbs),msg);
  
  snprintf(msg,BUFFERLEN-1,"%i",info->jdd.job.priority);
  gtk_label_set_text (GTK_LABEL(info->jdd.lpri),msg);

  snprintf(msg,BUFFERLEN-1,"%u,%u,%u,%u",
	   info->jdd.job.nprocs,
	   info->jdd.job.fleft,
	   info->jdd.job.fdone,
	   info->jdd.job.ffailed);
  gtk_label_set_text (GTK_LABEL(info->jdd.lfrldf),msg);

  gtk_label_set_text (GTK_LABEL(info->jdd.lavgt),time_str(info->jdd.job.avg_frame_time));
  
  snprintf(msg,BUFFERLEN-1,"%s",ctime(&info->jdd.job.est_finish_time));
  buf = strchr (msg,'\n');
  if (buf != NULL)
    *buf = '\0';
  gtk_label_set_text (GTK_LABEL(info->jdd.lestf),msg);

  /* Limits */
  if (info->jdd.job.limits.nmaxcpus == 65535)
    snprintf(msg,BUFFERLEN-1,"Maximum");
  else
    snprintf(msg,BUFFERLEN-1,"%u",info->jdd.job.limits.nmaxcpus);
  gtk_label_set_text(GTK_LABEL(info->jdd.limits.lnmaxcpus),msg);
  if (info->jdd.job.limits.nmaxcpuscomputer == 65535)
    snprintf(msg,BUFFERLEN-1,"Maximum");
  else
    snprintf(msg,BUFFERLEN-1,"%u",info->jdd.job.limits.nmaxcpuscomputer);
  gtk_label_set_text(GTK_LABEL(info->jdd.limits.lnmaxcpuscomputer),msg);
	
	// Limits Memory
	if (info->jdd.job.limits.memory == 0)
		snprintf(msg,BUFFERLEN,"No limit");
	else
		snprintf(msg,BUFFERLEN,"%u",info->jdd.job.limits.memory);
	gtk_label_set_text(GTK_LABEL(info->jdd.limits.lmemory),msg);
	
	// Pool
	snprintf (msg,BUFFERLEN,"%s",info->jdd.job.limits.pool);
	gtk_label_set_text(GTK_LABEL(info->jdd.limits.lpool),msg);

  /* Limits OS Flags */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->jdd.limits.cb_irix),
				info->jdd.job.limits.os_flags & OSF_IRIX);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->jdd.limits.cb_linux),
				info->jdd.job.limits.os_flags & OSF_LINUX);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->jdd.limits.cb_osx),
				info->jdd.job.limits.os_flags & OSF_OSX);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(info->jdd.limits.cb_freebsd),
				info->jdd.job.limits.os_flags & OSF_FREEBSD);


  /* Pixmap stuff */
  if (!w_mask) {
    toplevel = gtk_widget_get_toplevel(info->jdd.dialog);
    gtk_widget_realize(toplevel);
    w_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&w_mask,NULL,(gchar**)waiting_xpm);
    if (!r_mask)
      r_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&r_mask,NULL,(gchar**)running_xpm);
    if (!f_mask)
      f_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&f_mask,NULL,(gchar**)finished_xpm);
    if (!e_mask)
      e_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&e_mask,NULL,(gchar**)error_xpm);
  }

	// ScrolledWindow save the state
	adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(info->jdd.swindow));
	vadj_value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adj));
	adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(info->jdd.swindow));
	hadj_value = gtk_adjustment_get_value (GTK_ADJUSTMENT(adj));

  buff = (char**) g_malloc((ncols + 1) * sizeof(char*));
  for (i=0;i<ncols;i++)
    buff[i] = (char*) g_malloc (BUFFERLEN);
  buff[ncols] = NULL;
  
  gtk_clist_freeze(GTK_CLIST(info->jdd.clist));
  gtk_clist_clear(GTK_CLIST(info->jdd.clist));
  for (i=0; i < nframes; i++) {
    struct row_data *rdata;

    snprintf (buff[0],BUFFERLEN-1,"%i",job_frame_index_to_number (&info->jdd.job,i));
    strncpy(buff[1],job_frame_status_string(info->jdd.job.frame_info[i].status),BUFFERLEN);
    snprintf (buff[2],BUFFERLEN-1,"%i",info->jdd.job.frame_info[i].requeued);
    if (info->jdd.job.frame_info[i].start_time != 0) {
      strncpy(buff[3],ctime(&info->jdd.job.frame_info[i].start_time),BUFFERLEN); 
			buf = strchr (buff[3],'\n');
			if (buf != NULL)
				*buf = '\0';
      strncpy(buff[4],ctime(&info->jdd.job.frame_info[i].end_time),BUFFERLEN);
			buf = strchr (buff[4],'\n');
			if (buf != NULL)
				*buf = '\0';
    } else {
      strncpy(buff[3],"Not started",BUFFERLEN); 
      strncpy(buff[4],"Not started",BUFFERLEN);
    }
		// Depending on the exitcode we set the background
    snprintf (buff[5],BUFFERLEN,"%i",info->jdd.job.frame_info[i].exitcode);
    snprintf (buff[6],BUFFERLEN,"%i",info->jdd.job.frame_info[i].icomp);
    snprintf (buff[7],BUFFERLEN,"%i",info->jdd.job.frame_info[i].itask);
    gtk_clist_append(GTK_CLIST(info->jdd.clist),buff);
		if (info->jdd.job.frame_info[i].exitcode != 0) {
			GdkColor color;
			color.pixel = 0;
			color.red = 0xffff;
			color.green = 0x8000;
			color.blue = 0x8000;
			gtk_clist_set_background (GTK_CLIST(info->jdd.clist),i,&color);
		}
    switch (info->jdd.job.frame_info[i].status) {
    case FS_WAITING:
      gtk_clist_set_pixtext (GTK_CLIST(info->jdd.clist),i,1,
			     job_frame_status_string(info->jdd.job.frame_info[i].status), 2,
			     w_data,w_mask);
      break;
    case FS_ASSIGNED:
      gtk_clist_set_pixtext (GTK_CLIST(info->jdd.clist),i,1,
			     job_frame_status_string(info->jdd.job.frame_info[i].status), 2,
			     r_data,r_mask);
      break;
    case FS_FINISHED:
      gtk_clist_set_pixtext (GTK_CLIST(info->jdd.clist),i,1,
			     job_frame_status_string(info->jdd.job.frame_info[i].status), 2,
			     f_data,f_mask);
      break;
    case FS_ERROR:
      gtk_clist_set_pixtext (GTK_CLIST(info->jdd.clist),i,1,
			     job_frame_status_string(info->jdd.job.frame_info[i].status), 2,
			     e_data,e_mask);
      break;
    }

    /* Set the information for the row */
    rdata = gtk_clist_get_row_data (GTK_CLIST(info->jdd.clist),i);
    if (!rdata) {
      rdata = g_malloc (sizeof (*rdata));
    }
    rdata->frame = job_frame_index_to_number (&info->jdd.job,i);
    rdata->info = info;
    gtk_clist_set_row_data_full (GTK_CLIST(info->jdd.clist),i,(gpointer)rdata, g_free);
  }

  gtk_clist_sort (GTK_CLIST(info->jdd.clist));

  gtk_clist_thaw(GTK_CLIST(info->jdd.clist));

	// Recover the previous state of the scrolled window
	adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(info->jdd.swindow));
	gtk_adjustment_set_value (GTK_ADJUSTMENT(adj),vadj_value);
	adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(info->jdd.swindow));
	gtk_adjustment_set_value (GTK_ADJUSTMENT(adj),hadj_value);

  for(i=0;i<ncols;i++)
    g_free (buff[i]);
	g_free (buff);

  return 1;
}

static void jdd_add_blocked_host (GtkWidget *button, struct drqm_jobs_info *info)
{
  GList *sel;
  uint32_t icomp;
  
  if (!(sel = GTK_CLIST(info->jdd.bhdi_computers_info.clist)->selection)) {
    return;
  }

	// We need to remove hosts in reverse order
	sel = g_list_reverse (sel);	// Because we are removing indexes if remove 0 and then 1, after removing 0, 1 would be 0 again.
	

  for (;sel;sel = sel->next) {
    icomp = (uint32_t) gtk_clist_get_row_data(GTK_CLIST(info->jdd.bhdi_computers_info.clist), (gint)sel->data);
		request_job_add_blocked_host (info->jdd.job.id,icomp, CLIENT);
  }
}

static GtkWidget *CreateFrameInfoClist (void)
{
  gchar *titles[] = { "Number","Status","Requeued","Start","End","Exit Code","Icomp","Itask" };
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (8, titles);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_set_column_width (GTK_CLIST(clist),0,55);
  gtk_clist_set_column_width (GTK_CLIST(clist),1,95);
  gtk_clist_set_column_width (GTK_CLIST(clist),2,85);
  gtk_clist_set_column_width (GTK_CLIST(clist),3,180);
  gtk_clist_set_column_width (GTK_CLIST(clist),4,180);
  gtk_clist_set_column_width (GTK_CLIST(clist),5,85);
  gtk_clist_set_column_width (GTK_CLIST(clist),6,45);
  gtk_clist_set_column_width (GTK_CLIST(clist),7,45);

  gtk_clist_set_sort_type (GTK_CLIST(clist),GTK_SORT_ASCENDING);
  gtk_clist_set_compare_func (GTK_CLIST(clist),jdd_framelist_cmp_frame);

  gtk_widget_show(clist);

  return (clist);
}

static void jdd_destroy (GtkWidget *w, struct drqm_jobs_info *info)
{
  if (info->jdd.job.frame_info) {
    g_free (info->jdd.job.frame_info);
    info->jdd.job.frame_info = NULL;
  }

	if (info->jdd.job.blocked_host) {
		free (info->jdd.job.blocked_host);
		info->jdd.job.blocked_host = NULL;
	}
	
	g_free (info);
}

static GtkWidget *JobDetailsDialog (struct drqm_jobs_info *info)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *table;
  GtkWidget *label, *label2;
  GtkWidget *clist;
  GtkWidget *swin;
  GtkWidget *button;
	GtkWidget *main_vbox;
	GtkWidget *notebook;
  GtkTooltips *tooltips;
  struct drqm_jobs_info *newinfo;

  if (!info->njobs) {
    return NULL;
  }

	// We create a new info structure for this window.
  newinfo = g_malloc (sizeof (struct drqm_jobs_info));
  if (!newinfo) {
    return NULL;
  }
  memcpy (newinfo,info,sizeof(struct drqm_jobs_info));
  memcpy (&newinfo->jdd.job,&info->jobs[info->row],sizeof (struct job));
	newinfo->jdd.oldinfo = info;

  tooltips = TooltipsNew ();

  /* Dialog */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"Job Details");
	g_signal_connect (G_OBJECT(window),"destroy",G_CALLBACK(jdd_destroy),newinfo);
  // gtk_window_set_default_size(GTK_WINDOW(window),800,900);
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, TRUE, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER(window),5);
  newinfo->jdd.dialog = window;

	// Main vbox
	main_vbox = gtk_vbox_new (FALSE,2);
	gtk_container_add (GTK_CONTAINER(window),main_vbox);

	
	// Notebook
	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(main_vbox),notebook,TRUE,TRUE,1);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),GTK_POS_TOP);

	// First notebook page
	label = gtk_label_new ("Main Info");
	frame = gtk_frame_new ("Main Info");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),GTK_WIDGET(frame),GTK_WIDGET(label));

  /* vbox for the main info */
  vbox = gtk_vbox_new (FALSE,2);
	gtk_container_add (GTK_CONTAINER(frame),GTK_WIDGET(vbox));

  /* Label */
  label = gtk_label_new ("Detailed job information");
  gtk_label_set_pattern (GTK_LABEL(label),"________________________________");
  gtk_box_pack_start (GTK_BOX(vbox),label,FALSE,FALSE,4);

  /* Table */
  table = gtk_table_new ( 9, 3, FALSE );
  gtk_box_pack_start (GTK_BOX(vbox),table,FALSE,FALSE,2);

  /* Name of the job */
  label = gtk_label_new ("Name:");
  label2 = gtk_label_new (NULL);
  newinfo->jdd.lname = label2;
  jdd_table_pack (table, label, label2, NULL, 0);

  /* Owner */
  label = gtk_label_new ("Owner:");
  label2 = gtk_label_new (NULL);
  newinfo->jdd.lowner = label2;
  jdd_table_pack (table, label, label2, NULL, 1);

  /* Status */
  label = gtk_label_new ("Status:");
  label2 = gtk_label_new (NULL);
  newinfo->jdd.lstatus = label2;
  jdd_table_pack (table, label, label2, NULL, 2);


  /* Cmd of the job */
  label = gtk_label_new ("Command:");
  label2 = gtk_label_new (NULL);
  newinfo->jdd.lcmd = label2;
  jdd_table_pack (table, label, label2, NULL, 3);

  /* Start and End frames */
  label = gtk_label_new ("Start, end and step frames:");
  label2 = gtk_label_new (NULL);
  newinfo->jdd.lstartend = label2;
  button = gtk_button_new_with_label ("Change");
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_sesframes_bcp),newinfo);
  jdd_table_pack (table, label, label2, button, 4);
  

  /* Priority */
  label = gtk_label_new ("Priority:");
  label2 = gtk_label_new (NULL);
  newinfo->jdd.lpri = label2;
  button = gtk_button_new_with_label ("Change");
  gtk_signal_connect (GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_priority_bcp),newinfo);
  jdd_table_pack (table, label, label2, button, 5);

  /* Frames left, done and failed */
  label = gtk_label_new ("Frames running, left, done and failed:");
  label2 = gtk_label_new (NULL);
  newinfo->jdd.lfrldf = label2;
  jdd_table_pack (table, label, label2, NULL, 6);

	/* Block size */
	label = gtk_label_new ("Block size:");
	label2 = gtk_label_new (NULL);
	newinfo->jdd.lbs = label2;
	jdd_table_pack (table, label, label2, NULL, 7);

  /* Average time per frame */
  label = gtk_label_new ("Average frame time:");
  label2 = gtk_label_new (NULL);
  newinfo->jdd.lavgt = label2;
  jdd_table_pack (table, label, label2, NULL, 8);

  /* Estimated finish time */
  label = gtk_label_new ("Estimated finish time:");
  label2 = gtk_label_new (NULL);
  newinfo->jdd.lestf = label2;
  jdd_table_pack (table, label, label2, NULL, 9);

  /* KOJ */
  frame = jdd_koj_widgets (newinfo);
  gtk_box_pack_start (GTK_BOX(vbox),frame,FALSE,FALSE,2);

  /* Limits */
  frame = jdd_limits_widgets (newinfo);
  gtk_box_pack_start (GTK_BOX(vbox),frame,FALSE,FALSE,2);

  /* Flags */
  frame = jdd_flags_widgets (newinfo);
  gtk_box_pack_start (GTK_BOX(vbox),frame,FALSE,FALSE,2);

	// New notebook page
  /* Clist with the frame info */
  /* Frame */
  frame = gtk_frame_new ("Frame Information");
	label = gtk_label_new ("Frame Information");
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook),GTK_WIDGET(frame),GTK_WIDGET(label));
  /* Clist with the frame info inside a scrolled window */
  swin = gtk_scrolled_window_new (NULL,NULL);
	newinfo->jdd.swindow = swin;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER(frame),swin);
  clist = CreateFrameInfoClist ();
  gtk_signal_connect(GTK_OBJECT(clist),"click-column",
		     GTK_SIGNAL_FUNC(jdd_framelist_column_clicked),newinfo);
  gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);
  gtk_container_add (GTK_CONTAINER(swin),clist);
  newinfo->jdd.clist = clist;
	
  newinfo->jdd.menu = CreateMenuFrames(newinfo);

	// New notebook page
	frame = gtk_frame_new ("Block list");
	label = gtk_label_new ("Block list");
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook),GTK_WIDGET(frame),GTK_WIDGET(label));
	// Clist with the block list
  swin = gtk_scrolled_window_new (NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER(frame),swin);
  clist = CreateBlockedHostsClist ();
  gtk_container_add (GTK_CONTAINER(swin),clist);
  newinfo->jdd.clist_bh = clist;
	newinfo->jdd.menu_bh = CreateMenuBlockedHosts (newinfo);


  if (!jdd_update (window,newinfo)) {
    gtk_widget_destroy (GTK_WIDGET(window));
    return NULL;
  }

  /* Buttons */
  /* Stop */
  hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,5);
  button = gtk_button_new_with_label ("Stop");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(StopJob),newinfo);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_update),newinfo);
  gtk_tooltips_set_tip (tooltips,button,"Set the job as 'Stopped' but let the running frames finish",NULL);

  /* Hard Stop */
  button = gtk_button_new_with_label ("Hard Stop");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(HStopJob),newinfo);
  g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(jdd_update),newinfo);
  gtk_tooltips_set_tip (tooltips,button,"Set the job as 'Stopped' killing all running frames",NULL);
  gtk_widget_set_name (GTK_WIDGET(button),"warning");

  /* Continue */
  button = gtk_button_new_with_label ("Continue");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(ContinueJob),newinfo);
  g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(jdd_update),newinfo);
  gtk_tooltips_set_tip (tooltips,button,"Set a 'Stopped' job as 'Waiting' again",NULL);

  /* Delete */
  button = gtk_button_new_with_label ("Delete");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(DeleteJob),newinfo);
  gtk_tooltips_set_tip (tooltips,button,"Delete the job from the queue killing running frames",NULL);
  gtk_widget_set_name (GTK_WIDGET(button),"danger");


	// Out of the notebook
  /* Button Refresh */
  button = gtk_button_new_with_label ("Refresh");
  gtk_container_border_width (GTK_CONTAINER(button),5);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_update),newinfo);
  gtk_box_pack_start (GTK_BOX(main_vbox),button,FALSE,FALSE,2);

  gtk_widget_show_all(window);

  return window;
}

static GtkWidget *jdd_add_blocked_host_dialog_clist (struct drqm_jobs_info *info)
{
  gchar *titles[] = { "ID","Running","Name","OS","CPUs","Load Avg" };
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (6, titles);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_column_titles_passive(GTK_CLIST(clist));
  gtk_clist_set_column_width (GTK_CLIST(clist),0,75);
  gtk_clist_set_column_width (GTK_CLIST(clist),1,100);
  gtk_clist_set_column_width (GTK_CLIST(clist),2,100);
  gtk_clist_set_column_width (GTK_CLIST(clist),3,100);
  gtk_clist_set_column_width (GTK_CLIST(clist),4,45);
  gtk_clist_set_column_width (GTK_CLIST(clist),5,100);
  gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);
  gtk_widget_show(clist);

  return (clist);
}

static GtkWidget *jdd_add_blocked_host_dialog (struct drqm_jobs_info *info)
{
	GtkWidget *window;
	GtkWidget *vbox,*hbox;
	GtkWidget *button;
	GtkWidget *swindow;
	GtkWidget *clist;
	GtkTooltips *tooltips;

	tooltips = TooltipsNew();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"Add host(s) to block list");
  gtk_window_set_policy(GTK_WINDOW(window),TRUE,TRUE,TRUE);
	gtk_window_set_default_size (GTK_WINDOW(window),600,200);
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add(GTK_CONTAINER(window),vbox);

	// Scrolled window
	swindow = gtk_scrolled_window_new (NULL,NULL);
	info->jdd.bhdi_computers_info.swindow = swindow;
	gtk_box_pack_start (GTK_BOX(vbox),swindow,TRUE,TRUE,2);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	// Computers clist
	clist = jdd_add_blocked_host_dialog_clist (info);
	info->jdd.bhdi_computers_info.clist = clist;
	gtk_container_add(GTK_CONTAINER(swindow),clist);
  drqm_request_computerlist (&info->jdd.bhdi_computers_info);
  drqm_update_computerlist (&info->jdd.bhdi_computers_info);
	// Button to refresh the list
	button = gtk_button_new_with_label ("Refresh");
	gtk_box_pack_start (GTK_BOX(vbox),button,FALSE,FALSE,2);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",G_CALLBACK(drqm_request_computerlist),&info->jdd.bhdi_computers_info);
	g_signal_connect_swapped(G_OBJECT(button),"clicked",G_CALLBACK(drqm_update_computerlist),&info->jdd.bhdi_computers_info);

	hbox = gtk_hbox_new (FALSE,2);
	gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	button = gtk_button_new_with_label ("Add");
	gtk_tooltips_set_tip (tooltips,button,"Add selected host(s) to the blocked list",NULL);
	gtk_box_pack_start (GTK_BOX(hbox),button, TRUE, TRUE, 2);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(jdd_add_blocked_host),info);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(jdd_update),info);
	g_signal_connect_swapped (G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),window);
	button = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX(hbox),button, TRUE, TRUE, 2);
	g_signal_connect_swapped (G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),window);

	gtk_widget_show_all (window);

	return window;	
}

static gint PopupMenuFrames (GtkWidget *clist, GdkEvent *event, struct drqm_jobs_info *info)
{
  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button != 3)
      return FALSE;
    info->jdd.selected = gtk_clist_get_selection_info(GTK_CLIST(info->jdd.clist),
						      (int)bevent->x,(int)bevent->y,
						      &info->jdd.row,&info->jdd.column);
    gtk_menu_popup (GTK_MENU(info->jdd.menu), NULL, NULL, NULL, NULL,
		    bevent->button, bevent->time);
    return TRUE;
  }
  return FALSE;
}

static void SeeFrameLog (GtkWidget *w, struct drqm_jobs_info *info)
{
  GtkWidget *dialog;

  if (!info->jdd.selected)
    return;

  dialog = SeeFrameLogDialog(info);
  if (dialog)
    gtk_grab_add(dialog);
}

static GtkWidget *SeeFrameLogDialog (struct drqm_jobs_info *info)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *text;
	GtkTextBuffer *buffer;
  GtkWidget *swin;
  int fd;
  char buf[BUFFERLEN];
  struct task task;
	struct idle_info *iinfo;

  /* log_dumptask_open only uses the jobname and frame fields of the task */
  /* so I fill a task with only those two valid fields and so can use that */
  /* function */
  strncpy (task.jobname,info->jdd.job.name,MAXNAMELEN-1);
  task.frame = job_frame_index_to_number (&info->jdd.job,info->jdd.row);

	// Allocate memory for idle info
	iinfo = (struct idle_info *)g_malloc(sizeof (struct idle_info));

  /* Dialog */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"Frame log");
	g_signal_connect_swapped (G_OBJECT(window),"destroy",G_CALLBACK(g_idle_remove_by_data),iinfo);
	g_signal_connect_swapped (G_OBJECT(window),"destroy",G_CALLBACK(g_free),iinfo);
	gtk_window_set_default_size(GTK_WINDOW(window),600,200);
  gtk_container_set_border_width (GTK_CONTAINER(window),5);

  /* Frame */
  snprintf (buf,BUFFERLEN-1,"Log for frame %i on job %s",task.frame,task.jobname);
  frame = gtk_frame_new (buf);
  gtk_container_add (GTK_CONTAINER(window),frame);

  /* Text */
  swin = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER(frame),swin);
  text = gtk_text_view_new ();
	gtk_widget_set_sensitive (GTK_WIDGET(text),FALSE);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text));
  gtk_container_add (GTK_CONTAINER(swin),text);

  if ((fd = log_dumptask_open_ro (&task)) == -1) {
    char msg[] = "Couldn't open log file";
    gtk_text_buffer_set_text (buffer,msg,-1);
  } else {
		iinfo->fd = fd;
		iinfo->text = text;
		g_idle_add (show_log,iinfo);
		g_signal_connect_swapped (G_OBJECT(window),"destroy",G_CALLBACK(close),(gpointer)fd);
  }
	g_signal_connect_swapped (G_OBJECT(window),"destroy",G_CALLBACK(gtk_widget_destroy),(GtkObject*)window);

  gtk_widget_show_all (window);

  return window;
}

static gboolean show_log (gpointer data)
{
	struct idle_info *iinfo = data;
	int fd = iinfo->fd;
  char buf[BUFFERLEN];
	int n;
	GtkTextMark *mark;
  GtkWidget *text = iinfo->text;
	GtkTextBuffer *buffer;


	if ((n = read (fd,buf,BUFFERLEN)) != 0) {
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(text));
		gtk_text_buffer_insert_at_cursor (buffer,buf,n);
		mark = gtk_text_buffer_get_insert (buffer);
		gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(text),mark);
	} else {
		sleep (1);
	}

	return TRUE;
}

static void jdd_requeue_frames (GtkWidget *button,struct drqm_jobs_info *info_dj)
{
  /* Requeues the finished frames, sets them as waiting again */
  GList *sel;
  uint32_t frame;
  struct row_data *rdata;
  
  if (!(sel = GTK_CLIST(info_dj->jdd.clist)->selection)) {
    return;
  }

  for (;sel;sel = sel->next) {
    rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info_dj->jdd.clist), (gint)sel->data);
    frame = rdata->frame;
    drqm_request_job_frame_waiting (info_dj->jdd.job.id,frame);
  }
}

static void jdd_delete_blocked_host (GtkWidget *w, struct drqm_jobs_info *info)
{
	GList *sel;
	uint32_t ipos;	// Position on the blocked host list

	if (!(sel = GTK_CLIST(info->jdd.clist_bh)->selection)) {
		return;
	}
  
	for (;sel;sel = sel->next) {
		ipos = (uint32_t) gtk_clist_get_row_data (GTK_CLIST(info->jdd.clist_bh), (gint)sel->data);
    request_job_delete_blocked_host (info->jdd.job.id,ipos,CLIENT);
  }
}

static void jdd_kill_frames_confirm (GtkWidget *button, struct drqm_jobs_info *info_dj)
{
  GtkWidget *dialog;
  GList *cbs = NULL ;		/* callbacks */
  GList *sel;

  if (!(sel = GTK_CLIST(info_dj->jdd.clist)->selection)) {
    return;
  }
  
  cbs = g_list_append (cbs,jdd_kill_frames);
	cbs = g_list_append (cbs,info_dj);

  dialog = ConfirmDialog ("Do you really want to kill and requeue the running frames?\n"
			  "(You will lose the partially rendered images)",
			  cbs);

  g_list_free (cbs);

  gtk_grab_add(dialog);
}


static void jdd_kill_frames (GtkWidget *button,struct drqm_jobs_info *info)
{
  /* Signals the running frames and queues them again */
  GList *sel;
  uint32_t frame;
  struct row_data *rdata;

  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }

  for (;sel;sel = sel->next) {
    rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info->jdd.clist), (gint)sel->data);
    frame = rdata->frame;
    drqm_request_job_frame_kill (info->jdd.job.id,frame);
  }
}

static void jdd_finish_frames (GtkWidget *button,struct drqm_jobs_info *info)
{
  /* Sets the waiting frames as finished */
  GList *sel;
  uint32_t frame;
  struct row_data *rdata;

  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }

  for (;sel;sel = sel->next) {
    rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info->jdd.clist), (gint)sel->data);
    frame = rdata->frame;
    drqm_request_job_frame_finish (info->jdd.job.id,frame);
  }
}

static void jdd_frames_reset_requeued (GtkWidget *button,struct drqm_jobs_info *info)
{
  /* Sets the waiting frames as finished */
  GList *sel;
  uint32_t frame;
  struct row_data *rdata;

  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }

  for (;sel;sel = sel->next) {
    rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info->jdd.clist), (gint)sel->data);
    frame = rdata->frame;
    drqm_request_job_frame_reset_requeued (info->jdd.job.id,frame);
  }
}

static void jdd_kill_finish_frames_confirm (GtkWidget *button, struct drqm_jobs_info *info)
{
  GtkWidget *dialog;
  GList *cbs = NULL ;		/* callbacks */
  GList *sel;

  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }
  
  cbs = g_list_append (cbs,jdd_kill_finish_frames);
  cbs = g_list_append (cbs,info);

  dialog = ConfirmDialog ("Do you really want to kill and set as finished the running frames?\n"
			  "(Running frames won't be really finished even when they are marked as so)",
			  cbs);

  g_list_free (cbs);

  gtk_grab_add(dialog);
}

static void jdd_kill_finish_frames (GtkWidget *button,struct drqm_jobs_info *info)
{
  /* Sets the waiting frames as finished */
  GList *sel;
  uint32_t frame;
  struct row_data *rdata;

  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }

  for (;sel;sel = sel->next) {
    rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info->jdd.clist), (gint)sel->data);
    frame = rdata->frame;
    drqm_request_job_frame_kill_finish (info->jdd.job.id,frame);
  }
}

static gint PopupMenuBlockedHosts (GtkWidget *clist, GdkEvent *event, struct drqm_jobs_info *info)
{
	if (event->type == GDK_BUTTON_PRESS) {
		GdkEventButton *bevent = (GdkEventButton *) event;
		if (bevent->button != 3) {
			return FALSE;
		}
    info->jdd.selected = gtk_clist_get_selection_info(GTK_CLIST(info->jdd.clist),
						      (int)bevent->x,(int)bevent->y,
						      &info->jdd.row,&info->jdd.column);
		if (info->selected) {
			gtk_menu_popup (GTK_MENU(info->jdd.menu_bh), NULL, NULL, NULL, NULL,
											bevent->button, bevent->time);
			return TRUE;
		}
	}

	return FALSE;
}

static void jdd_maya_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* Sets the waiting frames as finished */
  GList *sel;
  uint32_t frame,iframe;
  const char *new_argv[4];
  extern char **environ;
  struct row_data *rdata;
  
  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }

  rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info->jdd.clist), (gint)sel->data);
  frame = rdata->frame;
  
  iframe = job_frame_number_to_index (&info->jdd.job,frame);

  if (fork() == 0) {
    new_argv[0] = SHELL_NAME;
    new_argv[1] = "-c";
    new_argv[2] = info->jdd.job.koji.maya.viewcmd;
    new_argv[3] = NULL;
    
    job_environment_set(&info->jdd.job,iframe);
    
    execve(SHELL_PATH,(char*const*)new_argv,environ);
    perror("execve");
    exit (1);
  }
}

static void jdd_mentalray_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* Sets the waiting frames as finished */
  GList *sel;
  uint32_t frame,iframe;
  const char *new_argv[4];
  extern char **environ;
  struct row_data *rdata;
  
  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }

  rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info->jdd.clist), (gint)sel->data);
  frame = rdata->frame;
  
  iframe = job_frame_number_to_index (&info->jdd.job,frame);

  if (fork() == 0) {
    new_argv[0] = SHELL_NAME;
    new_argv[1] = "-c";
    new_argv[2] = info->jdd.job.koji.mentalray.viewcmd;
    new_argv[3] = NULL;
    
    job_environment_set(&info->jdd.job,iframe);
    
    execve(SHELL_PATH,(char*const*)new_argv,environ);
    perror("execve");
    exit (1);
  }
}

static void jdd_blender_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* Sets the waiting frames as finished */
  GList *sel;
  uint32_t frame,iframe;
  const char *new_argv[4];
  extern char **environ;
  struct row_data *rdata;
  
  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }

  rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info->jdd.clist), (gint)sel->data);
  frame = rdata->frame;
  
  iframe = job_frame_number_to_index (&info->jdd.job,frame);

  if (fork() == 0) {
    new_argv[0] = SHELL_NAME;
    new_argv[1] = "-c";
    new_argv[2] = info->jdd.job.koji.blender.viewcmd;
    new_argv[3] = NULL;
    
    job_environment_set(&info->jdd.job,iframe);
    
    execve(SHELL_PATH,(char*const*)new_argv,environ);
    perror("execve");
    exit (1);
  }
}

static void jdd_bmrt_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* Sets the waiting frames as finished */
  GList *sel;
  uint32_t frame,iframe;
  const char *new_argv[4];
  extern char **environ;
  struct row_data *rdata;
  
  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }

  rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info->jdd.clist), (gint)sel->data);
  frame = rdata->frame;
  
  iframe = job_frame_number_to_index (&info->jdd.job,frame);

  if (fork() == 0) {
    new_argv[0] = SHELL_NAME;
    new_argv[1] = "-c";
    new_argv[2] = info->jdd.job.koji.bmrt.viewcmd;
    new_argv[3] = NULL;
    
    job_environment_set(&info->jdd.job,iframe);
    
    execve(SHELL_PATH,(char*const*)new_argv,environ);
    perror("execve");
    exit (1);
  }
}

static void jdd_pixie_viewcmd_exec (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* Sets the waiting frames as finished */
  GList *sel;
  uint32_t frame,iframe;
  const char *new_argv[4];
  extern char **environ;
  struct row_data *rdata;
  
  if (!(sel = GTK_CLIST(info->jdd.clist)->selection)) {
    return;
  }

  rdata = (struct row_data *) gtk_clist_get_row_data(GTK_CLIST(info->jdd.clist), (gint)sel->data);
  frame = rdata->frame;
  
  iframe = job_frame_number_to_index (&info->jdd.job,frame);

  if (fork() == 0) {
    new_argv[0] = SHELL_NAME;
    new_argv[1] = "-c";
    new_argv[2] = info->jdd.job.koji.pixie.viewcmd;
    new_argv[3] = NULL;
    
    job_environment_set(&info->jdd.job,iframe);
    
    execve(SHELL_PATH,(char*const*)new_argv,environ);
    perror("execve");
    exit (1);
  }
}

static GtkWidget *jdd_limits_widgets (struct drqm_jobs_info *info)
{
  GtkWidget *frame,*frame2;
  GtkWidget *vbox, *hbox, *hbox2;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *cbutton;

  frame = gtk_frame_new ("Limits");
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,TRUE,TRUE,2);
  label = gtk_label_new ("Maximum number of cpus:");
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,2);
  label = gtk_label_new ("-1");
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  info->jdd.limits.lnmaxcpus = label;
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(jdd_limits_nmaxcpus_bcp),info);

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Maximum number of cpus on one computer:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  hbox2 = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,2);
  label = gtk_label_new ("-1");
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  info->jdd.limits.lnmaxcpuscomputer = label;
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(jdd_limits_nmaxcpuscomputer_bcp),info);


	// Memory
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Minimum ammount of memory:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  hbox2 = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,2);
  label = gtk_label_new ("0");
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  info->jdd.limits.lmemory = label;
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(jdd_limits_lmemory_bcp),info);

	// Pool
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Pool:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  hbox2 = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,2);
  label = gtk_label_new (DEFAULT_POOL);
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  info->jdd.limits.lpool = label;
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(jdd_limits_lpool_bcp),info);

	// OS stuff
  frame2 = gtk_frame_new ("Operating Systems");
  gtk_box_pack_start (GTK_BOX(vbox),frame2,FALSE,FALSE,2);
  hbox = gtk_hbox_new (TRUE,2);
  gtk_container_add (GTK_CONTAINER(frame2),hbox);

  cbutton = gtk_check_button_new_with_label ("Irix");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  info->jdd.limits.cb_irix = cbutton;
  gtk_widget_set_sensitive (GTK_WIDGET(cbutton),FALSE);

  cbutton = gtk_check_button_new_with_label ("Linux");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  info->jdd.limits.cb_linux = cbutton;
  gtk_widget_set_sensitive (GTK_WIDGET(cbutton),FALSE);

  cbutton = gtk_check_button_new_with_label ("OS X");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  info->jdd.limits.cb_osx = cbutton;
  gtk_widget_set_sensitive (GTK_WIDGET(cbutton),FALSE);

  cbutton = gtk_check_button_new_with_label ("FreeBSD");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,TRUE,TRUE,2);
  info->jdd.limits.cb_freebsd = cbutton;
  gtk_widget_set_sensitive (GTK_WIDGET(cbutton),FALSE);

  return (frame);
}

static void jdd_limits_lmemory_bcp (GtkWidget *bclicked, struct drqm_jobs_info *info)
{
	GtkWidget *dialog;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *button;
	char buf[BUFFERLEN];

	dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW(dialog),"Change minimum amount of memory");

	// The stuff
	hbox = gtk_hbox_new (TRUE,2);
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),hbox);
	label = gtk_label_new ("New minumum amount of memory : ");
	gtk_box_pack_start (GTK_BOX(hbox),label, TRUE, TRUE, 2);
	entry = gtk_entry_new_with_max_length (BUFFERLEN);
	gtk_box_pack_start (GTK_BOX(hbox),entry, TRUE, TRUE, 2);
	info->jdd.limits.ememory = entry;
	snprintf (buf,BUFFERLEN,"%u",info->jdd.job.limits.memory);
	gtk_entry_set_text(GTK_ENTRY(entry),buf);
	
	// The buttons
	button = gtk_button_new_with_label ("Ok");
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->action_area), button, TRUE, TRUE, 2);
	GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(jdd_limits_lmemory_bcp_bokp),info);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(jdd_update),info);
	g_signal_connect_swapped (G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),dialog);

	button = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->action_area), button, TRUE, TRUE, 2);
	g_signal_connect_swapped (G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),dialog);

	gtk_widget_show_all (dialog);

	gtk_grab_add (dialog);
}

static void jdd_limits_lpool_bcp (GtkWidget *bclicked, struct drqm_jobs_info *info)
{
	GtkWidget *dialog;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *button;
	char buf[BUFFERLEN];

	dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW(dialog),"Change job pool");

	// The stuff
	hbox = gtk_hbox_new (TRUE,2);
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),hbox);
	label = gtk_label_new ("New pool : ");
	gtk_box_pack_start (GTK_BOX(hbox),label, TRUE, TRUE, 2);
	entry = gtk_entry_new_with_max_length (MAXNAMELEN);
	gtk_box_pack_start (GTK_BOX(hbox),entry, TRUE, TRUE, 2);
	info->jdd.limits.epool = entry;
	snprintf (buf,BUFFERLEN,"%s",info->jdd.job.limits.pool);
	gtk_entry_set_text(GTK_ENTRY(entry),buf);
	
	// The buttons
	button = gtk_button_new_with_label ("Ok");
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->action_area), button, TRUE, TRUE, 2);
	GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(jdd_limits_lpool_bcp_bokp),info);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(jdd_update),info);
	g_signal_connect_swapped (G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),dialog);

	button = gtk_button_new_with_label ("Cancel");
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->action_area), button, TRUE, TRUE, 2);
	g_signal_connect_swapped (G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),dialog);

	gtk_widget_show_all (dialog);

	gtk_grab_add (dialog);
}

static void jdd_limits_lpool_bcp_bokp (GtkWidget *bclicked, struct drqm_jobs_info *info)
{
	request_job_limits_pool_set(info->jdd.job.id,(char*)gtk_entry_get_text(GTK_ENTRY(info->jdd.limits.epool)),CLIENT);
}

static void jdd_limits_lmemory_bcp_bokp (GtkWidget *bclicked, struct drqm_jobs_info *info)
{
	uint32_t memory;
	
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->jdd.limits.ememory)),"%u",&memory) != 1)
    return;
	request_job_limits_memory_set(info->jdd.job.id,memory,CLIENT);
}

static void jdd_sesframes_bcp (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* Start, End, Step frames, button change pressed */
  GtkWidget *dialog;

  dialog = jdd_sesframes_change_dialog (info);
  if (dialog)
    gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

static void jdd_add_blocked_host_bp (GtkWidget *button, struct drqm_jobs_info *info)
{
	GtkWidget *dialog;

	dialog = jdd_add_blocked_host_dialog (info);
	if (dialog) {
		gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
	}
}

static GtkWidget *jdd_sesframes_change_dialog (struct drqm_jobs_info *info)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
  char msg[BUFFERLEN];

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"Change start, end, step frames");
  gtk_window_set_policy(GTK_WINDOW(window),FALSE,FALSE,TRUE);
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add(GTK_CONTAINER(window),vbox);

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Start:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  snprintf(msg,BUFFERLEN-1,"%i",info->jdd.job.frame_start);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,2);
  info->jdd.sesframes.eframe_start = entry;

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("End:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  snprintf(msg,BUFFERLEN-1,"%i",info->jdd.job.frame_end);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,2);
  info->jdd.sesframes.eframe_end = entry;

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Step:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  snprintf(msg,BUFFERLEN-1,"%i",info->jdd.job.frame_step);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,2);
  info->jdd.sesframes.eframe_step = entry;

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Block Size:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  snprintf(msg,BUFFERLEN-1,"%i",info->jdd.job.block_size);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,2);
  info->jdd.sesframes.eblock_size = entry;
  
	hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  button = gtk_button_new_with_label ("Submit");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_sesframes_cd_bsumbit_pressed),info);
/*    gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(cdd_update),info); */
  gtk_signal_connect_object (GTK_OBJECT(button),"clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     (gpointer) window);

  button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect_object (GTK_OBJECT(button),"clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     (gpointer) window);

  gtk_widget_show_all(window);

  return window;
}

static void jdd_sesframes_cd_bsumbit_pressed (GtkWidget *button, struct drqm_jobs_info *info)
{
  uint32_t frame_start,frame_end,frame_step,block_size;

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->jdd.sesframes.eframe_start)),"%u",&frame_start) != 1)
    return;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->jdd.sesframes.eframe_end)),"%u",&frame_end) != 1)
    return;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->jdd.sesframes.eframe_step)),"%u",&frame_step) != 1)
    return;
  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->jdd.sesframes.eblock_size)),"%u",&block_size) != 1)
    return;

  drqm_request_job_sesupdate (info->jdd.job.id,frame_start,frame_end,frame_step,block_size);

  return;
}

void jdd_limits_nmaxcpus_bcp (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* Job Details Dialog Limits nmaxcpus Button Change Pressed */
  GtkWidget *dialog;

  dialog = jdd_nmc_dialog (info);
  if (dialog)
    gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

GtkWidget *jdd_nmc_dialog (struct drqm_jobs_info *info)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
  char msg[BUFFERLEN];

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"New maximum number of cpus");
  gtk_window_set_policy(GTK_WINDOW(window),FALSE,FALSE,TRUE);
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add(GTK_CONTAINER(window),vbox);

  hbox = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("New maximum number of cpus:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  info->jdd.limits.enmaxcpus = entry;
  snprintf(msg,BUFFERLEN-1,"%hi",info->jdd.job.limits.nmaxcpus);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,2);

  hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  button = gtk_button_new_with_label ("Submit");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_nmcd_bsumbit_pressed),info);
/*    gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(cdd_update),info); */
  gtk_signal_connect_object (GTK_OBJECT(button),"clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     (gpointer) window);

  button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect_object (GTK_OBJECT(button),"clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     (gpointer) window);

  gtk_widget_show_all(window);

  return window;
}

void jdd_nmcd_bsumbit_pressed (GtkWidget *button, struct drqm_jobs_info *info)
{
  uint32_t nmaxcpus;
  char msg[BUFFERLEN];

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->jdd.limits.enmaxcpus)),"%u",&nmaxcpus) != 1)
    return;			/* Error in the entry */

  drqm_request_job_limits_nmaxcpus_set(info->jdd.job.id,nmaxcpus);

  info->jdd.job.limits.nmaxcpus = (uint16_t) nmaxcpus;

  snprintf(msg,BUFFERLEN-1,"%u",
	   info->jdd.job.limits.nmaxcpus);
  gtk_label_set_text (GTK_LABEL(info->jdd.limits.lnmaxcpus),msg);
}

void jdd_limits_nmaxcpuscomputer_bcp (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* Job Details Dialog Limits nmaxcpus Button Change Pressed */
  GtkWidget *dialog;

  dialog = jdd_nmcc_dialog (info);
  if (dialog)
    gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

GtkWidget *jdd_nmcc_dialog (struct drqm_jobs_info *info)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
  char msg[BUFFERLEN];

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"New maximum number of cpus in a single computer");
  gtk_window_set_policy(GTK_WINDOW(window),FALSE,FALSE,TRUE);
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add(GTK_CONTAINER(window),vbox);

  hbox = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("New maximum number of cpus on one computer:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  info->jdd.limits.enmaxcpuscomputer = entry;
  snprintf(msg,BUFFERLEN-1,"%hi",info->jdd.job.limits.nmaxcpuscomputer);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,2);

  hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  button = gtk_button_new_with_label ("Submit");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_nmccd_bsumbit_pressed),info);
  gtk_signal_connect_object (GTK_OBJECT(button),"clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     (gpointer) window);

  button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect_object (GTK_OBJECT(button),"clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     (gpointer) window);

  gtk_widget_show_all(window);

  return window;
}

void jdd_nmccd_bsumbit_pressed (GtkWidget *button, struct drqm_jobs_info *info)
{
  uint32_t nmaxcpuscomputer;
  char msg[BUFFERLEN];

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->jdd.limits.enmaxcpuscomputer)),"%u",&nmaxcpuscomputer) != 1)
    return;			/* Error in the entry */

  drqm_request_job_limits_nmaxcpuscomputer_set(info->jdd.job.id,nmaxcpuscomputer);

  info->jdd.job.limits.nmaxcpuscomputer = (uint16_t) nmaxcpuscomputer;

  snprintf(msg,BUFFERLEN-1,"%u",
	   info->jdd.job.limits.nmaxcpuscomputer);
  gtk_label_set_text (GTK_LABEL(info->jdd.limits.lnmaxcpuscomputer),msg);
}

static void jdd_priority_bcp (GtkWidget *button, struct drqm_jobs_info *info)
{
  /* Priority button change pressed */
  GtkWidget *dialog;

  dialog = jdd_priority_change_dialog (info);
  if (dialog)
    gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

GtkWidget *jdd_priority_change_dialog (struct drqm_jobs_info *info)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox,*hbox2;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
  GtkWidget *combo;
  GList *items = NULL;
  char msg[BUFFERLEN];

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"New priority");
  gtk_window_set_policy(GTK_WINDOW(window),FALSE,FALSE,TRUE);
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add(GTK_CONTAINER(window),vbox);

  hbox = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("New priority:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
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
  info->jdd.cpri = combo;
  entry = gtk_entry_new_with_max_length (MAXCMDLEN-1);
  info->jdd.epri = entry;
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  gtk_widget_show(entry);
  g_signal_connect (G_OBJECT(GTK_ENTRY(GTK_COMBO(combo)->entry)),
										"changed",G_CALLBACK(jdd_pcd_cpri_changed),&info->jdd);
  gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(combo)->entry),"Custom");
  snprintf(msg,BUFFERLEN-1,"%i",info->jdd.job.priority);
  gtk_entry_set_text (GTK_ENTRY(entry),msg);

  hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  button = gtk_button_new_with_label ("Submit");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(jdd_pcd_bsumbit_pressed),info);
  gtk_signal_connect_object (GTK_OBJECT(button),"clicked",
														 GTK_SIGNAL_FUNC(gtk_widget_destroy),
														 (gpointer) window);

  button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect_object (GTK_OBJECT(button),"clicked",
														 GTK_SIGNAL_FUNC(gtk_widget_destroy),
														 (gpointer) window);

  gtk_widget_show_all(window);

  return window;
}

static void jdd_pcd_cpri_changed (GtkWidget *entry, struct drqmj_jddi *info)
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
  } else if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry)),"Custom") == 0) {
    gtk_entry_set_editable (GTK_ENTRY(info->epri),TRUE);
    gtk_entry_set_text (GTK_ENTRY(info->epri),"500");
  } else {
    fprintf (stderr,"jdd_pcd_cpri_changed: Not listed!\n");
  }
}

static void jdd_pcd_bsumbit_pressed (GtkWidget *button, struct drqm_jobs_info *info)
{
  uint32_t priority;
  char msg[BUFFERLEN];

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->jdd.epri)),"%u",&priority) != 1)
    return;			/* Error in the entry */

  drqm_request_job_priority_update(info->jdd.job.id,priority);

  info->jdd.job.priority = priority;

  snprintf(msg,BUFFERLEN-1,"%u", info->jdd.job.priority);
  gtk_label_set_text (GTK_LABEL(info->jdd.lpri),msg);
}

GtkWidget *jdd_flags_widgets (struct drqm_jobs_info *info)
{
  GtkWidget *frame;
  GtkWidget *vbox, *hbox;
  GtkWidget *label;
  char msg[BUFFERLEN];

  frame = gtk_frame_new ("Flags");
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,TRUE,FALSE,2);

  if (info->jdd.job.flags & JF_MAILNOTIFY) {
    snprintf (msg,BUFFERLEN-1,"Mail notifications: ON going to email: %s",info->jdd.job.email);
    label = gtk_label_new (msg);
  } else {
    label = gtk_label_new ("Mail notifications: OFF");
  }
/*    GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(label),GTK_CAN_FOCUS); */
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);

	// Job dependencies
	if (info->jdd.job.flags & JF_JOBDEPEND) {
		hbox = gtk_hbox_new (TRUE,2);
		gtk_box_pack_start (GTK_BOX(vbox),hbox,TRUE,FALSE,2);
		snprintf (msg,BUFFERLEN,"Job depends on %i",info->jdd.job.dependid);
		label = gtk_label_new (msg);
		gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
	}

  return (frame);
}

GtkWidget *jdd_koj_widgets (struct drqm_jobs_info *info)
{
  GtkWidget *frame;
  GtkWidget *vbox, *hbox;
  GtkWidget *label;
  GtkWidget *koj_vbox = NULL;
  

  frame = gtk_frame_new ("Kind of job");
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,TRUE,FALSE,2);

  label = gtk_label_new ("Kind of job:");
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  label = gtk_label_new (job_koj_string(&info->jdd.job));
  gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);

  switch (info->jdd.job.koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
    koj_vbox = jdd_koj_maya_widgets (info);
    gtk_box_pack_start (GTK_BOX(vbox),koj_vbox,FALSE,FALSE,2);
    break;
  case KOJ_MENTALRAY:
    koj_vbox = jdd_koj_mentalray_widgets (info);
    gtk_box_pack_start (GTK_BOX(vbox),koj_vbox,FALSE,FALSE,2);
    break;
  case KOJ_BLENDER:
    koj_vbox = jdd_koj_blender_widgets (info);
    gtk_box_pack_start (GTK_BOX(vbox),koj_vbox,FALSE,FALSE,2);
    break;
  case KOJ_BMRT:
    koj_vbox = jdd_koj_bmrt_widgets (info);
    gtk_box_pack_start (GTK_BOX(vbox),koj_vbox,FALSE,FALSE,2);
    break;
  case KOJ_PIXIE:
    koj_vbox = jdd_koj_pixie_widgets (info);
    gtk_box_pack_start (GTK_BOX(vbox),koj_vbox,FALSE,FALSE,2);
    break;
  }

  return frame;
}

void jdd_table_pack (GtkWidget *table, GtkWidget *label1, GtkWidget *label2, GtkWidget *button, int row)
{
  GtkAttachOptions options = GTK_EXPAND | GTK_SHRINK | GTK_FILL ;

  gtk_misc_set_alignment (GTK_MISC(label1), 0, .5);
  gtk_table_attach (GTK_TABLE(table),GTK_WIDGET(label1), 0, 1, row, row+1, options, options, 1 , 1);
  gtk_label_set_justify (GTK_LABEL(label1),GTK_JUSTIFY_CENTER);
  
  gtk_label_set_line_wrap (GTK_LABEL(label2), TRUE);
  gtk_misc_set_alignment (GTK_MISC(label2), 0 , .5);
  gtk_label_set_justify (GTK_LABEL(label2),GTK_JUSTIFY_CENTER);
  gtk_table_attach (GTK_TABLE(table),GTK_WIDGET(label2), 1, 2, row, row+1, options, options, 1 , 1);

  if (button) {
    gtk_table_attach (GTK_TABLE(table),GTK_WIDGET(button), 2, 3, row, row+1, 0, 0, 1 , 1);
  }
}

void jdd_framelist_column_clicked (GtkCList *clist, gint column, struct drqm_jobs_info *info)
{
  static GtkSortType dir = GTK_SORT_ASCENDING;

  if (dir == GTK_SORT_DESCENDING) {
    dir = GTK_SORT_ASCENDING;
  } else {
    dir = GTK_SORT_DESCENDING;
  }

  /* ATTENTION this column numbers must be changed if the column order changes */
  if (column == 0) {
    gtk_clist_set_sort_type (GTK_CLIST(clist),dir);
    gtk_clist_set_compare_func (GTK_CLIST(clist),jdd_framelist_cmp_frame);
    gtk_clist_sort (GTK_CLIST(clist));
	} else if (column == 1) {
    gtk_clist_set_sort_type (GTK_CLIST(clist),dir);
    gtk_clist_set_compare_func (GTK_CLIST(clist),jdd_framelist_cmp_status);
    gtk_clist_sort (GTK_CLIST(clist));
  } else if (column == 2) {
    gtk_clist_set_sort_type (GTK_CLIST(clist),dir);
    gtk_clist_set_compare_func (GTK_CLIST(clist),jdd_framelist_cmp_requeued);
    gtk_clist_sort (GTK_CLIST(clist));
  } else if (column == 3) {
    gtk_clist_set_sort_type (GTK_CLIST(clist),dir);
    gtk_clist_set_compare_func (GTK_CLIST(clist),jdd_framelist_cmp_start_time);
    gtk_clist_sort (GTK_CLIST(clist));
  } else if (column == 4) {
    gtk_clist_set_sort_type (GTK_CLIST(clist),dir);
    gtk_clist_set_compare_func (GTK_CLIST(clist),jdd_framelist_cmp_end_time);
    gtk_clist_sort (GTK_CLIST(clist));
  } else if (column == 5) {
    gtk_clist_set_sort_type (GTK_CLIST(clist),dir);
    gtk_clist_set_compare_func (GTK_CLIST(clist),jdd_framelist_cmp_exitcode);
    gtk_clist_sort (GTK_CLIST(clist));
  } else if (column == 6) {
    gtk_clist_set_sort_type (GTK_CLIST(clist),dir);
    gtk_clist_set_compare_func (GTK_CLIST(clist),jdd_framelist_cmp_icomp);
    gtk_clist_sort (GTK_CLIST(clist));
  }
}

int jdd_framelist_cmp_requeued (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
	struct row_data *ra,*rb;
	char a,b;
	uint32_t ifa,ifb;

	ra = (struct row_data *) ((GtkCListRow*)ptr1)->data;
	rb = (struct row_data *) ((GtkCListRow*)ptr2)->data;

	ifa = job_frame_number_to_index (&ra->info->jdd.job,ra->frame);
	ifb = job_frame_number_to_index (&rb->info->jdd.job,rb->frame);

	a = ra->info->jdd.job.frame_info[ifa].requeued;
	b = rb->info->jdd.job.frame_info[ifb].requeued;

	if (a > b) {
		return 1;
	} else if (a == b) {
		return 0;
	} else {
		return -1;
	}

	return 0;
}

int jdd_framelist_cmp_frame (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  struct row_data *ra,*rb;
  uint32_t a,b;

  ra = (struct row_data *) ((GtkCListRow*)ptr1)->data;
  rb = (struct row_data *) ((GtkCListRow*)ptr2)->data;

  a = ra->frame;
  b = rb->frame;
				  
  if (a > b) {
    return 1;
  } else if (a == b) {
    return 0;
  } else {
    return -1;
  }

  return 0;
}

int jdd_framelist_cmp_exitcode (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  struct row_data *ra,*rb;
  char a,b;
  uint32_t ifa,ifb;

  ra = (struct row_data *) ((GtkCListRow*)ptr1)->data;
  rb = (struct row_data *) ((GtkCListRow*)ptr2)->data;

  ifa = job_frame_number_to_index (&ra->info->jdd.job,ra->frame);
  ifb = job_frame_number_to_index (&rb->info->jdd.job,rb->frame);

  a = ra->info->jdd.job.frame_info[ifa].exitcode;
  b = rb->info->jdd.job.frame_info[ifb].exitcode;
			
  if (a > b) {
    return 1;
  } else if (a == b) {
    return 0;
  } else {
    return -1;
  }

  return 0;
}

int jdd_framelist_cmp_status (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  struct row_data *ra,*rb;
  char a,b;
  uint16_t ifa,ifb;

  ra = (struct row_data *) ((GtkCListRow*)ptr1)->data;
  rb = (struct row_data *) ((GtkCListRow*)ptr2)->data;

  ifa = job_frame_number_to_index (&ra->info->jdd.job,ra->frame);
  ifb = job_frame_number_to_index (&rb->info->jdd.job,rb->frame);

  a = ra->info->jdd.job.frame_info[ifa].status;
  b = rb->info->jdd.job.frame_info[ifb].status;
			
  if (a > b) {
    return 1;
  } else if (a == b) {
    return 0;
  } else {
    return -1;
  }

  return 0;
}

int jdd_framelist_cmp_icomp (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  struct row_data *ra,*rb;
  uint32_t a,b;
  uint16_t ifa,ifb;

  ra = (struct row_data *) ((GtkCListRow*)ptr1)->data;
  rb = (struct row_data *) ((GtkCListRow*)ptr2)->data;

  ifa = job_frame_number_to_index (&ra->info->jdd.job,ra->frame);
  ifb = job_frame_number_to_index (&rb->info->jdd.job,rb->frame);

  a = ra->info->jdd.job.frame_info[ifa].icomp;
  b = rb->info->jdd.job.frame_info[ifb].icomp;
			
  if (a > b) {
    return 1;
  } else if (a == b) {
    return 0;
  } else {
    return -1;
  }

  return 0;
}

int jdd_framelist_cmp_start_time (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  struct row_data *ra,*rb;
  time_t a,b;
  uint16_t ifa,ifb;

  ra = (struct row_data *) ((GtkCListRow*)ptr1)->data;
  rb = (struct row_data *) ((GtkCListRow*)ptr2)->data;

  ifa = job_frame_number_to_index (&ra->info->jdd.job,ra->frame);
  ifb = job_frame_number_to_index (&rb->info->jdd.job,rb->frame);

  a = ra->info->jdd.job.frame_info[ifa].start_time;
  b = rb->info->jdd.job.frame_info[ifb].start_time;
			
  if (a > b) {
    return 1;
  } else if (a == b) {
    return 0;
  } else {
    return -1;
  }

  return 0;
}

int jdd_framelist_cmp_end_time (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  struct row_data *ra,*rb;
  time_t a,b;
  uint16_t ifa,ifb;

  ra = (struct row_data *) ((GtkCListRow*)ptr1)->data;
  rb = (struct row_data *) ((GtkCListRow*)ptr2)->data;

  ifa = job_frame_number_to_index (&ra->info->jdd.job,ra->frame);
  ifb = job_frame_number_to_index (&rb->info->jdd.job,rb->frame);

  a = ra->info->jdd.job.frame_info[ifa].end_time;
  b = rb->info->jdd.job.frame_info[ifb].end_time;
			
  if (a > b) {
    return 1;
  } else if (a == b) {
    return 0;
  } else {
    return -1;
  }

  return 0;
}
