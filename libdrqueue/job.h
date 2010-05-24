//
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
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

#ifndef _JOB_H_
#define _JOB_H_

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include <sys/types.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>

#include "constants.h"
#include "envvars.h"

#pragma pack(push,1)

/* FRAME SECTION */
typedef enum {
  FS_WAITING,    /* Waiting to be assigned */
  FS_ASSIGNED,   /* Currently assigned but not finished (so RUNNING) */
  FS_ERROR,      /* Finished with error */
  FS_FINISHED    /* Finished with success */
} t_framestatus;

/* Struct that holds information about a single frame */
struct frame_info {
  uint8_t  status;              // Status
  uint32_t start_time,end_time; // (time_t) Time of start and ending of the frame (32 bit integer)
  uint8_t  exitcode;            // Exit code of the process
  uint32_t icomp;               // Index to computer
  uint16_t itask;               // Index to task on computer
  uint16_t requeued;            // Number of times that this frame has been requeued, starts at 0
  uint16_t flags;               // Frame flags
};

// Frame Flags
#define FF_REQUEUE (1<<0) // Frame has to be requeued when finished.

// BLOCKED HOSTS SECTION
struct blocked_host {
  char name[MAXNAMELEN];
};

/* LIMITS SECTION */
struct job_limits {
  uint16_t nmaxcpus;  /* Maximum number of cpus running the job */
  uint16_t nmaxcpuscomputer; /* Maximum number of cpus running the job on one single computer */
  uint16_t os_flags;  /* In which OS will it run */
  uint32_t memory;
  char pool[MAXNAMELEN];
};

