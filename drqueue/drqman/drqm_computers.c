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

#include "drqman.h"
#include "drqm_request.h"
#include "drqm_computers.h"
#include "drqm_common.h"

#include "slave_icon.h"

/* Static functions declaration */
static GtkWidget *CreateComputersList(struct drqm_computers_info *info);
static GtkWidget *CreateClist (GtkWidget *window);
static GtkWidget *CreateButtonRefresh (struct drqm_computers_info *info);
static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct drqm_computers_info *info);
static GtkWidget *CreateMenu (struct drqm_computers_info *info);


/* COMPUTER DETAILS */
static void ComputerDetails(GtkWidget *menu_item, struct drqm_computers_info *info);
static GtkWidget *ComputerDetailsDialog (struct drqm_computers_info *info);
static int cdd_update (GtkWidget *w, struct drqm_computers_info *info);
static GtkWidget *CreateTasksClist (void);
static GtkWidget *CreateMenuTasks (struct drqm_computers_info *info);
static gint PopupMenuTasks (GtkWidget *clist, GdkEvent *event, struct drqm_computers_info *info);
/* nmaxcpus */
static void cdd_limits_nmaxcpus_bcp (GtkWidget *button, struct drqm_computers_info *info);
static GtkWidget *nmc_dialog (struct drqm_computers_info *info);
static void nmcd_bsumbit_pressed (GtkWidget *button, struct drqm_computers_info *info);
/* maxfreeloadcpu */
static void cdd_limits_maxfreeloadcpu_bcp (GtkWidget *button, struct drqm_computers_info *info);
static GtkWidget *mflc_dialog (struct drqm_computers_info *info);
static void mflcd_bsumbit_pressed (GtkWidget *button, struct drqm_computers_info *info);
/* autoenable */
static void cdd_limits_autoenable_bcp (GtkWidget *button, struct drqm_computers_info *info);
static GtkWidget *autoenable_change_dialog (struct drqm_computers_info *info);
static void aecd_bsumbit_pressed (GtkWidget *button, struct drqm_computers_info *info);
// pool
static void cdd_limits_pool_bcp (GtkWidget *bclicked, struct drqm_computers_info *info);
static void cdd_limits_pool_refresh_pool_list (GtkWidget *bclicked, struct drqm_computers_info *info);
static void cdd_limits_pool_add_clicked (GtkWidget *bclicked, struct drqm_computers_info *info);
static void cdd_limits_pool_remove_clicked (GtkWidget *bclicked, struct drqm_computers_info *info);
/* kill task */
static void KillTask (GtkWidget *menu_item, struct drqm_computers_info *info);
static void dtk_bok_pressed (GtkWidget *button,struct drqm_computers_info *info);


void CreateComputersPage (GtkWidget *notebook,struct info_drqm *info)
{
  GtkWidget *label;
  GtkWidget *container;
  GtkWidget *clist;
  GtkWidget *buttonRefresh;	/* Button to refresh the computer list */
  GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *image;
	GdkPixbuf *slave_icon_pb;

  /* Label */
  label = gtk_label_new ("Computers");
  container = gtk_frame_new ("Computer status");
  gtk_container_border_width (GTK_CONTAINER(container),2);
  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(container),vbox);

	// Image
	slave_icon_pb = gdk_pixbuf_new_from_inline (2028,slave_icon,0,NULL);
	image = gtk_image_new_from_pixbuf (slave_icon_pb);
	gtk_widget_show(image);
	hbox = gtk_hbox_new (FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),image,TRUE,TRUE,2);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,2);

  /* Clist */
  clist = CreateComputersList (&info->idc);
  gtk_box_pack_start(GTK_BOX(vbox),clist,TRUE,TRUE,2);
  
  /* Button refresh */
  buttonRefresh = CreateButtonRefresh (&info->idc);
  gtk_box_pack_end(GTK_BOX(vbox),buttonRefresh,FALSE,FALSE,2);

  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, hbox);

  /* Put the computers on the list */
  drqm_request_computerlist (&info->idc);
  drqm_update_computerlist (&info->idc);

  gtk_widget_show(clist);
  gtk_widget_show(vbox);
  gtk_widget_show(label);
  gtk_widget_show(container);
}

