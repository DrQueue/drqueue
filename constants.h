/* $Id: constants.h,v 1.8 2001/07/04 10:13:59 jorge Exp $ */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define SHELL_NAME "sh"
#define SHELL_PATH "/bin/sh"

#define MASTER 0
#define SLAVE 1
#define SLAVE_CHANDLER 2
#define CLIENT 3		/* General to clients, like sendjob or so */

#define SLAVEDELAY 15		/* Delay between each loop on the slave */
#define MAXJOBS 100		/* Maximum number of jobs */
#define MAXCOMPUTERS 100	/* Maximum number of computers */
#define MASTERPORT 1234		/* Master listening port */
#define SLAVEPORT 2345		/* Slave listening port */
#define MAXLISTEN 100		/* Maximum number of connections waiting to be accepted */
#define MAXTIMECONNECTION 30	/* Maximum time that can take to handle a request */
#define MAXTASKS 32		/* Maximum number of tasks in the same computer */
#define MAXNAMELEN 64		/* Maxumum name length for computers and owners */
#define MAXCMDLEN 256		/* Maximum command length for the task command */
#define BUFFERLEN 256		/* General buffer len for text */
#define MAXLOADAVG 80		/* Load average from which we consider a computer unavailable */
#define MASTERCCHECKSDELAY 10	/* Delay between every consistency check */
#define MAXTIMENOCONN 30	/* If this time is exceeded without connecting to the */
                                /* master the computer is erased from the db */
#define MAXFRAMES 1024		/* Maximum number of frames / 8 (so the real maximum is 8192) */
				/* cause I use 1 bit for each frame */
#define DFLTAVGFTIME 120	/* Default average frame time in seconds */

#endif /* _CONSTANTS_H_ */

