/* $Id: computer_info.c,v 1.2 2001/04/25 15:55:23 jorge Exp $ */

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "arch/computer_info.h"

void get_hwinfo (struct computer_hwinfo *hwinfo)
{
  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    exit (1);
  }
  hwinfo->arch = ARCH_INTEL;
  hwinfo->os = OS_LINUX;
  hwinfo->proctype = get_proctype();
}

t_proctype get_proctype (void)
{
  t_proctype proctype = PROCTYPE_UNKNOWN;
  FILE *cpuinfo;
  char buf[BUFFERLEN];
  int found = 0;
  
  if ((cpuinfo = fopen("/proc/cpuinfo","r")) == NULL) {
    perror ("get_proctype: fopen");
    exit (1);
  }

  while (!(found || feof (cpuinfo))) {
    fgets (buf,BUFFERLEN-1,cpuinfo);
    if (strstr(buf,"model name") != NULL) {
      if (strstr(buf,"Pentium III") != NULL) {
	proctype = PROCTYPE_PENTIUMIII;
      } else if (strstr(buf,"Pentium II") != NULL) {
	proctype = PROCTYPE_PENTIUMII;
      } else if (strstr(buf,"Pentium ") != NULL) {
	proctype = PROCTYPE_PENTIUM;
      }
    }
  }

  fclose (cpuinfo);

  return proctype;
}

void report_hwinfo (struct computer_hwinfo *hwinfo)
{
  printf ("Name:\t\t%s\n",hwinfo->name);
  printf ("Architecture:\t%s\n",archstring(hwinfo->arch));
  printf ("OS:\t\t%s\n",osstring(hwinfo->os));
  printf ("Processor type:\t%s\n",proctypestring(hwinfo->proctype));
}

char *osstring (t_os os)
{
  static char osstring[BUFFERLEN];

  osstring[MAXCMDLEN-1] = 0;
  switch (os) {
  case OS_UNKNOWN:
    strncpy (osstring,"UNKNOWN",BUFFERLEN-1);
    break;
  case OS_IRIX:
    strncpy (osstring,"Irix",BUFFERLEN-1);
    break;
  case OS_LINUX:
    strncpy (osstring,"Linux",BUFFERLEN-1);
    break;
  case OS_WINDOWS:
    strncpy (osstring,"Windows",BUFFERLEN-1);
    break;
  default:
    strncpy (osstring,"DEFAULT (?!)",BUFFERLEN-1);
    fprintf (stderr,"os == DEFAULT\n");
    exit (1);
  }

  return osstring;
}

char *archstring (t_arch arch)
{
  static char archstring[BUFFERLEN];

  archstring[BUFFERLEN-1] = 0;
  switch (arch) {
  case ARCH_UNKNOWN:
    strncpy (archstring,"UNKNOWN",BUFFERLEN-1);
    break;
  case ARCH_INTEL:
    strncpy (archstring,"Intel (Little Endian)",BUFFERLEN-1);
    break;
  case ARCH_MIPS:
    strncpy (archstring,"Mips (Big Endian)",BUFFERLEN-1);
    break;
  default:
    strncpy (archstring,"DEFAULT (?!)",BUFFERLEN-1);
    fprintf (stderr,"arch == DEFAULT\n");
    exit (1);
  }

  return archstring;
}

char *proctypestring (t_proctype proctype)
{
  static char proctypestring[BUFFERLEN];

  proctypestring[BUFFERLEN-1] = 0;
  switch (proctype) {
  case PROCTYPE_UNKNOWN:
    strncpy (proctypestring,"UNKNOWN",BUFFERLEN-1);
    break;
  case PROCTYPE_PENTIUM:
    strncpy (proctypestring,"Pentium",BUFFERLEN-1);
    break;
  case PROCTYPE_PENTIUMII:
    strncpy (proctypestring,"Pentium II",BUFFERLEN-1);
    break;
  case PROCTYPE_PENTIUMIII:
    strncpy (proctypestring,"Pentium III",BUFFERLEN-1);
    break;
  case PROCTYPE_MIPSR5000:
    strncpy (proctypestring,"R5000",BUFFERLEN-1);
    break;
  case PROCTYPE_MIPSR10000:
    strncpy (proctypestring,"R10000",BUFFERLEN-1);
    break;
  default:
    strncpy (proctypestring,"DEFAULT (?!)",BUFFERLEN-1);
    fprintf (stderr,"proctype == DEFAULT\n");
    exit (1);
  }

  return proctypestring;
}

