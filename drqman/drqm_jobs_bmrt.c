/* 
 * $Id: drqm_jobs_bmrt.c,v 1.3 2003/12/18 21:38:32 jorge Exp $ 
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>

#include "drqm_jobs.h"
#include "drqm_common.h"
#include "drqm_jobs_bmrt.h"

static void dnj_koj_frame_bmrt_script_search (GtkWidget *button, struct drqmj_koji_bmrt *info);
static void dnj_koj_frame_bmrt_script_set (GtkWidget *button, struct drqmj_koji_bmrt *info);
static void dnj_koj_frame_bmrt_scene_search (GtkWidget *button, struct drqmj_koji_bmrt *info);
static void dnj_koj_frame_bmrt_scene_set (GtkWidget *button, struct drqmj_koji_bmrt *info);
static void dnj_koj_frame_bmrt_bcreate_pressed (GtkWidget *button, struct drqmj_dnji *info);

static void dnj_flags_cbcrop_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_flags_cbsamples_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_flags_cbradiositysamples_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_flags_cbraysamples_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);

GtkWidget *dnj_koj_frame_bmrt (struct drqm_jobs_info *info)
{
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox,*hbox2;
  GtkWidget *label;
  GtkWidget *entry; 
  GtkWidget *button;
  GtkWidget *bbox;
	GtkWidget *cbutton;
  GtkTooltips *tooltips;

  tooltips = TooltipsNew ();

  /* Frame */
  frame = gtk_frame_new ("Bmrt job information");

  /* Main vbox */
  vbox = gtk_vbox_new (FALSE,2);
  gtk_container_add (GTK_CONTAINER(frame),vbox);

  /* Scene file */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Scene file:");
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  info->dnj.koji_bmrt.escene = entry;
  gtk_tooltips_set_tip(tooltips,entry,"File name of the bmrt scene file that should be rendered",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for the bmrt scene file",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",dnj_koj_frame_bmrt_scene_search,&info->dnj.koji_bmrt);

  /* Custom crop */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	cbutton = gtk_check_button_new_with_label ("Crop (xmin,xmax,ymin,ymax):");
	info->dnj.koji_bmrt.cbcrop = cbutton;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
	gtk_signal_connect (GTK_OBJECT(cbutton),"toggled",GTK_SIGNAL_FUNC(dnj_flags_cbcrop_toggled),info);
	gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

	entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
	gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
	info->dnj.koji_bmrt.ecropxmin = entry;
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

	entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
	gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
	info->dnj.koji_bmrt.ecropxmax = entry;
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

	entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
	gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
	info->dnj.koji_bmrt.ecropymin = entry;
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

	entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
	gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
	info->dnj.koji_bmrt.ecropymax = entry;
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);
 
  /* Custom samples */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	cbutton = gtk_check_button_new_with_label ("Samples:");
	info->dnj.koji_bmrt.cbsamples = cbutton;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
	gtk_signal_connect (GTK_OBJECT(cbutton),"toggled",GTK_SIGNAL_FUNC(dnj_flags_cbsamples_toggled),info);
	gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);
	entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
	gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
	info->dnj.koji_bmrt.exsamples = entry;
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);
	entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
	gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
	info->dnj.koji_bmrt.eysamples = entry;
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Stats */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	cbutton = gtk_check_button_new_with_label ("Display Stats");
	info->dnj.koji_bmrt.cbstats = cbutton;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);
  /* Verbose */
	cbutton = gtk_check_button_new_with_label ("Verbose");
	info->dnj.koji_bmrt.cbverbose = cbutton;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);
  /* Beep */
	cbutton = gtk_check_button_new_with_label ("Beep");
	info->dnj.koji_bmrt.cbbeep = cbutton;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

	/* Radiosity samples */
	hbox = gtk_hbox_new (TRUE,2);
	gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
	cbutton = gtk_check_button_new_with_label ("Radiosity samples:");
	info->dnj.koji_bmrt.cbradiositysamples = cbutton;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(cbutton),FALSE);
	gtk_signal_connect (GTK_OBJECT(cbutton),"toggled",GTK_SIGNAL_FUNC(dnj_flags_cbradiositysamples_toggled),info);
	gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);
	entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
	gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
	info->dnj.koji_bmrt.eradiositysamples = entry;
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);
	/* Ray samples */
	cbutton = gtk_check_button_new_with_label ("Ray samples:");
	info->dnj.koji_bmrt.cbraysamples = cbutton;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(cbutton),FALSE);
	gtk_signal_connect (GTK_OBJECT(cbutton),"toggled",GTK_SIGNAL_FUNC(dnj_flags_cbraysamples_toggled),info);
	gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);
	entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
	gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
	info->dnj.koji_bmrt.eraysamples = entry;
	gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);
 	
  /* View command */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("View command:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_tooltips_set_tip(tooltips,entry,"Command that will be executed when you select 'Watch image' "
		       "in the frames list (inside the detailed job view)",NULL);
  info->dnj.koji_bmrt.eviewcmd = entry;
  gtk_entry_set_text(GTK_ENTRY(entry),KOJ_BMRT_DFLT_VIEWCMD);
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Script directory */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Script directory:");
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_tooltips_set_tip(tooltips,entry,"Directory in which, in case of using the automatic "
		       "script generator, the command script will be saved.",NULL);
  info->dnj.koji_bmrt.escript = entry;
  gtk_entry_set_text (GTK_ENTRY(entry),bmrtsg_default_script_path());
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for the script directory",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  gtk_signal_connect (GTK_OBJECT(button),"clicked",dnj_koj_frame_bmrt_script_search,&info->dnj.koji_bmrt);

  /* Buttons */
  /* Create script */
  bbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),bbox,TRUE,TRUE,5);
  gtk_widget_show (bbox);
  button = gtk_button_new_with_label ("Create Script");
  gtk_tooltips_set_tip(tooltips,button,"Create automagically the script based on the given information",NULL);
  gtk_box_pack_start (GTK_BOX(bbox),button,TRUE,TRUE,2);
  switch (info->dnj.koj) {
  case KOJ_BMRT:
    gtk_signal_connect (GTK_OBJECT(button),"clicked",
			dnj_koj_frame_bmrt_bcreate_pressed,&info->dnj);
    break;
  default:
    fprintf (stderr,"What ?!\n");
    break;
  }

  gtk_widget_show_all(frame);

  return frame;
}

