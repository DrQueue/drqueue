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

# might use: pointer, constants


#struct autoenable {   /* I put autoenable on limits even */
#  uint32_t last;      /* Time of the last autoenable event happened */
#  uint8_t  h,m;       /* Hour and minute of wished autoenable */
#  uint8_t  flags;     // Autoenable flag
#};
#
#struct computer_limits {
#  uint8_t enabled;     // Computer enabled for rendering
#  uint16_t nmaxcpus;    /* Maximum number of cpus running */
#  uint16_t maxfreeloadcpu; /* Maximum load that a cpu can have to be considered free */
#  struct autoenable autoenable;
#  uint16_t npools;
#  int64_t poolshmid; // Pool's shared memory id
#  int64_t poolsemid; // Pool's semaphore id
#  uint32_t npoolsattached;
#  fptr_type (struct pool,pool);
#  fptr_type (struct pool,local_pool);
#};

class autoenable:
  """Computer autoenable objects"""
  
  def __init__(self):
    self.last = 0
    self.h = 0
    self.m = 0
    self.flags = 0
    

class limits:
  """Computer limits objects"""

  def __init__(self):
    self.enabled = 0
    self.nmaxcpus = 1
    self.maxfreeloadcpu = 1
    self.autoenable = autoenable()
    self.npools = 0
    self.poolshmid = 0
    self.poolsemid = 0
    self.npoolsattached = 0
    self.pool = 0
    self.local_pool = 0

  #FIXME: would be constructor
  #void computer_limits_init (struct computer_limits *cl); // This one should be used instead of the following
  def computer_limits_init(cl):
    return
  
  #FIXME: would be constructor
  #void computer_limits_cpu_init (struct computer *comp);
  def computer_limits_cpu_init(comp):
    return
  
  #void computer_limits_cleanup_received (struct computer_limits *cl);
  def computer_limits_cleanup_received(cl):
    return
  
  #void computer_limits_cleanup_to_send (struct computer_limits *cl);
  def computer_limits_cleanup_to_send(cl):
    return
  
