/* $Id: job.h,v 1.26 2001/11/09 15:01:05 jorge Exp $ */

#ifndef _JOB_H_
#define _JOB_H_

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
# else
#  error You need to define the OS, or OS defined not supported
# endif
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
};

/* LIMITS SECTION */
struct job_limits {
  uint16_t nmaxcpus;		/* Maximum number of cpus running the job */
  uint16_t nmaxcpuscomputer;	/* Maximum number of cpus running the job on one single computer */
};

/* KOJ SECTION */
/* this union must have the appropiate information for every kind of job */
union koj_info {		/* Kind of job information */
  struct koji_maya {
    char scene[BUFFERLEN];
    char project[BUFFERLEN];
    char image[BUFFERLEN];
    char viewcmd[BUFFERLEN];	/* something like "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi" */
  } maya;
};

/* Koj types */
#define KOJ_GENERAL 0		/* Not specific koj */
#define KOJ_MAYA    1		/* Maya koj */

/* koj constants */
#define KOJ_MAYA_DFLT_VIEWCMD "fcheck $PROJECT/images/$IMAGE.$PADFRAME.sgi"

/* JOB SECTION */
typedef enum {
  JOBSTATUS_WAITING,		/* Waiting to be dispatched */
  JOBSTATUS_ACTIVE,		/* Already dispatched */
  JOBSTATUS_STOPPED,		/* Stopped, waiting for current frames to finish */
  JOBSTATUS_FINISHED
} t_jobstatus;

struct job {
  char used;
  uint32_t id;			/* Id number for the job */
  uint16_t nprocs;		/* Number of procs currently assigned */
  uint16_t status;		/* Status of the job */
  uint32_t priority;		/* Priority */

  char name[MAXNAMELEN];
  char cmd[MAXCMDLEN];
  char owner[MAXNAMELEN];

  uint16_t koj;			/* Kind of job */
  union koj_info koji;		/* koj info */
  
  uint32_t frame_start,frame_end; /* first and last frames */
  uint32_t frame_step;		/* step */
  uint32_t fleft,fdone,ffailed;	/* Frames left,done and failed */
  time_t avg_frame_time;	/* Average frame time */
  time_t est_finish_time;	/* Estimated finish time */
  struct frame_info *frame_info; /* Status of every frame */
  int fishmid;			/* Shared memory id for the frame_info structure */

  uint32_t flags;		/* Job flags */

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
int job_first_frame_available (struct database *wdb,uint32_t ijob);
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

int priority_job_compare (const void *a,const void *b);

#endif /* _JOB_H_ */



