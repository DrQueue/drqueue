//
// (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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
int logonscreen = 0;
logtooltype logtool = DRQ_LOG_TOOL_UNKNOWN;

// PENDING:
// * log file locking
//   - log file struct (related to the job python wrapper)
// * log subsystems (computer,job...)
// * all ERROR messages should go to stderr too.
//

/* One important detail about the logger functions is that all of them */
/* add the trailing newline (\n). So the message shouldn't have it. */

void log_slave_task (struct task *task,int level,char *fmt,...) {
  FILE *f_log;
  char name[MAXNAMELEN];
  char buf[BUFFERLEN];  /* Buffer used to store ctime */
  char msg[BUFFERLEN];
  char msg2[BUFFERLEN];
  time_t now;
  va_list ap;

  if (level > loglevel)
    return;

  va_start (ap,fmt);
  vsnprintf (msg,BUFFERLEN-1,fmt,ap);
  va_end (ap);

  if (gethostname (name,MAXNAMELEN-1) == -1) {
    strcpy (name,"UNKNOWN");
  }

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (!logonscreen) {
    f_log = log_slave_open_task (task);
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }

  if (loglevel < L_DEBUG) {
    snprintf (msg2,BUFFERLEN,"%8s -> Job: %8s || Owner: %8s || Frame: %4i || %s: %s\n",name,
              task->jobname,task->owner,task->frame,log_level_str(level),msg);
  } else {
    snprintf (msg2,BUFFERLEN,"%8s -> Job: %8s || Owner: %8s || Frame: %4i || (%i) %s: %s\n",name,
              task->jobname,task->owner,task->frame,(int)getpid(),log_level_str(level),msg);
  }
  
  fprintf(f_log,"%8s : %s",buf,msg2);

  if (!logonscreen) {
    fclose (f_log);
    log_slave_computer (level,"COPY from task log: %s",msg2);
  }
}

FILE *log_slave_open_task (struct task *task) {
  FILE *f;
  char filename[BUFFERLEN];
  char dir[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
    logonscreen = 1;
    log_auto (L_ERROR,"log_slave_open_task(): environment variable DRQUEUE_LOGS not set. Logging on screen.");
    return NULL;
  }

  // Backup method
  snprintf(dir,BUFFERLEN-1,"%s/%s",basedir,task->jobname);
  snprintf(filename,BUFFERLEN-1,"%s/%s.log",dir,task->jobname);
  if ((f = fopen (filename, "a")) == NULL) {
    if (errno == ENOENT) {
      /* If its because the directory does not exist we try creating it first */
      if (mkdir (dir,0775) == -1) {
        logonscreen = 1;
        log_auto (L_ERROR,"log_slave_open_task(): couldn't create directory for task logging '%s', logging on the screen. (%s)",dir,strerror(errno));
        return f;
      }
      log_auto (L_INFO,"log_slave_open_task(): directory for task logs has been created '%s'.",dir);
      if ((f = fopen (filename, "a")) == NULL) {
        logonscreen = 1;
        log_auto (L_ERROR,"log_slave_open_task(): Couldn't open file for writing '%s', logging on the screen. (%s)",filename,strerror(errno));
        return f;
      }
      log_auto (L_INFO,"log_slave_open_task(): log file for task '%s' could be created after creating it's parent directory.",filename);
    }
  }

  return f;
}

void log_slave_computer (int level, char *fmt, ...) {
  FILE *f_log;
  char name2[MAXNAMELEN];
  char *name = NULL;  /* To only make a call to gethostname */
  char buf[BUFFERLEN];  /* Buffer to hold the current time */
  char msg[BUFFERLEN];
  time_t now;
  va_list ap;

  if (level > loglevel) {
    return;
  }

  va_start (ap,fmt);
  vsnprintf (msg,BUFFERLEN-1,fmt,ap);
  va_end (ap);

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (name == NULL) {
    if (gethostname(name2,MAXNAMELEN-1) == -1) {
      strcpy (name2,"UNKNOWN");
    }
    name = name2;
  }

  if (!logonscreen) {
    f_log = log_slave_open_computer (name);
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }

  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : %s: %s\n",buf,log_level_str(level),msg);
  else
    fprintf (f_log,"%8s : (%i) %s: %s\n",buf,(int) getpid(), log_level_str(level),msg);

  if (!logonscreen)
    fclose (f_log);
}

FILE *log_slave_open_computer (char *name) {
  FILE *f;
  char filename[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_LOGS not set. Aborting...\n");
    kill(0,SIGINT);
  }

  snprintf(filename,BUFFERLEN-1,"%s/%s.log",basedir,name);

  if ((f = fopen (filename,"a")) == NULL) {
    perror ("log_slave_open_computer: Couldn't open file for writing");
    fprintf (stderr,"So... logging on screen.\n");
    logonscreen = 1;
  }

  return f;
}

