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
// $Id$
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

void usage (void);
void print_jobs (struct job *job, int njobs);
void print_job (struct job *job);
void show_job (struct job *job);

enum operation {
	OP_NONE,
	OP_NUMBER,     // Returns the number of jobs in the queue
	OP_LIST,       // List basic info about all jobs in the queue
	OP_SHOWONE     // Should give more detailed information on a single job
};

int main (int argc,char *argv[])
{
  int opt;
  uint32_t ijob = -1;
	struct job *job = NULL;
	int njobs = 0;
	enum operation op = OP_NONE;

  while ((opt = getopt (argc,argv,"lnj:vh")) != -1) {
    switch (opt) {
    case 'j':
      ijob = atoi (optarg);
			op = OP_SHOWONE;
      break;
    case 'v':
      show_version (argv);
      exit (0);
		case 'l':
			op = OP_LIST;
			break;
		case 'n':
			op = OP_NUMBER;
			break;
    case '?':
    case 'h':
      usage();
      exit (1);
    }
  }

  if ((op == OP_NONE)) {
    usage ();
    exit (1);
  }

	set_default_env();

  if (!common_environment_check()) {
    fprintf (stderr,"Error checking the environment: %s\n",drerrno_str());
    exit (1);
  }

	switch (op) {
	case OP_NONE:
		break;
	case OP_LIST:
	case OP_NUMBER:
		if ((njobs = request_job_list (&job,CLIENT)) == -1) {
			fprintf (stderr,"ERROR: While trying to request the job list: %s\n",drerrno_str());
			exit (1);
		}
		break;
	case OP_SHOWONE:
		break;
	}
	
	switch (op) {
	case OP_NONE:
		printf ("OP_NONE\n");
		break;
	case OP_LIST:
		print_jobs (job,njobs);
		break;
	case OP_NUMBER:
		printf ("%i\n",njobs);
		break;
	case OP_SHOWONE:
		job = (struct job *) malloc (sizeof (struct job));
		if (!request_job_xfer(ijob,job,CLIENT)) {
			fprintf (stderr,"ERROR: Could not receive job information for ID (%i) : %s\n",ijob,drerrno_str());
			exit (1);
		}
		show_job (job);
		free (job);
		job = NULL;
		break;
	}

  exit (0);
}

void print_jobs (struct job *job, int njobs)
{
	int i;
	for (i=0;i<njobs;i++) {
		print_job (&job[i]);
	}
}

void print_job (struct job *job)
{
	printf ("ID: %i Name: %s\n",job->id,job->name);
}

void show_job (struct job *job)
{
	printf ("ID: %i Name: %s\n",job->id,job->name);
}

void usage (void)
{
    fprintf (stderr,"Usage: jobinfo [-vh] -l\n"
						 "Valid options:\n"
						 "\t-l list jobs\n"
						 "\t-n returns the number of jobs\n"
						 "\t-j <job_id> shows detailed information about a single job\n"
						 "\t-v print version\n"
						 "\t-h print this help\n");
}
