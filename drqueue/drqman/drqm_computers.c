/*
 * $Id: drqm_computers.c,v 1.8 2001/08/29 15:26:32 jorge Exp $
 */

#include <stdlib.h>
#include <string.h>

#include "drqman.h"
#include "drqm_request.h"
#include "drqm_computers.h"

/* Static functions declaration */
static GtkWidget *CreateComputersList(struct info_drqm_computers *info);
static GtkWidget *CreateClist (GtkWidget *window);
static GtkWidget *CreateButtonRefresh (struct info_drqm_computers *info);
static void PressedButtonRefresh (GtkWidget *b, struct info_drqm_computers *info);
static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct info_drqm_computers *info);
static GtkWidget *CreateMenu (struct info_drqm_computers *info);

static void ComputerDetails(GtkWidget *menu_item, struct info_drqm_computers *info);
static GtkWidget *ComputerDetailsDialog (struct info_drqm_computers *info);
static int cdd_update (GtkWidget *w, struct info_drqm_computers *info);
static GtkWidget *CreateTasksClist (void);


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

static GtkWidget *CreateComputersList(struct info_drqm_computers *info)
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

static GtkWidget *CreateButtonRefresh (struct info_drqm_computers *info)
{
  GtkWidget *b;
  
  b = gtk_button_new_with_label ("Refresh");
  gtk_container_border_width (GTK_CONTAINER(b),5);
  gtk_widget_show (GTK_WIDGET(b));
  gtk_signal_connect(GTK_OBJECT(b),"clicked",GTK_SIGNAL_FUNC(PressedButtonRefresh),info);

  return b;
}

static void PressedButtonRefresh (GtkWidget *b, struct info_drqm_computers *info)
{
  drqm_request_computerlist (info);
  drqm_update_computerlist (info);
}

void drqm_update_computerlist (struct info_drqm_computers *info)
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
    snprintf (buff[4],BUFFERLEN,"%i",info->computers[i].hwinfo.numproc);
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

static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct info_drqm_computers *info)
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

static GtkWidget *CreateMenu (struct info_drqm_computers *info)
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

static void ComputerDetails(GtkWidget *menu_item, struct info_drqm_computers *info)
{
  GtkWidget *dialog;

  if (!info->selected)
    return;

  dialog = ComputerDetailsDialog(info);
  if (dialog)
    gtk_grab_add(dialog);
}

static GtkWidget *ComputerDetailsDialog (struct info_drqm_computers *info)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox;
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
  window = gtk_window_new (GTK_WINDOW_DIALOG);
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
  gchar *titles[] = { "ID","Status","Job name","Job index","Owner","Frame","PID","Start","End"};
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

