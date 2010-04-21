//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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

#include "computer_info.h"
#include "logger.h"

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>

void
get_hwinfo (struct computer_hwinfo *hwinfo) {
  size_t bl;
  unsigned long long int sysctl_data;
  char buffer[128];

  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    kill(0,SIGINT);
  }
#if defined(__i386__)
  hwinfo->arch = ARCH_INTEL;
  bl=128;
  sysctlbyname("hw.model", &buffer,&bl,NULL,0);

  if (strstr(buffer,"Opteron(tm)") != NULL) {
    hwinfo->proctype = PROCTYPE_OPTERON;
  } else if (strstr(buffer,"Xeon(TM)") != NULL) {
    hwinfo->proctype = PROCTYPE_INTELXEON;
  } else if (strstr(buffer,"Pentium(R) 4")) {
    hwinfo->proctype = PROCTYPE_PENTIUM4;
  } else if (strstr(buffer,"Pentium(R) M") != NULL) {
    hwinfo->proctype = PROCTYPE_PENTIUMM;
  } else if (strstr(buffer,"Pentium III")) {
    hwinfo->proctype = PROCTYPE_PENTIUMIII;
  } else if (strstr(buffer,"Pentium II")) {
    hwinfo->proctype = PROCTYPE_PENTIUMII;
  } else if (strstr(buffer,"Athlon") ||
             strstr(buffer,"Duron")) {
    hwinfo->proctype = PROCTYPE_ATHLON;
  } else {
    hwinfo->proctype = PROCTYPE_PENTIUM;
  }
#else
  //Since BSD Runs on Aplhas and a variety of other architectures
  hwinfo->arch = ARCH_UNKNOWN;
  hwinfo->proctype = PROCTYPE_UNKNOWN;
#endif

  hwinfo->os = OS_FREEBSD;

  bl=8;
  sysctlbyname("machdep.tsc_freq", &sysctl_data,&bl,NULL,0);
  if (bl < 8)
    sysctl_data &= (2LL << (8*bl))-1;
  hwinfo->procspeed = (sysctl_data+500000)/1000000;

  bl=8;
  sysctlbyname("hw.ncpu", &sysctl_data,&bl,NULL,0);
  hwinfo->ncpus = sysctl_data;
  hwinfo->speedindex = get_speedindex (hwinfo);
  hwinfo->memory = get_memory ();
  hwinfo->nnbits = computer_info_nnbits();
}

uint32_t
get_memory (void) {
  uint64_t memory;
  size_t len = sizeof(uint64_t);

  sysctlbyname ("hw.physmem",&memory,&len,NULL,0);
  memory >>= 20 ;

  return (uint32_t)memory;
}
