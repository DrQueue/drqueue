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

#ifndef _JOB_H_
#define _JOB_H_

#if defined (__LINUX)
#include <stdint.h>
#elif defined (__IRIX)
#include <sys/types.h>
#elif defined (__OSX)
#include <stdint.h>
#elif defined (__FREEBSD)
#include <stdint.h>
#else
#error You need to define the OS, or OS defined not supported
#endif

#include <time.h>

#include "constants.h"



/* FRAME SECTION */
typedef enum {
  FS_WAITING,			/* Waiting to be assigned */
  FS_ASSIGNED,			/* Currently assigned but not finished (so RUNNING) */
  FS_ERROR,			/* Finished with error */
  FS_FINISHED			/* Finished with success */
} t_framestatus;

/* Struct that holds information about a single frame */
struct frame_info {
  char status;			/* Status */
  time_t start_time,end_time;	/* Time of start and ending of the frame (32 bit integer) */
  char exitcode;		/* Exit code of the process */
  uint32_t icomp;		/* Index to computer */
  uint16_t itask;		/* Index to task on computer */
	uint16_t requeued; // Number of times that this frame has been requeued, starts at 0
	uint16_t flags;    // Frame flags
};

// Frame Flags
#define FF_REQUEUE	(1<<0)	// Frame has to be requeued when finished.

// BLOCKED HOSTS SECTION
struct blocked_host {
	char name[MAXNAMELEN];
};

/* LIMITS SECTION */
struct job_limits {
  uint16_t nmaxcpus;		/* Maximum number of cpus running the job */
  uint16_t nmaxcpuscomputer;	/* Maximum number of cpus running the job on one single computer */
  uint16_t os_flags;		/* In which OS will it run */
	uint32_t memory;
	char pool[MAXNAMELEN];
};

/* KOJ SECTION */
/* this union must have the appropiate information for every kind of job */
union koj_info {		/* Kind of job information */
  struct koji_maya {
    char scene[BUFFERLEN];
    char renderdir[BUFFERLEN];	/* Output directory for the images */
    char image[BUFFERLEN];
    char viewcmd[BUFFERLEN];	/* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  } maya;
  struct koji_mentalray {
    char scene[BUFFERLEN];
    char renderdir[BUFFERLEN];	/* Output directory for the images */
    char image[BUFFERLEN];
    char viewcmd[BUFFERLEN];	/* something like "imf_disp $DRQUEUE_RD/$DRQUEUE_IMAGE.$DRQUEUE_FRAME.iff" */
  } mentalray;
  struct koji_blender {
    char scene[BUFFERLEN];
    char viewcmd[BUFFERLEN];	/* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  } blender;
	struct koji_bmrt {
		char scene[BUFFERLEN];
		char viewcmd[BUFFERLEN];
		char custom_crop;
		uint32_t xmin,xmax,ymin,ymax;
		char custom_samples;
		uint32_t xsamples,ysamples;
		char disp_stats;
		char verbose;
		char custom_beep;
		char custom_radiosity;
		uint32_t radiosity_samples;
		char custom_raysamples;
		uint32_t raysamples;
	} bmrt;
  struct koji_pixie {
    char scene[BUFFERLEN];
    char viewcmd[BUFFERLEN];	/* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  } pixie;
	struct kojo_3delight {
    char scene[BUFFERLEN];
    char viewcmd[BUFFERLEN];	/* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
	} threedelight;
};

/* Koj types */
#define KOJ_GENERAL     0	/* Not specific koj */
#define KOJ_MAYA        1	/* Maya koj */
#define KOJ_BLENDER     2	/* Blender koj */
#define KOJ_BMRT        3	/* BMRT koj */
#define KOJ_3DELIGHT    4	/* 3delight koj */
#define KOJ_PIXIE       5	/* Pixie koj */
#define KOJ_MENTALRAY   6	/* Mental Ray koj */

/* JOB SECTION */
typedef enum {
  JOBSTATUS_WAITING,						/* Waiting to be dispatched */
  JOBSTATUS_ACTIVE,							/* Already dispatched */
  JOBSTATUS_STOPPED, /* Stopped, waiting for current frames to finish */
  JOBSTATUS_FINISHED
} t_jobstatus;

/* JOB FLAGS */
#define JF_MAILNOTIFY     (1<<0) /* Mail notifications on events */
#define JF_MNDIFEMAIL	    (1<<1) /* Email address for notifications specified on "email" field */
#define JF_JOBDEPEND		 	(1<<2) // This job depends on another to start

/* OS FLAGS */
#define OSF_IRIX          (1<<0) /* If set will run on Irix */
#define OSF_LINUX         (1<<1) /* If set will run on Linux */
#define OSF_OSX						(1<<2) /* If set will run on OSX */
#define OSF_FREEBSD				(1<<3) /* If set will run on FreeBSD */

/* THE JOB ITSELF */
struct job {
  char used;
  uint32_t id;			/* Id number for the job */
  uint16_t nprocs;		/* Number of procs currently assigned */
  uint16_t status;		/* Status of the job */
  uint32_t priority;		/* Priority */

