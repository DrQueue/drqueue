/* $Id: computer_info.h,v 1.1 2001/04/25 10:45:41 jorge Exp $ */

#ifndef _COMPUTER_INFO_H_
#define _COMPUTER_INFO_H_

typedef enum {
  ARCH_UNKNOWN,
  ARCH_INTEL,
  ARCH_MIPS
} t_arch;

typedef enum {
  OS_UNKNOWN,
  OS_IRIX,
  OS_LINUX,
  OS_WINDOWS
} t_os;

typedef enum {
  PROCTYPE_UNKNOWN,
  PROCTYPE_PENTIUM,
  PROCTYPE_PENTIUMII,
  PROCTYPE_PENTIUMIII,
  PROCTYPE_MIPSR5000,
  PROCTYPE_MIPSR10000
} t_proctype;

struct computer_hwinfo {
  char computername 
  t_arch architecture;		/* type of architecture */
  t_os os;			/* type of operating system */
  t_proctype proctype;		/* type of processors */
  int procspeed;		/* speed of the processors */
  int numproc;			/* number of processors that the computer has */
  int speedindex;		/* global speed index for making comparisons between different computers */
};

int get_hwinfo (struct computer_hwinfo *hwinfo);

#endif /* _COMPUTER_INFO_H_ */



