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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
// 
// $Id: requeue.c 771 2004-10-14 18:32:12Z jorge $
//

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#include "libdrqueue.h"

#define ACTION_NONE   0
#define ACTION_STOP   1
#define ACTION_HSTOP  2
#define ACTION_CONT   3
#define ACTION_DEL    4
#define ACTION_STATUS	5

void usage (void);

int main (int argc,char *argv[])
{
  int opt;
  uint32_t ijob = -1;
	int action = ACTION_NONE;
	struct job job;

  while ((opt = getopt (argc,argv,"sdcktj:vh")) != -1) {
    switch (opt) {
    case 's':
			action = ACTION_STOP;
      break;
		case 'd':
			action = ACTION_DEL;
			break;
		case 'c':
			action = ACTION_CONT;
			break;
		case 'k':
			action = ACTION_HSTOP;
			break;
		case 't':
			action = ACTION_STATUS;
			break;
    case 'j':
      ijob = atoi (optarg);
      break;
    case 'v':
      show_version (argv);
      exit (0);
    case '?':
    case 'h':
      usage();
      exit (1);
    }
  }

  if ((ijob == -1) || (action == ACTION_NONE)) {
    usage ();
    exit (1);
  }

	set_default_env();

  if (!common_environment_check()) {
    fprintf (stderr,"Error checking the environment: %s\n",drerrno_str());
    exit (1);
  }
	
	switch (action) {
	case ACTION_STOP:
		printf ("Stopping job: %i\n",ijob);
		request_job_stop(ijob,CLIENT);
		break;
	case ACTION_HSTOP:
		printf ("Hard stopping job: %i\n",ijob);
		request_job_hstop(ijob,CLIENT);
		break;
	case ACTION_DEL:
		printf ("Deleting job: %i\n",ijob);
		request_job_delete(ijob,CLIENT);
		break;
	case ACTION_CONT:
		printf ("Continue job: %i\n",ijob);
		request_job_continue(ijob,CLIENT);
		break;
	case ACTION_STATUS:
		request_job_xfer(ijob,&job,CLIENT);
		printf ("%s\n",job_status_string(job.status));
		break;
	}

  exit (0);
}

void usage (void)
{
    fprintf (stderr,"Usage: blockhost [-vh] -[s|k|d|c|s] -j <job_id>\n"
						 "Valid options:\n"
						 "\t-s will do a soft stop\n"
						 "\t-k will do a hard stop (be careful, running tasks will be killed)\n"
						 "\t-d will delete the job (be careful, no confirmation asked)\n"
						 "\t-c will continue a previously stopped job\n"
						 "\t-t show the status of the job\n"
						 "\t-j <job_id>\n"
						 "\t-v print v	ersion\n"
						 "\t-h print this help	\n");
}
