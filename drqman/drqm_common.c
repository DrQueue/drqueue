/*
 * $Id: drqm_common.c,v 1.2 2001/09/24 16:10:19 jorge Exp $
 */

#include <gtk/gtk.h>

#include "drqm_common.h"

GtkWidget *ConfirmDialog (char *text, GList *callbacks, gpointer data)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;

  /* Dialog */
  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(dialog),"You Sure?");
  gtk_signal_connect_object(GTK_OBJECT(dialog),"destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (gpointer)dialog);

  /* Label */
  label = gtk_label_new (text);
  gtk_misc_set_padding (GTK_MISC(label), 10, 10);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,TRUE,5);
 
  /* Buttons */
  button = gtk_button_new_with_label ("Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  for (;callbacks;callbacks = callbacks->next) {
    gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(callbacks->data),data);
  }
  gtk_signal_connect_object(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)dialog);

  button = gtk_button_new_with_label ("No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  gtk_signal_connect_object(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)dialog);
  GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);

  gtk_widget_show_all (dialog);

  return dialog;
}

GtkTooltips *TooltipsNew (void)
{
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_delay (tooltips,TOOLTIPS_DELAY);

  return tooltips;
}
