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
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#ifdef __FREEBSD
# define SIGCLD SIGCHLD
#endif

#include "master.h"
#include "libdrqueue.h"

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

struct database *wdb;      /* whole database */
int icomp;   /* index to accepted computer, local to every child */
char conf[PATH_MAX];

#ifdef COMM_REPORT
time_t tstart;    /* Time at wich the master has started running */
#endif

int main (int argc, char *argv[]) {
  int sfd;   /* socket file descriptor */
  int csfd;   /* child sfd, the socket once accepted the connection */
  int64_t shmid;   /* shared memory id */
  int force = 0;  /* force even if shmem already exists */
  struct sockaddr_in addr; /* Address of the remote host */
  pid_t child,child_wait;
  int n_children = 0;
  int rc; // Return code
  int i; // For loops

#ifdef COMM_REPORT

  bsent = brecv = 0;
  tstart = time(NULL);
#endif

  master_get_options (&argc,&argv,&force);

  set_default_env(); // Config files overrides environment
  // Read the config file after reading the arguments, as those may change
  // the path to the config file
  config_parse_tool("master");
  log_master (L_INFO,"Starting...");

  if (!common_environment_check()) {
    fprintf (stderr,"Error checking the environment: %s\n",drerrno_str());
    exit (1);
  }
  set_signal_handlers ();

  shmid = get_shared_memory (force);

  if ((wdb = attach_shared_memory ((int)shmid)) == (void *) -1)
    exit (1);

  wdb->shmid = shmid;
  wdb->semid = get_semaphores (force);

  if (!database_load(wdb)) {
    fprintf (stderr,"Could not load database: %s. Initializing.\n",drerrno_str());
    database_init(wdb);
  }
  for (i=0;i<MAXCOMPUTERS;i++) {
    computer_init (&wdb->computer[i]);
  }

  if (fork() == 0) {
    /* Create the consistency checks process */
    set_signal_handlers_child_cchecks ();
    master_consistency_checks (wdb);
    exit (0);
  }
  if ((sfd = get_socket(MASTERPORT)) == -1) {
    kill(0,SIGINT);
  }

  printf ("Waiting for connections...\n");
  while (1) {
    if (n_children < MASTERNCHILDREN) {
      if ((child = fork()) == 0) {
        set_signal_handlers_child_conn_handler ();
        if ((csfd = accept_socket (sfd,wdb,&addr)) != -1) {
#ifdef COMM_REPORT
          long int bsentb = bsent; /* Bytes sent before */
          long int brecvb = brecv; /* Bytes received before */
#endif
          /* Create a connection handler */
          fflush(stderr);
          close (sfd);
          set_alarm ();
          icomp = computer_index_addr (wdb,addr.sin_addr);
          handle_request_master (csfd,wdb,icomp,&addr);
          close (csfd);
#ifdef COMM_REPORT

          semaphore_lock(wdb->semid);
          wdb->bsent += bsent - bsentb;
          wdb->brecv += brecv - brecvb;
          semaphore_release(wdb->semid);
#endif

        } else {
          log_master (L_ERROR,"Accepting connection.");
        }
        exit (0);
      } else if (child != -1) {
        n_children++;
      } else {
        log_master (L_ERROR,"Forking !!\n");
        sleep (5);
      }
    } else {
      if ((child_wait = wait (&rc)) != -1) {
        n_children--;
      }
    }
  }

  exit (0);
}

int64_t get_shared_memory (int force) {
  key_t key;
  int64_t shmid;
  int shmflg;
  char file[BUFFERLEN];
  char *root;

  root = getenv("DRQUEUE_BIN");
  snprintf (file,BUFFERLEN-1,KEY_MASTER,root);
  log_auto (L_INFO,"Using file '%s' to obtain shared memory id.",file);

  if ((key = ftok (file,'Z')) == -1) {
    perror ("Getting key for shared memory");
    exit (1);
  }

  if (force) {
    shmflg = IPC_CREAT|0600;
  } else {
    shmflg = IPC_EXCL|IPC_CREAT|0600;
  }

  if ((shmid = (int64_t) shmget (key,sizeof(struct database), shmflg)) == (int64_t)-1) {
    perror ("Getting shared memory");
    if (!force)
      fprintf (stderr,"Try with option -f (if you are sure that no other master is running)\n");
    exit (1);
  }

  return shmid;
}

