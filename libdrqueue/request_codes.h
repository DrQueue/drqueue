//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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

#ifndef _REQUEST_CODES_H_
#define _REQUEST_CODES_H_

#define R_R_REGISTER  1 /* Request register computer */
#define R_R_UCSTATUS  2 /* Request to update computer status */
#define R_R_REGISJOB  3 /* Register a new job */
#define R_R_AVAILJOB  4 /* Request available job */
#define R_R_TASKFINI  5 /* Request task finished, the slave send this when finishes a task */
#define R_R_LISTJOBS  6 /* Request a list of current jobs in the queue */
#define R_R_LISTCOMP  7 /* Request a list of current registered computers */
#define R_R_DELETJOB  8 /* Request a job to be deleted */
#define R_R_STOPJOB   9 /* Stop a job */
#define R_R_CONTJOB   10 /* Continue a stopped job */
#define R_R_HSTOPJOB  11 /* Hard stop a job */
#define R_R_JOBXFER   12 /* Job info transfer by index (identification) */
#define R_R_JOBXFERFI  13 /* Job frame info transfer by index (identification) */
#define R_R_COMPXFER  14 /* Computer info transfer by index (identification) */
#define R_R_JOBFWAIT  15 /* Job frame waiting (only non-running frames) */
#define R_R_JOBFKILL  16 /* Job frame kill (kills only [it's later set to waiting because of the usual handling]) */
#define R_R_JOBFFINI  17 /* Job frame finished (only waiting frames) */
#define R_R_JOBFKFIN  18 /* Job frame kill and finished (only running frames) */
#define R_R_UCLIMITS  19 /* Update computer limits */
#define R_R_SLAVEXIT  20 /* Slave exit, one slave must be removed from the queue */
#define R_R_JOBSESUP  21 /* SES (Start,End,Step frames) update */
#define R_R_JOBLNMCS  22 /* Job limit nmaxcpus set */
#define R_R_JOBLNMCCS  23 /* Job limit nmaxcpuscomputer set */
#define R_R_JOBPRIUP  24 /* Job priority update */
#define R_R_JOBFINFO  25 // Job frame info
#define R_R_JOBFRSTRQD 26 // Job frame reset requeued
#define R_R_JOBBLKHOST 27 // Job block host
#define R_R_JOBDELBLKHOST 28 // Job block host delete
#define R_R_JOBLSTBLKHOST 29 // Job block host list
#define R_R_JOBLMS    30 // Job limits memory set
#define R_R_JOBLPS    31 // Job limits pool set
#define R_R_RERUNJOB   32 // ReRun a whole job
#define R_R_JOBENVVARS          33 // Job environment variables
#define R_R_JOBBLKHOSTNAME      34 // Job block a host by name
#define R_R_JOBUNBLKHOSTNAME    35 // Job block a host by name
#define R_R_JOBNAME             36 // Requesting job name

/* Requests to the slave */
#define RS_R_KILLTASK      1 /* Request to slave to kill a task */
#define RS_R_SETNMAXCPUS    2 /* Request to set limits nmaxcpus */
#define RS_R_SETMAXFREELOADCPU 3 /* Request to set limits maxfreeloadcpu */
#define RS_R_SETAUTOENABLE   4 /* Request to set autoenable info */
#define RS_R_JOBAVAILABLE    5 /* Request indicates master has a job available */
#define RS_R_LIMITSPOOLADD   6 // Add pool
#define RS_R_LIMITSPOOLREMOVE  7 // Remove pool
#define RS_R_SETENABLED     8 // Enable/disable slave

#endif /* _REQUEST_CODES_H_ */
