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
/* $Id: requeue.c 771 2004-10-14 18:32:12Z jorge $ */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#include "libdrqueue.h"

int main (int argc,char *argv[])
{
	struct blocked_host *bh;
	uint16_t nblocked;
	int i;
	
	fprintf (stderr,"About to add host 0\n");
	request_job_add_blocked_host (0,0,CLIENT);
	fprintf (stderr,"About to add host 0 again\n");
	request_job_add_blocked_host (0,0,CLIENT);
	fprintf (stderr,"Requesting list of blocked hosts\n");
	request_job_list_blocked_host (0,&bh,&nblocked,CLIENT);
	fprintf (stderr,"Listing\n");
	for (i=0;i<nblocked;i++) {
		printf ("%s\n",bh[i].name);
	}
	fprintf (stderr,"About to delete blocked host\n");
	request_job_delete_blocked_host (0,0,CLIENT);
	fprintf (stderr,"About requesting listing again\n");
	request_job_list_blocked_host (0,&bh,&nblocked,CLIENT);
	fprintf (stderr,"Listing\n");
	for (i=0;i<nblocked;i++) {
		printf ("%s\n",bh[i].name);
	}
	fprintf (stderr,"About to add host 0 again\n");
	request_job_add_blocked_host (0,0,CLIENT);
	fprintf (stderr,"About requesting listing again\n");
	request_job_list_blocked_host (0,&bh,&nblocked,CLIENT);
	fprintf (stderr,"Listing\n");
	for (i=0;i<nblocked;i++) {
		printf ("%s\n",bh[i].name);
	}
	fprintf (stderr,"About to delete blocked host\n");
	request_job_delete_blocked_host (0,0,CLIENT);
	fprintf (stderr,"End\n");

	return 0;
}
