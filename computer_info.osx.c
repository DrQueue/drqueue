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

void get_hwinfo (struct computer_hwinfo *hwinfo)
{
	size_t len;
	uint64_t freq;

  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    kill(0,SIGINT);
  }
  hwinfo->arch = ARCH_PPC;
  hwinfo->os = OS_OSX;
  hwinfo->proctype = PROCTYPE_PPC;
  hwinfo->ncpus = get_numproc();
  hwinfo->speedindex = get_speedindex (hwinfo);
	len = 8; // Hardcoded (?)
	sysctlbyname ("hw.cpufrequency",&freq,&len,NULL,0);
  hwinfo->procspeed = freq / 10e5;
}

int get_numproc (void)
{
	size_t len;
	int ncpu;

	sysctlbyname ("hw.ncpu",NULL,&len,NULL,0);
	sysctlbyname ("hw.ncpu",&ncpu,&len,NULL,0);

	return ncpu;
}
