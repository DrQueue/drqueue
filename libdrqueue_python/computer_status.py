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

# might use: constants, tasks


#struct computer_status {
#  uint16_t loadavg[3];  /* load average last minute, last 5, and last 15 */
#  uint16_t ntasks;      // number of tasks (processes) being used
#  uint16_t nrunning;    // number of tasks on execution or loading
#  struct task task[MAXTASKS];
#};

class computer_status:
  """Computer status objects"""

  def __init__(self):
    self.loadavg = [0, 0, 0]
    self.ntasks = 0
    self.nrunning = 0
    self.task = task()

  # FIXME: can be acced directly by computer object
  #void get_computer_status (struct computer_status *cstatus, int64_t semid);
  def get_computer_status(cstatus, semid):
    return
  
  # FIXME: replaced by constructor
  #void computer_status_init (struct computer_status *cstatus);
  def computer_status_init(cstatus):
    return
  
  #void check_tasks (struct computer_status *cstatus, int64_t semid);
  def check_tasks(cstatus, semid):
    return
  
  #void get_loadavg (uint16_t *loadavg);
  def get_loadavg(loadavg):
    return
  
  #void report_computer_status (struct computer_status *cstatus);
  def report_computer_status(cstatus):
    return
  


