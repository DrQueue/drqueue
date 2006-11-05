//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
//
// DrQueue is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// DrQueue is distributed in the hope that it will be useful,
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
#include <stdint.h>

#include "database.h"
#include "computer.h"
#include "communications.h"
#include "drerrno.h"
#include "logger.h"

void
database_init (struct database *wdb) {
  uint32_t i;

  for (i = 0; i < MAXJOBS; i++) {
    envvars_init(&wdb->job[i].envvars); // First we set proper
                                        // defaults for the envvars to
                                        // avoid Warning messages
    job_init (&wdb->job[i]);
  }

  for (i = 0; i < MAXCOMPUTERS; i++) {
    computer_init (&wdb->computer[i]);
  }

#ifdef COMM_REPORT
  wdb->bsent = wdb->brecv = 0;
#endif

  wdb->lb.last_priority = 0;
  wdb->lb.next_i = 0;
}

uint32_t
database_version_id () {
  // The real version number stored on the database will depend on the
  // DB_VERSION number as well as the size of a pointer to void
  // (characteristic of 64/32 bits architecture)
  uint32_t version_id;
  version_id = ((DB_VERSION << 16) | sizeof (void*));
  return version_id;
}

int
database_load (struct database *wdb) {
  /* This function returns 1 on success and 0 on failure */
  /* It logs failure and maybe it should leave that task to the calling function */
  /* README : this function writes to the database without locking */
  struct database_hdr hdr;
  char *basedir;
  char filename[BUFFERLEN];
  int fd;
  int c;           /* counters */

  // TODO: no filename guessing.
  if ((basedir = getenv ("DRQUEUE_DB")) == NULL) {
    /* This should never happen because we check it at the beginning of the program */
    drerrno = DRE_NOENVROOT;
    return 0;
  }

#ifndef __CYGWIN
  snprintf (filename, BUFFERLEN - 1, "%s/drqueue.db", basedir);
#else

  snprintf (filename, BUFFERLEN - 1, "%s\\drqueue.db", basedir);
#endif

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
  if (hdr.version != database_version_id()) {
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
    job_init(&wdb->job[c]);
    if (!recv_job (fd, &wdb->job[c])) {
      // TODO
      return 0;
    }
    if (wdb->job[c].used) {
      if (!database_job_load_frames(fd,&wdb->job[c]))
	return 0;
      if (!database_job_load_blocked_hosts(fd,&wdb->job[c]))
	return 0;
    }
  }

  drerrno = DRE_NOERROR;
  close (fd);
  return 1;
}

int
database_backup (struct database *wdb) {
  // FIXME: to be written !!
  return 1;
}

int
database_save (struct database *wdb) {
  /* This function returns 1 on success and 0 on failure */
  /* It logs failure and maybe it should leave that task to the calling function */
  /* README : this function reads from the database memory without locking */
  struct database_hdr hdr;
  char *basedir;
  char dir[BUFFERLEN];
  char filename[BUFFERLEN];
  int fd;
  int c;

  // TODO: this all filename guessing should be inside a function
  if ((basedir = getenv ("DRQUEUE_DB")) == NULL) {
    /* This should never happen because we check it at the beginning of the program */
    log_auto (L_ERROR,"database_save() : DRQUEUE_DB environment variable could not be found. Master db cannot be saved.");
    drerrno = DRE_NOENVROOT;
    return 0;
  }

  snprintf (dir, BUFFERLEN - 1, "%s", basedir);
#ifndef __CYGWIN

  snprintf (filename, BUFFERLEN - 1, "%s/drqueue.db", dir);
#else

  snprintf (filename, BUFFERLEN - 1, "%s\\drqueue.db", dir);
#endif
  
  if (database_backup(wdb) == 0) {
    // TODO: filename should be a value returned by a function
    log_auto (L_ERROR,"database_save() : there was an error while backing up old database. NOT SAVING current one. (file: %s)",
	      filename);
  }

  log_auto (L_INFO,"Storing DB into: '%s'",filename);

  if ((fd = open (filename, O_CREAT | O_TRUNC | O_RDWR, 0664)) == -1) {
    if (errno == ENOENT) {
      /* If its because the directory does not exist we try creating it first */
      if (mkdir (dir, 0775) == -1) {
	drerrno_system = errno;
        log_auto (L_WARNING,"Could not create database directory. Check permissions: %s. (%s)",
		  dir,strerror(drerrno_system));
        drerrno = DRE_COULDNOTCREATE;
        return 0;
      }
      if ((fd = open (filename, O_CREAT | O_TRUNC | O_RDWR, 0664)) == -1) {
        log_auto (L_WARNING,"Could not open database file for writing. Check permissions: %s. (%s)",
                    filename,strerror(drerrno_system));
        drerrno = DRE_COULDNOTCREATE;
        return 0;
      }
    } else {
      /* could not open the file for other reasons */
      log_auto (L_WARNING,"Could not open database file for writing. Check permissions: %s",
                  filename);
      drerrno = DRE_COULDNOTCREATE;
      return 0;
    }
  }

  hdr.magic = DB_MAGIC;
  hdr.version = database_version_id();
  hdr.job_size = MAXJOBS;

  write_32b (fd, &hdr.magic);
  write_32b (fd, &hdr.version);
  write_16b (fd, &hdr.job_size);

  for (c = 0; c < MAXJOBS; c++) {
    logger_job = &wdb->job[c];
    if (!send_job (fd, &wdb->job[c])) {
      // TODO: report
      log_auto (L_ERROR,"could not save job. Phase 1. (%s)",strerror(drerrno_system));
      return 0;
    }
    if (wdb->job[c].used) {
      if (!database_job_save_frames(fd,&wdb->job[c])) {
	log_auto (L_ERROR,"could not save job frames. Phase 2. (%s)",strerror(drerrno_system));
	return 0;
      }
      if (!database_job_save_blocked_hosts(fd,&wdb->job[c])) {
	log_auto (L_ERROR,"could not save job blocked hosts. Phase 3. (%s)",strerror(drerrno_system));
	return 0;
      }
    }
  }
  
  log_auto (L_INFO,"Database saved successfully.");

  return 1;
}

