/*
 * $Id: drqm_jobs.c,v 1.21 2001/09/01 16:43:20 jorge Exp $
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

#include "waiting.xpm"
#include "running.xpm"
#include "finished.xpm"
#include "error.xpm"

#include "drqman.h"
#include "drqm_request.h"
#include "drqm_jobs.h"

/* Static functions declaration */
static GtkWidget *CreateJobsList(struct info_drqm_jobs *info);
static GtkWidget *CreateClist (GtkWidget *window);
static GtkWidget *CreateButtonRefresh (struct info_drqm_jobs *info);
static void PressedButtonRefresh (GtkWidget *b, struct info_drqm_jobs *info);
static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct info_drqm_jobs *info);
static GtkWidget *CreateMenu (struct info_drqm_jobs *info);
static int pri_cmp_clist (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2);

static void JobDetails(GtkWidget *menu_item, struct info_drqm_jobs *info);
static GtkWidget *JobDetailsDialog (struct info_drqm_jobs *info);
static GtkWidget *CreateFrameInfoClist (void);
static void jdd_destroy (GtkWidget *w, struct info_drqm_jobs *info);
static int jdd_update (GtkWidget *w, struct info_drqm_jobs *info);
static GtkWidget *CreateMenuFrames (struct info_drqm_jobs *info);
static gint PopupMenuFrames (GtkWidget *clist, GdkEvent *event, struct info_drqm_jobs *info);
static void SeeFrameLog (GtkWidget *w, struct info_drqm_jobs *info);
static GtkWidget *SeeFrameLogDialog (struct info_drqm_jobs *info);

static void NewJob (GtkWidget *menu_item, struct info_drqm_jobs *info);
static GtkWidget *NewJobDialog (struct info_drqm_jobs *info);
static void dnj_psearch (GtkWidget *button, struct info_dnj *info);
static void dnj_set_cmd (GtkWidget *button, struct info_dnj *info);
static void dnj_cpri_changed (GtkWidget *entry, struct info_dnj *info);
static void dnj_bsubmit_pressed (GtkWidget *button, struct info_dnj *info);
static int dnj_submit (struct info_dnj *info);
static void dnj_destroyed (GtkWidget *dialog, struct info_drqm_jobs *info);

static void DeleteJob (GtkWidget *menu_item, struct info_drqm_jobs *info);
static GtkWidget *DeleteJobDialog (struct info_drqm_jobs *info);
static void djd_bok_pressed (GtkWidget *button, struct info_drqm_jobs *info);

static void StopJob (GtkWidget *menu_item, struct info_drqm_jobs *info);

static void HStopJob (GtkWidget *menu_item, struct info_drqm_jobs *info);
static GtkWidget *HStopJobDialog (struct info_drqm_jobs *info);
static void djhs_bok_pressed (GtkWidget *button, struct info_drqm_jobs *info);

static void ContinueJob (GtkWidget *menu_item, struct info_drqm_jobs *info);

void CreateJobsPage (GtkWidget *notebook, struct info_drqm *info)
{
  /* This function receives the notebook widget to wich the new tab will append */
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
  clist = CreateJobsList (&info->idj);
  gtk_box_pack_start(GTK_BOX(vbox),clist,TRUE,TRUE,2);
  
  /* Button refresh */
  buttonRefresh = CreateButtonRefresh (&info->idj);
  gtk_box_pack_end(GTK_BOX(vbox),buttonRefresh,FALSE,FALSE,2);

  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, label);

  /* Put the jobs on the list */
  drqm_request_joblist (&info->idj);
  drqm_update_joblist (&info->idj);

  gtk_widget_show(clist);
  gtk_widget_show(vbox);
  gtk_widget_show(label);
  gtk_widget_show(container);
}

static GtkWidget *CreateJobsList(struct info_drqm_jobs *info)
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
  gchar *titles[] = { "ID","Name","Owner","Status","Processors","Left","Done","Failed","Pri" };
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (9, titles);
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

  gtk_clist_set_sort_column (GTK_CLIST(clist),8);
  gtk_clist_set_sort_type (GTK_CLIST(clist),GTK_SORT_ASCENDING);
