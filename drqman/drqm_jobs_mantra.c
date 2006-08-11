//
// Copyright (C) 2001,2002,2003,2004,2005 Jorge Daza Garcia-Blanes
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
// $Id: drqm_jobs_aqsis.c 1330 2005-07-05 03:50:01Z jorge $
//

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mantrasg.h"
#include "drqm_jobs.h"
#include "drqm_common.h"
#include "drqm_jobs_mantra.h"

static void dnj_koj_frame_mantra_script_search (GtkWidget *button, struct drqmj_koji_mantra *info);
static void dnj_koj_frame_mantra_script_set (GtkWidget *button, struct drqmj_koji_mantra *info);
static void dnj_koj_frame_mantra_scene_search (GtkWidget *button, struct drqmj_koji_mantra *info);
static void dnj_koj_frame_mantra_scene_set (GtkWidget *button, struct drqmj_koji_mantra *info);
static void dnj_koj_frame_mantra_bcreate_pressed (GtkWidget *button, struct drqmj_dnji *info);
static void dnj_koj_frame_mantra_renderdir_search (GtkWidget *button, struct drqmj_koji_mantra *info);
static void dnj_koj_frame_mantra_renderdir_set (GtkWidget *button, struct drqmj_koji_mantra *info);

static void dnj_koj_frame_mantra_cbbucket_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_koj_frame_mantra_cblod_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_koj_frame_mantra_cbvaryaa_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);

static void dnj_koj_frame_mantra_cbbd_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_koj_frame_mantra_cbzDepth_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_koj_frame_mantra_cbCracks_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);

static void dnj_koj_frame_mantra_cbQuality_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_koj_frame_mantra_cbQFiner_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_koj_frame_mantra_cbSMultiplier_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);

static void dnj_koj_frame_mantra_cbMPCache_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_koj_frame_mantra_cbMCache_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_koj_frame_mantra_cbSMPolygon_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);

static void dnj_flags_cbWH_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);
static void dnj_koj_frame_mantra_cbType_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);

//static void dnj_koj_frame_mantra_cbVerbose_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info);