static GtkWidget *CreateComputersList(struct drqm_computers_info *info)
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
  gchar *titles[] = { "ID","Running","Name","OS","CPUs","Load Avg" };
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (6, titles);
  gtk_container_add(GTK_CONTAINER(window),clist);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_column_titles_passive(GTK_CLIST(clist));
  gtk_clist_set_column_width (GTK_CLIST(clist),0,75);
  gtk_clist_set_column_width (GTK_CLIST(clist),1,100);
  gtk_clist_set_column_width (GTK_CLIST(clist),2,100);
  gtk_clist_set_column_width (GTK_CLIST(clist),3,100);
  gtk_clist_set_column_width (GTK_CLIST(clist),4,45);
  gtk_clist_set_column_width (GTK_CLIST(clist),5,100);
  gtk_widget_show(clist);

  return (clist);
}

static GtkWidget *CreateButtonRefresh (struct drqm_computers_info *info)
{
	GtkWidget *b;
  
	b = gtk_button_new_with_label ("Refresh");
	gtk_container_border_width (GTK_CONTAINER(b),5);
	gtk_widget_show (GTK_WIDGET(b));
	g_signal_connect_swapped(G_OBJECT(b),"clicked",G_CALLBACK(drqm_request_computerlist),info);
	g_signal_connect_swapped(G_OBJECT(b),"clicked",G_CALLBACK(drqm_update_computerlist),info);

	return b;
}

void drqm_update_computerlist (struct drqm_computers_info *info)
{
  int i;
  char **buff;
  int ncols = 6;
  
  buff = (char**) g_malloc((ncols+1) * sizeof(char*));
  for (i=0;i<ncols;i++)
    buff[i] = (char*) g_malloc (BUFFERLEN);
  buff[ncols] = NULL;
  
  gtk_clist_freeze(GTK_CLIST(info->clist));
  gtk_clist_clear(GTK_CLIST(info->clist));
  for (i=0; i < info->ncomputers; i++) {
    snprintf (buff[0],BUFFERLEN,"%u",info->computers[i].hwinfo.id);
    snprintf (buff[1],BUFFERLEN,"%i",info->computers[i].status.ntasks);
    strncpy(buff[2],info->computers[i].hwinfo.name,BUFFERLEN);
    snprintf (buff[3],BUFFERLEN,osstring(info->computers[i].hwinfo.os));
    snprintf (buff[4],BUFFERLEN,"%i",info->computers[i].hwinfo.ncpus);
    snprintf (buff[5],BUFFERLEN,"%i,%i,%i",
	      info->computers[i].status.loadavg[0],
	      info->computers[i].status.loadavg[1],
	      info->computers[i].status.loadavg[2]);
    gtk_clist_append(GTK_CLIST(info->clist),buff);
		gtk_clist_set_row_data (GTK_CLIST(info->clist),i,(gpointer)info->computers[i].hwinfo.id);
		
		// We don't need the pool any more
		computer_pool_free(&info->computers[i].limits);
	}
  gtk_clist_thaw(GTK_CLIST(info->clist));


  for(i=0;i<ncols;i++)
    g_free (buff[i]);
	g_free (buff);
}

static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct drqm_computers_info *info)
{
  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button != 3)
      return FALSE;
    info->selected = gtk_clist_get_selection_info(GTK_CLIST(info->clist),
						  (int)bevent->x,(int)bevent->y,
						  &info->row,&info->column);
    gtk_menu_popup (GTK_MENU(info->menu), NULL, NULL, NULL, NULL,
		    bevent->button, bevent->time);
    return TRUE;
  }
  return FALSE;
}

static GtkWidget *CreateMenu (struct drqm_computers_info *info)
{
  GtkWidget *menu;
  GtkWidget *menu_item;

  menu = gtk_menu_new ();
  menu_item = gtk_menu_item_new_with_label("Details");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(ComputerDetails),info);

  gtk_signal_connect(GTK_OBJECT((info->clist)),"event",GTK_SIGNAL_FUNC(PopupMenu),info);

  gtk_widget_show_all(menu);

  return (menu);
}

static void ComputerDetails(GtkWidget *menu_item, struct drqm_computers_info *info)
{
  GtkWidget *dialog;

  if (!info->selected)
    return;

  dialog = ComputerDetailsDialog(info);
  if (dialog)
    gtk_grab_add(dialog);
}

