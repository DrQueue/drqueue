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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "database.h"
#include "computer.h"
#include "communications.h"
#include "drerrno.h"
#include "logger.h"

void
database_init (struct database *wdb)
{
	int i;

	for (i = 0; i < MAXJOBS; i++) {
		job_init (&wdb->job[i]);
	}

	for (i = 0; i < MAXCOMPUTERS; i++) {
		computer_init (&wdb->computer[i]);
	}

#ifdef COMM_REPORT
	wdb->bsent = wdb->brecv = 0;
#endif
}

int
database_load (struct database *wdb)
{
	/* This function returns 1 on success and 0 on failure */
	/* It logs failure and maybe it should leave that task to the calling function */
	/* README : this function writes to the database without locking */
	struct database_hdr hdr;
	char *basedir;
	char filename[BUFFERLEN];
	int fd;
	int c, d;											/* counters */
	struct frame_info *fi;
	int nframes;

	if ((basedir = getenv ("DRQUEUE_DB")) == NULL) {
		/* This should never happen because we check it at the beginning of the program */
		drerrno = DRE_NOENVROOT;
		return 0;
	}

	snprintf (filename, BUFFERLEN - 1, "%s/drqueue.db", basedir);
	if ((fd = open (filename, O_RDONLY)) == -1) {
		drerrno = DRE_ERROROPENING;
		return 0;
	}

	read_32b (fd, &hdr.magic);
	if (hdr.magic != DB_MAGIC) {
		drerrno = DRE_DIFFILEFORMAT;
		close (fd);
		return 0;
	}
	read_32b (fd, &hdr.version);
	if (hdr.version != DB_VERSION) {
		drerrno = DRE_DIFVERSION;
		close (fd);
		return 0;
	}
	read_16b (fd, &hdr.job_size);
	if (hdr.job_size != MAXJOBS) {
		drerrno = DRE_DIFJOBSIZE;
		close (fd);
		return 0;
	}

	for (c = 0; c < hdr.job_size; c++) {
		recv_job (fd, &wdb->job[c]);
		if (wdb->job[c].used) {
			nframes = job_nframes (&wdb->job[c]);
			if ((wdb->job[c].fishmid = get_frame_shared_memory (nframes)) == -1) {
				drerrno = DRE_GETSHMEM;
				close (fd);
				return 0;
			}
			if ((fi =
					 attach_frame_shared_memory (wdb->job[c].fishmid)) == (void *) -1) {
				drerrno = DRE_ATTACHSHMEM;
				close (fd);
				return 0;
			}
			for (d = 0; d < nframes; d++) {
				if (!recv_frame_info (fd, &fi[d])) {
					/* CHECK : If there is an error we should FREE the allocated shared memory */
					job_delete(&wdb->job[c]);
					drerrno = DRE_ERRORREADING;
					close (fd);
					return 0;
				}
			}
			detach_frame_shared_memory (fi);
		}
	}

	drerrno = DRE_NOERROR;
	close (fd);
	return 1;
}

int
database_save (struct database *wdb)
{
	/* This function returns 1 on success and 0 on failure */
	/* It logs failure and maybe it should leave that task to the calling function */
	/* README : this function reads from the database without locking */
	struct database_hdr hdr;
	char *basedir;
	char dir[BUFFERLEN];
	char filename[BUFFERLEN];
	int fd;
	int c;
	struct frame_info *fi;

	if ((basedir = getenv ("DRQUEUE_DB")) == NULL) {
		/* This should never happen because we check it at the beginning of the program */
		drerrno = DRE_NOENVROOT;
		return 0;
	}

	snprintf (dir, BUFFERLEN - 1, "%s", basedir);
	snprintf (filename, BUFFERLEN - 1, "%s/drqueue.db", dir);
	if ((fd = open (filename, O_CREAT | O_TRUNC | O_RDWR, 0664)) == -1) {
		if (errno == ENOENT) {
			/* If its because the directory does not exist we try creating it first */
			if (mkdir (dir, 0775) == -1) {
				log_master (L_WARNING,
										"Could not create database directory. Check permissions: %s",
										dir);
				drerrno = DRE_COULDNOTCREATE;
				return 0;
			}
			if ((fd = open (filename, O_CREAT | O_TRUNC | O_RDWR, 0664)) == -1) {
				log_master (L_WARNING,
										"Could not open database file for writing. Check permissions: %s",
										filename);
				drerrno = DRE_COULDNOTCREATE;
				return 0;
			}
		}
		else {
			/* could not open the file for other reasons */
			log_master (L_WARNING,
									"Could not open database file for writing. Check permissions: %s",
									filename);
			drerrno = DRE_COULDNOTCREATE;
			return 0;
		}
	}

	hdr.magic = DB_MAGIC;
	hdr.version = DB_VERSION;
	hdr.job_size = MAXJOBS;

	write_32b (fd, &hdr.magic);
	write_32b (fd, &hdr.version);
	write_16b (fd, &hdr.job_size);

	for (c = 0; c < MAXJOBS; c++) {
		if (!send_job (fd, &wdb->job[c])) {
			return 0;
		}
		if (wdb->job[c].used) {
			int nframes = job_nframes (&wdb->job[c]);
			int i;
			if ((fi =
					 attach_frame_shared_memory (wdb->job[c].fishmid)) == (void *) -1) {
				/* If we fail to attach the frame shared memory we need to save empty frames */
				/* because we already save the info about the job and then when loading it will try */
				/* to load the number of frames there specified */
				struct frame_info fi2;
				job_frame_info_init (&fi2);
				for (i = 0; i < nframes; i++) {
					/* So we save empty frame infos */
					if (!send_frame_info (fd, &fi2)) {
						return 0;
					}
				}
			}
			else {
				/* We have the frame info attached */
				for (i = 0; i < nframes; i++) {
					if (!send_frame_info (fd, &fi[i])) {
						detach_frame_shared_memory (fi);
						return 0;
					}
				}
				detach_frame_shared_memory (fi);
			}
		}
	}

	return 1;
}
