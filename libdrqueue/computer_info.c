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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <stdint.h>
#include <logger.h>

#ifdef __IRIX
#include <sys/sysmp.h>
#endif

#ifdef __FREEBSD
#include <sys/sysctl.h>
#endif

#include "pointer.h"
#include "constants.h"
#include "computer_info.h"

#if defined (__LINUX)
#include "computer_info.linux.h"
#elif defined (__IRIX)
#include "computer_info.irix.h"
#elif defined (__OSX)
#include "computer_info.osx.h"
#elif defined (__FREEBSD)
#include "computer_info.freebsd.h"
#elif defined (__CYGWIN)
#include "computer_info.cygwin.h"
#else
#error You need to define the OS, or OS defined not supported
#endif

uint32_t
get_speedindex (struct computer_hwinfo *hwinfo) {
  uint32_t speedindex;

  speedindex = (hwinfo->proctype + 1) * hwinfo->procspeed;

  return speedindex;
}


void report_hwinfo (struct computer_hwinfo *hwinfo) {
  printf ("HWINFO Report\n");
  printf ("Name:\t\t\t%s\n",hwinfo->name);
  printf ("Architecture:\t\t%s\n",archstring((t_arch)hwinfo->arch));
  printf ("OS:\t\t\t%s\n",osstring((t_os)hwinfo->os));
  printf ("Processor type:\t\t%s\n",proctypestring((t_proctype)hwinfo->proctype));
  printf ("64bit-32bit cpu:\t%s\n",bitsstring(hwinfo->nnbits));
  printf ("Processor speed:\t%i MHz\n",hwinfo->procspeed);
  printf ("Number of processors:\t%i\n",hwinfo->ncpus);
  //printf ("Speed index:\t\t%i\n",hwinfo->speedindex);
  printf ("Memory:\t\t\t%i Mbytes\n",hwinfo->memory);
}

char *osstring (t_os os) {
  char *msg;

  switch (os) {
  case OS_UNKNOWN:
    msg = "UNKNOWN";
    break;
  case OS_IRIX:
    msg = "Irix";
    break;
  case OS_LINUX:
    msg = "Linux";
    break;
  case OS_WINDOWS:
  case OS_CYGWIN:
    msg = "Windows";
    break;
  case OS_OSX:
    msg = "Mac OSX";
    break;
  case OS_FREEBSD:
    msg = "FreeBSD";
    break;
  default:
    msg = "DEFAULT (ERROR)";
    fprintf (stderr,"os == DEFAULT\n");
  }

  return msg;
}


uint8_t
computer_info_nnbits () {
  // This function will try to guess if the computer is a 32 or 64 bit
  // one. For that we'll use the size of a pointer to void
  int bytes=sizeof(void*);
  if (bytes == 4) {
    // This seems to be a 32bit cpu (or a 64bit one emulating 32bit ?)
    return 32;
  } else if (bytes == 8) {
    // Well, a 64bit one is my best guess
    return 64;
  }

  // In other situations:
  return 0;
}

char *
bitsstring (uint8_t nnbits) {
  char *msg;

  switch (nnbits) {
  case 0:
    msg = "Unknown";
    break;
  case 32:
    msg = "32bit";
    break;
  case 64:
    msg = "64bit";
    break;
  default:
    msg = "ERROR: value not listed";
  }

  return msg;
}

char *archstring (t_arch arch) {
  char *msg;

  switch (arch) {
  case ARCH_UNKNOWN:
    msg = "UNKNOWN";
    break;
  case ARCH_INTEL:
    msg = "Intel";
    break;
  case ARCH_MIPS:
    msg = "Mips";
    break;
  case ARCH_PPC:
    msg = "PowerPC";
    break;
  case ARCH_SPARC:
    msg = "Sparc";
    break;
  case ARCH_POWER:
    msg = "Power";
    break;
  default:
    msg = "DEFAULT (ERROR)";
    fprintf (stderr,"arch == DEFAULT\n");
  }

  return msg;
}

char *proctypestring (t_proctype proctype) {
  char *msg;

  switch (proctype) {
  case PROCTYPE_UNKNOWN:
    msg = "UNKNOWN";
    break;
  case PROCTYPE_INTEL_UNKNOWN:
    msg = "Intel (not listed)";
    break;
  case PROCTYPE_PENTIUM:
    msg = "Pentium";
    break;
  case PROCTYPE_PENTIUMII:
    msg = "Pentium II";
    break;
  case PROCTYPE_PENTIUMIII:
    msg = "Pentium III";
    break;
  case PROCTYPE_PENTIUM4:
    msg = "Pentium 4";
    break;
  case PROCTYPE_INTELXEON:
    msg = "Xeon";
    break;
  case PROCTYPE_INTELIA64:
    msg = "IA-64";
    break;
  case PROCTYPE_PENTIUMM:
    msg = "Pentium M";
    break;
  case PROCTYPE_ATHLON:
    msg = "Athlon";
    break;
  case PROCTYPE_OPTERON:
    msg = "Opteron";
    break;
  case PROCTYPE_MIPSR5000:
    msg = "R5000";
    break;
  case PROCTYPE_MIPSR10000:
    msg = "R10000";
    break;
  case PROCTYPE_MIPSR12000:
    msg = "R12000";
    break;
  case PROCTYPE_PPC:
    msg = "PPC";
    break;
  case PROCTYPE_ULTRASPARC:
    msg = "Ultrasparc";
    break;
  case PROCTYPE_CELLBE:
    msg = "Cell Broadband Engine";
    break;
  default:
    msg = "DEFAULT (ERROR)";
    fprintf (stderr,"proctype == DEFAULT\n");
  }

  return msg;
}

