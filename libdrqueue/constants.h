//
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#if defined (__CYGWIN)
#define SHELL_NAME "sh"
#define SHELL_PATH "/usr/bin/sh"
#else
#define SHELL_NAME "sh"
#define SHELL_PATH "/bin/sh"
#endif

// Default config files
#define BASE_CONF_PATH "/etc/drqueue"      // Default path for config files
#define BASE_CONF_EXT "conf"               // Extension for
                                           // configuration files

// Pools
#define DEFAULT_POOL "Default"

/* Possible who values */
#define MASTER 0
#define SLAVE 10
#define SLAVE_CHANDLER 11 /* Connection handler */
#define SLAVE_LAUNCHER 12 /* Child that launches the task and waits for it to finish */
#define CLIENT 20  /* General to clients, like sendjob or drqman or so */

/* Sizes */
#define MAXNAMELEN 128   /* Maxumum name length for computers and owners */
#define MAXCMDLEN 1024   /* Maximum command length for the task command */
#define BUFFERLEN 4096   /* General buffer len for text */
#define MAXJOBS 100      /* Maximum number of jobs */
#define MAXCOMPUTERS 100 /* Maximum number of computers */
#define MAXTASKS 32      /* Maximum number of tasks in the same computer */

/* Ports */
#define MASTERPORT 1234  /* Master listening port */
#define SLAVEPORT 2345   /* Slave listening port */

/* Delays */
#define SLAVEDELAY 15  /* Delay between each loop on the slave */
#define MAXTIMECONNECTION 30 /* Maximum time that can take to handle a request */
#define MASTERCCHECKSDELAY 10 /* Delay between every consistency check */
#define MAXTIMENOCONN 240 /* If this time is exceeded without connecting to the */
                          /* master the computer is erased from the db */


// Timeouts
#define MAXTASKLOADINGTIME 60

/* Others */
#define MAXLISTEN 300  /* Maximum number of connections waiting to be accepted */
#define MAXLOADAVG 160  /* Load average from which we consider a computer unavailable */
#define DFLTAVGFTIME 120 /* Default average frame time in seconds */

#define MASTERNCHILDREN 10 // Number of master listener procs

// Autoenable
#define AE_DELAY 120  /* The time between possible autoenables */
#define AE_HOUR 21
#define AE_MIN 00

#endif /* _CONSTANTS_H_ */

