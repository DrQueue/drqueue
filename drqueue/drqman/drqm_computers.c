/*
 * $Id: drqm_computers.c,v 1.15 2001/09/09 22:03:39 jorge Exp $
 */

#include <stdlib.h>
#include <string.h>

#include "drqman.h"
#include "drqm_request.h"
#include "drqm_computers.h"

/* Static functions declaration */
static GtkWidget *CreateComputersList(struct drqm_computers_info *info);
static GtkWidget *CreateClist (GtkWidget *window);
static GtkWidget *CreateButtonRefresh (struct drqm_computers_info *info);
static void PressedButtonRefresh (GtkWidget *b, struct drqm_computers_info *info);
static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct drqm_computers_info *info);
static GtkWidget *CreateMenu (struct drqm_computers_info *info);

static void ComputerDetails(GtkWidget *menu_item, struct drqm_computers_info *info);
static GtkWidget *ComputerDetailsDialog (struct drqm_computers_info *info);
static int cdd_update (GtkWidget *w, struct drqm_computers_info *info);
static GtkWidget *CreateTasksClist (void);
static void cdd_limits_nmaxcpus_bcp (GtkWidget *button, struct drqm_computers_info *info);
static GtkWidget *nmc_dialog (struct drqm_computers_info *info);
static void nmcd_bsumbit_pressed (GtkWidget *button, struct drqm_computers_info *info);
static void cdd_limits_maxfreeloadcpu_bcp (GtkWidget *button, struct drqm_computers_info *info);
static GtkWidget *mflc_dialog (struct drqm_computers_info *info);
static void mflcd_bsumbit_pressed (GtkWidget *button, struct drqm_computers_info *info);

void CreateComputersPage (GtkWidget *notebook,struct info_drqm *info)
{
  GtkWidget *label;
  GtkWidget *container;
  GtkWidget *clist;
  GtkWidget *buttonRefresh;	/* Button to refresh the computer list */
  GtkWidget *vbox;

  /* Label */
  label = gtk_label_new ("Computers");
  container = gtk_frame_new ("Computer status");
  gtk_container_border_width (GTK_CONTAINER(container),2);
  vbox = gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(container),vbox);

  /* Clist */
  clist = CreateComputersList (&info->idc);
  gtk_box_pack_start(GTK_BOX(vbox),clist,TRUE,TRUE,2);
  
  /* Button refresh */
  buttonRefresh = CreateButtonRefresh (&info->idc);
  gtk_box_pack_end(GTK_BOX(vbox),buttonRefresh,FALSE,FALSE,2);

  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, label);

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
  gtk_signal_connect(GTK_OBJECT(b),"clicked",GTK_SIGNAL_FUNC(PressedButtonRefresh),info);

  return b;
}

static void PressedButtonRefresh (GtkWidget *b, struct drqm_computers_info *info)
{
  drqm_request_computerlist (info);
  drqm_update_computerlist (info);
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
  }
  gtk_clist_thaw(GTK_CLIST(info->clist));

  for(i=0;i<ncols;i++)
    g_free (buff[i]);
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
  gtk_widget_show(menu_item);

  gtk_signal_connect(GTK_OBJECT((info->clist)),"event",GTK_SIGNAL_FUNC(PopupMenu),info);

  gtk_widget_show(menu);

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
    info->icomp = atoi (buf);
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
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  gtk_widget_show (hbox2);
  label = gtk_label_new ("0");
  info->cdd.limits.lnmaxcpus = label;
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",cdd_limits_nmaxcpus_bcp,info);
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start(GTK_BOX(vbox2),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Maximum load a cpu can have to be considered free:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  gtk_widget_show (hbox2);
  label = gtk_label_new ("80");
  info->cdd.limits.lmaxfreeloadcpu = label;
  gtk_box_pack_start (GTK_BOX(hbox2),label,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Change");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",cdd_limits_maxfreeloadcpu_bcp,info);


  /* Clist with the task info */
  /* Frame */
  frame = gtk_frame_new ("Task information");
  gtk_box_pack_start (GTK_BOX(vbox),frame,TRUE,TRUE,2);
  swin = gtk_scrolled_window_new (NULL,NULL);
  gtk_container_add (GTK_CONTAINER(frame),swin);
  clist = CreateTasksClist ();
  gtk_container_add (GTK_CONTAINER(swin),clist);
  info->cdd.clist = clist;

  if (!cdd_update (window,info)) {
    gtk_widget_destroy (GTK_WIDGET(window));
    return NULL;
  }

  /* Button Refresh */
  button = gtk_button_new_with_label ("Refresh");
  gtk_container_border_width (GTK_CONTAINER(button),5);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(cdd_update),info);
  gtk_box_pack_start (GTK_BOX(vbox),button,FALSE,FALSE,2);

  gtk_widget_show_all(window);

  return window;
}

static GtkWidget *CreateTasksClist (void)
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

  gtk_widget_show(clist);

  return (clist);

}

