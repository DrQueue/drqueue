/*
 * $Header: /root/cvs/drqueue/drqman/drqm_common.h,v 1.2 2001/09/24 16:10:20 jorge Exp $
 */

#ifndef _DRQM_COMMON_H_
#define _DRQM_COMMON_H_

#include <gtk/gtk.h>
#include "libdrqueue.h"

#define TOOLTIPS_DELAY 1000

GtkWidget *ConfirmDialog (char *text, GList *callbacks);
GtkTooltips *TooltipsNew (void);

#endif /* _DRQM_COMMON_H */