/*    gtk_clist_set_compare_func (GTK_CLIST(clist),pri_cmp_clist); */

  gtk_widget_show(clist);

  return (clist);
}


static GtkWidget *CreateButtonRefresh (struct info_drqm_jobs *info)
{
  GtkWidget *b;
  
  b = gtk_button_new_with_label ("Refresh");
  gtk_container_border_width (GTK_CONTAINER(b),5);
  gtk_widget_show (GTK_WIDGET(b));
  gtk_signal_connect(GTK_OBJECT(b),"clicked",GTK_SIGNAL_FUNC(PressedButtonRefresh),info);

  return b;
}

static void PressedButtonRefresh (GtkWidget *b, struct info_drqm_jobs *info)
{
  drqm_request_joblist (info);
  drqm_update_joblist (info);
}

void drqm_update_joblist (struct info_drqm_jobs *info)
{
  int i;
  char **buff;
  int ncols = 9;

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
    snprintf (buff[8],BUFFERLEN,"%i",info->jobs[i].priority);
    gtk_clist_append(GTK_CLIST(info->clist),buff);
  }

  gtk_clist_sort (GTK_CLIST(info->clist));
  gtk_clist_thaw(GTK_CLIST(info->clist));

  for(i=0;i<ncols;i++)
    g_free (buff[i]);
}

static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct info_drqm_jobs *info)
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

static GtkWidget *CreateMenu (struct info_drqm_jobs *info)
{
  GtkWidget *menu;
  GtkWidget *menu_item;

  menu = gtk_menu_new ();
  menu_item = gtk_menu_item_new_with_label("Details");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(JobDetails),info);
  gtk_widget_show(menu_item);

  menu_item = gtk_menu_item_new_with_label("New Job");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(NewJob),info);
  gtk_widget_show(menu_item);

  menu_item = gtk_menu_item_new_with_label("Stop");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(StopJob),info);
  gtk_widget_show(menu_item);

  menu_item = gtk_menu_item_new_with_label("Hard Stop");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(HStopJob),info);
  gtk_widget_show(menu_item);

  menu_item = gtk_menu_item_new_with_label("Continue");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(ContinueJob),info);
  gtk_widget_show(menu_item);

  menu_item = gtk_menu_item_new_with_label("Delete");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(DeleteJob),info);
  gtk_widget_show(menu_item);

  gtk_signal_connect(GTK_OBJECT((info->clist)),"event",GTK_SIGNAL_FUNC(PopupMenu),info);

  gtk_widget_show_all(menu);

  return (menu);
}

static void JobDetails(GtkWidget *menu_item, struct info_drqm_jobs *info)
{
  GtkWidget *dialog;

  if (!info->selected)
    return;

  dialog = JobDetailsDialog(info);
  if (dialog)
    gtk_grab_add(dialog);
}

static void NewJob (GtkWidget *menu_item, struct info_drqm_jobs *info)
{
  GtkWidget *dialog;
  dialog = NewJobDialog(info);
  gtk_signal_connect (GTK_OBJECT(dialog),"destroy",
		      dnj_destroyed,info);
  gtk_grab_add(dialog);
}

