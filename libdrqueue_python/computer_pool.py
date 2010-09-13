#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''type info here'''
#
# Copyright (C) 2010 Andreas Schroeder
#
# This file is part of DrQueue
#
# DrQueue is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# DrQueue is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
# USA
#

# might use: computer, logger, constants


#struct pool {
#  char name[MAXNAMELEN];
#};



#int64_t computer_pool_get_shared_memory (uint16_t npools);
def computer_pool_get_shared_memory(npools):
  return 0

#struct pool *computer_pool_attach_shared_memory (struct computer_limits *cl);
def computer_pool_attach_shared_memory(cl):
  return 0

#int computer_pool_detach_shared_memory (struct computer_limits *cl);
def computer_pool_detach_shared_memory(cl):
  return 0

#void computer_pool_init (struct computer_limits *cl);
def computer_pool_init(cl):
  return

#int computer_pool_add (struct computer_limits *cl, char *pool);
def computer_pool_add(cd, pool):
  return 0

#int computer_pool_remove (struct computer_limits *cl, char *pool);
def computer_pool_remove(cl, pool):
  return 0

#void computer_pool_list (struct computer_limits *cl);
def computer_pool_list(cl):
  return

#int computer_pool_exists (struct computer_limits *cl, char *pool);
def computer_pool_exists(cl, pool):
  return 0

#int computer_pool_free (struct computer_limits *cl);
def computer_pool_free(cl):
  return 0

#void computer_pool_set_from_environment (struct computer_limits *cl);
def computer_pool_set_from_environment(cl):
  return

#void computer_pool_copy (struct computer_limits *cl_src, struct computer_limits *cl_dst);
def computer_pool_copy(cl_src, cl_dst):
  return

#int computer_pool_lock_check (struct computer_limits *cl);
def computer_pool_lock_check(cl):
  return 0

#int computer_pool_lock (struct computer_limits *cl);
def computer_pool_lock(cl):
  return 0

#int computer_pool_release (struct computer_limits *cl);
def computer_pool_release(cl):
  return 0