  char name[MAXNAMELEN];
  char cmd[MAXCMDLEN];
  char owner[MAXNAMELEN];
  char email[MAXNAMELEN];	/* Specific email address for email notifications */

  uint16_t koj;			/* Kind of job */
  union koj_info koji;		/* koj info */
  
	// FIXME: frames are uint32_t but the function that returns frames available is int 
  uint32_t frame_start,frame_end;
  uint32_t frame_step;
  uint32_t fleft,fdone,ffailed;	/* Frames left,done and failed */
	uint32_t block_size;
  time_t avg_frame_time;	/* Average frame time */
  time_t est_finish_time;	/* Estimated finish time */
  struct frame_info *frame_info; /* Status of every frame */
  int fishmid;		 /* Shared memory id for the frame_info structure */

	// Blocked hosts
	struct blocked_host *blocked_host;
	int	bhshmid;			// Shared memory id for the blocked_host structure
	uint16_t nblocked;						// Number of blocked hosts

  uint32_t flags;								/* Job flags */

	uint32_t dependid;						/* Jobid on which this one depends */

  struct job_limits limits;
};

struct database;

struct tpol {			/* Priority ordered list of jobs */
  uint32_t index;		/* index to unordered list */
  uint32_t pri;			/* priority of that job */
};

int job_index_free (void *pwdb);
void job_report (struct job *job);
char *job_status_string (char status);
char *job_frame_status_string (char status);
uint32_t job_frame_index_to_number (struct job *job,uint32_t index);
uint32_t job_frame_number_to_index (struct job *job,uint32_t number);
int job_frame_number_correct (struct job *job,uint32_t number);
uint32_t job_nframes (struct job *job);
void job_copy (struct job *src, struct job *dst);

int job_available (struct database *wdb,uint32_t ijob, int *iframe, uint32_t icomp);
int job_available_no_icomp (struct database *wdb,uint32_t ijob, int *iframe);
int job_first_frame_available (struct database *wdb,uint32_t ijob,uint32_t icomp);
int job_first_frame_available_no_icomp (struct database *wdb,uint32_t ijob);

void job_frame_waiting (struct database *wdb,uint32_t ijob, int iframe);
void job_update_assigned (struct database *wdb,uint32_t ijob, int iframe, int icomp, int itask);
void job_init_registered (struct database *wdb,uint32_t ijob,struct job *job);
void job_init (struct job *job);
void job_frame_info_init (struct frame_info *fi);
void job_delete (struct job *job);
int job_njobs_masterdb (struct database *wdb);
void job_update_info (struct database *wdb,uint32_t ijob);
void job_check_frame_status (struct database *wdb,uint32_t ijob, uint32_t iframe);
void job_stop (struct job *job);
void job_continue (struct job *job);
int job_index_correct_master (struct database *wdb,uint32_t ijob);

void job_init_limits (struct job *job);
int job_limits_passed (struct database *wdb, uint32_t ijob, uint32_t icomp);

void job_environment_set (struct job *job, uint32_t iframe);
void job_logs_remove (struct job *job);

int get_frame_shared_memory (int nframes); /* ipc shared memory */
void *attach_frame_shared_memory (int shmid);
void detach_frame_shared_memory (struct frame_info *fishp);

// Blocked hosts
int get_blocked_host_shared_memory (int nframes); /* ipc shared memory */
void *attach_blocked_host_shared_memory (int shmid);
void detach_blocked_host_shared_memory (struct blocked_host *bhshp);

int priority_job_compare (const void *a,const void *b);

char *job_koj_string (struct job *job);

#endif /* _JOB_H_ */



