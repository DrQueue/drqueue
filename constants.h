/* $Id: constants.h,v 1.14 2001/11/23 15:29:07 jorge Exp $ */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define SHELL_NAME "sh"
#define SHELL_PATH "/bin/sh"

/* Possible who values */
#define MASTER 0
#define SLAVE 10
#define SLAVE_CHANDLER 11	/* Connection handler */
#define SLAVE_LAUNCHER 12	/* Child that launches the task and waits for it to finish */
#define CLIENT 20		/* General to clients, like sendjob or drqman or so */

/* Sizes */
#define MAXNAMELEN 64		/* Maxumum name length for computers and owners */
#define MAXCMDLEN 256		/* Maximum command length for the task command */
#define BUFFERLEN 256		/* General buffer len for text */
#define MAXJOBS 100		/* Maximum number of jobs */
#define MAXCOMPUTERS 100	/* Maximum number of computers */
#define MAXTASKS 32		/* Maximum number of tasks in the same computer */

/* Ports */
#define MASTERPORT 1234		/* Master listening port */
#define SLAVEPORT 2345		/* Slave listening port */

/* Delays */
#define SLAVEDELAY 15		/* Delay between each loop on the slave */
#define MAXTIMECONNECTION 20	/* Maximum time that can take to handle a request */
#define MASTERCCHECKSDELAY 10	/* Delay between every consistency check */
#define MAXTIMENOCONN 30	/* If this time is exceeded without connecting to the */
                                /* master the computer is erased from the db */

/* Others */
#define MAXLISTEN 100		/* Maximum number of connections waiting to be accepted */
#define MAXLOADAVG 80		/* Load average from which we consider a computer unavailable */
#define DFLTAVGFTIME 120	/* Default average frame time in seconds */

#endif /* _CONSTANTS_H_ */

