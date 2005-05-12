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
void print_computers (struct computer *computer, int ncomputers);
void print_computer (struct computer *computer);

enum operation {
	OP_NONE,
	OP_NUMBER,
	OP_LIST
};

int main (int argc,char *argv[])
{
  int opt;
  uint32_t icomp = -1;
	struct computer *computer;
	int ncomputers;
	enum operation op = OP_NONE;

  while ((opt = getopt (argc,argv,"lnc:vh")) != -1) {
    switch (opt) {
    case 'c':
      icomp = atoi (optarg);
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

  if ((ncomputers = request_computer_list (&computer,CLIENT)) == -1) {
    fprintf (stderr,"ERROR: While trying to request the computer list: %s\n",drerrno_str());
    exit (1);
  }

	switch (op) {
	case OP_NONE:
		printf ("OP_NONE\n");
		break;
	case OP_LIST:
		print_computers (computer,ncomputers);
		break;
	case OP_NUMBER:
		printf ("%i\n",ncomputers);
		break;
	}

  exit (0);
}

void print_computers (struct computer *computer, int ncomputers)
{
	int i;
	for (i=0;i<ncomputers;i++) {
		print_computer (&computer[i]);
	}
}

void print_computer (struct computer *computer)
{
	printf ("ID: %i Name: %s\n",computer->hwinfo.id,computer->hwinfo.name);
}

void usage (void)
{
    fprintf (stderr,"Usage: compinfo [-vh] -l\n"
						 "Valid options:\n"
						 "\t-l list computers\n"
						 "\t-j <computer_id>\n"
						 "\t-v print version\n"
						 "\t-h print this help\n");
}