int64_t get_semaphores (int force) {
  key_t key;
  int64_t semid;
  struct sembuf op;
  int semflg;
  char file[BUFFERLEN];
  char *root;

  root = getenv("DRQUEUE_BIN");
  snprintf (file,BUFFERLEN-1,KEY_MASTER,root);

  if ((key = ftok (file,'Z')) == -1) {
    perror ("Getting key for semaphores");
    kill (0,SIGINT);
  }

  if (force) {
    semflg = IPC_CREAT|0600;
  } else {
    semflg = IPC_EXCL|IPC_CREAT|0600;
  }

  if ((semid = semget (key,1,semflg)) == (int64_t)-1) {
    perror ("Getting semaphores");
    if (!force)
      fprintf (stderr,"Try with option -f (if you are sure that no other master is running)\n");
    kill (0,SIGINT);
  }
  if (semctl ((int)semid,0,SETVAL,1) == -1) {
    perror ("semctl SETVAL -> 1");
    kill (0,SIGINT);
  }
  if (semctl ((int)semid,0,GETVAL) == 0) {
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    if (semop((int)semid,&op,1) == -1) {
      perror ("semaphore_release");
      kill(0,SIGINT);
    }
  }

  /*  fprintf (stderr,"semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid); */
  return semid;
}

void *attach_shared_memory (int64_t shmid) {
  void *rv;   /* return value */

  if ((rv = shmat ((int)shmid,0,0)) == (void *)-1) {
    perror ("master shmat");
    return ((void *) -1);
  }

  return rv;
}

void set_signal_handlers (void) {
  struct sigaction clean;
  struct sigaction ignore;

  clean.sa_sigaction = clean_out;
  sigemptyset (&clean.sa_mask);
  clean.sa_flags = SA_SIGINFO;
  sigaction (SIGINT, &clean, NULL);
  sigaction (SIGTERM, &clean, NULL);
  sigaction (SIGSEGV, &clean, NULL);

  ignore.sa_handler = SIG_IGN;
  sigemptyset (&ignore.sa_mask);
  ignore.sa_flags = 0;
  sigaction (SIGHUP, &ignore, NULL); // So we keep running as a daemon
#ifdef __OSX
  // sigaction (SIGCHLD, &ignore, NULL);
#else
  // sigaction (SIGCLD, &ignore, NULL);
#endif
}


void set_signal_handlers_child_conn_handler (void) {
  struct sigaction action_dfl;
  struct sigaction action_alarm;
  struct sigaction action_pipe;
  struct sigaction action_sigsegv;

  action_dfl.sa_sigaction = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
  action_dfl.sa_flags = SA_SIGINFO;
  sigaction (SIGINT, &action_dfl, NULL);
  sigaction (SIGTERM, &action_dfl, NULL);

  action_alarm.sa_sigaction = sigalarm_handler;
  sigemptyset (&action_alarm.sa_mask);
  action_alarm.sa_flags = SA_SIGINFO;
  sigaction (SIGALRM, &action_alarm, NULL);
  action_pipe.sa_sigaction = sigpipe_handler;
  sigemptyset (&action_pipe.sa_mask);
  action_pipe.sa_flags = SA_SIGINFO;
  sigaction (SIGPIPE, &action_pipe, NULL);

  /* segv */
  action_sigsegv.sa_sigaction = sigsegv_handler;
  sigemptyset (&action_sigsegv.sa_mask);
  action_sigsegv.sa_flags = SA_SIGINFO;
  sigaction (SIGSEGV, &action_sigsegv, NULL);
}

void set_signal_handlers_child_cchecks (void) {
  struct sigaction action_dfl;
  struct sigaction action_sigsegv;

  action_dfl.sa_sigaction = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
  action_dfl.sa_flags = SA_SIGINFO;
  sigaction (SIGINT, &action_dfl, NULL);
  sigaction (SIGTERM, &action_dfl, NULL);

  // segv
  action_sigsegv.sa_sigaction = sigsegv_handler;
  sigemptyset (&action_sigsegv.sa_mask);
  action_sigsegv.sa_flags = SA_SIGINFO;
  sigaction (SIGSEGV, &action_sigsegv, NULL);
}

