/* $Id$ */

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

#ifdef __LINUX			/* __LINUX */

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

#else 
# ifdef __IRIX
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
# else
#  ifdef __OSX
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
#  else
#   ifdef __FREEBSD
void get_hwinfo (struct computer_hwinfo *hwinfo)
{
  size_t bl;
  int sysctl_data;
  char buffer[128];

  if (gethostname (hwinfo->name,MAXNAMELEN-1) == -1) {
    perror ("get_hwinfo: gethostname");
    kill(0,SIGINT);
  }
#if defined(__i386__)
  hwinfo->arch = ARCH_INTEL;
  bl=128;
  sysctlbyname("hw.model", &buffer,&bl,NULL,0);

  if (strstr(buffer,"Pentium 4")) {
    hwinfo->proctype = PROCTYPE_PENTIUM4;
  } else if (strstr(buffer,"Pentium III")) {
    hwinfo->proctype = PROCTYPE_PENTIUMIII;
  } else if (strstr(buffer,"Pentium II")) {
    hwinfo->proctype = PROCTYPE_PENTIUMII;
  } else if (strstr(buffer,"K6")) {
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

  bl=4;
  sysctlbyname("machdep.tsc_freq", &sysctl_data,&bl,NULL,0);
  hwinfo->procspeed = (sysctl_data+500000)/1000000;

  bl=4;
  sysctlbyname("hw.ncpu", &sysctl_data,&bl,NULL,0);
  hwinfo->ncpus = sysctl_data;
  hwinfo->speedindex = get_speedindex (hwinfo);
}


#   else 
#    error You need to define the OS, or OS defined not supported
#   endif // __FREEBSD
#  endif // __OSX
# endif // __IRIX
#endif // __LINUX

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

