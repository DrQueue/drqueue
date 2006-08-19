//
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
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
// $Id$
//

#include <sys/types.h>
#include <sys/sysctl.h>

#define STR_ARCH_INTEL "i386"

t_arch get_architecture (void);

void get_hwinfo (struct computer_hwinfo *hwinfo) {
  size_t len;
  uint64_t freq;

  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    kill(0,SIGINT);
  }
  hwinfo->arch = get_architecture();
  hwinfo->os = OS_OSX;
  hwinfo->proctype = PROCTYPE_PPC;
  hwinfo->ncpus = get_numproc();
  hwinfo->speedindex = get_speedindex (hwinfo);
  len = 8; // FIXME Hardcoded (?)
  sysctlbyname ("hw.cpufrequency",&freq,&len,NULL,0);
  hwinfo->procspeed = freq / 10e5;
  hwinfo->memory = get_memory ();
}

uint32_t get_memory (void) {
  size_t len;
  uint64_t memory_64;
  uint32_t memory_32;
  int retcode;

  retcode = sysctlbyname ("hw.memsize",NULL,&len,NULL,0);
  if (retcode == -1) {
    return 0;
  }
  if (len == 4) {
    // CHECK: is this needed ??
    //retcode = sysctlbyname ("hw.memsize",&memory_32,&len,NULL,0);
    //if (retcode == -1) {
    //  return 0;
    //}
    return 0;
  } else if (len == 8) {
    // Old working code
    retcode = sysctlbyname ("hw.memsize",&memory_64,&len,NULL,0);
    if (retcode == -1) {
      return 0;
    }
    memory_64 >>= 20;
    return memory_64;
  }

  return 0;
}

int get_numproc (void) {
  size_t len;
  int ncpu;

  sysctlbyname ("hw.ncpu",NULL,&len,NULL,0);
  sysctlbyname ("hw.ncpu",&ncpu,&len,NULL,0);

  return ncpu;
}

t_arch get_architecture (void) {
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
  
  // FIXME: Do al Intel Mac return "i386" ??
  if (strncmp(buffer,STR_ARCH_INTEL,strlen(STR_ARCH_INTEL)) == 0) {
    return ARCH_INTEL;
  }

  // FIXME: If it's not intel it's PPC... (?)
  return ARCH_PPC;
}

t_proctype get_proctype (void) {
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

int get_procspeed (void) {
  // README: found on "hw.cpufrequency"
  size_t len;
  int procspeed;
  int retcode;

  retcode = sysctlbyname ("hw.cpufrequency",NULL,&len,NULL,0);
  if (retcode == -1) {
    return 0;
  }
  retcode = sysctlbyname ("hw.cpufrequency",&procspeed,&len,NULL,0);
  if (retcode == -1) {
    return 0;
  }
  // procspeed is returned in hertz we should return Mhz
  procspeed /= 1000000;

  return procspeed;
}
