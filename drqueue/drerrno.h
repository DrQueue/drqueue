/* $Id: drerrno.h,v 1.8 2001/10/31 15:59:38 jorge Exp $ */

#ifndef _DRERRNO_H_
#define _DRERRNO_H_

extern int drerrno;		/* global errno */

#define DRE_NOERROR         0	/* No error */
/* FILLME */
#define DRE_NOTRESOLV       2	/* Could not resolve hostname */
#define DRE_NOSOCKET        3	/* Could not open socket */
#define DRE_NOCONNECT       4	/* Could not connect */
#define DRE_ANSWERNOTLISTED 5	/* Answer received not listed */
#define DRE_ANSWERNOTRIGHT  6	/* Not the right answer to this request */
#define DRE_ERRORSENDING    7	/* Error sending :) */
#define DRE_ERRORRECEIVING  8	/* Error receiving */
#define DRE_NOTREGISTERED   9	/* Not registered */
#define DRE_NOENVMASTER     10	/* Environment variable for master not set */
#define DRE_NOENVROOT       11	/* Environment variable for root not set */
#define DRE_COULDNOTCREATE  12	/* Could not create the file or directory */
#define DRE_NOTCOMPLETE     13	/* Information not complete */

char *drerrno_str (void);

#endif /* _DRERRNO_H_ */