static int cdd_update (GtkWidget *w, struct info_drqm_computers *info)
{
  /* This function depends on info->icomp properly set */
  char msg[BUFFERLEN];
  char *buf;
  char **buff;			/* for hte clist stuff */
  int ncols = 7;
  int i;

  if (!request_job_xfer(info->ijob,&info->jobs[info->ijob],CLIENT)) {
    if (drerrno == DRE_NOTREGISTERED) {
      gtk_object_destroy (GTK_OBJECT(info->jdd.dialog));
/*        gtk_widget_destroy (info->jdd.dialog); */
/*        gtk_signal_emit_by_name (GTK_OBJECT(info->jdd.dialog),"destroy"); */
    } else {
      fprintf (stderr,"Error request job xfer: %s\n",drerrno_str());
    }
    return 0;
  }

  nframes = job_nframes (&info->jobs[info->ijob]);

  if (!info->jobs[info->ijob].frame_info) {
    if (!(fi = malloc(sizeof (struct frame_info) * nframes))) {
      fprintf (stderr,"Error allocating memory for frame information\n");
      return 0;
    }

    if (!request_job_xferfi (info->ijob,fi,nframes,CLIENT)) {
      fprintf (stderr,"Error request job frame info xfer: %s\n",drerrno_str());
      free (fi);
      return 0;
    }

    info->jobs[info->ijob].frame_info = fi;
  }

  gtk_label_set_text (GTK_LABEL(info->jdd.lname),info->jobs[info->ijob].name);
  gtk_label_set_text (GTK_LABEL(info->jdd.lcmd),info->jobs[info->ijob].cmd);
  gtk_label_set_text (GTK_LABEL(info->jdd.lstatus),job_status_string(info->jobs[info->ijob].status));
  
  snprintf(msg,BUFFERLEN-1,"From %i to %i every %i",
	   info->jobs[info->ijob].frame_start,
	   info->jobs[info->ijob].frame_end,
	   info->jobs[info->ijob].frame_step);
  gtk_label_set_text (GTK_LABEL(info->jdd.lstartend),msg);
  
  snprintf(msg,BUFFERLEN-1,"%i",info->jobs[info->ijob].priority);
  gtk_label_set_text (GTK_LABEL(info->jdd.lpri),msg);

  snprintf(msg,BUFFERLEN-1,"%i,%i,%i,%i",
	   info->jobs[info->ijob].nprocs,
	   info->jobs[info->ijob].fleft,
	   info->jobs[info->ijob].fdone,
	   info->jobs[info->ijob].ffailed);
  gtk_label_set_text (GTK_LABEL(info->jdd.lfrldf),msg);

  if ((info->jobs[info->ijob].avg_frame_time / 3600) > 0) {
    snprintf(msg,BUFFERLEN-1,"%li hours %li minutes %li seconds",
	     info->jobs[info->ijob].avg_frame_time / 3600,
	     (info->jobs[info->ijob].avg_frame_time % 3600) / 60,
	     (info->jobs[info->ijob].avg_frame_time % 3600) % 60);
  } else if ((info->jobs[info->ijob].avg_frame_time / 60) > 0) {
    snprintf(msg,BUFFERLEN-1,"%li minutes %li seconds",
	     (info->jobs[info->ijob].avg_frame_time) / 60,
	     (info->jobs[info->ijob].avg_frame_time) % 60);
  } else {
    snprintf(msg,BUFFERLEN-1,"%li seconds",
	     info->jobs[info->ijob].avg_frame_time);
  }
  gtk_label_set_text (GTK_LABEL(info->jdd.lavgt),msg);
  
  snprintf(msg,BUFFERLEN-1,"%s",ctime(&info->jobs[info->ijob].est_finish_time));
  buf = strchr (msg,'\n');
  if (buf != NULL)
    *buf = '\0';
  gtk_label_set_text (GTK_LABEL(info->jdd.lestf),msg);


  buff = (char**) g_malloc((ncols + 1) * sizeof(char*));
  for (i=0;i<ncols;i++)
    buff[i] = (char*) g_malloc (BUFFERLEN);
  buff[ncols] = NULL;
  
  gtk_clist_freeze(GTK_CLIST(info->jdd.clist));
  gtk_clist_clear(GTK_CLIST(info->jdd.clist));
  for (i=0; i < nframes; i++) {
    snprintf (buff[0],BUFFERLEN-1,"%i",job_frame_index_to_number (&info->jobs[info->ijob],i));
    strncpy(buff[1],job_frame_status_string(info->jobs[info->ijob].frame_info[i].status),BUFFERLEN);
    if (info->jobs[info->ijob].frame_info[i].status != FS_WAITING) {
      strncpy(buff[2],ctime(&info->jobs[info->ijob].frame_info[i].start_time),BUFFERLEN); 
      strncpy(buff[3],ctime(&info->jobs[info->ijob].frame_info[i].end_time),BUFFERLEN);
    } else {
      strncpy(buff[2],"Not started",BUFFERLEN); 
      strncpy(buff[3],"Not started",BUFFERLEN);
    }      
    snprintf (buff[4],BUFFERLEN,"%i",info->jobs[info->ijob].frame_info[i].exitcode);
    snprintf (buff[5],BUFFERLEN,"%i",info->jobs[info->ijob].frame_info[i].icomp);
    snprintf (buff[6],BUFFERLEN,"%i",info->jobs[info->ijob].frame_info[i].itask);
    gtk_clist_append(GTK_CLIST(info->jdd.clist),buff);
  }

  gtk_clist_thaw(GTK_CLIST(info->jdd.clist));

  for(i=0;i<ncols;i++)
    g_free (buff[i]);

  return 1;
}


