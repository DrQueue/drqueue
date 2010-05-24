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

#ifndef _LIBDRQUEUE_H_
#define _LIBDRQUEUE_H_

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include <errno.h>

#if defined __CYGWIN32__ && !defined __CYGWIN__
/* For backwards compatibility with Cygwin b19 and
 *       earlier, we define __CYGWIN__ here, so that
 *             we can rely on checking just for that macro. */
#  define __CYGWIN__  __CYGWIN32__
#endif

#if defined _WIN32 && !defined __CYGWIN__
/* Use Windows separators on all _WIN32 defining
 *       environments, except Cygwin. */
#  define DIR_SEPARATOR_CHAR        '\\'
#  define DIR_SEPARATOR_STR         "\\"
#  define PATH_SEPARATOR_CHAR       ';'
#  define PATH_SEPARATOR_STR        ";"
#endif
#ifndef DIR_SEPARATOR_CHAR
/* Assume that not having this is an indicator that all
 *       are missing. */
#  define DIR_SEPARATOR_CHAR        '/'
#  define DIR_SEPARATOR_STR     "/"
#  define PATH_SEPARATOR_CHAR       ':'
#  define PATH_SEPARATOR_STR        ":"
#endif /* !DIR_SEPARATOR_CHAR */

#pragma pack(push,1)

extern int drerrno;

#include "pointer.h"
#include "slavedb.h"
#include "computer.h"
#include "job.h"
#include "task.h"
#include "logger.h"
#include "communications.h"
#include "request.h"
#include "drerrno.h"
#include "database.h"
#include "semaphores.h"
#include "common.h"
#include "envvars.h"
#include "jobscript.h"
#include "config.h"
#include "list.h"
#include "computer_pool.h"

/* Script generators */
#include "generalsg.h"
#include "mayasg.h"
#include "mentalraysg.h"
#include "blendersg.h"
#include "pixiesg.h"
#include "3delightsg.h"
#include "lightwavesg.h"
#include "aftereffectssg.h"
#include "shakesg.h"
#include "terragensg.h"
#include "nukesg.h"
#include "aqsissg.h"
#include "mantrasg.h"
#include "turtlesg.h"
#include "xsisg.h"
#include "cinema4dsg.h"
#include "luxrendersg.h"
#include "vraysg.h"
#include "3dsmaxsg.h"

extern int phantom[2];         /* FIXME: This should be local to the
				  slave */

#pragma pack(pop)

#ifdef __CPLUSPLUS
}
#endif

#endif /* _libdrqueue_h_ */