GtkWidget *dnj_koj_frame_mantra (struct drqm_jobs_info *info) {
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox,*hbox2;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
  GtkWidget *bbox;
  GtkWidget *cbutton;
  GtkTooltips *tooltips;

  struct passwd *pw;

  tooltips = TooltipsNew ();

  /* Frame */
  frame = gtk_frame_new ("Mantra job information");

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
  info->dnj.koji_mantra.escene = entry;
  gtk_tooltips_set_tip(tooltips,entry,"File name of the mantra scene file that should be rendered",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for the mantra scene file",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
                    G_CALLBACK(dnj_koj_frame_mantra_scene_search),&info->dnj.koji_mantra);

  /* Render directory */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Render directory:");
  gtk_box_pack_start (GTK_BOX(hbox),label,TRUE,TRUE,2);
  hbox2 = gtk_hbox_new (FALSE,0);
  gtk_box_pack_start (GTK_BOX(hbox),hbox2,TRUE,TRUE,0);
  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_tooltips_set_tip(tooltips,entry,"Directory where the images should be stored",NULL);
  info->dnj.koji_mantra.erenderdir = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Folder name of the mantra scene file that should be send to",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for the mantra render directory",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
                    G_CALLBACK(dnj_koj_frame_mantra_renderdir_search),&info->dnj.koji_mantra);

  // Raytracing Mode ?
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  cbutton = gtk_check_button_new_with_label ("Raytracing Mode");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,FALSE,FALSE,2);
  gtk_tooltips_set_tip(tooltips,cbutton,"Should we render without micro polygon ?",NULL);
  info->dnj.koji_mantra.cbraytrace = cbutton;
  // Antialias ?
  cbutton = gtk_check_button_new_with_label ("Antialias off");
  gtk_box_pack_start (GTK_BOX(hbox),cbutton,FALSE,FALSE,2);
  gtk_tooltips_set_tip(tooltips,cbutton,"Should we render without anti-aliasing ?",NULL);
  info->dnj.koji_mantra.cbaaoff = cbutton;

  /* Custom bucketsize */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  cbutton = gtk_check_button_new_with_label ("BucketSize:");
  info->dnj.koji_mantra.cbbucket = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbbucket_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.ebucket = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify bucket size for render (eg. 64)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Level of Details */
  cbutton = gtk_check_button_new_with_label ("Level of Details:");
  info->dnj.koji_mantra.cblod = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cblod_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.elod = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Global level of detail factor (ray-traced shading rate)(eg. 1)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* variance of antialias */
  cbutton = gtk_check_button_new_with_label ("Variance of Anti-Alias:");
  info->dnj.koji_mantra.cbvaryaa = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbvaryaa_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.evaryaa = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify variance anti-aliasing threshold (eg. 0.015)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* bit of depth */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  cbutton = gtk_check_button_new_with_label ("Bit-Depth:");
  info->dnj.koji_mantra.cbbd = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbbd_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.ebd = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify bit-depth of output image (color image only)(eg. 0)\n"
                       "0 - Use the 'natural' bit depth\n"
                       "8 - Generate 8 bits per color channel\n"
                       "16 - Generate 16 bits per color channel\n"
                       "32 - Floating point data per color channel",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Z depth image */
  cbutton = gtk_check_button_new_with_label ("Z-Depth Image:");
  info->dnj.koji_mantra.cbzDepth = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbzDepth_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.ezDepth = entry;
  gtk_tooltips_set_tip(tooltips,entry,"z-depth image (eg. average)\n"
                       "\"average\" - average z-depth\n"
                       "\"closest\" - closest z-depth",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Cracks */
  cbutton = gtk_check_button_new_with_label ("Cracks:");
  info->dnj.koji_mantra.cbCracks = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbCracks_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eCracks = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Set coving options for dealing with patch cracks (eg. 0)\n"
                       "0 - No coving of patch cracks\n"
                       "1 - Cove displaced and sub-division surfaces\n"
                       "2 - Forced coving of all primitives",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Quality */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  cbutton = gtk_check_button_new_with_label ("Quality:");
  info->dnj.koji_mantra.cbQuality = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbQuality_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eQuality = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Set render quality (0 to 10)(eg. 0)\n"
                       "Below 9 - turns off motion blur\n"
                       "Below 8 - turns off depth of field\n"
                       "Below 5 - turns off ray tracing",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Quality Finer */
  cbutton = gtk_check_button_new_with_label ("Quality Finer:");
  info->dnj.koji_mantra.cbQFiner = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbQFiner_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eQFiner = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Finer control over render quality (eg. 'b' or 'bdr')\n"
                       "If the arg contains a 'b' - motion blur is turned off\n"
                       "If it contains a 'd' - depth of field is turned off\n"
                       "If it contains a 'r' - all ray tracing is turned off\n"
                       "If it contains a 'i' - all irradiance/occlusion is turned off",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Shading Multiplier */
  cbutton = gtk_check_button_new_with_label ("Shading Multiplier:");
  info->dnj.koji_mantra.cbSMultiplier = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbSMultiplier_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eSMultiplier = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify shading quality multiplier (eg. 1)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Multi-Polygon Cache */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  cbutton = gtk_check_button_new_with_label ("Multi-Polygon Cache:");
  info->dnj.koji_mantra.cbMPCache = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbMPCache_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eMPCache = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify micro-polygon cache size (eg. 4096)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Mesh Cache */
  cbutton = gtk_check_button_new_with_label ("Mesh Cache:");
  info->dnj.koji_mantra.cbMCache = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbMCache_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eMCache = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify ray mesh cache size (eg. 1024)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Split Micro-Polygon */
  cbutton = gtk_check_button_new_with_label ("Split Micro-Polygon:");
  info->dnj.koji_mantra.cbSMPolygon = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbSMPolygon_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eSMPolygon = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify micro-polygon maximum splits (eg. 10)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Custom width and height for output image */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  cbutton = gtk_check_button_new_with_label ("Output Image Size (width,height):");
  info->dnj.koji_mantra.cbWH = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_flags_cbWH_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eWidth = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify width of output image (eg. 720)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eHeight = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify height of output image (eg. 574)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* Output Type */
  cbutton = gtk_check_button_new_with_label ("Output Type:");
  info->dnj.koji_mantra.cbType = cbutton;
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton),FALSE);
  g_signal_connect (G_OBJECT(cbutton),"toggled",G_CALLBACK(dnj_koj_frame_mantra_cbType_toggled),info);
  gtk_box_pack_start(GTK_BOX(hbox),cbutton,FALSE,FALSE,2);

  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
  info->dnj.koji_mantra.eType = entry;
  gtk_tooltips_set_tip(tooltips,entry,"Specify type output image (eg. exr or tga)",NULL);
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,2);

  /* File Owner */
  hbox = gtk_hbox_new (TRUE,2);
  gtk_box_pack_start (GTK_BOX(vbox),hbox,FALSE,FALSE,2);
  label = gtk_label_new ("Owner of rendered files:");
  gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,2);
  entry = gtk_entry_new_with_max_length (BUFFERLEN-1);
  gtk_tooltips_set_tip(tooltips,entry,"After rendering the ownership of the "
                       "rendered files will be changed to this. By default it "
                       "is the same as the owner of the job",NULL);
  info->dnj.koji_mantra.efile_owner = entry;
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
  info->dnj.koji_mantra.eviewcmd = entry;
  gtk_entry_set_text(GTK_ENTRY(entry),KOJ_MANTRA_DFLT_VIEWCMD);
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
  info->dnj.koji_mantra.escript = entry;
  gtk_entry_set_text (GTK_ENTRY(entry),mantrasg_default_script_path());
  gtk_box_pack_start (GTK_BOX(hbox2),entry,TRUE,TRUE,2);
  button = gtk_button_new_with_label ("Search");
  gtk_tooltips_set_tip(tooltips,button,"File selector for the script directory",NULL);
  gtk_box_pack_start (GTK_BOX(hbox2),button,FALSE,FALSE,2);
  g_signal_connect (G_OBJECT(button),"clicked",
                    G_CALLBACK(dnj_koj_frame_mantra_script_search),&info->dnj.koji_mantra);

  /* Buttons */
  /* Create script */
  bbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX(vbox),bbox,TRUE,TRUE,5);
  gtk_widget_show (bbox);
  button = gtk_button_new_with_label ("Create Script");
  gtk_tooltips_set_tip(tooltips,button,"Create automagically the script based on the given information",NULL);
  gtk_box_pack_start (GTK_BOX(bbox),button,TRUE,TRUE,2);
  switch (info->dnj.koj) {
  case KOJ_MANTRA:
    g_signal_connect (G_OBJECT(button),"clicked", G_CALLBACK(dnj_koj_frame_mantra_bcreate_pressed),&info->dnj);
    break;
  default:
    fprintf (stderr,"What ?!\n");
    break;
  }

  gtk_widget_show_all(frame);

  return frame;
}