void dnj_flags_cbcrop_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info)
{
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbcrop)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.ecropxmin),TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.ecropxmax),TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.ecropymin),TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.ecropymax),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.ecropxmin),FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.ecropxmax),FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.ecropymin),FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.ecropymax),FALSE);
  }
}

void dnj_flags_cbsamples_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info)
{
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbsamples)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.exsamples),TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.eysamples),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.exsamples),FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.eysamples),FALSE);
  }
}

void dnj_flags_cbradiositysamples_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info)
{
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbradiositysamples)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.eradiositysamples),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.eradiositysamples),FALSE);
  }
}

void dnj_flags_cbraysamples_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info)
{
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_bmrt.cbraysamples)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.eraysamples),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_bmrt.eraysamples),FALSE);
  }
}

GtkWidget *jdd_koj_bmrt_widgets (struct drqm_jobs_info *info)
{
  GtkWidget *table;
  GtkWidget *label;
  GtkAttachOptions options = GTK_EXPAND | GTK_SHRINK | GTK_FILL ;
  char *labels[] = { "Scene:", info->jdd.job.koji.bmrt.scene,
		     "View command:", info->jdd.job.koji.bmrt.viewcmd,
		     NULL };
  char **cur;
  int r,c;			/* Rows and columns */

  table = gtk_table_new (2,2, FALSE);

  cur = labels;
  r = 0;
  while ( *cur ) {
    c = 0;			/* First column */
    label = gtk_label_new (*cur);
    gtk_misc_set_alignment (GTK_MISC(label), 0, .5);
    gtk_table_attach (GTK_TABLE(table),GTK_WIDGET(label), c, c+1, r, r+1, options, options, 1 , 1 );
    cur++;			/* Next label */
    c++;			/* New column */
    label = gtk_label_new (*cur);
    gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
    gtk_misc_set_alignment (GTK_MISC(label), 1, .5);
    gtk_table_attach (GTK_TABLE(table),GTK_WIDGET(label), c, c+1, r, r+1, options, options, 1 , 1 );
    cur++;
    r++;			/* New row */
  }

  return table;
}

