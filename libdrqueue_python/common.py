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

# might use: drq_stat, drerrno, constants, logger, libdrqueue, pointer, job, git_rev

#define VERSION_MAJOR  0
#define VERSION_MINOR  65
#define VERSION_PATCH  0
#define VERSION_POST   0
#define VERSION_PRE    0

#int common_environment_check (void);
def environment_check():
  return 0

#void show_version (char **argv);
def show_version(argv):
  return

#int rmdir_check_str (char *path);
def rmdir_check_str(path):
  return 0

#int remove_dir (char *dir);
def remove_dir(dir):
  return 0

#char *time_str (uint32_t nseconds);
def time_str(nseconds):
  return ""

#void set_default_env(void);
def set_default_env():
  return

#int common_date_check (void);
def common_date_check():
  return 0

#void mn_job_finished (struct job *job);
def mn_job_finished(job):
  return

#char *get_version_prepost (void);
def get_version_prepost():
  return ""

#char *get_revision_string (void);
def get_revision_string():
  return ""

#char *get_version_complete (void);
def get_version_complete():
  return ""

#uint64_t dr_hton64(uint64_t);
def dr_hton64(num):
  return num

#uint64_t dr_ntoh64(uint64_t);
def dr_ntoh64(num):
  return num

#void dr_copy_path(char *pDest, const char *pSrc, int nLen);
def dr_copy_path(pDest, pSrc, nLen):
  return