static GtkWidget *ComputerDetailsDialog (struct drqm_computers_info *info)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *vbox,*vbox2;
  GtkWidget *hbox,*hbox2;
  GtkWidget *label;
  GtkWidget *clist;
  GtkWidget *swin;
  GtkWidget *button;
  char *buf;

  if (info->ncomputers) {
    gtk_clist_get_text(GTK_CLIST(info->clist),info->row,0,&buf);
    info->icomp = atoi (buf);	/* This is not needed usually */
  } else {
    return NULL;
  }

  /* Dialog */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"Computer Details");
  gtk_signal_connect_object(GTK_OBJECT(window),"destroy",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)window);
  gtk_window_set_default_size(GTK_WINDOW(window),1000,500);
  gtk_container_set_border_width (GTK_CONTAINER(window),5);
  info->cdd.dialog = window;

  /* Frame */
  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER(window),frame);

  /* Main vbox */
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  /* Label */
  label = gtk_label_new ("Detailed computer information");
  gtk_label_set_pattern (GTK_LABEL(label),"________________________________");
  gtk_box_pack_start (GTK_BOX(vbox),label,FALSE,FALSE,4);

  /* Name of the computer */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Name:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->cdd.lname = label;

  /* OS information */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("OS:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->cdd.los = label;

  /* CPU information */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("CPU Info:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->cdd.lcpuinfo = label;

	// Memory
	hbox = gtk_hbox_new (TRUE,2);
	gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	label = gtk_label_new ("System memory:");
	gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
	label = gtk_label_new (NULL);
	gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
	info->cdd.lmemory = label;

  /* Load average */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Load average:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->cdd.lloadavg = label;

  /* Number of tasks running */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Number of tasks running:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->cdd.lntasks = label;

  /* Limits stuff */
  frame = gtk_frame_new ("Limits information");
  gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,2);
  vbox2 = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox2);
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Maximum number of cpus:");
	gtk_misc_set_alignment (GTK_MISC(label), 0, .5);
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  label = gtk_label_new ("0");
	gtk_misc_set_alignment (GTK_MISC(label), 1, .5);
  info->cdd.limits.lnmaxcpus = label;
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(cdd_limits_nmaxcpus_bcp),info);
  /* Limits maxload */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Maximum load a cpu can have to be considered free:");
	gtk_misc_set_alignment (GTK_MISC(label), 0, .5);
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  label = gtk_label_new ("80");
	gtk_misc_set_alignment (GTK_MISC(label), 1, .5);
  info->cdd.limits.lmaxfreeloadcpu = label;
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(cdd_limits_maxfreeloadcpu_bcp),info);
  /* Limits autoenable */
  hbox = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(vbox2),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Autoenable Time:");
	gtk_misc_set_alignment (GTK_MISC(label), 0, .5);
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  label = gtk_label_new ("21:00");
  info->cdd.limits.lautoenabletime = label;
	gtk_misc_set_alignment (GTK_MISC(label), 1, .5);
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(cdd_limits_autoenable_bcp),info);
	// Limits pools,
  hbox = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(vbox2),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Pools this computer belongs to:");
	gtk_misc_set_alignment (GTK_MISC(label), 0, .5);
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
	hbox2 = gtk_hbox_new (FALSE,0);
	gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  label = gtk_label_new ("Default");
	gtk_misc_set_alignment (GTK_MISC(label), 1, .5);
  info->cdd.limits.lpools = label;
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(cdd_limits_pool_bcp),info);

  /* Clist with the task info */
  /* Frame */
  frame = gtk_frame_new ("Task information");
  gtk_box_pack_start (GTK_BOX(vbox),frame,TRUE,TRUE,2);
  swin = gtk_scrolled_window_new (NULL,NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER(frame),swin);
  clist = CreateTasksClist ();
  gtk_container_add (GTK_CONTAINER(swin),clist);
  info->cdd.clist = clist;

  info->cdd.menu = CreateMenuTasks(info);

  if (!cdd_update (window,info)) {
    gtk_widget_destroy (GTK_WIDGET(window));
    return NULL;
  }

  /* Button Refresh */
  button = gtk_button_new_with_label ("Refresh");
  gtk_container_border_width (GTK_CONTAINER(button),5);
  g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(cdd_update),info);
  gtk_box_pack_start (GTK_BOX(vbox),button,FALSE,FALSE,2);

  gtk_widget_show_all(window);

  return window;
}

