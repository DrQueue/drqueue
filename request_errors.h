/* $Id: request_errors.h,v 1.4 2001/07/05 10:53:24 jorge Exp $ */

#ifndef _REQUEST_ERRORS_H_
#define _REQUEST_ERRORS_H_

#define RERR_NOERROR 0		/* No error */
#define RERR_ALREADY 1		/* Computer or job already registered */
#define RERR_NOSPACE 2		/* No space on database */
#define RERR_NOREGIS 3		/* Computer or job not registered */
#define RERR_NOAVJOB 4		/* No available job */
#define RERR_NOTINRA 5		/* Not in range frame number */

#endif /* _REQUEST_ERRORS_H_ */
