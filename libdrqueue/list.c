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

#include <stdlib.h>
#include "list.h"

struct list *list_new (void *data) {

  struct list *new_list = (struct list *) malloc (sizeof (struct list));

  new_list->first = NULL;
  new_list->current = NULL;
  new_list->nitems = 0;

  if ( data == NULL ) {
    return new_list;
  }
  
  return list_add (new_list,data);
}

struct list *list_add (struct list *list, void *data) {
  struct list_item *item = (struct list_item *) malloc (sizeof (struct list_item));
  struct list_item *old_first = list->first;
  item->data = data;
  item->next = old_first;
  list->first = item;
  list->nitems += 1;
  return list;
}

int list_count (struct list *list) {
  int count = 0;
  struct list_item *current = list->first;
  while ( current != NULL ) {
    count += 1;
    current = current->next;
  }
  return count;
}

void list_reset (struct list *list) {
  list->current = list->first;
}

void *list_get_seq (struct list *list) {
  if ( !list )
    return NULL;
  if ( !list->current ) {
    list->current = list->first;
    return NULL;
  }
  void *data = list->current->data;
  list->current = list->current->next;
  return data;
}