GtkWidget *CreateTasksClist (void)
{
  gchar *titles[] = { "ID","Status","Job name","Job id","Owner","Frame","PID","Start","End"};
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (9, titles);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_column_titles_passive(GTK_CLIST(clist));
  gtk_clist_set_column_width (GTK_CLIST(clist),0,40);
  gtk_clist_set_column_width (GTK_CLIST(clist),1,95);
  gtk_clist_set_column_width (GTK_CLIST(clist),2,150);
  gtk_clist_set_column_width (GTK_CLIST(clist),3,75);
  gtk_clist_set_column_width (GTK_CLIST(clist),4,85);
  gtk_clist_set_column_width (GTK_CLIST(clist),5,45);
  gtk_clist_set_column_width (GTK_CLIST(clist),6,45);
  gtk_clist_set_column_width (GTK_CLIST(clist),7,180);
  gtk_clist_set_column_width (GTK_CLIST(clist),8,180);

  gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);

  gtk_widget_show(clist);

  return (clist);

}

int cdd_update (GtkWidget *w, struct drqm_computers_info *info)
{
  /* This function depends on info->icomp and info->row properly set */
  /* info->icomp and info->row are related, the first is the id of the computer in */
  /* the master, the second is the index in the local structure of computer */
  char msg[BUFFERLEN];
  char msg2[BUFFERLEN];
  char **buff;			/* for hte clist stuff */
  int ncols = 9;
  int i,row;
  

  if (!request_comp_xfer(info->icomp,&info->computers[info->row],CLIENT)) {
    if (drerrno == DRE_NOTREGISTERED) {
      fprintf (stderr,"Not registered anymore !\n");
    } else {
      fprintf (stderr,"Error request computer xfer: %s\n",drerrno_str());
    }
    return 0;
  }

  gtk_label_set_text (GTK_LABEL(info->cdd.lname),info->computers[info->row].hwinfo.name);
  gtk_label_set_text (GTK_LABEL(info->cdd.los),osstring(info->computers[info->row].hwinfo.os));

  snprintf(msg,BUFFERLEN-1,"%i %s %i MHz",
	   info->computers[info->row].hwinfo.ncpus,
	   proctypestring(info->computers[info->row].hwinfo.proctype),
	   info->computers[info->row].hwinfo.procspeed);
  gtk_label_set_text (GTK_LABEL(info->cdd.lcpuinfo),msg);

	// Memory
	snprintf (msg,BUFFERLEN,"%i Mb",info->computers[info->row].hwinfo.memory);
	gtk_label_set_text (GTK_LABEL(info->cdd.lmemory),msg);
  
  snprintf(msg,BUFFERLEN-1,"%i %i %i",
	   info->computers[info->row].status.loadavg[0],
	   info->computers[info->row].status.loadavg[1],
	   info->computers[info->row].status.loadavg[2]);
  gtk_label_set_text (GTK_LABEL(info->cdd.lloadavg),msg);
		      
  snprintf(msg,BUFFERLEN-1,"%i",
	   info->computers[info->row].status.ntasks);
  gtk_label_set_text (GTK_LABEL(info->cdd.lntasks),msg);

  /* Limits */
  snprintf(msg,BUFFERLEN-1,"%i",
	   info->computers[info->row].limits.nmaxcpus);
  gtk_label_set_text (GTK_LABEL(info->cdd.limits.lnmaxcpus),msg);
  snprintf(msg,BUFFERLEN-1,"%i",
	   info->computers[info->row].limits.maxfreeloadcpu);
  gtk_label_set_text (GTK_LABEL(info->cdd.limits.lmaxfreeloadcpu),msg);
	// Limits autoenable
	if (info->computers[info->row].limits.autoenable.flags &= AEF_ACTIVE) {
		snprintf(msg,BUFFERLEN-1,"%i:%02i",
						 info->computers[info->row].limits.autoenable.h,
						 info->computers[info->row].limits.autoenable.m);
		gtk_label_set_text (GTK_LABEL(info->cdd.limits.lautoenabletime),msg);
	} else {
		gtk_label_set_text (GTK_LABEL(info->cdd.limits.lautoenabletime),"OFF");
	}

	if (info->computers[info->row].limits.npools) {
		struct pool *pool;
		if ((pool = computer_pool_attach_shared_memory(info->computers[info->row].limits.poolshmid)) != (void*)-1) {
			snprintf (msg,BUFFERLEN,"%s",pool[0].name);
			for (i=1;i<info->computers[info->row].limits.npools;i++) {
				snprintf (msg2,BUFFERLEN,"%s,%s",msg,pool[i].name);
				strncpy (msg,msg2,BUFFERLEN-1);
			}
			gtk_label_set_text (GTK_LABEL(info->cdd.limits.lpools),msg);
			computer_pool_detach_shared_memory(pool);
			computer_pool_free(&info->computers[info->row].limits);
		} else {
			gtk_label_set_text (GTK_LABEL(info->cdd.limits.lpools),"WARNING: Could not attach pool shared memory");
		}
	} else {
		gtk_label_set_text (GTK_LABEL(info->cdd.limits.lpools),"WARNING: This computer doesn't belong to any pool");
	}

  /* Tasks clist */
  buff = (char**) g_malloc((ncols + 1) * sizeof(char*));
  for (i=0;i<ncols;i++)
    buff[i] = (char*) g_malloc (BUFFERLEN);
  buff[ncols] = NULL;
  
  gtk_clist_freeze(GTK_CLIST(info->cdd.clist));
  gtk_clist_clear(GTK_CLIST(info->cdd.clist));
  row = 0;
  for (i=0; i < MAXTASKS; i++) {
    if (info->computers[info->row].status.task[i].used) {
      snprintf (buff[0],BUFFERLEN-1,"%i",info->computers[info->row].status.task[i].itask);
      snprintf (buff[1],BUFFERLEN-1,"%s",
		task_status_string(info->computers[info->row].status.task[i].status));
      snprintf (buff[2],BUFFERLEN-1,"%s",info->computers[info->row].status.task[i].jobname);
      snprintf (buff[3],BUFFERLEN-1,"%i",info->computers[info->row].status.task[i].ijob);
      snprintf (buff[4],BUFFERLEN-1,"%s",info->computers[info->row].status.task[i].owner);
      snprintf (buff[5],BUFFERLEN-1,"%i",info->computers[info->row].status.task[i].frame);
      snprintf (buff[6],BUFFERLEN-1,"%i",info->computers[info->row].status.task[i].pid);
      strncpy(buff[7],"Not yet implemented",BUFFERLEN); 
      strncpy(buff[8],"Not yet implemented",BUFFERLEN);
      gtk_clist_append(GTK_CLIST(info->cdd.clist),buff);
      
      /* Row data */
      gtk_clist_set_row_data (GTK_CLIST(info->cdd.clist),row,
			      (gpointer)(uint32_t)info->computers[info->row].status.task[i].itask);
      row++;
    }
  }

  gtk_clist_thaw(GTK_CLIST(info->cdd.clist));

  for(i=0;i<ncols;i++)
    g_free (buff[i]);

  return 1;
}

