/* $Id: drerrno.h,v 1.1 2001/05/28 14:21:31 jorge Exp $ */

#ifndef _DRERRNO_H_
#define _DRERRNO_H_

int drerrno;			/* global errno */

#define DRE_NODRMAENV 1		/* No DRQUEUE_MASTER on the environment */
#define DRE_DRMANONVA 2		/* DRQUEUE_MASTER non valid */
#define DRE_NOSOCKET  3		/* Could not open socket */
#define DRE_NOCONNECT 4		/* Could not connect */

char *drerrno_str (void);

#endif /* _DRERRNO_H_ */
