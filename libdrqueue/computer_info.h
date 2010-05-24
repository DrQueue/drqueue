//
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
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

#ifndef _COMPUTER_INFO_H_
#define _COMPUTER_INFO_H_

#include "constants.h"

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#pragma pack(push,1)

typedef enum {
  ARCH_UNKNOWN,
  ARCH_INTEL,
  ARCH_MIPS,
  ARCH_PPC,
  ARCH_SPARC,
  ARCH_POWER
} t_arch;

typedef enum {
  OS_UNKNOWN,
  OS_IRIX,
  OS_LINUX,
  OS_CYGWIN,
  OS_WINDOWS,
  OS_OSX,
  OS_FREEBSD
} t_os;

typedef enum {
  PROCTYPE_UNKNOWN,
  PROCTYPE_INTEL_UNKNOWN,
  PROCTYPE_PENTIUM,
  PROCTYPE_PENTIUMII,
  PROCTYPE_PENTIUMIII,
  PROCTYPE_PENTIUM4,
  PROCTYPE_INTELXEON,
  PROCTYPE_INTELIA64,
  PROCTYPE_PENTIUMM,
  PROCTYPE_ATHLON,
  PROCTYPE_OPTERON,
  PROCTYPE_MIPSR5000,
  PROCTYPE_MIPSR10000,
  PROCTYPE_MIPSR12000,
  PROCTYPE_PPC,
  PROCTYPE_INTEL_CORE2,
  PROCTYPE_ULTRASPARC,
  PROCTYPE_CELLBE
} t_proctype;

struct computer_hwinfo {
  char     name[MAXNAMELEN];   /* Name of the computer */
  uint32_t id;                 /* Identification number */
  uint8_t  arch;               /* type of architecture */
  uint8_t  os;                 /* type of operating system */
  uint8_t  proctype;           /* type of processors */
  uint32_t procspeed;          /* speed of the processors */
  uint16_t ncpus;              /* number of processors that the computer has */
  uint32_t speedindex;         /* global speed index for making comparisons between different computers */
  uint32_t memory;             /* Memory in Mbytes */
  uint8_t  nnbits;             // 64/32 or 0 (Unknown) cpu
};

#pragma pack(pop)

void get_hwinfo (struct computer_hwinfo *hwinfo);
t_proctype get_proctype (void);
uint32_t get_procspeed (void);
uint16_t get_numproc (void);
uint32_t get_speedindex (struct computer_hwinfo *hwinfo);
uint32_t get_memory (void);
uint8_t computer_info_nnbits (void);

void report_hwinfo (struct computer_hwinfo *hwinfo);

char *bitsstring (uint8_t nnbits);
char *osstring (t_os os);
char *archstring (t_arch arch);
char *proctypestring (t_proctype proctype);

#endif /* _COMPUTER_INFO_H_ */