void cdd_limits_nmaxcpus_bcp (GtkWidget *button, struct drqm_computers_info *info)
{
  /* Computer Details Dialog Limits nmaxcpus Button Change Pressed */
  GtkWidget *dialog;

  dialog = nmc_dialog (info);
  if (dialog)
    gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

GtkWidget *nmc_dialog (struct drqm_computers_info *info)
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
  info->cdd.limits.enmaxcpus = entry;
  snprintf(msg,BUFFERLEN-1,"%i",info->computers[info->row].limits.nmaxcpus);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,2);

  hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  button = gtk_button_new_with_label ("Submit");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(nmcd_bsumbit_pressed),info);
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

void nmcd_bsumbit_pressed (GtkWidget *button, struct drqm_computers_info *info)
{
  uint32_t nmaxcpus;
  char msg[BUFFERLEN];

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->cdd.limits.enmaxcpus)),"%u",&nmaxcpus) != 1)
    return;			/* Error in the entry */

  drqm_request_slave_limits_nmaxcpus_set(info->computers[info->row].hwinfo.name,nmaxcpus);

  info->computers[info->row].limits.nmaxcpus = nmaxcpus;

  snprintf(msg,BUFFERLEN-1,"%u",
	   info->computers[info->row].limits.nmaxcpus);
  gtk_label_set_text (GTK_LABEL(info->cdd.limits.lnmaxcpus),msg);
}