int
database_job_save_frames (int sfd,struct job *job) {
  int nframes = job_nframes (job);
  struct frame_info *fi;
  int i;

  if ((fi = attach_frame_shared_memory (job->fishmid)) == (void *) -1) {
    // Store empty frames in an attemp to save other jobs
    // TODO: Warning CORRUPT
    struct frame_info fi2;
    job_frame_info_init (&fi2);
    for (i = 0; i < nframes; i++) {
      if (!send_frame_info (sfd, &fi2)) {
	return 0;
      }
    }
  } else {
    for (i = 0; i < nframes; i++) {
      if (!send_frame_info (sfd, &fi[i])) {
	detach_frame_shared_memory (fi);
	return 0;
      }
    }
    detach_frame_shared_memory (fi);
  }
  return 1;
}

int
database_job_load_frames (int sfd,struct job *job) {
  uint32_t nframes = job_nframes (job);
  struct frame_info *fi;
  int d;

  if (nframes) {
    if ((job->fishmid = get_frame_shared_memory (nframes)) == (int64_t)-1) {
      drerrno = DRE_GETSHMEM;
      return 0;
    }
    if ((fi = attach_frame_shared_memory (job->fishmid)) == (void *) -1) {
      drerrno = DRE_ATTACHSHMEM;
      return 0;
    }
    for (d = 0; d < nframes; d++) {
      if (!recv_frame_info (sfd, &fi[d])) {
	drerrno = DRE_ERRORREADING;
	close (sfd);
	detach_frame_shared_memory(fi);
	job_delete(job);
	return 0;
      }
    }
    detach_frame_shared_memory (fi);
  } else {
    job->fishmid = -1;
    job->frame_info = NULL;
  }
  

  return 1;
}

int
database_job_save_blocked_hosts (int sfd, struct job *job) {
  struct blocked_host *obh;

  if (!job) {
    return 0;
  }
  
  if (job->nblocked) {
    if ((obh = (struct blocked_host *)attach_blocked_host_shared_memory(job->bhshmid)) == (void*)-1) {
      return 0;
    }
  } else {
    obh = NULL;
  }

  if (!send_blocked_host_list(sfd,obh,job->nblocked,0)) {
    detach_blocked_host_shared_memory(obh);
    return 0;
  }

  if (obh) {
    detach_blocked_host_shared_memory(obh);
  }

  return 1;
}
 
int
database_job_load_blocked_hosts (int sfd, struct job *job) {
  struct blocked_host *obh,*tbh;
  int64_t bhshmid = -1;
  uint32_t nblocked = 0;
  
  if (!job) {
    return 0;
  }
    
  if (!recv_blocked_host_list(sfd,&obh,&nblocked,0)) {
    return 0;
  }

  if (nblocked) {
    if ((bhshmid = get_blocked_host_shared_memory(nblocked)) == (int64_t)-1) {
      return 0;
    }

    if ((tbh = attach_blocked_host_shared_memory(bhshmid)) == (void*)-1) {
      return 0;
    }

    memcpy (tbh,obh,sizeof(*obh)*nblocked);
    free (obh);
    detach_blocked_host_shared_memory(tbh);
  }

  job->bhshmid = bhshmid;
  job->nblocked = nblocked;
  job->blocked_host = NULL;

  return 1;
}