static GtkWidget *NewJobDialog (struct info_drqm_jobs *info)
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
  GList *items = NULL;

  /* Dialog */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"New Job");
  gtk_signal_connect_object(GTK_OBJECT(window),"destroy",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)window);
  gtk_window_set_default_size(GTK_WINDOW(window),600,200);
  gtk_container_set_border_width (GTK_CONTAINER(window),5);
  info->dnj.dialog = window;

  /* Frame */
  frame = gtk_frame_new ("Give job information");
  gtk_container_add (GTK_CONTAINER(window),frame);
  gtk_widget_show (frame);

  /* Main vbox */
  vbox = gtk_vbox_new (FALSE,2);
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
/*    gtk_misc_set_padding (GTK_MISC(label),20,2); */
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  gtk_widget_show(label);
  entry = gtk_entry_new_with_max_length (MAXNAMELEN-1);
  info->dnj.ename = entry;
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);
  gtk_widget_show(entry);

  /* Cmd of the job */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  gtk_widget_show (hbox);
  label = gtk_label_new ("Command:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
/*    gtk_misc_set_padding (GTK_MISC(label),20,2); */
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  gtk_widget_show(label);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  gtk_widget_show (hbox2);
  entry = gtk_entry_new_with_max_length (MAXCMDLEN-1);
  info->dnj.ecmd = entry;
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  gtk_widget_show(entry);
  button = gtk_button_new_with_label ("Search");
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",dnj_psearch,&info->dnj);

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
  gtk_signal_connect (GTK_OBJECT(GTK_ENTRY(GTK_COMBO(combo)->entry)),
		      "changed",dnj_cpri_changed,&info->dnj);
  gtk_entry_set_text (GTK_ENTRY(GTK_COMBO(combo)->entry),"Normal");

  /* Buttons */
  /* submit */
  bbox = gtk_hbutton_box_new ();
  gtk_hbutton_box_set_layout_default (GTK_BUTTONBOX_SPREAD);
  gtk_box_pack_start (GTK_BOX(vbox),bbox,TRUE,TRUE,5);
  gtk_widget_show (bbox);
  button = gtk_button_new_with_label ("Submit");
  gtk_box_pack_start (GTK_BOX(bbox),button,TRUE,TRUE,2);
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",
		      dnj_bsubmit_pressed,&info->dnj);

  /* cancel */
  button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX(bbox),button,TRUE,TRUE,2);
  gtk_widget_show (button);
  gtk_signal_connect_object (GTK_OBJECT(button),"clicked",
			     GTK_SIGNAL_FUNC(gtk_widget_destroy),
			     (gpointer) window);

  gtk_widget_show_all(window);

  return window;
}

static void dnj_psearch (GtkWidget *button, struct info_dnj *info)
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

static void dnj_set_cmd (GtkWidget *button, struct info_dnj *info)
{
  gtk_entry_set_text (GTK_ENTRY(info->ecmd),gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fs)));
}

static void dnj_cpri_changed (GtkWidget *entry, struct info_dnj *info)
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
    fprintf (stderr,"Not listed!\n");
  }
}

static void dnj_bsubmit_pressed (GtkWidget *button, struct info_dnj *info)
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

static int dnj_submit (struct info_dnj *info)
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

  if (!register_job (&job))
    return 0;

  return 1;
}

static void dnj_destroyed (GtkWidget *dialog, struct info_drqm_jobs *info)
{
  drqm_request_joblist (info);
  drqm_update_joblist (info);
}

static int pri_cmp_clist (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
  uint32_t a,b;

  sscanf(ptr1,"%u",&a);
  sscanf(ptr2,"%u",&b);
  fprintf (stderr,"%u %u\n",a,b);
  fprintf (stderr,"%u %u\n",(int)ptr1,(int)ptr2);
  fprintf (stderr,"%u %u\n",*((int*)ptr1),*((int*)ptr2));
  
  if (a > b) {
    return 1;
  } else if (a == b) {
    return 0;
  } else {
    return -1;
  }

  return 0;
}

static void DeleteJob (GtkWidget *menu_item, struct info_drqm_jobs *info)
{
  GtkWidget *dialog;

  if (!info->selected)
    return;

  dialog = DeleteJobDialog(info);
  gtk_signal_connect (GTK_OBJECT(dialog),"destroy",
		      dnj_destroyed,info); /* Updates the list */
  gtk_grab_add(dialog);
}

static GtkWidget *DeleteJobDialog (struct info_drqm_jobs *info)
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
  gtk_widget_show(GTK_WIDGET(label));
 
  /* Buttons */
  button = gtk_button_new_with_label ("Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(djd_bok_pressed),info);
  gtk_signal_connect_object(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)dialog);
  gtk_widget_show (button);
  button = gtk_button_new_with_label ("No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  gtk_signal_connect_object(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)dialog);
  GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);

  gtk_widget_show (dialog);

  return dialog;
}

static void djd_bok_pressed (GtkWidget *button, struct info_drqm_jobs *info)
{
  drqm_request_job_delete (info);
}

static void StopJob (GtkWidget *menu_item, struct info_drqm_jobs *info)
{
  if (!info->selected)
    return;

  drqm_request_job_stop (info);
  dnj_destroyed(menu_item,info); /* updates the list */
}

