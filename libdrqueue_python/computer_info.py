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


#typedef enum {
#  ARCH_UNKNOWN,
#  ARCH_INTEL,
#  ARCH_MIPS,
#  ARCH_PPC,
#  ARCH_SPARC,
#  ARCH_POWER
#} t_arch;
#
#typedef enum {
#  OS_UNKNOWN,
#  OS_IRIX,
#  OS_LINUX,
#  OS_CYGWIN,
#  OS_WINDOWS,
#  OS_OSX,
#  OS_FREEBSD
#} t_os;
#
#typedef enum {
#  PROCTYPE_UNKNOWN,
#  PROCTYPE_INTEL_UNKNOWN,
#  PROCTYPE_PENTIUM,
#  PROCTYPE_PENTIUMII,
#  PROCTYPE_PENTIUMIII,
#  PROCTYPE_PENTIUM4,
#  PROCTYPE_INTELXEON,
#  PROCTYPE_INTELCOREDUO,
#  PROCTYPE_INTELCORE2DUO,
#  PROCTYPE_INTELIA64,
#  PROCTYPE_PENTIUMM,
#  PROCTYPE_ATHLON,
#  PROCTYPE_OPTERON,
#  PROCTYPE_MIPSR5000,
#  PROCTYPE_MIPSR10000,
#  PROCTYPE_MIPSR12000,
#  PROCTYPE_PPC,
#  PROCTYPE_INTEL_CORE2,
#  PROCTYPE_ULTRASPARC,
#  PROCTYPE_CELLBE
#} t_proctype;
#
#struct computer_hwinfo {
#  char     name[MAXNAMELEN];   /* Name of the computer */
#  char     address[MAXNAMELEN]; /* cached IP address */ 
#  uint32_t id;                 /* Identification number */
#  uint8_t  arch;               /* type of architecture */
#  uint8_t  os;                 /* type of operating system */
#  uint8_t  proctype;           /* type of processors */
#  uint32_t procspeed;          /* speed of the processors */
#  uint16_t ncpus;              /* number of processors that the computer has */
#  uint32_t speedindex;         /* global speed index for making comparisons between different computers */
#  uint32_t memory;             /* Memory in Mbytes */
#  uint8_t  nnbits;             // 64/32 or 0 (Unknown) cpu
#};


class info:
  """Computer info objects"""

  def __init__(self):
    self.name = ""
    self.address = ""
    self.id = 0
    self.arch = 0
    self.os = 0
    self.proctype = 0
    self.procspeed = 0
    self.ncpus = 0
    self.speedindex = 0
    self.memory = 0
    self.nbits = 0

  # FIXME: will be accessed diretly by computer class
  #void get_hwinfo (struct computer_hwinfo *hwinfo);
  def get_hwinfo(self, hwinfo):
    return
  
  #t_proctype get_proctype (void);
  def get_proctype(self):
    return self.proctype
  
  #uint32_t get_procspeed (void);
  def get_procspeed(self):
    return self.procspeed
  
  #uint16_t get_numproc (void);
  def get_numproc(self):
    return self.numproc
  
  #uint32_t get_speedindex (struct computer_hwinfo *hwinfo);
  def get_speedindex(self, hwinfo):
    return self.speedindex
  
  #uint32_t get_memory (void);
  def get_memory(self):
    return self.memory
  
  #uint8_t computer_info_nnbits (void);
  def computer_info_nnbits(self):
    return self.nbits
  
  #void report_hwinfo (struct computer_hwinfo *hwinfo);
  def report_hwinfo(self, hwinfo):
    hwinfo = self
    return hwinfo
  
  #char *bitsstring (uint8_t nnbits);
  def bitsstring(nnbits):
    return ""
  
  #char *osstring (t_os os);
  def osstring(os):
    return ""
  
  #char *archstring (t_arch arch);
  def archstring(arch):
    return ""
  
  #char *proctypestring (t_proctype proctype);
  def proctypestring(proctype):
    return ""
  
  