void dnj_koj_frame_mantra_cbbucket_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbbucket)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.ebucket),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.ebucket),FALSE);
  }
}

void dnj_koj_frame_mantra_cblod_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cblod)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.elod),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.elod),FALSE);
  }
}

void dnj_koj_frame_mantra_cbvaryaa_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbvaryaa)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.evaryaa),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.evaryaa),FALSE);
  }
}

void dnj_koj_frame_mantra_cbbd_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbbd)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.ebd),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.ebd),FALSE);
  }
}

void dnj_koj_frame_mantra_cbzDepth_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbzDepth)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.ezDepth),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.ezDepth),FALSE);
  }
}

void dnj_koj_frame_mantra_cbCracks_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbCracks)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eCracks),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eCracks),FALSE);
  }
}

void dnj_koj_frame_mantra_cbQuality_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbQuality)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eQuality),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eQuality),FALSE);
  }
}

void dnj_koj_frame_mantra_cbQFiner_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbQFiner)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eQFiner),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eQFiner),FALSE);
  }
}

void dnj_koj_frame_mantra_cbSMultiplier_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbSMultiplier)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eSMultiplier),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eSMultiplier),FALSE);
  }
}

void dnj_koj_frame_mantra_cbMPCache_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbMPCache)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eMPCache),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eMPCache),FALSE);
  }
}

