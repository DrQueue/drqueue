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
// $Id$ */
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#ifdef __IRIX
#include <sys/sysmp.h>
#endif
#ifdef __FREEBSD
#include <sys/sysctl.h>
#endif

#include "constants.h"
#include "computer_info.h"

#if defined (__LINUX)
#include "computer_info.linux.c"
#elif defined (__IRIX)
#include "computer_info.irix.c"
#elif defined (__OSX)
#include "computer_info.osx.c"
#elif defined (__FREEBSD)
#include "computer_info.freebsd.c"
#else 
#error You need to define the OS, or OS defined not supported
#endif

int get_speedindex (struct computer_hwinfo *hwinfo)
{
  int speedindex;

  speedindex = (hwinfo->proctype + 1) * hwinfo->procspeed;

  return speedindex;
}


void report_hwinfo (struct computer_hwinfo *hwinfo)
{
  printf ("HWINFO Report\n");
  printf ("Name:\t\t\t%s\n",hwinfo->name);
  printf ("Architecture:\t\t%s\n",archstring(hwinfo->arch));
  printf ("OS:\t\t\t%s\n",osstring(hwinfo->os));
  printf ("Processor type:\t\t%s\n",proctypestring(hwinfo->proctype));
  printf ("Processor speed:\t%i MHz\n",hwinfo->procspeed);
  printf ("Number of processors:\t%i\n",hwinfo->ncpus);
  printf ("Speed index:\t\t%i\n",hwinfo->speedindex);
}

char *osstring (t_os os)
{
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

char *archstring (t_arch arch)
{
  char *msg;

  switch (arch) {
  case ARCH_UNKNOWN:
    msg = "UNKNOWN";
    break;
  case ARCH_INTEL:
    msg = "Intel (Little Endian)";
    break;
  case ARCH_MIPS:
    msg = "Mips (Big Endian)";
    break;
  case ARCH_PPC:
    msg = "PowerPC";
    break;
  default:
    msg = "DEFAULT (ERROR)";
    fprintf (stderr,"arch == DEFAULT\n");
  }

  return msg;
}

char *proctypestring (t_proctype proctype)
{
  char *msg;

  switch (proctype) {
  case PROCTYPE_UNKNOWN:
    msg = "UNKNOWN";
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
  case PROCTYPE_PPC:
    msg = "PPC";
    break;
  default:
    msg = "DEFAULT (ERROR)";
    fprintf (stderr,"proctype == DEFAULT\n");
  }

  return msg;
}

