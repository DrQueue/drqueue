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

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#if defined (__CYGWIN)
#define SHELL_NAME "sh"
#define SHELL_PATH "/usr/bin/sh"
#else
#define SHELL_NAME "sh"
#define SHELL_PATH "/bin/sh"
#endif

// Default config files
#define BASE_CONF_PATH "/etc/drqueue"      // Default path for config files
#define BASE_CONF_EXT "conf"               // Extension for
                                           // configuration files

// Pools
#define DEFAULT_POOL "Default"

/* Possible who values */
#define MASTER 0
#define SLAVE 10
#define SLAVE_CHANDLER 11 /* Connection handler */
#define SLAVE_LAUNCHER 12 /* Child that launches the task and waits for it to finish */
#define CLIENT 20  /* General to clients, like sendjob or drqman or so */

/* Sizes */
#define MAXNAMELEN 128   /* Maxumum name length for computers and owners */
#define MAXCMDLEN 1024   /* Maximum command length for the task command */
#define BUFFERLEN 4096   /* General buffer len for text */
#define MAXJOBS 100      /* Maximum number of jobs */
#define MAXCOMPUTERS 100 /* Maximum number of computers */
#define MAXTASKS 32      /* Maximum number of tasks in the same computer */

/* Ports */
#define MASTERPORT 1234  /* Master listening port */
#define SLAVEPORT 2345   /* Slave listening port */

/* Delays */
#define SLAVEDELAY 15  /* Delay between each loop on the slave */
#define MAXTIMECONNECTION 30 /* Maximum time that can take to handle a request */
#define MASTERCCHECKSDELAY 10 /* Delay between every consistency check */
#define MAXTIMENOCONN 240 /* If this time is exceeded without connecting to the */
                          /* master the computer is erased from the db */


// Timeouts
#define MAXTASKLOADINGTIME 60

/* Others */
#define MAXLISTEN 300  /* Maximum number of connections waiting to be accepted */
#define MAXLOADAVG 160  /* Load average from which we consider a computer unavailable */
#define DFLTAVGFTIME 120 /* Default average frame time in seconds */

#define MASTERNCHILDREN 10 // Number of master listener procs

// Autoenable
#define AE_DELAY 120  /* The time between possible autoenables */
#define AE_HOUR 21
#define AE_MIN 00

/*
 * The following types have been taken from the stdint.h that is part of the mingw.
 */
#ifndef HAVE_STDINT_H
/* 7.18.1.1  Exact-width integer types */
typedef signed char int8_t;
typedef unsigned char   uint8_t;
typedef short  int16_t;
typedef unsigned short  uint16_t;
typedef int  int32_t;
typedef unsigned   uint32_t;
typedef long long  int64_t;
typedef unsigned long long   uint64_t;

/* 7.18.1.2  Minimum-width integer types */
typedef signed char int_least8_t;
typedef unsigned char   uint_least8_t;
typedef short  int_least16_t;
typedef unsigned short  uint_least16_t;
typedef int  int_least32_t;
typedef unsigned   uint_least32_t;
typedef long long  int_least64_t;
typedef unsigned long long   uint_least64_t;

/*  7.18.1.3  Fastest minimum-width integer types 
 *  Not actually guaranteed to be fastest for all purposes
 *  Here we use the exact-width types for 8 and 16-bit ints. 
 */
typedef char int_fast8_t;
typedef unsigned char uint_fast8_t;
typedef short  int_fast16_t;
typedef unsigned short  uint_fast16_t;
typedef int  int_fast32_t;
typedef unsigned  int  uint_fast32_t;
typedef long long  int_fast64_t;
typedef unsigned long long   uint_fast64_t;

/* 7.18.1.4  Integer types capable of holding object pointers */

#ifndef _INTPTR_T_DEFINED
#define _INTPTR_T_DEFINED
#ifdef _WIN64
  typedef __int64 intptr_t;
#else
  typedef int intptr_t;
#endif
#endif

#ifndef _UINTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
#ifdef _WIN64
  typedef unsigned __int64 uintptr_t;
#else
  typedef unsigned int uintptr_t;