static int cdd_update (GtkWidget *w, struct drqm_computers_info *info)
{
  /* This function depends on info->icomp properly set */
  char msg[BUFFERLEN];
  char **buff;			/* for hte clist stuff */
  int ncols = 9;
  int i;
  

  if (!request_comp_xfer(info->icomp,&info->computers[info->icomp],CLIENT)) {
    if (drerrno == DRE_NOTREGISTERED) {
      fprintf (stderr,"Not registered anymore !\n");
    } else {
      fprintf (stderr,"Error request computer xfer: %s\n",drerrno_str());
    }
    return 0;
  }

  gtk_label_set_text (GTK_LABEL(info->cdd.lname),info->computers[info->icomp].hwinfo.name);
  gtk_label_set_text (GTK_LABEL(info->cdd.los),osstring(info->computers[info->icomp].hwinfo.os));

  snprintf(msg,BUFFERLEN-1,"%i %s %i MHz",
	   info->computers[info->icomp].hwinfo.ncpus,
	   proctypestring(info->computers[info->icomp].hwinfo.proctype),
	   info->computers[info->icomp].hwinfo.procspeed);
  gtk_label_set_text (GTK_LABEL(info->cdd.lcpuinfo),msg);
  
  snprintf(msg,BUFFERLEN-1,"%i %i %i",
	   info->computers[info->icomp].status.loadavg[0],
	   info->computers[info->icomp].status.loadavg[1],
	   info->computers[info->icomp].status.loadavg[2]);
  gtk_label_set_text (GTK_LABEL(info->cdd.lloadavg),msg);
		      
  snprintf(msg,BUFFERLEN-1,"%i",
	   info->computers[info->icomp].status.ntasks);
  gtk_label_set_text (GTK_LABEL(info->cdd.lntasks),msg);

  /* Limits */
  snprintf(msg,BUFFERLEN-1,"%i",
	   info->computers[info->icomp].limits.nmaxcpus);
  gtk_label_set_text (GTK_LABEL(info->cdd.limits.lnmaxcpus),msg);
  snprintf(msg,BUFFERLEN-1,"%i",
	   info->computers[info->icomp].limits.maxfreeloadcpu);
  gtk_label_set_text (GTK_LABEL(info->cdd.limits.lmaxfreeloadcpu),msg);

  /* Tasks clist */
  buff = (char**) g_malloc((ncols + 1) * sizeof(char*));
  for (i=0;i<ncols;i++)
    buff[i] = (char*) g_malloc (BUFFERLEN);
  buff[ncols] = NULL;
  
  gtk_clist_freeze(GTK_CLIST(info->cdd.clist));
  gtk_clist_clear(GTK_CLIST(info->cdd.clist));
  for (i=0; i < MAXTASKS; i++) {
    if (info->computers[info->icomp].status.task[i].used) {
      snprintf (buff[0],BUFFERLEN-1,"%i",i);
      snprintf (buff[1],BUFFERLEN-1,"%s",
		task_status_string(info->computers[info->icomp].status.task[i].status));
      snprintf (buff[2],BUFFERLEN-1,"%s",info->computers[info->icomp].status.task[i].jobname);
      snprintf (buff[3],BUFFERLEN-1,"%i",info->computers[info->icomp].status.task[i].ijob);
      snprintf (buff[4],BUFFERLEN-1,"%s",info->computers[info->icomp].status.task[i].owner);
      snprintf (buff[5],BUFFERLEN-1,"%i",info->computers[info->icomp].status.task[i].frame);
      snprintf (buff[6],BUFFERLEN-1,"%i",info->computers[info->icomp].status.task[i].pid);
      strncpy(buff[7],"Not yet implemented",BUFFERLEN); 
      strncpy(buff[8],"Not yet implemented",BUFFERLEN);
      gtk_clist_append(GTK_CLIST(info->cdd.clist),buff);
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

  window = gtk_window_new (GTK_WINDOW_DIALOG);
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
  snprintf(msg,BUFFERLEN-1,"%i",info->computers[info->icomp].limits.nmaxcpus);
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

  drqm_request_slave_limits_nmaxcpus_set(info->computers[info->icomp].hwinfo.name,nmaxcpus);

  info->computers[info->icomp].limits.nmaxcpus = nmaxcpus;

  snprintf(msg,BUFFERLEN-1,"%u",
	   info->computers[info->icomp].limits.nmaxcpus);
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

  window = gtk_window_new (GTK_WINDOW_DIALOG);
  gtk_window_set_title (GTK_WINDOW(window),"New maximum free load");
  gtk_window_set_policy(GTK_WINDOW(window),FALSE,FALSE,TRUE);
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add(GTK_CONTAINER(window),vbox);

  hbox = gtk_hbox_new (FALSE,2);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("New maximum free load:");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length(BUFFERLEN);
  info->cdd.limits.emaxfreeloadcpu = entry;
  snprintf(msg,BUFFERLEN-1,"%i",info->computers[info->icomp].limits.maxfreeloadcpu);
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

  drqm_request_slave_limits_maxfreeloadcpu_set(info->computers[info->icomp].hwinfo.name,maxfreeloadcpu);

  info->computers[info->icomp].limits.maxfreeloadcpu = maxfreeloadcpu;

  snprintf(msg,BUFFERLEN-1,"%u",
	   info->computers[info->icomp].limits.maxfreeloadcpu);
  gtk_label_set_text (GTK_LABEL(info->cdd.limits.lmaxfreeloadcpu),msg);
}
