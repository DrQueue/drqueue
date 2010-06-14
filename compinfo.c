//
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
// Copyright (C) 2010 Andreas Schroeder
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

#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "libdrqueue.h"

void usage (void);
void print_computers (struct computer *computer, int ncomputers);
void print_computer (struct computer *computer);
void print_computer_details (struct computer *computer);

enum operation {
  OP_NONE,
  OP_NUMBER,
  OP_LIST,
  OP_DETAILS
};

int main (int argc,char *argv[]) {
  int opt;
  int icomp = -1;
  struct computer *computer;
  int ncomputers;
  enum operation op = OP_NONE;

  while ((opt = getopt (argc,argv,"lndc:vh")) != -1) {
    switch (opt) {
    case 'd':
      op = OP_DETAILS;
      break;
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

  switch (op) {
  case OP_NONE:
    printf ("OP_NONE\n");
    break;
  case OP_LIST:
    if ((ncomputers = request_computer_list (&computer,CLIENT)) == -1) {
      fprintf (stderr,"ERROR: While trying to request the computer list: %s\n",drerrno_str());
      exit (1);
    }
    print_computers (computer,ncomputers);
    break;
  case OP_NUMBER:
    if ((ncomputers = request_computer_list (&computer,CLIENT)) == -1) {
      fprintf (stderr,"ERROR: While trying to request the computer list: %s\n",drerrno_str());
      exit (1);
    }
    printf ("%i\n",ncomputers);
    break;
  case OP_DETAILS:
    if (icomp != -1) {
      computer = (struct computer *) malloc (sizeof (struct computer));
      computer_init(computer);
      if (!computer) {
        fprintf (stderr,"ERROR: Not enough memory\n");
        exit (1);
      }
      if (!request_comp_xfer(icomp,computer,CLIENT)) {
        fprintf (stderr,"ERROR: While trying to request the computer transfer: %s\n",drerrno_str());
        exit (1);
      }
      print_computer_details (computer);
    } else {
      fprintf (stderr,"You need to specify the computer id using -c <computer_id>\n");
      exit(1);
    }
  }

  exit (0);
}

void print_computers (struct computer *computer, int ncomputers) {
  int i;
  for (i=0;i<ncomputers;i++) {
    print_computer (&computer[i]);
  }
}

void print_computer (struct computer *computer) {
  printf ("ID: %i Name: %s\n",computer->hwinfo.id,computer->hwinfo.name);
  // tidy up shared memory piece of computer
  computer_free(computer);
}

void print_computer_details (struct computer *computer) {
  printf ("ID: %i Name: %s\n",computer->hwinfo.id,computer->hwinfo.name);
  // tidy up shared memory piece of computer
  computer_free(computer);
}

void usage (void) {
  fprintf (stderr,"Usage: compinfo [-vh] -l|-n\n"
           "Valid options:\n"
           "\t-l list computers\n"
           "\t-d single computer details (needs -c)\n"
           "\t-n returns the number of computers\n"
           "\t-c <computer_id>\n"
           "\t-v print version\n"
           "\t-h print this help\n");
}
