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

#define ACTION_NONE 0
#define ACTION_ADD  1
#define ACTION_DEL  2
#define ACTION_LIST 3

void usage (void);

int main (int argc,char *argv[])
{
  int opt;
  uint32_t ijob = -1;
	uint32_t icomp = -1;
	int action = ACTION_NONE;
	struct blocked_host *bh;
	uint16_t nblocked;
	int i;
	
  while ((opt = getopt (argc,argv,"lj:a:d:vh")) != -1) {
    switch (opt) {
    case 'a':
			action = ACTION_ADD;
      icomp = atoi (optarg);
      break;
		case 'd':
			action = ACTION_DEL;
			icomp = atoi (optarg);
			break;
		case 'l':
			action = ACTION_LIST;
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
		case ACTION_ADD:
  		if (!request_job_add_blocked_host (ijob,icomp,CLIENT)) {
		    fprintf (stderr,"ERROR: While trying to add host to block list: %s\n",drerrno_str());
		    exit (1);
  		}
  		printf ("Host blocked successfully\n");
			break;
		case ACTION_DEL:
  		if (!request_job_delete_blocked_host (ijob,icomp,CLIENT)) {
		    fprintf (stderr,"ERROR: While trying to delete host from block list: %s\n",drerrno_str());
		    exit (1);
  		}
  		printf ("Host unblocked successfully\n");
			break;
		case ACTION_LIST:
			request_job_list_blocked_host (ijob,&bh,&nblocked,CLIENT);
			for (i=0;i<nblocked;i++) {
				printf ("%i\t%s\n",i,bh[i].name);
			}
			break;
	}

  exit (0);
}

void usage (void)
{
    fprintf (stderr,"Usage: blockhost [-vh] -c <computer_id> -j <job_id>\n"
										"       blockhost [-vh] -d <computer_pos> -j <job_id>\n"
										"       blockhost [-vh] -l -j <job_id>\n"
	     "Valid options:\n"
	     "\t-a <computer_id> adds computer to the list of blocked hosts\n"
			 "\t-d <computer_pos> deletes computer on position computer_pos from the list of blocked hosts\n"
	     "\t-l will print a list of blocked hosts for a job\n"
	     "\t-j <job_id>\n"
	     "\t-v print version\n"
	     "\t-h print this help\n");
}
