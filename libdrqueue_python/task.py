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

# might use: constants

#define DR_EXITEDFLAG (0x0100)
#define DR_SIGNALEDFLAG (0x0200)

#define DR_WIFEXITED(stat)  ((stat)&DR_EXITEDFLAG)
#define DR_WEXITSTATUS(stat) ((stat)&0xff)
#define DR_WIFSIGNALED(stat) ((stat)&DR_SIGNALEDFLAG)
#define DR_WTERMSIG(stat)   ((stat)&0xff)


#typedef enum {
#  TASKSTATUS_LOADING,  /* Assigned but not running yet */
#  TASKSTATUS_RUNNING,
#  TASKSTATUS_FINISHED  // Finished but reporting to master still
#} t_taskstatus;
#
#
#struct task {
#  uint8_t used;
#  char jobname[MAXNAMELEN]; /* jobname */
#  uint32_t ijob;  /* index to the job in the global db */
#  uint32_t icomp;  // Index to the computer that renders this frame
#  uint16_t itask;  /* index to the task in the computer !not in the global db! */
#  char jobcmd[MAXCMDLEN]; /* string that will be executed */
#  char owner[MAXNAMELEN]; /* owner of the job */
#  uint32_t frame;  /* current _real_ frame number (!!not index!!) */
#  uint32_t frame_start,frame_end;
#  uint8_t  frame_pad;
#  uint32_t frame_step;
#  uint32_t block_size;
#  int32_t  pid;   /* pid */
#  int32_t  exitstatus;  /* exit status */
#  uint8_t  status;  /* status */
#  uint64_t start_loading_time;
#};


class task:
  """Task objects"""

  def __init__(self):
    self.used = 0
    self.jobname = ""
    self.ijob = 0
    self.icomp = 0
    self.jobcmd = ""
    self.owner = ""
    self.frame = 0
    self.frame_start = 0
    self.frame_end = 0
    self.frame_pad = 0
    self.frame_step = 1
    self.block_size = 1
    self.pid = 0
    self.exitstatus = 0
    self.status = 0
    self.start_loading_time = 0
  
  #void task_init_all (struct task *task);
  def task_init_all(task): 
    return
    
  #void task_init (struct task *task);
  def task_init(task):
    return

  #uint16_t task_available (struct slave_database *sdb);
  def task_available(sdb):
    return 0
  
  #void task_report (struct task *task);
  def task_report(task):
    return

  #char *task_status_string (unsigned char status);
  def task_status_string(status):
    return ""
  
  #void task_environment_set (struct task *task);
  def task_environment_set(task):
    return
  
  #int task_set_to_job_frame (struct task *task, struct job *job, uint32_t frame);
  def task_set_to_job_frame(task, job, frame):
    return 0
  
  #int task_is_running (struct task *task);
  def task_is_running(task):
    return 0
