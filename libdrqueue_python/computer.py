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

import computer_limits as limits
import computer_info as info
import computer_pool as pool
import computer_status as status


#struct computer {
#  struct computer_hwinfo hwinfo;
#  struct computer_status status;
#  struct computer_limits limits;
#  uint32_t  lastconn;   /* Time of last connection to the master */
#  uint8_t   used;       /* Is this computer record being used or not ? */
#  int64_t   semid;      // Semaphore id for the computer
#};

class computer:
  """Computer objects"""

  def __init__(self):
    self.hwinfo = info()
    self.status = status()
    self.limits = limits()
    self.lastconn = 0
    self.used = 0
    self.semid = 0
  
  #int computer_index_addr (void *pwdb, struct in_addr addr); /* I use pointers to void instead to struct database */
  def computer_index_addr(pwdb, addr):
    return 0
  
  #int computer_index_name (void *pwdb, char *name);     /* because if I did I would have to create a dependency loop */
  def computer_index_name(pwdb, name):
    return 0
  
  #int computer_index_free (void *pwdb);
  def computer_index_free(pwdb):
    return 0
  
  #int computer_available (struct computer *computer);
  def computer_available(computer):
    return 0
  
  #uint16_t computer_ntasks (struct computer *computer);
  def computer_ntasks(computer):
    return 0
  
  #uint16_t computer_nrunning (struct computer *computer);
  def computer_nrunning(computer):
    return 0
  
  #uint16_t computer_nrunning_job (struct computer *computer, uint32_t ijob);
  def computer_nrunning_job(computer, ijob):
    return 0
  
  #void computer_update_assigned (struct database *wdb, uint32_t ijob, uint32_t iframe, uint32_t icomp, uint16_t itask);
  def computer_update_assigned(wdb, ijob, iframe, icomp, itask):
    return
  
  #void computer_init (struct computer *computer);
  def computer_init(computer):
    return 0
  
  #int computer_free (struct computer *computer);
  def computer_free(computer):
    return 0
  
  #int computer_ncomputers_masterdb (struct database *wdb);
  def computer_ncomputers_masterdb(wdb):
    return 0
  
  #int computer_index_correct_master (struct database *wdb, uint32_t icomp);
  def computer_index_correct_master(wdb, icomp):
    return 0
  
  #void computer_autoenable_check (struct slave_database *sdb);
  def computer_autoenable_check(sdb):
    return
  
  #int computer_lock_check (struct computer *computer);
  def computer_lock_check(computer):
    return 0
  
  #int computer_lock (struct computer *computer);
  def computer_lock(computer):
    return 0
  
  #int computer_release (struct computer *computer);
  def computer_release(computer):
    return 0
  
  #int computer_attach (struct computer *computer);
  def computer_attach(computer):
    return 0
  
  #int computer_detach (struct computer *computer);
  def computer_detach(computer):
    return 0
  
  