static void ContinueJob (GtkWidget *menu_item, struct info_drqm_jobs *info)
{
  if (!info->selected)
    return;

  drqm_request_job_continue (info);
  dnj_destroyed(menu_item,info); /* Updates the list */
}

static void HStopJob (GtkWidget *menu_item, struct info_drqm_jobs *info)
{
  GtkWidget *dialog;

  if (!info->selected)
    return;

  dialog = HStopJobDialog(info);
  gtk_signal_connect (GTK_OBJECT(dialog),"destroy",
		      dnj_destroyed,info); /* Updates the list */
  gtk_grab_add(dialog);
}

static GtkWidget *JobDetailsDialog (struct info_drqm_jobs *info)
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

  if (info->njobs) {
    gtk_clist_get_text(GTK_CLIST(info->clist),info->row,0,&buf);
    info->ijob = atoi (buf);
  } else {
    return NULL;
  }

  /* Dialog */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"Job Details");
  gtk_signal_connect (GTK_OBJECT(window),"destroy",GTK_SIGNAL_FUNC(jdd_destroy),info);
  gtk_signal_connect_object(GTK_OBJECT(window),"destroy",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)window);
  gtk_window_set_default_size(GTK_WINDOW(window),800,500);
  gtk_container_set_border_width (GTK_CONTAINER(window),5);
  info->jdd.dialog = window;

  /* Frame */
  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER(window),frame);

  /* Main vbox */
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  /* Label */
  label = gtk_label_new ("Detailed job information");
  gtk_label_set_pattern (GTK_LABEL(label),"________________________________");
  gtk_box_pack_start (GTK_BOX(vbox),label,FALSE,FALSE,4);

  /* Name of the job */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Name:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->jdd.lname = label;

  /* Status */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Status:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->jdd.lstatus = label;


  /* Cmd of the job */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Command:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
/*    gtk_misc_set_padding (GTK_MISC(label),20,2); */
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->jdd.lcmd = label;

  /* Start and End frames */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Start, end and step frames:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->jdd.lstartend = label;

  /* Priority */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Priority:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->jdd.lpri = label;

  /* Frames left, done and failed */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Frames running, left, done and failed:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->jdd.lfrldf = label;

  /* Average time per frame */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Average frame time:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->jdd.lavgt = label;

  /* Estimated finish time */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Estimated finish time:");
  gtk_label_set_justify (GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  info->jdd.lestf = label;

  /* Clist with the frame info */
  /* Frame */
  frame = gtk_frame_new ("Frame information");
  gtk_box_pack_start (GTK_BOX(vbox),frame,TRUE,TRUE,2);
  /* Clist with the frame info */
  swin = gtk_scrolled_window_new (NULL,NULL);
  gtk_container_add (GTK_CONTAINER(frame),swin);
  clist = CreateFrameInfoClist ();
  gtk_container_add (GTK_CONTAINER(swin),clist);
  info->jdd.clist = clist;

  info->jdd.menu = CreateMenuFrames(info);

  if (!jdd_update (window,info)) {
    gtk_widget_destroy (GTK_WIDGET(window));
    return NULL;
  }

  /* Buttons */
  /* Stop */
  hbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,5);
  button = gtk_button_new_with_label ("Stop");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",
		      StopJob,info);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_update),info);
  /* Hard Stop */
  button = gtk_button_new_with_label ("Hard Stop");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",
		      GTK_SIGNAL_FUNC(HStopJob),info);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_update),info);
  /* Continue */
  button = gtk_button_new_with_label ("Continue");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",
		      GTK_SIGNAL_FUNC(ContinueJob),info);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_update),info);
  /* Delete */
  button = gtk_button_new_with_label ("Delete");
  gtk_box_pack_start (GTK_BOX(hbox),button,TRUE,TRUE,2);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",
		      GTK_SIGNAL_FUNC(DeleteJob),info);
/*    gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_update),info); */

  /* Button Refresh */
  button = gtk_button_new_with_label ("Refresh");
  gtk_container_border_width (GTK_CONTAINER(button),5);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(jdd_update),info);
  gtk_box_pack_start (GTK_BOX(vbox),button,FALSE,FALSE,2);

  gtk_widget_show_all(window);

  return window;
}

