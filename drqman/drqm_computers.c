/*
 * $Id: drqm_computers.c,v 1.2 2001/07/19 10:23:44 jorge Exp $
 */

#include <string.h>

#include "drqm_request.h"
#include "drqm_computers.h"

/* Global variable */
static struct info_drqm_computers info;

/* Static functions declaration */
static GtkWidget *CreateComputersList(struct info_drqm_computers *info);
static GtkWidget *CreateClist (GtkWidget *window);
static GtkWidget *CreateButtonRefresh (struct info_drqm_computers *info);
static void PressedButtonRefresh (GtkWidget *b, struct info_drqm_computers *info);
static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct info_drqm_computers *info);
static GtkWidget *CreateMenu (struct info_drqm_computers *info);
static void ComputerDetails(GtkWidget *menu_item, struct info_drqm_computers *info);



void CreateComputersPage (GtkWidget *notebook)
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
  clist = CreateComputersList (&info);
  gtk_box_pack_start(GTK_BOX(vbox),clist,TRUE,TRUE,2);
  
  /* Button refresh */
  buttonRefresh = CreateButtonRefresh (&info);
  gtk_box_pack_end(GTK_BOX(vbox),buttonRefresh,FALSE,FALSE,2);

  /* Append the page */
  gtk_notebook_append_page (GTK_NOTEBOOK(notebook), container, label);

  /* Put the computers on the list */
  drqm_request_computerlist (&info);
  drqm_update_computerlist (&info);

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
  gchar *titles[] = { "Running","Name","Status","Procs" };
  GtkWidget *clist;

  clist = gtk_clist_new_with_titles (5, titles);
  gtk_container_add(GTK_CONTAINER(window),clist);
  gtk_clist_column_titles_show(GTK_CLIST(clist));
  gtk_clist_column_titles_passive(GTK_CLIST(clist));
  gtk_clist_set_column_width (GTK_CLIST(clist),0,75);
  gtk_clist_set_column_width (GTK_CLIST(clist),1,100);
  gtk_clist_set_column_width (GTK_CLIST(clist),2,100);
  gtk_clist_set_column_width (GTK_CLIST(clist),4,45);
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
  
  buff = (char**) g_malloc(5 * sizeof(char*));
  buff[0] = (char*) g_malloc (BUFFERLEN);
  buff[1] = (char*) g_malloc (BUFFERLEN);
  buff[2] = (char*) g_malloc (BUFFERLEN);
  buff[3] = (char*) g_malloc (BUFFERLEN);
  buff[4] = NULL;
  
  gtk_clist_freeze(GTK_CLIST(info->clist));
  gtk_clist_clear(GTK_CLIST(info->clist));
  for (i=0; i < info->ncomputers; i++) {
    snprintf (buff[0],BUFFERLEN,"%i",info->computers[i].status.ntasks);
    strncpy(buff[1],info->computers[i].hwinfo.name,BUFFERLEN);
    snprintf (buff[2],BUFFERLEN,"REGISTERED");
    snprintf (buff[3],BUFFERLEN,"%i",info->computers[i].hwinfo.numproc);
    gtk_clist_append(GTK_CLIST(info->clist),buff);
  }
  gtk_clist_thaw(GTK_CLIST(info->clist));
}

static gint PopupMenu(GtkWidget *clist, GdkEvent *event, struct info_drqm_computers *info)
{
  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button != 3)
      return FALSE;
    gtk_clist_get_selection_info(GTK_CLIST(info->clist),
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
/*    GtkWidget *dialog; */
/*    dialog = AddDivisionDialog(info); */
/*    gtk_grab_add(dialog); */
}