void dnj_koj_frame_mantra_cbMCache_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbMCache)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eMCache),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eMCache),FALSE);
  }
}

void dnj_koj_frame_mantra_cbSMPolygon_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbSMPolygon)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eSMPolygon),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eSMPolygon),FALSE);
  }
}

void dnj_flags_cbWH_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbWH)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eWidth),TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eHeight),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eWidth),FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eHeight),FALSE);
  }
}

void dnj_koj_frame_mantra_cbType_toggled (GtkWidget *cbutton, struct drqm_jobs_info *info) {
  if (GTK_TOGGLE_BUTTON(info->dnj.koji_mantra.cbType)->active) {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eType),TRUE);
  } else {
    gtk_widget_set_sensitive (GTK_WIDGET(info->dnj.koji_mantra.eType),FALSE);
  }
}

GtkWidget *jdd_koj_mantra_widgets (struct drqm_jobs_info *info) {
  GtkWidget *table;
  GtkWidget *label;
  GtkAttachOptions options = GTK_EXPAND | GTK_SHRINK | GTK_FILL ;
  char *labels[] = { "Scene:", info->jdd.job.koji.mantra.scene,
                     "Render directory:", info->jdd.job.koji.mantra.renderdir,
                     "View command:", info->jdd.job.koji.mantra.viewcmd,
                     NULL };
  char **cur;
  int r,c;   /* Rows and columns */

  table = gtk_table_new (4,2, FALSE);

  cur = labels;
  r = 0;
  while ( *cur ) {
    c = 0;   /* First column */
    label = gtk_label_new (*cur);
    gtk_misc_set_alignment (GTK_MISC(label), 0, .5);
    gtk_table_attach (GTK_TABLE(table),GTK_WIDGET(label), c, c+1, r, r+1, options, options, 1 , 1 );
    cur++;   /* Next label */
    c++;   /* New column */
    label = gtk_label_new (*cur);
    gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
    gtk_misc_set_alignment (GTK_MISC(label), 1, .5);
    gtk_table_attach (GTK_TABLE(table),GTK_WIDGET(label), c, c+1, r, r+1, options, options, 1 , 1 );
    cur++;
    r++;   /* New row */
  }

  return table;
}

static void dnj_koj_frame_mantra_scene_search (GtkWidget *button, struct drqmj_koji_mantra *info) {
  GtkWidget *dialog;

#ifndef __CYGWIN

  dialog = gtk_file_selection_new ("Please select a scene file");
  info->fsscene = dialog;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(info->escene)))) {
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(dialog),gtk_entry_get_text(GTK_ENTRY(info->escene)));
  }

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
                      "clicked", GTK_SIGNAL_FUNC (dnj_koj_frame_mantra_scene_set), info);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (gpointer) dialog);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->cancel_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (gpointer) dialog);
  gtk_widget_show (dialog);
  gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
#else

  gtk_entry_set_text (GTK_ENTRY(info->escene), cygwin_file_dialog(NULL, NULL, NULL, 0));
#endif
}

static void dnj_koj_frame_mantra_scene_set (GtkWidget *button, struct drqmj_koji_mantra *info) {
  char buf[BUFFERLEN];

  strncpy(buf,gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fsscene)),BUFFERLEN-1);
  gtk_entry_set_text (GTK_ENTRY(info->escene),buf);
}

