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

void get_hwinfo (struct computer_hwinfo *hwinfo)
{
  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    kill(0,SIGINT);
  }
  hwinfo->arch = ARCH_MIPS;
  hwinfo->os = OS_IRIX;
  hwinfo->proctype = get_proctype();
  hwinfo->procspeed = get_procspeed();
  hwinfo->ncpus = get_numproc();
  hwinfo->speedindex = get_speedindex (hwinfo);
	hwinfo->memory = get_memory ();
}

uint32_t get_memory (void)
{
	FILE *hinv;
	char buf[BUFFERLEN];
	char* offset;
	int memsize = 0;                      /* number of cpus */
	int found = 0;

	if ((hinv = popen ("/sbin/hinv","r")) == NULL) {
		fprintf (stderr,"Warning: Problems executing '/sbin/hinv'\n");
		return memsize;
	}
	
	while (fgets (buf,BUFFERLEN,hinv) != NULL) {
		if ((offset=strstr(buf,"memory size:")) != NULL) {
			/* system memory in mbytes is on this line */
			if (sscanf (offset+13,"%i",&memsize) == 1) {
				found = 1;
			}
		}
	}
	
	if (!found) {
		fprintf (stderr,"ERROR: Memory size not found\n");
	}
	
	pclose (hinv);
	
	return memsize;
}

t_proctype get_proctype (void)
{
  FILE *hinv;
  char buf[BUFFERLEN];
  t_proctype proctype = PROCTYPE_UNKNOWN;
  int found = 0;

  if ((hinv = popen ("/sbin/hinv","r")) == NULL) {
    fprintf (stderr,"Warning: Problems executing '/sbin/hinv'\n");
    return proctype;
  }

  while (fgets (buf,BUFFERLEN,hinv) != NULL) {
    if (strstr(buf,"CPU:") != NULL) {
      if (strstr(buf,"R5000") != NULL) {
				proctype = PROCTYPE_MIPSR5000;
				found = 1;
      } else if (strstr(buf,"R10000") != NULL) {
				proctype = PROCTYPE_MIPSR10000;
				found = 1;
      }
    }
  }

  pclose (hinv);

  if (!found) {
    fprintf (stderr,"Warning processor type not listed or couldn't be found\n");
  }
  
  return proctype;
}

int get_procspeed (void)
{
  FILE *hinv;
  int procspeed = 1;
  char buf[BUFFERLEN];
  int nprocs;			/* number of cpus */
  int found = 0;

  if ((hinv = popen ("/sbin/hinv","r")) == NULL) {
    fprintf (stderr,"Warning: Problems executing '/sbin/hinv'\n");
    return procspeed;
  }

  while (fgets (buf,BUFFERLEN,hinv) != NULL) {
    if (strstr(buf,"MHZ") != NULL) {
      /* The MHz are the second number on this line */
      if (sscanf (buf,"%i %i",&nprocs,&procspeed) == 2) {
				found = 1;
      } else if (sscanf (buf,"Processor %i:  %i",&nprocs,&procspeed) == 2) {
				found = 1;
      }
    }
  }

  if (!found) {
    fprintf (stderr,"ERROR: Proc speed not found on\n");
  }

  pclose (hinv);

  return procspeed;
}

int get_numproc (void)
{
  return sysmp (MP_NPROCS);
}
