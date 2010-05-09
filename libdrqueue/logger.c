//
// (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>

#include "logger.h"
#include "task.h"
#include "job.h"
#include "computer.h"
#include "libdrqueue.h"

int loglevel = L_INFO;
logtooltype logtool = DRQ_LOG_TOOL_UNKNOWN;
struct job *logger_job = NULL;
struct task *logger_task = NULL;
struct computer *logger_computer = NULL;

// PENDING:
// * log file locking
//   - log file struct (related to the job python wrapper)
// * log subsystems (computer,job...)
// * all ERROR messages should go to stderr too.
//

/* One important detail about the logger functions is that all of them */
/* add the trailing newline (\n). So the message shouldn't have it. */

FILE *log_slave_open_task (int level, struct task *task);
FILE *log_slave_open_computer (int level, char *name);
FILE *log_master_open (int level);

int log_level_dest (int level);
int log_on_screen (void);
int log_job_path_get (uint32_t jobid,char *path,int pathlen);
int log_task_filename_get (struct task *task, char *path, int pathlen);

void
log_slave_task (struct task *task,int level,char *fmt,...) {
  va_list ap;

  if (!log_level_dest(level))
    return;
  
  logger_task = task;
  va_start (ap,fmt);
  log_auto (level,fmt,ap);
  va_end (ap);
}

int
log_on_screen (void) {
  if ((L_OUTMASK & loglevel) & L_ONSCREEN) {
    return 1;
  }
  return 0;
}

FILE *log_slave_open_task (int level, struct task *task) {
  FILE *f;
  char filename[PATH_MAX];

  if (log_on_screen()) {
    return stderr;
  } 

  if (!log_task_filename_get(task,filename,PATH_MAX)) {
    // FIXME: missing warning
    return stderr;
  }

  if ((f = fopen (filename, "a")) == NULL) {
    drerrno_system = errno;
    return stderr;
  }

  return f;
}

void log_slave_computer (int level, char *fmt, ...) {
  char name2[MAXNAMELEN];
  char *name = NULL;  /* To only make a call to gethostname */
  va_list ap;

  if (!log_level_dest(level)) {
    return;
  }

  if (name == NULL) {
    if (gethostname(name2,MAXNAMELEN-1) == -1) {
      strcpy (name2,"UNKNOWN");
    }
    name = name2;
  }

  va_start (ap,fmt);
  log_auto(level,fmt,ap);
  va_end (ap);
}

FILE *log_slave_open_computer (int level, char *name) {
  FILE *f;
  char filename[BUFFERLEN];
  char *basedir;

  if (log_on_screen()) {
    return stderr;
  } 

  if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_LOGS not set. Logging on screen...\n");
    log_level_out_set (L_ONSCREEN);
    return stderr;
  }

  snprintf(filename,BUFFERLEN-1,"%s/%s.log",basedir,name);

  if ((f = fopen (filename,"a")) == NULL) {
    perror ("log_slave_open_computer: Couldn't open file for writing");
    fprintf (stderr,"So... logging on screen.\n");
    log_level_out_set (L_ONSCREEN);
    return stderr;
  }

  return f;
}

void log_master_job (struct job *job, int level, char *fmt, ...) {
  va_list ap;

  if (!log_level_dest (level))
    return;

  logger_job = job;
  va_start (ap,fmt);
  log_auto(level,fmt,ap);
  va_end (ap);
  logger_job = NULL;

  return;
}

void
log_master_computer (struct computer *computer, int level, char *fmt, ...) {
  va_list ap;

  if (!log_level_dest(level))
    return;

  logger_computer = computer;
  va_start (ap,fmt);
  log_auto (level,fmt,ap);
  va_end (ap);
  logger_computer = NULL;
  
  return;  
}

int
log_level_dest (int level) {
  if ((level & L_LEVELMASK) > (loglevel & L_LEVELMASK))
    return 0;
  return 1;
}

