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
/* 
 * $Id$ 
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "drqm_jobs.h"
#include "drqm_common.h"
#include "drqm_jobs_terragen.h"

#ifdef __CYGWIN
#include "drqm_cygwin.h"
#endif

static void dnj_koj_frame_terragen_worldfile_search (GtkWidget *button, struct drqmj_koji_terragen *info);
static void dnj_koj_frame_terragen_worldfile_set (GtkWidget *button, struct drqmj_koji_terragen *info);
static void dnj_koj_frame_terragen_terrainfile_search (GtkWidget *button, struct drqmj_koji_terragen *info);
static void dnj_koj_frame_terragen_terrainfile_set (GtkWidget *button, struct drqmj_koji_terragen *info);
static void dnj_koj_frame_terragen_script_search (GtkWidget *button, struct drqmj_koji_terragen *info);
static void dnj_koj_frame_terragen_script_set (GtkWidget *button, struct drqmj_koji_terragen *info);
static void dnj_koj_frame_terragen_scriptfile_search (GtkWidget *button, struct drqmj_koji_terragen *info);
static void dnj_koj_frame_terragen_scriptfile_set (GtkWidget *button, struct drqmj_koji_terragen *info);
static void dnj_koj_frame_terragen_bcreate_pressed (GtkWidget *button, struct drqmj_dnji *info);

GtkWidget *dnj_koj_frame_terragen (struct drqm_jobs_info *info)
{
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox,*hbox2;
  GtkWidget *label;
  GtkWidget *entry; 
  GtkWidget *button;
  GtkWidget *bbox;
  GtkTooltips *tooltips;
  struct passwd *pw;

  tooltips = TooltipsNew ();

  /* Frame */
  frame = gtk_frame_new ("Terragen job information");

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
  info->dnj.koji_terragen.escriptfile = entry;
  gtk_tooltips_set_tip(tooltips,entry,"File name of the terragen scene file that should be rendered",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for the terragen scene file",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(dnj_koj_frame_terragen_scriptfile_search),&info->dnj.koji_terragen);

  /* Worldfile */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("World File:");
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
//  gtk_tooltips_set_tip(tooltips,entry,"World File",NULL);
  info->dnj.koji_terragen.eworldfile = entry;
  gtk_tooltips_set_tip(tooltips,entry,"File name of the terragen world file",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for the terragen worldfile",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(dnj_koj_frame_terragen_worldfile_search),&info->dnj.koji_terragen);

  /* Terrainfile directory */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Terrain File:");
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
//  gtk_tooltips_set_tip(tooltips,entry,"Terragen Terrain File",NULL);
  info->dnj.koji_terragen.eterrainfile = entry;
  gtk_tooltips_set_tip(tooltips,entry,"File name of the terragen terrain file",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for the terragen terrain",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(dnj_koj_frame_terragen_terrainfile_search),&info->dnj.koji_terragen);

  /* File Owner */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Owner of rendered files:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_tooltips_set_tip(tooltips,entry,"After rendering the ownership of the "
		       "rendered files will be changed to this. By default it "
		       "is the same as the owner of the job",NULL);
  info->dnj.koji_terragen.efile_owner = entry;
  if (!(pw = getpwuid(geteuid()))) {
    gtk_entry_set_text(GTK_ENTRY(entry),"ERROR");
  } else {
    gtk_entry_set_text(GTK_ENTRY(entry),pw->pw_name);
  }
  gtk_box_pack_start (GTK_BOX(hbox),entry,TRUE,TRUE,2);


  /* View command */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("View command:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_tooltips_set_tip(tooltips,entry,"Command that will be executed when you select 'Watch image' "
		       "in the frames list (inside the detailed job view)",NULL);
  info->dnj.koji_terragen.eviewcmd = entry;
  gtk_entry_set_text(GTK_ENTRY(entry),KOJ_TERRAGEN_DFLT_VIEWCMD);
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
  info->dnj.koji_terragen.escript = entry;
  gtk_entry_set_text (GTK_ENTRY(entry),terragensg_default_script_path());
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for the script directory",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
										G_CALLBACK(dnj_koj_frame_terragen_script_search),&info->dnj.koji_terragen);

  /* Buttons */
  /* Create script */
  bbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),bbox,TRUE,TRUE,5);
  gtk_widget_show (bbox);
  button = gtk_button_new_with_label ("Create Script");
  gtk_tooltips_set_tip(tooltips,button,"Create automagically the script based on the given information",NULL);
  gtk_box_pack_start (GTK_BOX(bbox),button,TRUE,TRUE,2);
  switch (info->dnj.koj) {
  case KOJ_TERRAGEN:
    g_signal_connect (G_OBJECT(button),"clicked",
											G_CALLBACK(dnj_koj_frame_terragen_bcreate_pressed),&info->dnj);
    break;
  default:
    fprintf (stderr,"What ?!\n");
    break;
  }

  gtk_widget_show_all(frame);

  return frame;
}