void clean_out (int signal, siginfo_t *info, void *data) {
  int rc;
  pid_t child_pid;
  int i;
  struct sigaction ignore;
#ifdef COMM_REPORT

  time_t tstop;   /* Time at wich the master stops running */
  time_t ttotal;  /* Total time */
#endif

  /* Ignore new int signals that could arrive during clean up */
  ignore.sa_handler = SIG_IGN;
  sigemptyset (&ignore.sa_mask);
  ignore.sa_flags = 0;
  sigaction (SIGINT, &ignore, NULL);

#ifdef COMM_REPORT

  tstop = time(NULL);
  ttotal = tstop - tstart;
  printf ("Report of communications:\n");
  printf ("Kbytes sent:\t\t%lli\tBytes:\t%lli\n",wdb->bsent/1024,wdb->bsent);
  printf ("Kbytes recv:\t\t%lli\tBytes:\t%lli\n",wdb->brecv/1024,wdb->brecv);
  printf ("Kbytes sent/second:\t%f\n",(float)(wdb->bsent/1024)/ttotal);
  printf ("Kbytes recv/second:\t%f\n",(float)(wdb->brecv/1024)/ttotal);
#endif

  kill(0,SIGINT);  /* Kill all the children (Wow, I don't really want to do that...) */
  while ((child_pid = wait (&rc)) != -1) {
    //  printf ("Child arrived ! %i\n",(int)child_pid);
  }

  log_master (L_INFO,"Saving...");
  database_save(wdb);

  log_master (L_INFO,"Cleaning...");
  for (i=0;i<MAXJOBS;i++) {
    job_delete(&wdb->job[i]);
  }
  for (i=0;i<MAXCOMPUTERS;i++) {
    computer_free (&wdb->computer[i]);
  }

  if (semctl ((int)wdb->semid,0,IPC_RMID,NULL) == -1) {
    perror ("wdb->semid");
  }
  if (shmctl ((int)wdb->shmid,IPC_RMID,NULL) == -1) {
    perror ("wdb->shmid");
  }

  fprintf (stderr,"PID,Signal that caused death: %i,%i\n",(int)getpid(),signal);

  exit (1);
}

void set_alarm (void) {
  alarm (MAXTIMECONNECTION);
}

void sigalarm_handler (int signal, siginfo_t *info, void *data) {
  char *msg = "Connection time exceeded";

  if (icomp != -1)
    log_master_computer (&wdb->computer[icomp],L_WARNING,msg);
  else
    log_master (L_WARNING,msg);
  exit (1);
}

void sigpipe_handler (int signal, siginfo_t *info, void *data) {
  char *msg = "Broken connection while reading or writing (SIGPIPE)";

  if (icomp != -1)
    log_master_computer (&wdb->computer[icomp],L_WARNING,msg);
  else
    log_master (L_WARNING,msg);
  exit (1);
}

void sigsegv_handler (int signal, siginfo_t *info, void *data) {
  char *msg = "Segmentation fault... too bad";

  if (icomp != -1)
    log_master_computer (&wdb->computer[icomp],L_ERROR,msg);
  else
    log_master (L_ERROR,msg);
  exit (1);
}

void master_consistency_checks (struct database *wdb) {
  uint32_t i;

  while (1) {
    log_master (L_DEBUG,"Master consistency checks loop");
    check_lastconn_times (wdb);

    for (i=0;i<MAXJOBS;i++) {
      if (wdb->job[i].used) {
        /* In this function will requeue non running frames registered as running */
        job_update_info (wdb,i);
      }
    }

    sleep (MASTERCCHECKSDELAY);
  }
}

void check_lastconn_times (struct database *wdb) {
  int i;
  time_t now;

  time(&now);
  for (i=0;i<MAXCOMPUTERS;i++) {
    semaphore_lock(wdb->semid);
    if (wdb->computer[i].used) {
      if ((now - wdb->computer[i].lastconn) > MAXTIMENOCONN) {
        log_master_computer (&wdb->computer[i],L_WARNING,
                             "Maximum time without connecting exceeded. Deleting. (Time not connected: %i)",
                             (int) (now - wdb->computer[i].lastconn));
        /* We only need to remove it this  way, without requeueing its frames because */
        /* the frames will be requeued on the consistency checks (job_update_info) */
        computer_free (&wdb->computer[i]);
      }
    }
    semaphore_release(wdb->semid);
  }
}

void usage (void) {
  fprintf (stderr,"Valid options:\n"
           "\t-f to force continuing if shared memory already exists\n"
           "\t-l <loglevel> From 0 to 3 (0=errors,1=warnings,2=info,3=debug).\n\t\tDefaults to 2. Each level logs all the previous levels\n"
           "\t-o log on screen instead of on files\n"
           "\t-c <config_file> use this config file\n"
           "\t-v version information\n"
           "\t-h prints this help\n");
}

void master_get_options (int *argc,char ***argv, int *force) {
  int opt;

  while ((opt = getopt (*argc,*argv,"c:fl:ohv")) != -1) {
    switch (opt) {
    case 'f':
      *force = 1;
      fprintf (stderr,"WARNING: Forcing usage of pre-existing shared memory (-f). Do not do this unless you really know what it means.\n");
      break;
    case 'l':
      loglevel = atoi (optarg);
      printf ("Logging level set to: %i (%s)\n",loglevel,log_level_str(loglevel));
      break;
    case 'c':
      strncpy(conf,optarg,PATH_MAX-1);
      printf ("Reading config file from: '%s'\n",conf);
      break;
    case 'o':
      logonscreen = 1;
      printf ("Logging on screen.\n");
      break;
    case 'v':
      show_version (*argv);
      exit (0);
      break;
    case '?':
    case 'h':
      usage();
      exit (1);
    }
  }
}

