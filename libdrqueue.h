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
// $Id$
//

#ifndef _LIBDRQUEUE_H_
#define _LIBDRQUEUE_H_

#ifdef __CPLUSPLUS
extern "C" {
#endif 

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

/* Script generators */
#include "mayasg.h"
#include "mentalraysg.h"
#include "blendersg.h"
#include "bmrtsg.h"
#include "pixiesg.h"
#include "3delightsg.h"
#include "lightwavesg.h"
#include "aftereffectssg.h"
#include "shakesg.h"
#include "terragensg.h"
#include "nukesg.h"
#include "aqsissg.h"

extern int phantom[2];									/* FIXME: This should be local to the slave */

#ifdef __CPLUSPLUS
}
#endif 

#endif /* _libdrqueue_h_ */