static void dnj_koj_frame_mantra_renderdir_search (GtkWidget *button, struct drqmj_koji_mantra *info) {
  GtkWidget *dialog;
  char dir[BUFFERLEN];

#ifndef __CYGWIN

  dialog = gtk_file_selection_new ("Please select the output directory");
  info->fsrenderdir = dialog;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(info->erenderdir)))) {
    strncpy (dir,gtk_entry_get_text(GTK_ENTRY(info->erenderdir)),BUFFERLEN-1);
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(dialog),strcat(dir,"/"));
  }

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
                      "clicked", GTK_SIGNAL_FUNC (dnj_koj_frame_mantra_renderdir_set), info);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (gpointer) dialog);
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->cancel_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (gpointer) dialog);
  gtk_widget_show (dialog);
  gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
#else

  gtk_entry_set_text (GTK_ENTRY(info->erenderdir), cygwin_dir_dialog(NULL));
#endif

}

static void dnj_koj_frame_mantra_renderdir_set (GtkWidget *button, struct drqmj_koji_mantra *info) {
  struct stat s;
  char buf[BUFFERLEN];
  char *p;

  strncpy(buf,gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fsrenderdir)),BUFFERLEN-1);
  stat(buf, &s);
  if (!S_ISDIR(s.st_mode)) {
    p = strrchr(buf,'/');
    if (p)
      *p = 0;
  }
  gtk_entry_set_text (GTK_ENTRY(info->erenderdir),buf);
}