/* KOJ SECTION */
/* this union must have the appropiate information for every kind of job */
union koj_info {  /* Kind of job information */
  struct koji_general {
    char scriptdir[PATH_MAX];
  }
  general;
  struct koji_maya {
    char scene[BUFFERLEN];
    char renderdir[BUFFERLEN]; /* Output directory for the images */
    char projectdir[BUFFERLEN];
    char precommand[BUFFERLEN];
    char postcommand[BUFFERLEN];
    char image[BUFFERLEN];
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  maya;
  struct koji_mentalray {
    char scene[BUFFERLEN];
    char renderdir[BUFFERLEN]; /* Output directory for the images */
    char image[BUFFERLEN];
    char viewcmd[BUFFERLEN]; /* something like "imf_disp $DRQUEUE_RD/$DRQUEUE_IMAGE.$DRQUEUE_FRAME.iff" */
  }
  mentalray;
  struct koji_blender {
    char scene[BUFFERLEN];
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  blender;
  struct koji_pixie {
    char scene[BUFFERLEN];
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  pixie;
  struct koji_3delight {
    char scene[BUFFERLEN];
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  threedelight;
  struct koji_lightwave {
    char scene[BUFFERLEN];
    char projectdir[BUFFERLEN]; /* Project directory */
    char configdir[BUFFERLEN]; /* Config directory */
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  lightwave;
  struct koji_nuke {
    char scene[BUFFERLEN];
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  nuke;
  struct koji_terragen {
    char scriptfile[BUFFERLEN];
    char worldfile[BUFFERLEN]; /* World File */
    char terrainfile[BUFFERLEN]; /* Terrain File */
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  terragen;
  struct koji_aqsis {
    char scene[BUFFERLEN];
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  aqsis;
  struct koji_mantra {
    char scene[BUFFERLEN];
    char renderdir[BUFFERLEN];
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  mantra;
  struct koji_aftereffects {
    char project[BUFFERLEN];
    char comp[BUFFERLEN];
    char viewcmd[BUFFERLEN];
  }
  aftereffects;
  struct koji_shake {
    char script[BUFFERLEN];
    char viewcmd[BUFFERLEN];
  }
  shake;
  struct koji_turtle {
    char scene[BUFFERLEN]; // -geometry
    char renderdir[BUFFERLEN]; // -imageOutputPath
    char projectdir[BUFFERLEN]; // -projectPath
    char camera[BUFFERLEN]; // -camera
    uint32_t resx, resy; // -resolution x y
    char image[BUFFERLEN]; // -imageName
    char viewcmd[BUFFERLEN]; /* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  }
  turtle;
  struct koji_xsi {
    char scene[BUFFERLEN];
    char xsipass[BUFFERLEN];
    char renderdir[BUFFERLEN]; /* Output directory for the images */
    char image[BUFFERLEN];
    char imageExt[BUFFERLEN];
    char viewcmd[BUFFERLEN]; /* something like "$DRQUEUE_BIN/viewcmd/imf_batch" */
  }
  xsi;
  struct koji_luxrender {
    char scene[BUFFERLEN];
    char script[BUFFERLEN];
    char viewcmd[BUFFERLEN];
  }
  luxrender;
  struct koji_vray {
    char script[BUFFERLEN];
    char viewcmd[BUFFERLEN];
  }
  vray;
  struct koji_threedsmax {
    char scene[BUFFERLEN];
    char script[BUFFERLEN];
    char image[BUFFERLEN];
    char viewcmd[BUFFERLEN];
  }
  threedsmax;

};

/* Koj types */
#define KOJ_GENERAL       0  // Not specific koj
#define KOJ_MAYA          1  // Maya koj
#define KOJ_BLENDER       2  // Blender koj
#define KOJ_3DELIGHT      4  // 3delight koj
#define KOJ_PIXIE         5  // Pixie koj
#define KOJ_MENTALRAY     6  // Mental Ray koj
#define KOJ_LIGHTWAVE     7  // Lightwave koj
#define KOJ_AFTEREFFECTS  8  // After Effects koj
#define KOJ_SHAKE         9  // Shake koj
#define KOJ_AQSIS         10 // Aqsis koj
#define KOJ_TERRAGEN      11 // Terragen koj
#define KOJ_NUKE          12 // Nuke koj
#define KOJ_TURTLE        13 // Turtle koj
#define KOJ_MANTRA        14 // Mantra koj
#define KOJ_XSI           15 // XSI koj
#define KOJ_LUXRENDER     16 // LUXRENDER koj
#define KOJ_VRAY		  17 // VRAY koj
#define KOJ_3DSMAX        18 // 3DSMAX koj

/* JOB SECTION */
typedef enum {
  JOBSTATUS_WAITING,            // Waiting to be dispatched. Wants to
                                // start rendering
  JOBSTATUS_ACTIVE,             // Already dispatched and supposedly
                                // running
  JOBSTATUS_STOPPED,            // Stopped. No waiting frames will get assigned
                                // to jobs in this situation (CHECK:
                                // what happens on requeue)
  JOBSTATUS_FINISHED            // After completion. Job has finished
                                // processing all it frames/tasks
} t_jobstatus;

/* JOB FLAGS */
#define JF_MAILNOTIFY   (1<<0) // Mail notifications on events
#define JF_MNDIFEMAIL   (1<<1) // Email address for notifications specified on "email" field
#define JF_JOBDEPEND    (1<<2) // This job depends on another to start
#define JF_JOBDELETE    (1<<3) // Delete job when finished

/* OS FLAGS */
#define OSF_ALL       0xffff  // All OS flags set
#define OSF_IRIX      (1<<0)  // If set will run on Irix
#define OSF_LINUX     (1<<1)  // If set will run on Linux
#define OSF_OSX       (1<<2)  // If set will run on OSX
#define OSF_FREEBSD   (1<<3)  // If set will run on FreeBSD
#define OSF_CYGWIN    (1<<4)  // If set will run on Windows

/* THE JOB ITSELF */
struct job {
  uint8_t  used;
  uint32_t id;        // Id number for the job
  uint16_t nprocs;    // Number of procs currently assigned
  uint16_t status;    // Status of the job
  uint32_t priority;  // Priority

  char name[MAXNAMELEN];
  char cmd[MAXCMDLEN];
  char owner[MAXNAMELEN];
  char email[MAXNAMELEN]; // Specific email address for email notifications
  uint8_t autoRequeue;


  uint16_t koj;         // Kind of job
  union koj_info koji;  // koj info

  // FIXME: frames are uint32_t but the function that returns frames available is int
  uint32_t frame_start,frame_end;
  uint8_t  frame_pad;
  uint32_t frame_step;
  uint32_t fleft,fdone,ffailed; // Frames left,done and failed