static void jdd_destroy (GtkWidget *w, struct info_drqm_jobs *info)
{
  if (info->jobs[info->ijob].frame_info) {
    free (info->jobs[info->ijob].frame_info);
    info->jobs[info->ijob].frame_info = NULL;
  }
}

static GtkWidget *CreateFrameInfoClist (void)
{
  gchar *titles[] = { "Number","Status","Start","End","Exit Code","Icomp","Itask" };
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (7, titles);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_column_titles_passive(GTK_CLIST(clist));
  gtk_clist_set_column_width (GTK_CLIST(clist),0,75);
  gtk_clist_set_column_width (GTK_CLIST(clist),1,95);
  gtk_clist_set_column_width (GTK_CLIST(clist),2,180);
  gtk_clist_set_column_width (GTK_CLIST(clist),3,180);
  gtk_clist_set_column_width (GTK_CLIST(clist),4,85);
  gtk_clist_set_column_width (GTK_CLIST(clist),5,45);
  gtk_clist_set_column_width (GTK_CLIST(clist),6,45);

  gtk_widget_show(clist);

  return (clist);

}

static int jdd_update (GtkWidget *w, struct info_drqm_jobs *info)
{
  /* This function depends on info->ijob properly set */
  int nframes;
  struct frame_info *fi;
  char msg[BUFFERLEN];
  char *buf;
  char **buff;			/* for hte clist stuff */
  int ncols = 7;
  int i;
  GtkWidget *toplevel;

  static GdkBitmap *w_mask = NULL;
  static GdkPixmap *w_data = NULL;
  static GdkBitmap *r_mask = NULL;
  static GdkPixmap *r_data = NULL;
  static GdkBitmap *f_mask = NULL;
  static GdkPixmap *f_data = NULL;
  static GdkBitmap *e_mask = NULL;
  static GdkPixmap *e_data = NULL;

  if (!request_job_xfer(info->ijob,&info->jobs[info->ijob],CLIENT)) {
    if (drerrno == DRE_NOTREGISTERED) {
/*        gtk_object_destroy (GTK_OBJECT(info->jdd.dialog)); */
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

  
  /* Pixmap stuff */
  if (!w_mask) {
    toplevel = gtk_widget_get_toplevel(info->jdd.dialog);
    w_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&w_mask,NULL,(gchar**)waiting_xpm);
    if (!r_mask)
      r_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&r_mask,NULL,(gchar**)running_xpm);
    if (!f_mask)
      f_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&f_mask,NULL,(gchar**)finished_xpm);
    if (!e_mask)
      e_data = gdk_pixmap_create_from_xpm_d (GTK_WIDGET(toplevel)->window,&e_mask,NULL,(gchar**)error_xpm);
  }

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
    switch (info->jobs[info->ijob].frame_info[i].status) {
    case FS_WAITING:
      gtk_clist_set_pixtext (GTK_CLIST(info->jdd.clist),i,1,
			     job_frame_status_string(info->jobs[info->ijob].frame_info[i].status), 2,
			     w_data,w_mask);
      break;
    case FS_ASSIGNED:
      gtk_clist_set_pixtext (GTK_CLIST(info->jdd.clist),i,1,
			     job_frame_status_string(info->jobs[info->ijob].frame_info[i].status), 2,
			     r_data,r_mask);
      break;
    case FS_FINISHED:
      gtk_clist_set_pixtext (GTK_CLIST(info->jdd.clist),i,1,
			     job_frame_status_string(info->jobs[info->ijob].frame_info[i].status), 2,
			     f_data,f_mask);
      break;
    case FS_ERROR:
      gtk_clist_set_pixtext (GTK_CLIST(info->jdd.clist),i,1,
			     job_frame_status_string(info->jobs[info->ijob].frame_info[i].status), 2,
			     e_data,e_mask);
      break;
    }			    
  }

  gtk_clist_thaw(GTK_CLIST(info->jdd.clist));

  for(i=0;i<ncols;i++)
    g_free (buff[i]);

  return 1;
}