FILE *log_master_open (int level) {
  FILE *f;
  char filename[BUFFERLEN];
  char *basedir;

  if (log_on_screen()) {
    return stderr;
  }

  if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_LOGS not set. Aborting...\n");
    log_level_out_set(L_ONSCREEN);
    return stderr;
  }

  snprintf(filename,BUFFERLEN-1,"%s/master.log",basedir);
#ifdef __LINUX

  if ((f = fopen (filename,"a")) == NULL) {
    perror ("log_master_open: Couldn't open file for writing");
    fprintf (stderr,"So... logging on screen.\n");
    log_level_out_set(L_ONSCREEN);
    return stderr;
  }
#else
  if ((f = fopen (filename,"ab")) == NULL) {
    perror ("log_master_open: Couldn't open file for writing");
    fprintf (stderr,"So... logging on screen.\n");
    log_level_out_set(L_ONSCREEN);
    return stderr;
  }
#endif

  return f;
}

void
log_level_severity_set (int severity) {
  loglevel &= L_OUTMASK;
  loglevel |= severity & L_LEVELMASK;
}

void
log_level_out_set (int outlevel) {
  loglevel &= L_LEVELMASK;
  loglevel |= outlevel & L_OUTMASK;
}

char *log_level_str (int level) {
  char *msg;

  switch (level & L_LEVELMASK) {
  case L_ERROR:
    msg = "ERROR";
    break;
  case L_WARNING:
    msg = "Warning";
    break;
  case L_INFO:
    msg = "Info";
    break;
  case L_DEBUG:
    msg = "Debug";
    break;
  case L_DEBUG2:
    msg = "Debug2";
    break;
  case L_DEBUG3:
    msg = "Debug3";
    break;
  default:
    msg = "UNKNOWN";
  }

  return msg;
}

