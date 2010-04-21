//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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

#ifndef _DRERRNO_H_
#define _DRERRNO_H_

#include <errno.h>

extern int drerrno;              // global errno
extern int drerrno_system;       // system's errno at the time of the error

#define DRE_NOERROR     0 /* No error */
#define DRE_ERROROPENING  1  /* Could not open file or directory */
#define DRE_NOTRESOLVE   2 /* Could not resolve hostname */
#define DRE_NOSOCKET    3 /* Could not open socket */
#define DRE_NOCONNECT    4 /* Could not connect */
#define DRE_ANSWERNOTLISTED 5 /* Answer received not listed */
#define DRE_ANSWERNOTRIGHT 6 /* Not the right answer to this request */
#define DRE_ERRORWRITING  7 /* Could not write on a file or socket */
#define DRE_ERRORREADING  8 /* Could not read from file or socket */
#define DRE_NOTREGISTERED  9 /* Not registered */
#define DRE_NOENVMASTER   10 /* Environment variable for master not set */
#define DRE_NOENVROOT    11 /* Environment variable for root not set */
#define DRE_COULDNOTCREATE 12 /* Could not create the file or directory */
#define DRE_NOTCOMPLETE   13 /* Information not complete */
#define DRE_DIFFILEFORMAT  14 /* Different file format than expected */
#define DRE_DIFVERSION   15 /* Different version number than expected */
#define DRE_DIFJOBSIZE   16 /* Different jobsize (number of jobs) than expected */
#define DRE_GETSHMEM    17 /* Could not allocate (Get) shared memory */
#define DRE_ATTACHSHMEM   18 /* Could not attach shared memory */
#define DRE_NOTMPDIR    19 /* No temporary directory */
#define DRE_NODBDIR     20 /* No database directory */
#define DRE_NOLOGDIR    21 /* No logs directory */
#define DRE_NOBINDIR    22 /* No bin directory */
#define DRE_NOETCDIR    23 /* No etc directory */
#define DRE_RMSHMEM     24 // Remove shared memory
#define DRE_NOMEMORY    25 // Not enough memory
#define DRE_DTSHMEM         26  // Detach shared memory
#define DRE_CONNMASTER      27  // No connection to master
#define DRE_COMMPROBLEM     28  // Communications problem

char *drerrno_str (void);

#endif /* _DRERRNO_H_ */
