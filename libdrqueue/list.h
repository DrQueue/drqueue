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

#ifndef _LIST_H_
#define _LIST_H_

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>

#pragma pack(push,1)

struct list_item;
struct list_item {
  void *data;
  struct list_item *next;
};
struct list {
  struct list_item *first;
  struct list_item *current;
  uint32_t nitems;
};

#pragma pack(pop)

struct list *list_new (void *data);
struct list *list_add (struct list *list, void *data);
int list_count (struct list *list);
void list_reset (struct list *list);
void *list_get_seq (struct list *list);

#ifdef __CPLUSPLUS
}
#endif

#endif 	    /* !_LIST_H_ */