void cdd_limits_maxfreeloadcpu_bcp (GtkWidget *button, struct drqm_computers_info *info)
{
  /* Computer Details Dialog Limits maxfreeloadcpu Button Change Pressed */
  GtkWidget *dialog;

  dialog = mflc_dialog (info);
  if (dialog)
    gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

GtkWidget *mflc_dialog (struct drqm_computers_info *info)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
  char msg[BUFFERLEN];

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"New maximum free load");
/*   gtk_window_set_policy(GTK_WINDOW(window),FALSE,FALSE,TRUE); */
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add(GTK_CONTAINER(window),vbox);

  hbox = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("New maximum free load:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  info->cdd.limits.emaxfreeloadcpu = entry;
  snprintf(msg,BUFFERLEN-1,"%i",info->computers[info->row].limits.maxfreeloadcpu);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,2);

  hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  button = gtk_button_new_with_label ("Submit");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(mflcd_bsumbit_pressed),info);
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

void mflcd_bsumbit_pressed (GtkWidget *button, struct drqm_computers_info *info)
{
  uint32_t maxfreeloadcpu;
  char msg[BUFFERLEN];

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->cdd.limits.emaxfreeloadcpu)),"%u",&maxfreeloadcpu) != 1)
    return;			/* Error in the entry */

  drqm_request_slave_limits_maxfreeloadcpu_set(info->computers[info->row].hwinfo.name,maxfreeloadcpu);

  info->computers[info->row].limits.maxfreeloadcpu = maxfreeloadcpu;

  snprintf(msg,BUFFERLEN-1,"%u",
	   info->computers[info->row].limits.maxfreeloadcpu);
  gtk_label_set_text (GTK_LABEL(info->cdd.limits.lmaxfreeloadcpu),msg);
}

static void KillTask (GtkWidget *menu_item, struct drqm_computers_info *info)
{
  GtkWidget *dialog;
  static GList *cbs = NULL;		/* callbacks */

  if (!info->selected)
    return;

  if (!cbs) {
    cbs = g_list_append (cbs,dtk_bok_pressed);
		cbs = g_list_append (cbs,info);
    cbs = g_list_append (cbs,cdd_update);
		cbs = g_list_append (cbs,info);
  }

  dialog = ConfirmDialog ("Do you really want to kill the tasks ?",
			  cbs);
  if (dialog)
    gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
}

static void dtk_bok_pressed (GtkWidget *button,struct drqm_computers_info *info)
{
  /* Kill the tasks */
  /* Requeues the finished frames, sets them as waiting again */
  GList *sel;
  uint16_t itask;

  if (!(sel = GTK_CLIST(info->cdd.clist)->selection)) {
    return;
  }

  for (;sel;sel = sel->next) {
    itask = (uint16_t)(uint32_t) gtk_clist_get_row_data(GTK_CLIST(info->cdd.clist),(gint)sel->data);
    drqm_request_slave_task_kill (info->computers[info->row].hwinfo.name,itask);
/*      printf ("Killing task: %i on computer: %s\n",itask,info->computers[info->row].hwinfo.name); */
  }
}

static GtkWidget *CreateMenuTasks (struct drqm_computers_info *info)
{
  GtkWidget *menu;
  GtkWidget *menu_item;

  menu = gtk_menu_new ();
  menu_item = gtk_menu_item_new_with_label("Kill");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(KillTask),info);

  gtk_signal_connect(GTK_OBJECT((info->cdd.clist)),"event",GTK_SIGNAL_FUNC(PopupMenuTasks),info);

  gtk_widget_show_all(menu);

  return (menu);
}

