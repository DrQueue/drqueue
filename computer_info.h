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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
// 
// $Id: /drqueue/remote/trunk/computer_info.h 2252 2005-05-02T02:35:47.989705Z jorge	$
//

#ifndef _COMPUTER_INFO_H_
#define _COMPUTER_INFO_H_


#if defined (__LINUX)
#include <stdint.h>
#elif defined (__IRIX)
#include <sys/types.h>
#elif defined (__OSX)
#include <stdint.h>
#elif defined (__FREEBSD)
#include <stdint.h>
#elif defined (__CYGWIN)
#include <stdint.h>
#else
#error You need to define the OS, or OS defined not supported
#endif

#include "constants.h"

typedef enum {
	ARCH_UNKNOWN,
	ARCH_INTEL,
	ARCH_MIPS,
	ARCH_PPC
} t_arch;

typedef enum {
	OS_UNKNOWN,
	OS_IRIX,
	OS_LINUX,
	OS_CYGWIN,
	OS_OSX,
	OS_FREEBSD
} t_os;

typedef enum {
	PROCTYPE_UNKNOWN,
	PROCTYPE_PENTIUM,
	PROCTYPE_PENTIUMII,
	PROCTYPE_PENTIUMIII,
	PROCTYPE_PENTIUM4,
	PROCTYPE_INTELXEON,
	PROCTYPE_INTELIA64,
	PROCTYPE_PENTIUMM,
	PROCTYPE_ATHLON,
	PROCTYPE_OPTERON,
	PROCTYPE_MIPSR5000,
	PROCTYPE_MIPSR10000,
	PROCTYPE_PPC
} t_proctype;

struct computer_hwinfo {
	char name[MAXNAMELEN];				/* Name of the computer */
	uint32_t id;									/* Identification number */
	unsigned char arch;						/* type of architecture */
	unsigned char os;							/* type of operating system */
	unsigned char proctype;				/* type of processors */
	uint32_t procspeed;						/* speed of the processors */
	uint16_t ncpus;			/* number of processors that the computer has */
	uint32_t speedindex; /* global speed index for making comparisons between different computers */
	uint32_t memory;							/* Memory in Mbytes */
};

void get_hwinfo (struct computer_hwinfo *hwinfo);
t_proctype get_proctype (void);
int get_procspeed (void);
int get_numproc (void);
int get_speedindex (struct computer_hwinfo *hwinfo);
uint32_t get_memory (void);

void report_hwinfo (struct computer_hwinfo *hwinfo);

char *osstring (t_os os);
char *archstring (t_arch arch);
char *proctypestring (t_proctype proctype);

#endif /* _COMPUTER_INFO_H_ */