void log_master_job (struct job *job, int level, char *fmt, ...) {
  FILE *f_log;
  char buf[BUFFERLEN];  /* Buffer to hold the time */
  char msg[BUFFERLEN];
  time_t now;
  va_list ap;

  if (level > loglevel)
    return;

  va_start (ap,fmt);
  vsnprintf (msg,BUFFERLEN-1,fmt,ap);
  va_end (ap);

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (!logonscreen) {
    f_log = log_master_open ();
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }

  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : Job: %8s || Owner: %8s || %s: %s\n",buf,job->name,job->owner,log_level_str(level),msg);
  else
    fprintf (f_log,"%8s : Job: %8s || Owner: %8s || (%i) %s: %s\n",buf,job->name,job->owner,(int)getpid(),
             log_level_str(level),msg);
  if (!logonscreen)
    fclose (f_log);
}

void log_master_computer (struct computer *computer, int level, char *fmt, ...) {
  FILE *f_log;
  char buf[BUFFERLEN];
  char msg[BUFFERLEN];
  time_t now;
  va_list ap;

  if (level > loglevel)
    return;

  va_start (ap,fmt);
  vsnprintf (msg,BUFFERLEN-1,fmt,ap);
  va_end (ap);

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (!logonscreen) {
    f_log = log_master_open ();
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }

  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : Computer: %8s || %s: %s\n",buf,computer->hwinfo.name,log_level_str(level),msg);
  else
    fprintf (f_log,"%8s : Computer: %8s || (%i) %s: %s\n",buf,computer->hwinfo.name,(int) getpid(),log_level_str(level),msg);

  if (!logonscreen)
    fclose (f_log);
}

void log_master (int level,char *fmt, ...) {
  FILE *f_log;
  char buf[BUFFERLEN];  /* Buffer to hold the time */
  char msg[BUFFERLEN];
  time_t now;
  va_list ap;

  if (level > loglevel)
    return;

  va_start (ap,fmt);
  vsnprintf (msg,BUFFERLEN-1,fmt,ap);
  va_end (ap);

  time (&now);
  strncpy (buf,ctime(&now),BUFFERLEN-1);
  buf[strlen(buf)-1] = '\0';

  if (!logonscreen) {
    f_log = log_master_open ();
    if (!f_log)
      f_log = stdout;
  } else {
    f_log = stdout;
  }

  if (loglevel < L_DEBUG) {
    fprintf (f_log,"%8s : %s: %s\n",buf,log_level_str(level),msg);
  } else {
    fprintf (f_log,"%8s : (%i) %s: %s\n",buf,(int)getpid(),log_level_str(level),msg);
  }

  if (!logonscreen)
    fclose (f_log);
}

FILE *log_master_open (void) {
  FILE *f;
  char filename[BUFFERLEN];
  char *basedir;

  if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_LOGS not set. Aborting...\n");
    kill(0,SIGINT);
  }

  snprintf(filename,BUFFERLEN-1,"%s/master.log",basedir);
#ifdef __LINUX

  if ((f = fopen (filename,"a")) == NULL) {
    perror ("log_master_open: Couldn't open file for writing");
    fprintf (stderr,"So... logging on screen.\n");
    logonscreen = 1;
  }
#else
  if ((f = fopen (filename,"ab")) == NULL) {
    perror ("log_master_open: Couldn't open file for writing");
    fprintf (stderr,"So... logging on screen.\n");
    logonscreen = 1;
  }
#endif

  return f;
}