static gint PopupMenuTasks (GtkWidget *clist, GdkEvent *event, struct drqm_computers_info *info)
{
  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button != 3)
      return FALSE;
    info->cdd.selected = gtk_clist_get_selection_info(GTK_CLIST(info->cdd.clist),
						      (int)bevent->x,(int)bevent->y,
						      &info->cdd.row,&info->cdd.column);
    gtk_menu_popup (GTK_MENU(info->cdd.menu), NULL, NULL, NULL, NULL,
		    bevent->button, bevent->time);
    return TRUE;
  }
  return FALSE;
}

void cdd_limits_pool_bcp (GtkWidget *bclicked, struct drqm_computers_info *info)
{
  /* Computer Details Dialog Limits pool Button Change Pressed */
	GtkWidget *dialog;
	GtkWidget *swindow;
	GtkWidget *button;
	GtkWidget *entry;

	// TreeView stuff
	GtkCellRenderer *renderer;
	GtkTreeModel *model;
	GtkWidget *view;
	// Store
	GtkListStore *store;

	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW(dialog),"List of pools");
	gtk_window_set_default_size (GTK_WINDOW(dialog),300,200);

	swindow = gtk_scrolled_window_new (NULL,NULL);
	// Scrolled window
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swindow),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),swindow,TRUE,TRUE,2);
	// Refresh button
	button = gtk_button_new_with_label("Refresh");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),button,FALSE,FALSE,2);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(cdd_limits_pool_refresh_pool_list),info);
	// Entry
	entry = gtk_entry_new_with_max_length (MAXNAMELEN-1);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),entry,FALSE,FALSE,2);
	info->cdd.limits.epool = entry;

	// The view
	view = gtk_tree_view_new();
	info->cdd.limits.pool_view = GTK_TREE_VIEW (view);
	gtk_container_add(GTK_CONTAINER(swindow),view);
	


	// Column 1
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(view),
																							 -1,
																							 "Pool name",
																							 renderer,
																							 "text",CDD_POOL_COL_NAME,
																							 NULL);
	
	// Store & TreeView
	store = gtk_list_store_new (CDD_POOL_NUM_COLS, G_TYPE_STRING);
	info->cdd.limits.pool_store = store;
	model = GTK_TREE_MODEL (store);
	gtk_tree_view_set_model (GTK_TREE_VIEW(view),model);
	g_object_unref (model);

	// Add
	button = gtk_button_new_with_label ("Add");
	gtk_box_pack_end(GTK_BOX(GTK_DIALOG(dialog)->action_area),button,TRUE,TRUE,2);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(cdd_limits_pool_add_clicked),info);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(cdd_limits_pool_refresh_pool_list),info);
	// Remove
	button = gtk_button_new_with_label ("Remove");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button,TRUE,TRUE,2);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(cdd_limits_pool_remove_clicked),info);
	g_signal_connect (G_OBJECT(button),"clicked",G_CALLBACK(cdd_limits_pool_refresh_pool_list),info);
	
	cdd_limits_pool_refresh_pool_list (button,info);

	gtk_widget_show_all (dialog);

	gtk_grab_add (dialog);
}

void cdd_limits_pool_add_clicked (GtkWidget *bclicked, struct drqm_computers_info *info)
{
	drqm_request_slave_limits_pool_add (info->computers[info->row].hwinfo.name,
																			(char *)gtk_entry_get_text(GTK_ENTRY(info->cdd.limits.epool)));
	cdd_limits_pool_refresh_pool_list (bclicked,info);
}

void cdd_limits_pool_remove_clicked (GtkWidget *bclicked, struct drqm_computers_info *info)
{
	GtkTreeSelection *selection = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(info->cdd.limits.pool_view));

	if (gtk_tree_selection_get_selected (selection,&model,&iter)) {
		char *buf;

		gtk_tree_model_get (GTK_TREE_MODEL(model),&iter,CDD_POOL_COL_NAME,&buf,-1);
		drqm_request_slave_limits_pool_remove(info->computers[info->row].hwinfo.name,buf);
		cdd_limits_pool_refresh_pool_list (bclicked,info);
	}
}


