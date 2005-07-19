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
// $Id: /drqueue/remote/trunk/constants.h 2252 2005-05-02T02:35:47.989705Z jorge	$
//

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#if defined (__CYGWIN)
#define SHELL_NAME "tcsh.exe"
#define SHELL_PATH ""
#else
#define SHELL_NAME "sh"
#define SHELL_PATH "/bin/sh"
#endif

// Pools
#define DEFAULT_POOL "Default"

/* Possible who values */
#define MASTER 0
#define SLAVE 10
#define SLAVE_CHANDLER 11 /* Connection handler */
#define SLAVE_LAUNCHER 12 /* Child that launches the task and waits for it to finish */
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
#ifdef __CYGWIN
#define SLAVEDELAY 10		/* Delay between each loop on the slave */
#define MAXTIMECONNECTION 30	/* Maximum time that can take to handle a request */
#define MASTERCCHECKSDELAY 10 /* Delay between every consistency check */
#else
#define SLAVEDELAY 15		/* Delay between each loop on the slave */
#define MAXTIMECONNECTION 60	/* Maximum time that can take to handle a request */
#define MASTERCCHECKSDELAY 10 /* Delay between every consistency check */
#endif
#define MAXTIMENOCONN 240 /* If this time is exceeded without connecting to the */
																/* master the computer is erased from the db */

/* Others */
#define MAXLISTEN 100		/* Maximum number of connections waiting to be accepted */
#define MAXLOADAVG 80		/* Load average from which we consider a computer unavailable */
#define DFLTAVGFTIME 120	/* Default average frame time in seconds */

#ifdef __CYGWIN
#define MASTERNCHILDREN 20 // Number of master listener procs
#else
#define MASTERNCHILDREN 20 // Number of master listener procs
#endif

// Autoenable
#define AE_DELAY 120		/* The time between possible autoenables */
#define AE_HOUR 21
#define AE_MIN	00

#endif /* _CONSTANTS_H_ */