static GtkWidget *HStopJobDialog (struct info_drqm_jobs *info)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;

  /* Dialog */
  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(dialog),"You Sure?");

  /* Label */
  label = gtk_label_new ("Do you really want to hard stop the job?\n(This will kill all current running processes)");
  gtk_misc_set_padding (GTK_MISC(label), 10, 10);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,TRUE,5);
  gtk_widget_show(GTK_WIDGET(label));
 
  /* Buttons */
  button = gtk_button_new_with_label ("Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(djhs_bok_pressed),info);
  gtk_signal_connect_object(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)dialog);
  gtk_widget_show (button);
  button = gtk_button_new_with_label ("No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  gtk_signal_connect_object(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)dialog);
  GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);

  gtk_widget_show (dialog);

  return dialog;
}

static void djhs_bok_pressed (GtkWidget *button, struct info_drqm_jobs *info)
{
  drqm_request_job_hstop (info);
}

static GtkWidget *CreateMenuFrames (struct info_drqm_jobs *info)
{
  GtkWidget *menu;
  GtkWidget *menu_item;

  menu = gtk_menu_new ();
  menu_item = gtk_menu_item_new_with_label("Set Waiting (requeue)");
  gtk_menu_append(GTK_MENU(menu),menu_item);
/*    gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(JobDetails),info); */

  menu_item = gtk_menu_item_new_with_label("Set Finished");
  gtk_menu_append(GTK_MENU(menu),menu_item);
/*    gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(NewJob),info); */

  menu_item = gtk_menu_item_new_with_label("Set ERROR");
  gtk_menu_append(GTK_MENU(menu),menu_item);
/*    gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(StopJob),info); */

  /* Separation bar */
  menu_item = gtk_menu_item_new ();
  gtk_menu_append(GTK_MENU(menu),menu_item);

  menu_item = gtk_menu_item_new_with_label("Watch frame Log");
  gtk_menu_append(GTK_MENU(menu),menu_item);
  gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(SeeFrameLog),info);


  gtk_signal_connect(GTK_OBJECT((info->jdd.clist)),"event",GTK_SIGNAL_FUNC(PopupMenuFrames),info);

  gtk_widget_show_all(menu);

  return (menu);
}


static gint PopupMenuFrames (GtkWidget *clist, GdkEvent *event, struct info_drqm_jobs *info)
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

static void SeeFrameLog (GtkWidget *w, struct info_drqm_jobs *info)
{
  GtkWidget *dialog;

  if (!info->jdd.selected)
    return;

  dialog = SeeFrameLogDialog(info);
  if (dialog)
    gtk_grab_add(dialog);
}

static GtkWidget *SeeFrameLogDialog (struct info_drqm_jobs *info)
{
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *text;
  GtkWidget *swin;
  int fd;
  struct task task;
  char buf[BUFFERLEN];
  int n;

  /* log_dumptask_open only uses the jobname and frame fields of the task */
  /* so I fill a task with only those two valid fields and so can use that */
  /* function */
  strncpy (task.jobname,info->jobs[info->row].name,MAXNAMELEN-1);
  task.frame = job_frame_index_to_number (&info->jobs[info->row],info->jdd.row);

  /* Dialog */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window),"New Job");
  gtk_signal_connect_object(GTK_OBJECT(window),"destroy",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)window);
  gtk_window_set_default_size(GTK_WINDOW(window),600,200);
  gtk_container_set_border_width (GTK_CONTAINER(window),5);

  /* Frame */
  snprintf (buf,BUFFERLEN-1,"Log for frame %i on job %s",task.frame,task.jobname);
  frame = gtk_frame_new (buf);
  gtk_container_add (GTK_CONTAINER(window),frame);

  /* Text */
  swin = gtk_scrolled_window_new(NULL,NULL);
  gtk_container_add (GTK_CONTAINER(frame),swin);
  text = gtk_text_new (NULL,NULL);
  gtk_container_add (GTK_CONTAINER(swin),text);

  if ((fd = log_dumptask_open_ro (&task)) == -1) {
    char msg[] = "Couldn't open log file";
    gtk_text_insert (GTK_TEXT(text),NULL,NULL,NULL,msg,strlen(msg));
  } else {
    while ((n = read (fd,buf,BUFFERLEN))) {
      gtk_text_insert (GTK_TEXT(text),NULL,NULL,NULL,buf,n);
    }
  }

  gtk_widget_show_all (window);

  return window;
}
