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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void get_hwinfo (struct computer_hwinfo *hwinfo)
{
  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    kill(0,SIGINT);
  }
  hwinfo->arch = ARCH_INTEL;
  hwinfo->os = OS_LINUX;
  hwinfo->proctype = get_proctype();
  hwinfo->procspeed = get_procspeed();
  hwinfo->ncpus = get_numproc();
  hwinfo->speedindex = get_speedindex (hwinfo);
	hwinfo->memory = get_memory ();
}

uint32_t get_memory (void)
{
	uint32_t memory = 0;
	FILE *meminfo;
	char buf[BUFFERLEN];
	int found = 0;
	char *token;

  if ((meminfo = fopen("/proc/meminfo","r")) == NULL) {
    perror ("get_memory: fopen");
    kill (0,SIGINT);
  }

  while (!(found || feof (meminfo))) {
    fgets (buf,BUFFERLEN-1,meminfo);
    if (strstr(buf,"MemTotal") != NULL) {
			token = strtok (buf,": ");
			token = strtok (NULL,": ");
			memory = atoi(token) / 1024;
      found = 1;
    }
  }

  if (!found) {
    fprintf (stderr,"ERROR: Memory not found on /proc/meminfo\n");
    kill(0,SIGINT);
  }

  fclose (meminfo);

  return memory;
}

t_proctype get_proctype (void)
{
  t_proctype proctype = PROCTYPE_UNKNOWN;
  FILE *cpuinfo;
  char buf[BUFFERLEN];
  int found = 0;

  if ((cpuinfo = fopen("/proc/cpuinfo","r")) == NULL) {
    perror ("get_proctype: fopen");
    kill (0,SIGINT);
  }

  while (!(found || feof (cpuinfo))) {
    fgets (buf,BUFFERLEN-1,cpuinfo);
    if (strstr(buf,"model name") != NULL) {
			if (strstr(buf,"Opteron") != NULL) {
				proctype = PROCTYPE_OPTERON;
      } else if (strstr(buf,"AMD Athlon") != NULL) {
				proctype = PROCTYPE_ATHLON;
			} else if (strstr(buf,"Xeon") != NULL) {
				proctype = PROCTYPE_INTELXEON;
      } else if (strstr(buf,"Pentium(R) 4") != NULL) {
				proctype = PROCTYPE_PENTIUM4;
      } else if (strstr(buf,"Pentium III") != NULL) {
				proctype = PROCTYPE_PENTIUMIII;
      } else if (strstr(buf,"Pentium II") != NULL) {
				proctype = PROCTYPE_PENTIUMII;
      } else if (strstr(buf,"Pentium ") != NULL) {
				proctype = PROCTYPE_PENTIUM;
      }
      found = 1;
    }
  }

  if (!found) {
    fprintf (stderr,"ERROR: Proc type not found on /proc/cpuinfo\n");
    kill(0,SIGINT);
  }

  fclose (cpuinfo);

  return proctype;
}

int get_procspeed (void)
{
  FILE *cpuinfo;
  int procspeed = 1;
  char buf[BUFFERLEN];
  float st;			/* speed temp */
  int found = 0;
  int index = 0;
  
  if ((cpuinfo = fopen("/proc/cpuinfo","r")) == NULL) {
    perror ("get_procspeed: fopen");
    kill (0,SIGINT);
  }

  while (!(found || feof (cpuinfo))) {
    fgets (buf,BUFFERLEN-1,cpuinfo);
    if (strstr(buf,"cpu MHz") != NULL) {
      while (!isdigit(buf[index]))
				index++;
      sscanf (&buf[index],"%f\n",&st);
      procspeed = (int) st;
      found = 1;
    }
  }

  if (!found) {
    fprintf (stderr,"ERROR: Proc speed not found on /proc/cpuinfo\n");
    kill (0,SIGINT);
  }

  fclose (cpuinfo);

  return procspeed;
}

int get_numproc (void)
{
  FILE *cpuinfo;
  int numproc = 0;
  char buf[BUFFERLEN];
  
  if ((cpuinfo = fopen("/proc/cpuinfo","r")) == NULL) {
    perror ("get_numproc: fopen");
    kill (0,SIGINT);
  }

  while (!feof (cpuinfo)) {
    fgets (buf,BUFFERLEN-1,cpuinfo);
    if (strstr(buf,"processor") != NULL) {
      numproc++;
    }
  }

  fclose (cpuinfo);

  return numproc;
}
