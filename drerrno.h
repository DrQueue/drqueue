/* $Id: drerrno.h,v 1.11 2001/11/21 10:15:22 jorge Exp $ */

#ifndef _DRERRNO_H_
#define _DRERRNO_H_

extern int drerrno;		/* global errno */

#define DRE_NOERROR         0	/* No error */
#define DRE_ERROROPENING    1   /* Could not open file or directory */
#define DRE_NOTRESOLV       2	/* Could not resolve hostname */
#define DRE_NOSOCKET        3	/* Could not open socket */
#define DRE_NOCONNECT       4	/* Could not connect */
#define DRE_ANSWERNOTLISTED 5	/* Answer received not listed */
#define DRE_ANSWERNOTRIGHT  6	/* Not the right answer to this request */
#define DRE_ERRORWRITING    7	/* Could not write on a file or socket */
#define DRE_ERRORREADING    8	/* Could not read from file or socket */
#define DRE_NOTREGISTERED   9	/* Not registered */
#define DRE_NOENVMASTER     10	/* Environment variable for master not set */
#define DRE_NOENVROOT       11	/* Environment variable for root not set */
#define DRE_COULDNOTCREATE  12	/* Could not create the file or directory */
#define DRE_NOTCOMPLETE     13	/* Information not complete */
#define DRE_DIFFILEFORMAT   14	/* Different file format than expected */
#define DRE_DIFVERSION      15	/* Different version number than expected */
#define DRE_DIFJOBSIZE      16	/* Different jobsize (number of jobs) than expected */
#define DRE_GETSHMEM        17	/* Could not allocate (Get) shared memory */
#define DRE_ATTACHSHMEM     18	/* Could not attach shared memory */
#define DRE_NOTMPDIR        19	/* No temporary directory */
#define DRE_NODBDIR         20	/* No database directory */
#define DRE_NOLOGDIR        21  /* No logs directory */
#define DRE_NOBINDIR        22  /* No bin directory */
#define DRE_NOETCDIR        23	/* No etc directory */

char *drerrno_str (void);

#endif /* _DRERRNO_H_ */
