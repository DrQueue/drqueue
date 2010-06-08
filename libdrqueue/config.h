//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include "list.h"
#include <stdio.h>

struct config_node;
struct config_item;
#define CFG_ITEM_NAME_LEN 32
#define CFG_ITEM_VALUE_LEN PATH_MAX
struct config_item {
  char name[CFG_ITEM_NAME_LEN];
  char value[CFG_ITEM_VALUE_LEN];
};
struct config_node {
  char name[CFG_ITEM_NAME_LEN];
  char value[CFG_ITEM_VALUE_LEN];
  long filepos;
  struct list *item_list;
  struct list *node_list;
  struct config_node *parent;
};

void config_get_default_file (char *dst,char *tool,int dstlen);
void config_parse_tool (char *tool);
void config_parse (char *cfg);


// New config tools
struct config_node *config_node_new (struct config_node *parent, char *name, char *desc, FILE *file);
struct config_node *config_read (char *filename,char *tool);
struct config_item *config_item_new (struct config_node *base, char *line);
void config_add_node (struct config_node *base, FILE *file, char *line);
void config_read_item (struct config_node *base, FILE *file);
int config_find_nodes (struct config_node *base, FILE *file);
int config_end_node (struct config_node *base, FILE *file);
int config_line_ends_node (struct config_node *base, char *line);

#ifdef __CPLUSPLUS
}
#endif

#endif 	    /* !_CONFIG_H_ */