int log_job_path_get (uint32_t jobid, char *path, int pathlen) {
  char *log_basedir;
  char *jobname;
  int nwritten; // number of bytes written

  if ((log_basedir = getenv("DRQUEUE_LOGS")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_LOGS not set.\n");
    return -1;
  }

  if (!path) {
    // TODO: Show error
    //fprintf (stderr,"Non valid values for path or pathlen.\n");
    return -1;
  }

  if (!request_job_name (jobid,&jobname,CLIENT)) {
    return -1;
  }

  nwritten = snprintf (path,pathlen,"%s/%03u.%s",log_basedir,jobid,jobname);
  free (jobname);

  // we do not need the job anymore

  if (nwritten >= pathlen) {
    // TODO: Show error
    return -1;
  }
  
  return nwritten;
}

int log_task_filename_get (struct task *task, char *path, int pathlen) {
  // Returns len of the written string or -1 on failure
  char job_path[PATH_MAX];
  int nwritten;
  struct task *ttask = NULL;

  if (!path) {
    // TODO: Show error
    //fprintf (stderr,"Non valid values for path or pathlen.\n");
    return -1;
  }

  if ((nwritten = log_job_path_get(task->ijob,job_path,PATH_MAX)) == -1) {
    // TODO: Show error
    fprintf (stderr,"log_task_filename_get(): could not obtain job logs path. (%s)",strerror(drerrno_system));
    logger_task = ttask;
    return -1;
  }
  
  nwritten = snprintf(path,pathlen,"%s/%s.%04u",job_path,task->jobname,task->frame);
  if (nwritten >= pathlen) {
    // TODO: Show error
    return -1;
  }

  drerrno = DRE_NOERROR;
  return nwritten;
}

int
log_path_exists (char *path) {
  struct stat info;

  if (stat(path,&info) == -1) {
    drerrno_system = errno;
    return 0;
  }

  return 1;
}

int
log_path_create (char *path) {
  int rv;

  if ((rv = mkdir (path,0777)) == -1) {
    drerrno_system = errno;
    return 0;
  }

  return 1;
}

int log_dumptask_open (struct task *t) {
  int lfd;
  char task_filename[PATH_MAX];
  char job_path[PATH_MAX];
  char hostname[MAXNAMELEN];
  time_t tm;
  char buf[BUFFERLEN];


  if (log_job_path_get(t->ijob,job_path,PATH_MAX) == -1) {
    // TODO: looking more like...
/*     char config_logs_path[PATH_MAX]; */
/*     if (!config_get_logs_path(config_logs_path,PATH_MAX)) { */
/*     } */
    // Backup code
    char *basedir;
    if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
      //log_auto (L_WARNING,"log_dumptask_open(): environment variable DRQUEUE_LOGS not set.");
      return -1;
    }
    // TODO: path
    snprintf(job_path,PATH_MAX,"%s/%03u.%s.DEFAULT",basedir,t->ijob,t->jobname);
  }

  //log_auto(L_DEBUG2,"log_dumptask_open(): logs for job go to directory on path '%s'",job_path);

  if (!log_path_exists(job_path)) {
    if (!log_path_create(job_path)) {
      //log_auto(L_ERROR,"log_dumptask_open(): could not create log path directory '%s'. (%s)",job_path,strerror(drerrno_system));
      return -1;
    }
  }

  if (log_task_filename_get(t,task_filename,PATH_MAX) == -1) {
    // Backup code
    // TODO: path
    snprintf(task_filename,PATH_MAX,"%s/%s.%04i.DEFAULT",job_path,t->jobname,t->frame);
  }

  //log_auto(L_DEBUG,"log_dumptask_open(): logs for this task go to path '%s'",task_filename);


  // TODO: Check for directory and creation on another function.
  if ((lfd = open (task_filename, O_CREAT|O_APPEND|O_RDWR, 0664)) == -1) {
    drerrno_system = errno;
    //log_auto (L_ERROR,"log_dumptask_open(): error on open. (%s)",strerror(drerrno_system));
    return -1;
  }

  time (&tm);
  gethostname (hostname,MAXNAMELEN-1);
  snprintf(buf,BUFFERLEN,"Log started at %sComputer: %s\nLog filename: %s\n\n",ctime(&tm),hostname,task_filename);
  write(lfd,buf,strlen(buf));
  return lfd;
}

int log_dumptask_open_ro (struct task *t) {
  /* Open in read only for clients */
  int lfd;
  char task_filename[PATH_MAX];

  if (log_task_filename_get(t,task_filename,PATH_MAX) == -1) {
    char job_path[PATH_MAX];
    // Backup code
    if (log_job_path_get(t->ijob,job_path,PATH_MAX) == -1) {
      // Backup code 
      char *basedir;
      if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
        //log_auto (L_ERROR,"log_dumptask_open_ro(): environment variable DRQUEUE_LOGS not set.");
        return -1;
      }
      // TODO: path
      snprintf(job_path,PATH_MAX,"%s/%03u.%s.DEFAULT",basedir,t->ijob,t->jobname);
    }
    // TODO: path
    snprintf(task_filename,PATH_MAX,"%s/%s.%04i.DEFAULT",job_path,t->jobname,t->frame);
  }
  
  //log_auto(L_DEBUG,"log_dumptask_open_ro(): trying to read task log from path '%s'",task_filename);

  if ((lfd = open (task_filename,O_RDONLY)) == -1) {
    //log_auto (L_ERROR,"log_dumptask_open_ro(): couldn't open log file for task on '%s'",task_filename);
    return -1;
  }

  return lfd;
}

void log_get_time_str (char *timebuf,int buflen) {
  time_t now;
  size_t len = 0;
  char tbuf[MAXLOGLINELEN];

  if ((timebuf == NULL) || (buflen <= 0))
    return;
  time (&now);
  snprintf (timebuf,buflen,"%s",ctime(&now));
  len = strlen(timebuf);
  timebuf[len-1] = '\0';
  snprintf (tbuf,buflen,"%s (PID:%i)",timebuf,getpid());
  strcpy(timebuf,tbuf);
}