static void dnj_koj_frame_bmrt_scene_search (GtkWidget *button, struct drqmj_koji_bmrt *info)
{
  GtkWidget *dialog;

  dialog = gtk_file_selection_new ("Please select a scene file");
  info->fsscene = dialog;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(info->escene)))) {
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(dialog),gtk_entry_get_text(GTK_ENTRY(info->escene)));
  }

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (dnj_koj_frame_bmrt_scene_set), info);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->cancel_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_widget_show (dialog);
  gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

static void dnj_koj_frame_bmrt_scene_set (GtkWidget *button, struct drqmj_koji_bmrt *info)
{
  char buf[BUFFERLEN];
  char *p;
  
  strncpy(buf,gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fsscene)),BUFFERLEN-1);
  /* This removed the path part of the filename */
/*    p = strrchr(buf,'/'); */
/*    p = ( p ) ? p+1 : buf; */
  /* We need the whole scene path */
  p = buf;
  gtk_entry_set_text (GTK_ENTRY(info->escene),p);
}

static void dnj_koj_frame_bmrt_bcreate_pressed (GtkWidget *button, struct drqmj_dnji *info)
{
  struct bmrtsgi bmrtsgi;	/* Bmrt script generator info */
  char *file;

  strncpy (bmrtsgi.scene,gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.escene)),BUFFERLEN-1);
  strncpy (bmrtsgi.scriptdir,gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.escript)),BUFFERLEN-1);
	bmrtsgi.custom_crop = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbcrop)->active;
	if (bmrtsgi.custom_crop) {
		if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.ecropxmin)),"%u",&bmrtsgi.xmin) != 1)
			return;
		if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.ecropxmax)),"%u",&bmrtsgi.xmax) != 1)
			return;
		if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.ecropymin)),"%u",&bmrtsgi.ymin) != 1)
			return;
		if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.ecropymax)),"%u",&bmrtsgi.ymax) != 1)
			return;
	}
	bmrtsgi.custom_samples = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbsamples)->active;
	if (bmrtsgi.custom_samples) {
		if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.exsamples)),"%u",&bmrtsgi.xsamples) != 1)
			return;
		if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.eysamples)),"%u",&bmrtsgi.ysamples) != 1)
			return;
	}
	bmrtsgi.disp_stats = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbstats)->active;
	bmrtsgi.verbose = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbverbose)->active;
	bmrtsgi.custom_beep = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbbeep)->active;
	/* Custom radiosity */
	bmrtsgi.custom_radiosity = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbradiositysamples)->active;
	if (bmrtsgi.custom_radiosity) {
		if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.eradiositysamples)),"%u",&bmrtsgi.radiosity_samples) != 1)
			return;
	}
	/* Custom ray samples */
	bmrtsgi.custom_raysamples = GTK_TOGGLE_BUTTON(info->koji_bmrt.cbraysamples)->active;
	if (bmrtsgi.custom_raysamples) {
		if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_bmrt.eraysamples)),"%u",&bmrtsgi.raysamples) != 1)
			return;
	}
  
  if ((file = bmrtsg_create (&bmrtsgi)) == NULL) {
    fprintf (stderr,"ERROR: %s\n",drerrno_str());
    return;
  } else {
    gtk_entry_set_text(GTK_ENTRY(info->ecmd),file);
  } 
}

static void dnj_koj_frame_bmrt_script_search (GtkWidget *button, struct drqmj_koji_bmrt *info)
{
  GtkWidget *dialog;

  dialog = gtk_file_selection_new ("Please select a script directory");
  info->fsscript = dialog;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(info->escript)))) {
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(dialog),gtk_entry_get_text(GTK_ENTRY(info->escript)));
  }

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (dnj_koj_frame_bmrt_script_set), info);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->cancel_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_widget_show (dialog);
  gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
}

static void dnj_koj_frame_bmrt_script_set (GtkWidget *button, struct drqmj_koji_bmrt *info)
{
  char buf[BUFFERLEN];
  char *p;
  
  strncpy(buf,gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fsscript)),BUFFERLEN-1);
  p = strrchr(buf,'/');
  if (p)
    *p = 0;
  gtk_entry_set_text (GTK_ENTRY(info->escript),buf);
}