static void dnj_koj_frame_mantra_bcreate_pressed (GtkWidget *button, struct drqmj_dnji *info) {
  struct mantrasgi mantrasgi; /* Mantra script generator info */
  char *file;

  mantrasgi.raytrace = GTK_TOGGLE_BUTTON(info->koji_mantra.cbraytrace)->active;
  mantrasgi.aaoff = GTK_TOGGLE_BUTTON(info->koji_mantra.cbaaoff)->active;

  strncpy (mantrasgi.scene,gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.escene)),BUFFERLEN-1);
  strncpy (mantrasgi.renderdir,gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.erenderdir)),BUFFERLEN-1);
  strncpy (mantrasgi.scriptdir,gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.escript)),BUFFERLEN-1);

  /* Z Depth Image */
  mantrasgi.custom_zDepth = GTK_TOGGLE_BUTTON(info->koji_mantra.cbzDepth)->active;
  if (mantrasgi.custom_zDepth) {
    strncpy (mantrasgi.zDepth,gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.ezDepth)),BUFFERLEN-1);
  }

  /* Quality Finer */
  mantrasgi.custom_QFiner = GTK_TOGGLE_BUTTON(info->koji_mantra.cbQFiner)->active;
  if (mantrasgi.custom_QFiner) {
    strncpy (mantrasgi.QFiner,gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eQFiner)),BUFFERLEN-1);
  }

  mantrasgi.custom_bucket = GTK_TOGGLE_BUTTON(info->koji_mantra.cbbucket)->active;
  if (mantrasgi.custom_bucket) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.ebucket)),"%u",&mantrasgi.bucketSize) != 1)
      return;
  }

  /* Level of detail */
  mantrasgi.custom_lod = GTK_TOGGLE_BUTTON(info->koji_mantra.cblod)->active;
  if (mantrasgi.custom_lod) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.elod)),"%u",&mantrasgi.LOD) != 1)
      return;
  }

  /* Variance of Anti-Alias */
  mantrasgi.custom_varyaa = GTK_TOGGLE_BUTTON(info->koji_mantra.cbvaryaa)->active;
  if (mantrasgi.custom_varyaa) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.evaryaa)),"%f",&mantrasgi.varyAA) != 1)
      return;
  }

  /* Bit Depth */
  mantrasgi.custom_bDepth = GTK_TOGGLE_BUTTON(info->koji_mantra.cbbd)->active;
  if (mantrasgi.custom_bDepth) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.ebd)),"%u",&mantrasgi.bDepth) != 1)
      return;
  }

  /* Cracks */
  mantrasgi.custom_Cracks = GTK_TOGGLE_BUTTON(info->koji_mantra.cbCracks)->active;
  if (mantrasgi.custom_Cracks) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eCracks)),"%u",&mantrasgi.Cracks) != 1)
      return;
  }

  /* Quality */
  mantrasgi.custom_Quality = GTK_TOGGLE_BUTTON(info->koji_mantra.cbQuality)->active;
  if (mantrasgi.custom_Quality) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eQuality)),"%u",&mantrasgi.Quality) != 1)
      return;
  }

  /* Shading Multiplier */
  mantrasgi.custom_SMultiplier = GTK_TOGGLE_BUTTON(info->koji_mantra.cbSMultiplier)->active;
  if (mantrasgi.custom_SMultiplier) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eSMultiplier)),"%u",&mantrasgi.SMultiplier) != 1)
      return;
  }

  /* Multi-Polygon Cache */
  mantrasgi.custom_MPCache = GTK_TOGGLE_BUTTON(info->koji_mantra.cbMPCache)->active;
  if (mantrasgi.custom_MPCache) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eMPCache)),"%u",&mantrasgi.MPCache) != 1)
      return;
  }

  /* Mesh Cache */
  mantrasgi.custom_MCache = GTK_TOGGLE_BUTTON(info->koji_mantra.cbMCache)->active;
  if (mantrasgi.custom_MCache) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eMCache)),"%u",&mantrasgi.MCache) != 1)
      return;
  }

  /* Split Micro-Polygon */
  mantrasgi.custom_SMPolygon = GTK_TOGGLE_BUTTON(info->koji_mantra.cbSMPolygon)->active;
  if (mantrasgi.custom_SMPolygon) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eSMPolygon)),"%u",&mantrasgi.SMPolygon) != 1)
      return;
  }

  /* Width and Height for output image */
  mantrasgi.custom_WH = GTK_TOGGLE_BUTTON(info->koji_mantra.cbWH)->active;
  if (mantrasgi.custom_WH) {
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eWidth)),"%u",&mantrasgi.Width) != 1)
      return;
    if (sscanf(gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eHeight)),"%u",&mantrasgi.Height) != 1)
      return;
  }

  /* Output Type */
  mantrasgi.custom_Type = GTK_TOGGLE_BUTTON(info->koji_mantra.cbType)->active;
  if (mantrasgi.custom_Type) {
    strncpy (mantrasgi.Type,gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.eType)),BUFFERLEN-1);
  }

  strncpy (mantrasgi.file_owner,gtk_entry_get_text(GTK_ENTRY(info->koji_mantra.efile_owner)),BUFFERLEN-1);

  if ((file = mantrasg_create (&mantrasgi)) == NULL) {
    fprintf (stderr,"ERROR: %s\n",drerrno_str());
    return;
  } else {
    gtk_entry_set_text(GTK_ENTRY(info->ecmd),file);
  }
}

static void dnj_koj_frame_mantra_script_search (GtkWidget *button, struct drqmj_koji_mantra *info) {
  GtkWidget *dialog;

#ifndef __CYGWIN

  dialog = gtk_file_selection_new ("Please select a script directory");
  info->fsscript = dialog;

  if (strlen(gtk_entry_get_text(GTK_ENTRY(info->escript)))) {
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(dialog),gtk_entry_get_text(GTK_ENTRY(info->escript)));
  }

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dialog)->ok_button),
                      "clicked", GTK_SIGNAL_FUNC (dnj_koj_frame_mantra_script_set), info);
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

static void dnj_koj_frame_mantra_script_set (GtkWidget *button, struct drqmj_koji_mantra *info) {
  char buf[BUFFERLEN];
  char *p;

  strncpy(buf,gtk_file_selection_get_filename(GTK_FILE_SELECTION(info->fsscript)),BUFFERLEN-1);
  p = strrchr(buf,'/');
  if (p)
    *p = 0;
  gtk_entry_set_text (GTK_ENTRY(info->escript),buf);
}