void
log_get_job_str (char *buffer,int buflen) {
  if (!buffer) {
    return;
  }

  if (!logger_job || !logger_job->used) {
    return;
  }

  snprintf (buffer,buflen,"Job: name:'%s' owner:'%s' id:'%u'",logger_job->name,logger_job->owner,logger_job->id);
}

void
log_get_computer_str (char *buffer, int buflen) {
  if (!buffer) {
    return;
  }

  if (!logger_computer || !logger_computer->used) {
    return;
  }

  snprintf (buffer,buflen,"Computer: name:'%s' id:'%u'",logger_computer->hwinfo.name,logger_computer->hwinfo.id);
}
  
void
log_get_task_str (char *buffer, int buflen) {
  if (!buffer) {
    return;
  }

  if (!logger_task) {
    return;
  }

  snprintf (buffer,buflen,"Task: Jobid:'%i' Jobname:'%s' Frame:'%i' Taskid:'%i' Compid:'%i'",logger_task->ijob,
	    logger_task->jobname,logger_task->frame,logger_task->itask,logger_task->icomp);
}

void log_auto (int level, char *fmt, ...) {
  // this will be the way to send log messages when no one is known
  // for sure.
  FILE *f_log = stderr;
  char time_buf[BUFFERLEN];
  char job_buf[BUFFERLEN];
  char task_buf[BUFFERLEN];
  char computer_buf[BUFFERLEN];
  char msg[MAXLOGLINELEN];
  char bkpmsg[MAXLOGLINELEN];
  char origmsg[MAXLOGLINELEN];

  va_list ap;

  if (!log_level_dest(level))
    return;

  if (log_on_screen()) {
    f_log = stderr;
  } else {
    switch (logtool) {
    case DRQ_LOG_TOOL_MASTER:
      f_log = log_master_open(level);
      break;
    case DRQ_LOG_TOOL_SLAVE_TASK:
      if (logger_task) {
	f_log = log_slave_open_task(level,logger_task);
	break;
      }
    case DRQ_LOG_TOOL_SLAVE:
    default:
      if (gethostname(computer_buf,BUFFERLEN) != -1) {
	f_log = log_slave_open_computer(level,computer_buf);
      } else {
	log_level_out_set(L_ONSCREEN);
	f_log = stderr;
      }
    }
  }

  va_start (ap,fmt);
  vsnprintf (origmsg,MAXLOGLINELEN,fmt,ap);
  va_end (ap);

  log_get_time_str (time_buf,BUFFERLEN);
  snprintf (bkpmsg,MAXLOGLINELEN,"%s :",time_buf); // Time and pid
  strcpy(msg,bkpmsg);
  snprintf (bkpmsg,MAXLOGLINELEN,"%s | %6s |",msg,log_level_str(level)); // Log level
  strcpy(msg,bkpmsg);

  if (logger_job) {
    log_get_job_str (job_buf,BUFFERLEN);
    snprintf (bkpmsg,MAXLOGLINELEN,"%s %s :",msg,job_buf);
    strcpy(msg,bkpmsg);
  }						       

  if (logger_computer) {
    log_get_computer_str (computer_buf,BUFFERLEN);
    snprintf (bkpmsg,MAXLOGLINELEN,"%s %s :",msg,computer_buf);
    strcpy(msg,bkpmsg);
  }						       
  if (logger_task) {
    log_get_task_str (task_buf,BUFFERLEN);
    snprintf (bkpmsg,MAXLOGLINELEN,"%s %s :",msg,task_buf);
    strcpy(msg,bkpmsg);
  }

  fprintf (f_log,"%s -> MSG: %s\n",msg,origmsg);

  if (fileno(f_log) != fileno(stderr))
    fclose(f_log);

}
