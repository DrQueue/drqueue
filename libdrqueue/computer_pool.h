//
// Copyright (C) 2006 Jorge Daza Garcia-Blanes
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

#ifndef _COMPUTER_POOL_H_
#define _COMPUTER_POOL_H_

#include <stdint.h>
#include <sys/types.h>

#include "computer.h"
#include "logger.h"

#pragma pack(push,1)
struct pool {
  char name[MAXNAMELEN];
};
#pragma pack(pop)

int64_t computer_pool_get_shared_memory (uint16_t npools);
struct pool *computer_pool_attach_shared_memory (struct computer_limits *cl);
int computer_pool_detach_shared_memory (struct computer_limits *cl);
void computer_pool_init (struct computer_limits *cl);
int computer_pool_add (struct computer_limits *cl, char *pool);
int computer_pool_remove (struct computer_limits *cl, char *pool);
void computer_pool_list (struct computer_limits *cl);
int computer_pool_exists (struct computer_limits *cl,char *pool);
int computer_pool_free (struct computer_limits *cl);
void computer_pool_set_from_environment (struct computer_limits *cl);
void computer_pool_copy (struct computer_limits *cl_src, struct computer_limits *cl_dst);

int computer_pool_lock_check (struct computer_limits *cl);
int computer_pool_lock (struct computer_limits *cl);
int computer_pool_release (struct computer_limits *cl);

#endif