GtkWidget *jdd_koj_terragen_widgets (struct drqm_jobs_info *info)
{
  GtkWidget *table;
  GtkWidget *label;
  GtkAttachOptions options = GTK_EXPAND | GTK_SHRINK | GTK_FILL ;
  char *labels[] = { "Terragen Script:", info->jdd.job.koji.terragen.scriptfile,
		     "World File:", info->jdd.job.koji.terragen.worldfile,
		     "Terrain File:", info->jdd.job.koji.terragen.terrainfile,
		     "View command:", info->jdd.job.koji.terragen.viewcmd,
		     NULL };
  char **cur;
  int r,c;			/* Rows and columns */

  table = gtk_table_new (4,2, FALSE);

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

static void dnj_koj_frame_terragen_worldfile_search (GtkWidget *button, struct drqmj_koji_terragen *info)
{
  GtkWidget *dialog;
  char dir[BUFFERLEN];

#ifndef __CYGWIN
  dialog = gtk_file_selection_new ("Please select the Terragen Worldfile");
  info->fsworldfile = dialog;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(info->eworldfile)))) {
    strncpy (dir,gtk_entry_get_text(GTK_ENTRY(info->eworldfile)),BUFFERLEN-1);
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(dialog),strcat(dir,"/"));
  }

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (dnj_koj_frame_terragen_worldfile_set), info);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->cancel_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_widget_show (dialog);
  gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
#else
  gtk_entry_set_text (GTK_ENTRY(info->eworldfile), cygwin_file_dialog(NULL, NULL, NULL, 0));
#endif

}


static void dnj_koj_frame_terragen_worldfile_set (GtkWidget *button, struct drqmj_koji_terragen *info)
{
  struct stat s;
  char buf[BUFFERLEN];
  char *p;
  
  strncpy(buf,gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fsworldfile)),BUFFERLEN-1);
  stat(buf, &s);
  if (!S_ISDIR(s.st_mode)) {
    p = strrchr(buf,'/');
    if (p)
      *p = 0;
  }
  gtk_entry_set_text (GTK_ENTRY(info->eworldfile),buf);
}

static void dnj_koj_frame_terragen_terrainfile_search (GtkWidget *button, struct drqmj_koji_terragen *info)
{
  GtkWidget *dialog;
  char dir[BUFFERLEN];

#ifndef __CYGWIN
  dialog = gtk_file_selection_new ("Please select the Terragen Terrain File");
  info->fsterrainfile = dialog;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(info->eterrainfile)))) {
    strncpy (dir,gtk_entry_get_text(GTK_ENTRY(info->eterrainfile)),BUFFERLEN-1);
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(dialog),strcat(dir,"/"));
  }

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (dnj_koj_frame_terragen_terrainfile_set), info);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->cancel_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_widget_show (dialog);
  gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
#else
  gtk_entry_set_text (GTK_ENTRY(info->eterrainfile), cygwin_file_dialog(NULL, NULL, NULL, 0));
#endif

}


static void dnj_koj_frame_terragen_terrainfile_set (GtkWidget *button, struct drqmj_koji_terragen *info)
{
  struct stat s;
  char buf[BUFFERLEN];
  char *p;
  
  strncpy(buf,gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fsterrainfile)),BUFFERLEN-1);
  stat(buf, &s);
  if (!S_ISDIR(s.st_mode)) {
    p = strrchr(buf,'/');
    if (p)
      *p = 0;
  }
  gtk_entry_set_text (GTK_ENTRY(info->eterrainfile),buf);
}

