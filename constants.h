/* $Id: constants.h,v 1.3 2001/05/02 16:12:33 jorge Exp $ */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define MAXJOBS 100		/* Maximum number of jobs */
#define MAXCOMPUTERS 100	/* Maximum number of computers */
#define MASTERPORT 1234		/* Master listening port */
#define SLAVEPORT 2345		/* Slave listening port */
#define MAXLISTEN 100		/* Maximum number of connections waiting to be accepted */

#define MAXTASKS 32		/* Maximum number of tasks in the same computer */
#define MAXNAMELEN 64		/* Maxumum name length for computers and owners */
#define MAXCMDLEN 256		/* Maximum command length for the task command */
#define BUFFERLEN 256		/* General buffer len for text */

#endif /* _CONSTANTS_H_ */