#endif
#endif

/* 7.18.1.5  Greatest-width integer types */
typedef long long  intmax_t;
typedef unsigned long long uintmax_t;

/* 7.18.2  Limits of specified-width integer types */
#if !defined ( __cplusplus) || defined (__STDC_LIMIT_MACROS)

/* 7.18.2.1  Limits of exact-width integer types */
#define INT8_MIN (-128) 
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN  (-9223372036854775807LL - 1)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL

#define UINT8_MAX 0xff /* 255U */
#define UINT16_MAX 0xffff /* 65535U */
#define UINT32_MAX 0xffffffff  /* 4294967295U */
#define UINT64_MAX 0xffffffffffffffffULL /* 18446744073709551615ULL */

/* 7.18.2.2  Limits of minimum-width integer types */
#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN

#define INT_LEAST8_MAX INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX

#define UINT_LEAST8_MAX UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

/* 7.18.2.3  Limits of fastest minimum-width integer types */
#define INT_FAST8_MIN INT8_MIN
#define INT_FAST16_MIN INT16_MIN
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST64_MIN INT64_MIN

#define INT_FAST8_MAX INT8_MAX
#define INT_FAST16_MAX INT16_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST64_MAX INT64_MAX

#define UINT_FAST8_MAX UINT8_MAX
#define UINT_FAST16_MAX UINT16_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX

/* 7.18.2.4  Limits of integer types capable of holding
    object pointers */
#ifdef _WIN64
#define INTPTR_MIN INT64_MIN
#define INTPTR_MAX INT64_MAX
#define UINTPTR_MAX UINT64_MAX
#else
#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX
#define UINTPTR_MAX UINT32_MAX
#endif

/* 7.18.2.5  Limits of greatest-width integer types */
#define INTMAX_MIN INT64_MIN
#define INTMAX_MAX INT64_MAX
#define UINTMAX_MAX UINT64_MAX

/* 7.18.3  Limits of other integer types */
#define PTRDIFF_MIN INTPTR_MIN
#define PTRDIFF_MAX INTPTR_MAX

#define SIG_ATOMIC_MIN INTPTR_MIN
#define SIG_ATOMIC_MAX INTPTR_MAX

#ifndef WCHAR_MIN  /* also in wchar.h */ 
#define WCHAR_MIN 0
#define WCHAR_MAX 0xffff /* UINT16_MAX */
#endif

/*
 * wint_t is unsigned short for compatibility with MS runtime
 */
#define WINT_MIN 0
#define WINT_MAX 0xffff /* UINT16_MAX */

#endif /* !defined ( __cplusplus) || defined __STDC_LIMIT_MACROS */


/* 7.18.4  Macros for integer constants */
#if !defined ( __cplusplus) || defined (__STDC_CONSTANT_MACROS)

/* 7.18.4.1  Macros for minimum-width integer constants

    Accoding to Douglas Gwyn <gwyn@arl.mil>:
	"This spec was changed in ISO/IEC 9899:1999 TC1; in ISO/IEC
	9899:1999 as initially published, the expansion was required
	to be an integer constant of precisely matching type, which
	is impossible to accomplish for the shorter types on most
	platforms, because C99 provides no standard way to designate
	an integer constant with width less than that of type int.
	TC1 changed this to require just an integer constant
	*expression* with *promoted* type."
*/

#define INT8_C(val) ((int8_t) + (val))
#define UINT8_C(val) ((uint8_t) + (val##U))
#define INT16_C(val) ((int16_t) + (val))
#define UINT16_C(val) ((uint16_t) + (val##U))

#define INT32_C(val) val##L
#define UINT32_C(val) val##UL
#define INT64_C(val) val##LL
#define UINT64_C(val) val##ULL

/* 7.18.4.2  Macros for greatest-width integer constants */
#define INTMAX_C(val)  INT64_C(val)
#define UINTMAX_C(val) UINT64_C(val)

#endif  /* !defined ( __cplusplus) || defined __STDC_CONSTANT_MACROS */
#endif

#endif /* _CONSTANTS_H_ */