static void dnj_koj_frame_terragen_scriptfile_search (GtkWidget *button, struct drqmj_koji_terragen *info)
{
  GtkWidget *dialog;

#ifndef __CYGWIN
  dialog = gtk_file_selection_new ("Please select a Terragen Scriptfile");
  info->fsscriptfile = dialog;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(info->escriptfile)))) {
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(dialog),gtk_entry_get_text(GTK_ENTRY(info->escriptfile)));
  }

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (dnj_koj_frame_terragen_scriptfile_set), info);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->cancel_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_widget_show (dialog);
  gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
#else
  gtk_entry_set_text (GTK_ENTRY(info->escriptfile), cygwin_file_dialog(NULL, NULL, NULL, 0));
#endif
}

static void dnj_koj_frame_terragen_scriptfile_set (GtkWidget *button, struct drqmj_koji_terragen *info)
{
  char buf[BUFFERLEN];
  char *p;
  
  strncpy(buf,gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fsscriptfile)),BUFFERLEN-1);
  /* This removed the path part of the filename */
/*    p = strrchr(buf,'/'); */
/*    p = ( p ) ? p+1 : buf; */
  /* We need the whole scene path */
  p = buf;
  gtk_entry_set_text (GTK_ENTRY(info->escriptfile),p);
}

static void dnj_koj_frame_terragen_bcreate_pressed (GtkWidget *button, struct drqmj_dnji *info)
{
  struct terragensgi terragensgi;	/* Terragen script generator info */
  char *file;

  strncpy (terragensgi.worldfile,gtk_entry_get_text(GTK_ENTRY(info->koji_terragen.eworldfile)),BUFFERLEN-1);
  strncpy (terragensgi.terrainfile,gtk_entry_get_text(GTK_ENTRY(info->koji_terragen.eterrainfile)),BUFFERLEN-1);
  strncpy (terragensgi.scriptfile,gtk_entry_get_text(GTK_ENTRY(info->koji_terragen.escriptfile)),BUFFERLEN-1);
  strncpy (terragensgi.scriptdir,gtk_entry_get_text(GTK_ENTRY(info->koji_terragen.escript)),BUFFERLEN-1);
  strncpy (terragensgi.file_owner,gtk_entry_get_text(GTK_ENTRY(info->koji_terragen.efile_owner)),BUFFERLEN-1);
  strncpy (terragensgi.camera,"",BUFFERLEN-1);
  terragensgi.res_x = terragensgi.res_y = -1;
  strncpy (terragensgi.format,"",BUFFERLEN-1);

  if ((file = terragensg_create (&terragensgi)) == NULL) {
    fprintf (stderr,"ERROR: %s\n",drerrno_str());
    return;
  } else {
    gtk_entry_set_text(GTK_ENTRY(info->ecmd),file);
  } 
}

static void dnj_koj_frame_terragen_script_search (GtkWidget *button, struct drqmj_koji_terragen *info)
{
  GtkWidget *dialog;

#ifndef __CYGWIN
  dialog = gtk_file_selection_new ("Please select a script directory");
  info->fsscript = dialog;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(info->escript)))) {
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(dialog),gtk_entry_get_text(GTK_ENTRY(info->escript)));
  }

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (dnj_koj_frame_terragen_script_set), info);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->cancel_button),
			     "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);
  gtk_widget_show (dialog);
  gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
#else
  gtk_entry_set_text (GTK_ENTRY(info->escript), cygwin_dir_dialog(NULL));
#endif

}

static void dnj_koj_frame_terragen_script_set (GtkWidget *button, struct drqmj_koji_terragen *info)
{
  char buf[BUFFERLEN];
  char *p;
  
  strncpy(buf,gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fsscript)),BUFFERLEN-1);
  p = strrchr(buf,'/');
  if (p)
    *p = 0;
  gtk_entry_set_text (GTK_ENTRY(info->escript),buf);
}
