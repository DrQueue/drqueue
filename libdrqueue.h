/* $Id: libdrqueue.h,v 1.7 2004/03/09 18:53:22 jorge Exp $ */
#ifndef _LIBDRQUEUE_H_
#define _LIBDRQUEUE_H_

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
#include "blendersg.h"
#include "bmrtsg.h"
#include "pixiesg.h"

int phantom[2];									/* FIXME: This should be local to the slave */

#endif /* _libdrqueue_h_ */