  uint32_t old_fdone;           // Number of frames finished last time
                                // we checked.
  uint16_t old_nprocs;          // Number of procs running last time
                                // we checked.
  uint32_t block_size;          // Number of tasks that should be
                                // executed by slave before
                                // requesting a new task
  uint32_t submit_time;         // Time when the job was submitted
  uint32_t avg_frame_time;      // Average frame time
  uint32_t est_finish_time;     // Estimated finish time
                                // we checked.

  fptr_type (struct frame_info,frame_info);      // Status of every frame (1 pointer)
  int64_t fishmid;               // Frame info shared memory id

  fptr_type (struct blocked_host,blocked_host);  // Blocked hosts    (1 pointer)
  int64_t bhshmid;               // Shared memory id for the blocked_host structure
  uint16_t nblocked;             // Number of blocked hosts

  uint32_t flags;                // Job flags

  uint32_t dependid;             // Jobid on which this one depends

  struct job_limits limits;           // Job limits
  struct envvars envvars;             // Environment variables (1 pointer)
};

struct database;

struct tpol {           // Priority ordered list of jobs
  uint32_t index;       // index to unordered list
  uint32_t pri;         // priority of that job
  uint64_t submit_time; // submission time of the job
};

#pragma pack(pop)

uint32_t job_index_free (void *pwdb);
void job_report (struct job *job);
char *job_status_string (uint16_t status);
char *job_frame_status_string (uint8_t status);
uint32_t job_frame_index_to_number (struct job *job,uint32_t index);
uint32_t job_frame_number_to_index (struct job *job,uint32_t number);
int job_frame_number_correct (struct job *job,uint32_t number);
uint32_t job_nframes (struct job *job);
void job_copy (struct job *src, struct job *dst);
void job_fix_received_invalid (struct job *job);
void job_bswap_from_network (struct job *orig, struct job *dest);
void job_bswap_to_network (struct job *orig, struct job *dest);

int job_available (struct database *wdb,uint32_t ijob, uint32_t *iframe, uint32_t icomp);
int job_available_no_icomp (struct database *wdb,uint32_t ijob, uint32_t *iframe);
uint32_t job_first_frame_available (struct database *wdb,uint32_t ijob,uint32_t icomp);
uint32_t job_first_frame_available_no_icomp (struct database *wdb,uint32_t ijob);

void job_frame_waiting (struct database *wdb,uint32_t ijob, uint32_t iframe);
void job_update_assigned (struct database *wdb,uint32_t ijob, uint32_t iframe, uint32_t icomp, uint16_t itask);
void job_init_registered (struct database *wdb,uint32_t ijob,struct job *job);
void job_init (struct job *job);
void job_frame_info_init (struct frame_info *fi);
void job_delete (struct job *job);
uint32_t job_njobs_masterdb (struct database *wdb);
void job_update_info (struct database *wdb,uint32_t ijob);
int job_check_frame_status (struct database *wdb,uint32_t ijob, uint32_t iframe, struct frame_info *fi); // bool
void job_stop (struct job *job);
void job_continue (struct job *job);
int job_index_correct_master (struct database *wdb,uint32_t ijob); // bool

void job_limits_init (struct job_limits *limits);
int job_limits_passed (struct database *wdb, uint32_t ijob, uint32_t icomp); // bool
void job_limits_bswap_from_network (struct job_limits *orig, struct job_limits *dest);
void job_limits_bswap_to_network (struct job_limits *orig, struct job_limits *dest);

void job_environment_set (struct job *job, uint32_t iframe);
void job_logs_remove (struct job *job);   // WARN: this function
                                          // removes the logs
                                          // directory for a job

int64_t get_frame_shared_memory (uint32_t nframes); /* ipc shared memory */
struct frame_info *attach_frame_shared_memory (int64_t shmid);
void detach_frame_shared_memory (struct frame_info *fishp);

// Blocked hosts
int64_t get_blocked_host_shared_memory (uint32_t nhosts); /* ipc shared memory */
struct blocked_host *attach_blocked_host_shared_memory (int64_t shmid);
void detach_blocked_host_shared_memory (struct blocked_host *bhshp);

int priority_job_compare (const void *a,const void *b);

char *job_koj_string (struct job *job);


int job_block_host_add_by_name (struct job *job, char *name);
int job_block_host_remove_by_name (struct job *job, char *name);
int job_block_host_exists_by_name (struct job *job, char *name);

#ifdef __CPLUSPLUS
}
#endif

#endif /* _JOB_H_ */
