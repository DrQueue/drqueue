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

#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <netdb.h>

#define STR_ARCH_INTEL "i386"
#define STR_ARCH_PPC "Power Macintosh"

t_arch get_architecture (void);

void
get_hwinfo (struct computer_hwinfo *hwinfo) {
  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    kill(0,SIGINT);
  }
  hwinfo->arch = get_architecture();
  hwinfo->os = OS_OSX;
  hwinfo->proctype = get_proctype();
  hwinfo->ncpus = get_numproc();
  hwinfo->speedindex = get_speedindex (hwinfo);
  hwinfo->procspeed = get_procspeed();
  hwinfo->memory = get_memory ();
  hwinfo->nnbits = computer_info_nnbits();
}

uint32_t
get_memory (void) {
  size_t len;
  uint64_t memory;
  int retcode;

  len = sizeof(memory);
  retcode = sysctlbyname ("hw.memsize",&memory,&len,NULL,0);
  if (retcode == -1) {
    return 0;
  }
  memory >>= 20;

  return (uint32_t)memory;
}

uint16_t
get_numproc (void) {
  size_t len;
  int ncpu;

  len = sizeof(ncpu);
  //sysctlbyname ("hw.ncpu",NULL,&len,NULL,0);
  sysctlbyname ("hw.ncpu",&ncpu,&len,NULL,0);

  return (uint16_t)ncpu;
}

t_arch
get_architecture (void) {
  size_t len;
  char *buffer;
  int retcode;

  retcode = sysctlbyname ("hw.machine",NULL,&len,NULL,0);
  if (retcode == -1) {
    return ARCH_UNKNOWN;
  }
  buffer = (char *)malloc(len);
  if (buffer == NULL) {
    return ARCH_UNKNOWN;
  }
  retcode = sysctlbyname ("hw.machine",buffer,&len,NULL,0);
  if (retcode == -1) {
    return ARCH_UNKNOWN;
  }
  
  // FIXME: Do all Intel Mac return "i386" ??
  if (strncmp(buffer,STR_ARCH_INTEL,strlen(STR_ARCH_INTEL)) == 0) {
    return ARCH_INTEL;
  } else if (strncmp(buffer,STR_ARCH_PPC,strlen(STR_ARCH_PPC)) == 0) {
    // FIXME: And do al PPC Mac return "Power Macintosh" ?
    return ARCH_PPC;
  }

  return ARCH_UNKNOWN;
}

t_proctype
get_proctype (void) {
  // TODO: check values "hw.cputype" and "hw.cpusubtype"
  t_arch arch;

  arch = get_architecture();

  if (arch == ARCH_INTEL) {
    return PROCTYPE_INTEL_UNKNOWN;
  } else if (arch == ARCH_PPC) {
    return PROCTYPE_PPC;
  }

  return PROCTYPE_UNKNOWN;
}

uint32_t
get_procspeed (void) {
  // README: found on "hw.cpufrequency"
  size_t len;
  uint64_t procspeed;
  int retcode;

  len = sizeof(procspeed);
  retcode = sysctlbyname ("hw.cpufrequency",&procspeed,&len,NULL,0);
  if (retcode == -1) {
    return 0;
  }
  // procspeed is returned in hertz we should return Mhz
  procspeed /= 1000000;

  return (uint32_t)procspeed;
}
