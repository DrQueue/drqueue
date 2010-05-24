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

#include "logger.h"
#include "computer_info.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdint.h>

uint8_t get_architecture(void);

void
get_hwinfo (struct computer_hwinfo *hwinfo) {
  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    kill(0,SIGINT);
  }
  hwinfo->arch = get_architecture();
  hwinfo->os = OS_LINUX;
  hwinfo->proctype = get_proctype();
  hwinfo->procspeed = get_procspeed();
  hwinfo->ncpus = get_numproc();
  hwinfo->speedindex = get_speedindex (hwinfo);
  hwinfo->memory = get_memory ();
  hwinfo->nnbits = computer_info_nnbits();
}

uint32_t
get_memory (void) {
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

t_proctype
get_proctype (void) {
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
      } else if (strstr(buf,"Pentium(R) M") != NULL) {
        proctype = PROCTYPE_PENTIUMM;
      } else if (strstr(buf,"Pentium III") != NULL) {
        proctype = PROCTYPE_PENTIUMIII;
      } else if (strstr(buf,"Pentium II") != NULL) {
        proctype = PROCTYPE_PENTIUMII;
      } else if (strstr(buf,"Pentium ") != NULL) {
        proctype = PROCTYPE_PENTIUM;
      }
      found = 1;
    } else if ((strstr(buf,"arch") != NULL) &&  (strstr(buf,"IA-64") != NULL)) {
      proctype = PROCTYPE_INTELIA64;
      found = 1;
    } else if ((strstr(buf,"platform") != NULL) && (strstr(buf,"PowerMac") != NULL)) {
      proctype = PROCTYPE_PPC;
      found = 1;
    } else if ((strstr(buf,"cpu") != NULL) && (strstr(buf,"UltraSparc") != NULL)) {
      proctype = PROCTYPE_ULTRASPARC;
      found = 1;
    } else if ((strstr(buf,"cpu") != NULL) && (strstr(buf,"Cell Broadband Engine") != NULL)) {
      proctype = PROCTYPE_CELLBE;
      found = 1;
    } else if ((strstr(buf,"cpu model") != NULL) && (strstr(buf,"R5000") != NULL)) {
      proctype = PROCTYPE_MIPSR5000;
      found = 1;
    } else if ((strstr(buf,"cpu model") != NULL) && (strstr(buf,"R10000") != NULL)) {
      proctype = PROCTYPE_MIPSR10000;
      found = 1;
    } else if ((strstr(buf,"cpu model") != NULL) && (strstr(buf,"R12000") != NULL)) {
      proctype = PROCTYPE_MIPSR12000;
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

uint32_t
get_procspeed (void) {
  FILE *cpuinfo;
  uint32_t procspeed = 1;
  char buf[BUFFERLEN];
  float st;   /* speed temp */
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
    } else if (strstr(buf,"clock") != NULL) {
      while (!isdigit(buf[index]))
        index++;
      sscanf (&buf[index],"%fMHz\n",&st);
      procspeed = (int) st;
      found = 1;
    } else if (strstr(buf,"ClkTck") != NULL) {
      // UltraSparc II case mentioned in the site's forums
      // NOTE: We consider all CPUs run at the same clock speed as the first found.
      uint64_t clockspeed = 0;

      // First digit is the cpu number, jump over
      while (!isdigit(buf[index]))
        index++;
      index++;
      // Second should be the first 0 of the hex string containing the clock speed
      while (!isdigit(buf[index]))
        index++;
      clockspeed = strtoll (&buf[index],NULL,16);
      procspeed = (uint32_t)(clockspeed / 1e6);
      found = 1;
    } else if (strstr(buf,"BogoMIPS") != NULL) {
      // on SGI MIPS procspeed is equal to BogoMIPS
      while (!isdigit(buf[index]))
        index++;
      sscanf (&buf[index],"%fMHz\n",&st);
      procspeed = (int) st;
      found = 1;
    }
  }

  if (!found) {
    log_auto (L_ERROR,"Proc speed not found on /proc/cpuinfo\n");
    kill (0,SIGINT);
  }

  fclose (cpuinfo);

  return procspeed;
}

uint16_t
get_numproc (void) {
  FILE *cpuinfo;
  int numproc = 0;
  char buf[BUFFERLEN];
  int index = 0;

  if ((cpuinfo = fopen("/proc/cpuinfo","r")) == NULL) {
    perror ("get_numproc: fopen");
    kill (0,SIGINT);
  }

  while (!feof (cpuinfo)) {
    fgets (buf,BUFFERLEN-1,cpuinfo);
    if (strstr(buf,"ncpus active") != NULL) {
      // UltraSparc issue
      while (!isdigit(buf[index]))
        index++;
      sscanf (&buf[index],"%i\n",&numproc);
      break;
    } else if (strcasestr(buf,"processor") != NULL) {
      numproc++;
    }    
  }

  fclose (cpuinfo);

  return (uint16_t)numproc;
}

uint8_t
get_architecture (void) {
  FILE *cpuinfo;
  int architecture = ARCH_INTEL;
  char buf[BUFFERLEN];

  if ((cpuinfo = fopen("/proc/cpuinfo","r")) == NULL) {
    perror ("get_architecture: fopen");
    kill (0,SIGINT);
  }

  while (!feof (cpuinfo)) {
    fgets (buf,BUFFERLEN-1,cpuinfo);
    if (strstr(buf,"Sparc") != NULL) {
      // UltraSparc issue
      architecture = ARCH_SPARC;
      break;
    }
    if ((strstr(buf,"system type") != NULL) && (strstr(buf,"SGI") != NULL)) {
      // SGI MIPS issue
      architecture = ARCH_MIPS;
      break;
    }
    if ((strstr(buf,"model") != NULL) && (strstr(buf,"SonyPS3") != NULL)) {
      // Sony PS3 issue
      architecture = ARCH_POWER;
      break;
    }
  }

  fclose (cpuinfo);

  return (uint8_t)architecture;

}