char *log_level_str (int level) {
  char *msg;

  switch (level) {
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
  struct job job;
  char *log_basedir;
  int nwritten; // number of bytes written

  if ((log_basedir = getenv("DRQUEUE_LOGS")) == NULL) {
    fprintf (stderr,"Environment variable DRQUEUE_LOGS not set.\n");
    return -1;
  }

  if ((path ==NULL) || (pathlen <= 0)) {
    // TODO: Show error
    //fprintf (stderr,"Non valid values for path or pathlen.\n");
    return -1;
  }

  if (!request_job_xfer (jobid,&job,CLIENT)) {
    //log_message (L_WARNING,"log_get_job_path: Could not retrieve job information on jobid %u\n",jobid);
    return -1;
  }

  nwritten = snprintf (path,pathlen,"%s/%03u.%s",log_basedir,job.id,job.name);

  // we do not need the job anymore
  job_delete (&job);

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

  if ((nwritten = log_job_path_get(task->ijob,job_path,PATH_MAX)) == -1) {
    // TODO: Show error
    return -1;
  }
  
  if ((path ==NULL) || (pathlen <= 0)) {
    // TODO: Show error
    //fprintf (stderr,"Non valid values for path or pathlen.\n");
    return -1;
  }

  nwritten = snprintf(path,pathlen,"%s/%s.%04u",job_path,task->jobname,task->frame);
  if (nwritten >= pathlen) {
    // TODO: Show error
    return -1;
  }

  return nwritten;
}

int log_dumptask_open (struct task *t) {
  int lfd;
  char task_filename[PATH_MAX];
  char job_path[PATH_MAX];
  char hostname[MAXNAMELEN];
  time_t tm;
  char buf[BUFFERLEN];


  if (log_job_path_get(t->ijob,job_path,PATH_MAX) == -1) {
    // Backup code
    char *basedir;
    if ((basedir = getenv("DRQUEUE_LOGS")) == NULL) {
      fprintf (stderr,"WARNING: Environment variable DRQUEUE_LOGS not set.\n");
      return -1;
    }
    // TODO: path
    snprintf(job_path,PATH_MAX,"%s/%03u.%s.DEFAULT",basedir,t->ijob,t->jobname);
  }

  log_auto(L_DEBUG2,"log_dumptask_open(): logs for job go to directory on path '%s'",job_path);

  if (log_task_filename_get(t,task_filename,PATH_MAX) == -1) {
    // Backup code
    // TODO: path
    snprintf(task_filename,PATH_MAX,"%s/%s.%04i.DEFAULT",job_path,t->jobname,t->frame);
  }

  log_auto(L_DEBUG,"log_dumptask_open(): logs for this task go to path '%s'",task_filename);

  // TODO: Check for directory and creation on another function.
  if ((lfd = open (task_filename, O_CREAT|O_APPEND|O_RDWR, 0664)) == -1) {
    if (errno == ENOENT) {
      /* If its because the directory does not exist we try creating it first */
      if (mkdir (job_path,0775) == -1) {
        log_slave_task (t,L_ERROR,"Couldn't create directory for task logs on '%s'",job_path);
        return -1;
      }
      if ((lfd = open (task_filename, O_CREAT|O_APPEND|O_RDWR, 0664)) == -1) {
        log_slave_task (t,L_ERROR,"Couldn't open or create file for task log on '%s'",task_filename);
        return -1;
      }
    }
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
        log_auto (L_ERROR,"log_dumptask_open_ro(): environment variable DRQUEUE_LOGS not set.");
        return -1;
      }
      // TODO: path
      snprintf(job_path,PATH_MAX,"%s/%03u.%s.DEFAULT",basedir,t->ijob,t->jobname);
    }
    // TODO: path
    snprintf(task_filename,PATH_MAX,"%s/%s.%04i.DEFAULT",job_path,t->jobname,t->frame);
  }
  
  log_auto(L_DEBUG,"log_dumptask_open_ro(): trying to read task log from path '%s'",task_filename);

  if ((lfd = open (task_filename,O_RDONLY)) == -1) {
    log_auto (L_ERROR,"log_dumptask_open_ro(): couldn't open log file for task on '%s'",task_filename);
    return -1;
  }

  return lfd;
}

void log_get_time_str (char *timebuf,int buflen) { 
  time_t now;
  size_t len = 0;
  if ((timebuf == NULL) || (buflen <= 0))
    return;
  time (&now);
  snprintf (timebuf,buflen,"%s",ctime(&now));
  len = strlen(timebuf);
  timebuf[len-1] = '\0';
}

char *log_get_job_str(struct job *job) {
  static char jobmsg[BUFFERLEN];

  snprintf (jobmsg,BUFFERLEN,"Name: '%8s' || Owner: '%8s'",job->name,job->owner);

  return jobmsg;
}

void log_auto_job (struct job *job, int level, char *fmt, ...) {
  FILE *f_log;
  char time_buf[BUFFERLEN];
  char msg[BUFFERLEN];
  va_list ap;

  if (level > loglevel)
    return;

  va_start (ap,fmt);
  vsnprintf (msg,BUFFERLEN-1,fmt,ap);
  va_end (ap);

  log_get_time_str (time_buf,BUFFERLEN);

  f_log = stderr;

  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : (JOB: %s) %s: %s\n",time_buf,log_get_job_str(job),log_level_str(level),msg);
  else
    fprintf (f_log,"%8s : (JOB: %s) (%i) %s: %s\n",time_buf,log_get_job_str(job),(int) getpid(), log_level_str(level),msg);
}

void log_auto (int level, char *fmt, ...) {
  // this will be the way to send log messages when no one is known
  // for sure.
  FILE *f_log;
  char hostname_buf[MAXNAMELEN];
  char *hostname_ptr = NULL;
  char time_buf[BUFFERLEN];
  char msg[BUFFERLEN];
  va_list ap;

  if (level > loglevel)
    return;

  va_start (ap,fmt);
  vsnprintf (msg,BUFFERLEN,fmt,ap);
  va_end (ap);

  log_get_time_str (time_buf,BUFFERLEN);

  if (hostname_ptr == NULL) {
    if (gethostname(hostname_buf,MAXNAMELEN-1) == -1) {
      strcpy (hostname_buf,"UNKNOWN_hostname.error_on_gethostname");
    }
    hostname_ptr = hostname_buf;
  }

/*   if (!logonscreen) { */
/*     f_log = log_slave_open_computer (name); */
/*     if (!f_log) */
/*       f_log = stdout; */
/*   } else { */
/*     f_log = stdout; */
/*   } */

  f_log = stderr;

  if (loglevel < L_DEBUG)
    fprintf (f_log,"%8s : %s: %s\n",time_buf,log_level_str(level),msg);
  else
    fprintf (f_log,"%8s : (%i) %s: %s\n",time_buf,(int) getpid(), log_level_str(level),msg);

/*   if (!logonscreen) */
/*     fclose (f_log); */
}
