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

void get_hwinfo (struct computer_hwinfo *hwinfo)
{
  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    kill(0,SIGINT);
  }
  hwinfo->arch = ARCH_PPC;
  hwinfo->os = OS_OSX;
  hwinfo->proctype = PROCTYPE_PPC;
  hwinfo->procspeed = 1000;
  hwinfo->ncpus = get_numproc();
  hwinfo->speedindex = get_speedindex (hwinfo);
}

int get_numproc (void)
{
	FILE *system_profiler;
	int nprocs = 1;
	char buf[BUFFERLEN];
	int found = 1;
	char *fd;

	if ((system_profiler = popen ("/usr/sbin/system_profiler -detailLevel -2","r")) == NULL) {
		fprintf (stderr, "Warning: Problems executing '/usr/sbin/system_profiler'\n");
		return nprocs;
	}

	while (!found || (fgets(buf,BUFFERLEN,system_profiler) != NULL)) {
		if (strstr(buf,"CPUs") != NULL) {
			fd = buf;
			while (!isdigit((int)*fd))
				fd++;
			if (sscanf (fd,"%i\n",&nprocs) == 1) {
				found = 1;
			} else {
				fprintf (stderr,"Warning: get_numproc. Found but not read !\n");
			}
		}
	}

	pclose (system_profiler);

	return nprocs;
}
