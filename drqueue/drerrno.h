/* $Id: drerrno.h,v 1.5 2001/08/27 08:14:13 jorge Exp $ */

#ifndef _DRERRNO_H_
#define _DRERRNO_H_

extern int drerrno;		/* global errno */

#define DRE_NOERROR         0	/* No error */
#define DRE_NODRMAENV       1	/* No DRQUEUE_MASTER on the environment */
#define DRE_NOTRESOLV       2	/* Could not resolve hostname */
#define DRE_NOSOCKET        3	/* Could not open socket */
#define DRE_NOCONNECT       4	/* Could not connect */
#define DRE_ANSWERNOTLISTED 5	/* Answer received not listed */
#define DRE_ANSWERNOTRIGHT  6	/* Not the right answer to this request */
#define DRE_ERRORSENDING    7	/* Error sending :) */
#define DRE_ERRORRECEIVING  8	/* Error receiving */
#define DRE_NOTREGISTERED   9	/* Not registered */

char *drerrno_str (void);

#endif /* _DRERRNO_H_ */