void cdd_limits_pool_refresh_pool_list (GtkWidget *bclicked, struct drqm_computers_info *info)
{
	GtkListStore *store = info->cdd.limits.pool_store;
	GtkTreeIter iter;
	int i;

	gtk_list_store_clear (GTK_LIST_STORE(store));
	// cdd_update(bclicked,info);
	// Because cdd_update removes the list of pools from shared memory
  if (!request_comp_xfer(info->icomp,&info->computers[info->row],CLIENT)) {
    if (drerrno == DRE_NOTREGISTERED) {
      fprintf (stderr,"Not registered anymore !\n");
    } else {
      fprintf (stderr,"Error request computer xfer: %s\n",drerrno_str());
    }
    return;
  }
	if (info->computers[info->row].limits.npools) {
		struct pool *pool;
		pool = computer_pool_attach_shared_memory(info->computers[info->row].limits.poolshmid);
		for (i=0;i<info->computers[info->row].limits.npools;i++) {
			gtk_list_store_append (store,&iter);
			gtk_list_store_set (store, &iter,
													CDD_POOL_COL_NAME,pool[i].name,
													-1);
		}
		computer_pool_detach_shared_memory(pool);
		computer_pool_free(&info->computers[info->row].limits);
	}
}


void cdd_limits_autoenable_bcp (GtkWidget *button, struct drqm_computers_info *info)
{
  /* Computer Details Dialog Limits autoenable Button Change Pressed */
  GtkWidget *dialog;

  dialog = autoenable_change_dialog (info);

  if (dialog)
    gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

GtkWidget *autoenable_change_dialog (struct drqm_computers_info *info)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox,*hbox2;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
	GtkWidget *cbutton;
  char msg[BUFFERLEN];

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"Change autoenable settings");
  gtk_window_set_policy(GTK_WINDOW(window),FALSE,FALSE,TRUE);
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add(GTK_CONTAINER(window),vbox);


  hbox = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);

	// Checkbutton
	cbutton = gtk_check_button_new_with_label ("Enabled");
	gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),
															 (info->computers[info->row].limits.autoenable.flags &= AEF_ACTIVE));
	info->cdd.limits.cautoenable = cbutton;

	// Label
  label = gtk_label_new ("New time:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);

  hbox2 = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start(GTK_BOX(hbox),hbox2,FALSE,FALSE,2);
  /* hour */
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  info->cdd.limits.eautoenabletime_h = entry;
  snprintf(msg,BUFFERLEN-1,"%i",info->computers[info->row].limits.autoenable.h);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox2),entry,FALSE,FALSE,2);
  /* colon */
  label = gtk_label_new (":");
  gtk_box_pack_start(GTK_BOX(hbox2),label,FALSE,FALSE,2);
  /* minute */
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  info->cdd.limits.eautoenabletime_m = entry;
  snprintf(msg,BUFFERLEN-1,"%02i",info->computers[info->row].limits.autoenable.m);
  gtk_entry_set_text(GTK_ENTRY(entry),msg);
  gtk_box_pack_start(GTK_BOX(hbox2),entry,FALSE,FALSE,2);

  hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  button = gtk_button_new_with_label ("Submit");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(aecd_bsumbit_pressed),info);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(cdd_update),info);
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

void aecd_bsumbit_pressed (GtkWidget *button, struct drqm_computers_info *info)
{
  uint32_t h,m;			/* Hour, minute */
	unsigned char flags ;
  char msg[BUFFERLEN];

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->cdd.limits.eautoenabletime_h)),"%u",&h) != 1)
    return;			/* Error in the entry */

  if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->cdd.limits.eautoenabletime_m)),"%u",&m) != 1)
    return;			/* Error in the entry */

	flags = info->computers[info->row].limits.autoenable.flags;
	if (GTK_TOGGLE_BUTTON(info->cdd.limits.cautoenable)->active) {
		flags |= AEF_ACTIVE;
	} else {
		flags &= !AEF_ACTIVE;
	}

  h = h % 24;
  m = m % 60;

  drqm_request_slave_limits_autoenable_set(info->computers[info->row].hwinfo.name,h,m,flags);

  info->computers[info->row].limits.autoenable.h = h;
  info->computers[info->row].limits.autoenable.m = m;

  snprintf(msg,BUFFERLEN-1,"%i:%02i",
	   info->computers[info->row].limits.autoenable.h,
	   info->computers[info->row].limits.autoenable.m);
  gtk_label_set_text (GTK_LABEL(info->cdd.limits.lautoenabletime),msg);
}
