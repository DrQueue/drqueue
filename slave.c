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

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#ifdef __OSX
# include <sys/select.h>
#endif
#ifdef __FREEBSD
# define SIGCLD SIGCHLD
#endif

#include "slave.h"
#include "libdrqueue.h"

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

struct slave_database sdb;   /* slave database */
fd_set read_set;

/* Phantom is declared in libdrqueue.h */
struct timeval timeout;
int rs;             /* Result from select */
char buffer[BUFFERLEN];     /* Buffer to read from phantom */

int main (int argc,char *argv[]) {
  int force = 0;
  pid_t consistency_pid;
  pid_t listener_pid;

  slave_get_options(&argc,&argv,&force,&sdb);
  
  // Set some standard defaults based on DRQUEUE_ROOT (must be already set!)
  set_default_env(); 
  
  // Config files overrides environment CHANGE (?)
  // Read the config file after reading the arguments, as those may change
  // the path to the config file
  config_parse_tool("slave");
  
  if (!common_environment_check()) {
    log_auto (L_ERROR,"Error checking the environment: %s",drerrno_str());
    exit (1);
  }

  log_slave_computer (L_INFO,"Starting...");

  set_signal_handlers ();

  //system ("env | grep DRQUEUE");
  sdb.shmid = get_shared_memory_slave (force);
  sdb.comp = attach_shared_memory_slave (sdb.shmid);
  logger_computer = sdb.comp;
  sdb.semid = get_semaphores_slave ();

  computer_init (sdb.comp);
  sdb.comp->used = 1;
  get_hwinfo (&sdb.comp->hwinfo);
  computer_init_limits (sdb.comp); /* computer_init_limits depends on the hardware information */
  slave_set_limits (&sdb);

  report_hwinfo (&sdb.comp->hwinfo);

  register_slave (sdb.comp);
  // Before sending the limits we have to set the pools
  computer_pool_set_from_environment (&sdb.comp->limits);
  computer_pool_list (&sdb.comp->limits);
  update_computer_limits(&sdb.comp->limits); /* Does not need to be locked because at this point */
  /* because there is only one process running. The rest of the time */
  /* either we call it locked or we make a copy of the limits while locked */
  /* and then we send that copy */

  if (pipe(phantom) != 0) {
    fprintf (stderr,"Phantom pipe could not be created\n");
    exit (1);
  }

  if ((listener_pid = fork()) == 0) {
    /* Create the listening process */
    log_auto (L_INFO,"Listener process starting...");
    set_signal_handlers_child_listening ();
    slave_listening_process (&sdb);
    log_auto (L_INFO,"Listener process exiting...");
    exit (0); 
  } else if (listener_pid == -1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"Could not create the listener process. (%s)", strerror(drerrno_system));
    slave_exit(1);
  }

  if ((consistency_pid = fork()) == 0) {
    // Create the consistency checks process
    // Signal are treated the same way as the listening process
    log_auto (L_INFO,"Consistency process starting...");
    set_signal_handlers_child_listening ();
    slave_consistency_process (&sdb);
    log_auto (L_INFO,"Consistency process exiting...");
    exit (0);
  } else if (consistency_pid == -1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"Could not create the listener process. (%s)", strerror(drerrno_system));
    slave_exit(1);
  } 

  while (1) {
    get_computer_status (&sdb.comp->status,sdb.semid);

    computer_autoenable_check (&sdb); /* Check if it's time for autoenable */

    while (computer_available(sdb.comp)) {
      uint16_t itask;
      if (request_job_available(&sdb,&itask)) {
        launch_task(&sdb,itask);
        update_computer_status (&sdb);
      } else {
        // computer not available
        break;   // break the while loop
      }
    } /* WARNING could be in this loop forever if no care is taken !! */

    update_computer_status (&sdb); /* sends the computer status to the master */
    /* Does not need to be locked because we lock inside it */

    FD_ZERO(&read_set);
    FD_SET(phantom[0],&read_set);
    timeout.tv_sec = SLAVEDELAY;
    timeout.tv_usec = 0;
    rs = select (phantom[0]+1,&read_set,NULL,NULL,&timeout);
    switch (rs) {
    case -1:
      /* Error in select */
      log_slave_computer(L_ERROR,"Select call failed");
    case 0:
      log_slave_computer(L_DEBUG,"Slave loop (select call timeout)");
      break;
    default:
      if (FD_ISSET(phantom[0],&read_set)) {
        log_slave_computer(L_DEBUG,"Select call, notification came. Available for reading.");
        read(phantom[0],buffer,BUFFERLEN);
      } else {
        log_slave_computer(L_WARNING,"Select call, report this message, please. It should never happen.");
      }
    }
  }

  exit (0);
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
  sigaction (SIGHUP, &ignore, NULL);
#ifdef __OSX

  sigaction (SIGCHLD, &ignore, NULL);
#else

  sigaction (SIGCLD, &ignore, NULL);
#endif
}

void clean_out (int signal, siginfo_t *info, void *data) {
  int rc;
  pid_t child_pid;
  int i;
  struct sigaction ignore;
  struct sigaction action_dfl;

  /* Ignore new int signals that could arrive during clean up */
  ignore.sa_handler = SIG_IGN;
  sigemptyset (&ignore.sa_mask);
  ignore.sa_flags = 0;
  sigaction (SIGINT, &ignore, NULL);

  // Handle SIGCLD properly
  action_dfl.sa_handler = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
#ifdef __OSX

  sigaction (SIGCHLD, &action_dfl, NULL);
#else

  sigaction (SIGCLD, &action_dfl, NULL);
#endif


  log_slave_computer (L_INFO,"Cleaning...");

  for (i=0;i<MAXTASKS;i++) {
    if (sdb.comp->status.task[i].used)
      kill(-sdb.comp->status.task[i].pid,SIGINT);
  }
  kill (0,SIGINT);

  while ((child_pid = wait (&rc)) != -1) {
    printf ("Child arrived ! %i\n",(int)child_pid);
  }

  request_slavexit (sdb.comp->hwinfo.id,SLAVE);

  computer_free (sdb.comp);

  if (semctl ((int)sdb.semid,0,IPC_RMID,NULL) == -1) {
    perror ("semid");
  }
  if (shmctl ((int)sdb.shmid,IPC_RMID,NULL) == -1) {
    perror ("shmid");
  }

  exit (0);
}


int64_t get_shared_memory_slave (int force) {
  key_t key;
  int64_t shmid;
  int shmflg;
  char file[BUFFERLEN];
  char *root;

  root = getenv("DRQUEUE_BIN");
  if (root) {
    snprintf (file,BUFFERLEN-1,"%s/%s",root,KEY_SLAVE);
  } else {
    log_auto (L_ERROR,"get_shared_memory_slave(): environment variable DRQUEUE_BIN not defined.");
    exit (1);
  }
  log_auto (L_DEBUG,"get_shared_memory_slave(): using file '%s' as key for shared resources.",file);
  
  if ((key = ftok (file,'A')) == -1) {
    log_auto (L_ERROR,"get_shared_memory_slave(): error obtaining key for shared memory (ftok): %s", strerror(errno));
    exit (1);
  }

  if (force) {
    shmflg = IPC_CREAT|0600;
  } else {
    shmflg = IPC_EXCL|IPC_CREAT|0600;
  }

  if ((shmid = (int64_t)shmget (key,sizeof(struct computer),shmflg)) == (int64_t)-1) {
    log_auto (L_ERROR,"get_shared_memory_slave(): error allocating shared memory space (shmget): %s", strerror(errno));
    if (!force) {
      fprintf (stderr,"Try with option -f (if you are _sure_ that no other slave is running)\n");
    }
    exit (1);
  }

  return shmid;
}

int64_t get_semaphores_slave (void) {
  key_t key;
  int64_t semid;
  struct sembuf op;
  char file[BUFFERLEN];
  char *root;

  root = getenv("DRQUEUE_BIN");
  if (root) {
    snprintf (file,BUFFERLEN-1,"%s/%s",root,KEY_SLAVE);
  } else {
    log_auto (L_ERROR,"get_semaphores_slave(): environment variable DRQUEUE_BIN not defined.");
    exit (1);
  }
  log_auto (L_DEBUG,"get_semaphores_slave(): using file '%s' as key for shared resources.",file);
  

  if ((key = ftok (file,'A')) == -1) {
    log_slave_computer (L_ERROR,"Getting key for semaphores");
    kill (0,SIGINT);
  }

  if ((semid = (int64_t)semget (key,1, IPC_CREAT|0600)) == (int64_t)-1) {
    log_slave_computer (L_ERROR,"Getting semaphores: %s",strerror(errno));
    kill (0,SIGINT);
  }

  if (semctl ((int)semid,0,SETVAL,1) == -1) {
    log_slave_computer (L_ERROR,"semctl SETVAL -> 1 : (%s)",strerror(errno));
    kill (0,SIGINT);
  }
  if (semctl ((int)semid,0,GETVAL) == 0) {
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    if (semop((int)semid,&op,1) == -1) {
      log_slave_computer (L_ERROR,"semaphore_release: %s",strerror(errno));
      kill(0,SIGINT);
    }
  }

  return semid;
}

void *attach_shared_memory_slave (int64_t shmid) {
  void *rv;   /* return value */

  if ((rv = shmat ((int)shmid,0,0)) == (void *)-1) {
    log_slave_computer (L_ERROR,"Problem attaching slave shared memory segment");
    kill(0,SIGINT);
  }

  return rv;
}

void set_signal_handlers_child_listening (void) {
  struct sigaction action_dfl;

  action_dfl.sa_handler = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
  sigaction (SIGINT, &action_dfl, NULL);
  sigaction (SIGTERM, &action_dfl, NULL);
}

void set_signal_handlers_child_chandler (void) {
  struct sigaction action_alarm;
  struct sigaction action_pipe;

  action_alarm.sa_sigaction = sigalarm_handler;
  sigemptyset (&action_alarm.sa_mask);
  action_alarm.sa_flags = SA_SIGINFO;
  sigaction (SIGALRM, &action_alarm, NULL);
  action_pipe.sa_sigaction = sigpipe_handler;
  sigemptyset (&action_pipe.sa_mask);
  action_pipe.sa_flags = SA_SIGINFO;
  sigaction (SIGPIPE, &action_pipe, NULL);
}

void set_signal_handlers_child_launcher (void) {
  struct sigaction action_ignore;
  struct sigaction action_dfl;

  action_ignore.sa_handler = SIG_IGN;
  sigemptyset (&action_ignore.sa_mask);
  sigaction (SIGINT, &action_ignore, NULL);
  sigaction (SIGTERM, &action_ignore, NULL);

  action_dfl.sa_handler = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
#ifdef __OSX

  sigaction (SIGCHLD, &action_dfl, NULL);
#else

  sigaction (SIGCLD, &action_dfl, NULL);
#endif
}

void set_signal_handlers_task_exec (void) {
  struct sigaction action_dfl;

  action_dfl.sa_handler = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
  sigaction (SIGINT, &action_dfl, NULL);
  sigaction (SIGTERM, &action_dfl, NULL);
#ifdef __OSX

  sigaction (SIGCHLD, &action_dfl, NULL);
#else

  sigaction (SIGCLD, &action_dfl, NULL);
#endif
}

void slave_consistency_process (struct slave_database *sdb) {
  int i;

  while (1) {
    for (i=0;i<MAXTASKS;i++) {
      if ((sdb->comp->status.task[i].used)
          && (sdb->comp->status.task[i].status != TASKSTATUS_LOADING)
          && (kill(sdb->comp->status.task[i].pid,0) == -1))
        {
          // There is process registered as running, but not running.
          semaphore_lock(sdb->semid);
          sdb->comp->status.task[i].used = 0;
          semaphore_release(sdb->semid);
          log_slave_computer(L_WARNING,"Process registered as running was not running. Removed.");
        }
    }
    sleep (SLAVEDELAY);
  }
}

void slave_listening_process (struct slave_database *sdb) {
  pid_t child_pid;
  int sfd,csfd,highest_fd;

  if ((sfd = get_socket(SLAVEPORT)) == -1) {
    log_slave_computer (L_ERROR,"Unable to open socket (server)");
    slave_exit(1);
  }
  highest_fd = sfd+1;
  log_auto (L_DEBUG,"Highest file descriptor after initialization %i",highest_fd);
  log_auto (L_INFO,"Slave waiting for remote requests");
  while (1) {
    if ((csfd = accept_socket_slave (sfd)) != -1) {
      // Ignore children exit codes & do not let zombies around
      signal(SIGCHLD,SIG_IGN); // FIXME: sigaction
      if ((child_pid = fork()) == 0) {
        // Child process
        set_signal_handlers_child_chandler ();
        close (sfd);
        alarm (MAXTIMECONNECTION);
        handle_request_slave (csfd,sdb);
        close (csfd);
        exit (0);
      } else if (child_pid == -1) {
        // Parent still
        drerrno_system = errno;
        log_auto (L_ERROR,"slave_listening_process(): error forking on slave_listening_process. (%s)",strerror(drerrno_system));
      }
      // Parent
      close (csfd);
      if (csfd > highest_fd)
        log_auto (L_DEBUG,"slave_listening_process(): csfd has grown over the default highest (csfd=%i)",csfd);
    } else { // csfd == -1
      log_auto (L_ERROR,"slave_listening_process(): error accepting connection. (%s)",strerror(drerrno_system));
    }
  }
  exit (0);
}

void sigalarm_handler (int signal, siginfo_t *info, void *data) {
  /* This is not an error because it only happens on a connection handler */
  log_slave_computer (L_WARNING,"Connection time exceeded");
  exit (1);
}

void sigpipe_handler (int signal, siginfo_t *info, void *data) {
  /* This is not an error because it only happens on a connection handler */
  log_slave_computer (L_WARNING,"Broken connection while reading or writing");
  exit (1);
}

void launch_task (struct slave_database *sdb, uint16_t itask) {
  /* Here we get the job ready in the process task structure pointed by itask */
  int rc;
  pid_t task_pid,waiter_pid;
  extern char **environ;
  char *exec_path;

  if ((waiter_pid = fork()) == 0) {
    //
    // WAITER PROCESS
    // This process reports the execution of the command itself
    //
    set_signal_handlers_child_launcher ();
    if ((task_pid = fork()) == 0) {
      //
      // TASK PROCESS
      // This process executes the task
      // This child also creates the directory for logging if it doesn't exist
      // and prepares the file descriptors so every output will be logged
      //

      const char *new_argv[4];
      int lfd;   /* logger fd */

#ifdef __CYGWIN

      new_argv[0] = SHELL_NAME;
      if ((new_argv[1] = malloc(MAXCMDLEN)) == NULL)
        exit (1);
      cygwin_conv_to_posix_path(sdb->comp->status.task[itask].jobcmd,(char*)new_argv[1]);
      new_argv[2] = NULL;
#else

      new_argv[0] = SHELL_NAME;
      new_argv[1] = "-c";
      new_argv[2] = sdb->comp->status.task[itask].jobcmd;
      new_argv[3] = NULL;
#endif

      setpgid(0,0);  /* So this process doesn't receive signals from the others */
      set_signal_handlers_task_exec ();

      if ((lfd = log_dumptask_open (&sdb->comp->status.task[itask])) != -1) {
        // Log on the logger file whatever goes to stdout and stderr
        dup2 (lfd,STDOUT_FILENO);
        dup2 (lfd,STDERR_FILENO);
        close (lfd);
      }

      task_environment_set(&sdb->comp->status.task[itask]);

#ifdef __CYGWIN

      exec_path = malloc(PATH_MAX);
      char *dr_bin = getenv("DRQUEUE_BIN");
      if (dr_bin) {
        snprintf (exec_path,PATH_MAX,"%s/tcsh.exe",dr_bin);
#else

      exec_path = SHELL_PATH;
#endif

      execve(exec_path,(char*const*)new_argv,environ);
      // Wouldn't reach this point unless error on execve
      drerrno_system = errno;
      log_auto(L_ERROR,"launch_task(): error on execve. (%s)",strerror(drerrno_system));
      exit(drerrno_system);
    } else if (task_pid == -1) {
      // FIXME <<__EOFIX
      log_auto(L_ERROR,"lauch_task(): Fork failed. Task not created.");
      log_slave_task(&sdb->comp->status.task[itask],L_ERROR,"lauch_task(): Fork failed. Task not created.");
      // __EOFIX

      //semaphore_lock(sdb->semid);
      //sdb->comp->status.task[itask].used = 0; /* We don't need the task anymore */
      //semaphore_release(sdb->semid);
    }

    /* Then we set the process as running */
    semaphore_lock(sdb->semid);
    sdb->comp->status.task[itask].status = TASKSTATUS_RUNNING;
    sdb->comp->status.task[itask].pid = task_pid;
    semaphore_release(sdb->semid);

    if (waitpid(task_pid,&rc,0) == -1) {
      // It forked on task_pid but waitpid says it doesn't exist.
      drerrno_system = errno;
      // FIXME <<__EOFIX
      log_auto(L_ERROR,"lauch_task(): task process (%i) does not exist. (%s)",task_pid,strerror(drerrno_system));
      log_slave_task(&sdb->comp->status.task[itask],L_ERROR,"lauch_task(): task process (%i) does not exist. (%s)",
                     task_pid,strerror(drerrno_system));
      // __EOFIX
      semaphore_lock(sdb->semid);
      sdb->comp->status.task[itask].used = 0; /* We don't need the task anymore */
      semaphore_release(sdb->semid);
      // TODO: notify the master ?
    } else {
      // waitpid returned successfully
      /* We have to clean the task and send the info to the master */
      /* consider WIFSIGNALED(status), WTERMSIG(status), WEXITSTATUS(status) */
      /* we pass directly the status (translated to DR) to the master and he decides what to do with the frame */
      semaphore_lock(sdb->semid);
      sdb->comp->status.task[itask].exitstatus = 0;
      sdb->comp->status.task[itask].status = TASKSTATUS_FINISHED;
      sdb->comp->status.nrunning--;
      if (WIFSIGNALED(rc)) {
        /* Process exited abnormally either killed by us or by itself (SIGSEGV) */
        /*  printf ("\n\nSIGNALED with %i\n",WTERMSIG(rc)); */
        sdb->comp->status.task[itask].exitstatus |= DR_SIGNALEDFLAG ;
        sdb->comp->status.task[itask].exitstatus |= WTERMSIG(rc);
        log_slave_task(&sdb->comp->status.task[itask],L_INFO,"Task signaled");
      } else {
        if (WIFEXITED(rc)) {
          /*   printf ("\n\nEXITED with %i\n",WEXITSTATUS(rc)); */
          sdb->comp->status.task[itask].exitstatus |= DR_EXITEDFLAG ;
          sdb->comp->status.task[itask].exitstatus |= WEXITSTATUS(rc);
          /* printf ("\n\nEXITED with %i\n",DR_WEXITSTATUS(sdb->comp->status.task[itask].exitstatus)); */
          log_slave_task(&sdb->comp->status.task[itask],L_INFO,"Task finished");
        }
      }
      semaphore_release(sdb->semid);

      request_task_finished (sdb,itask);

      semaphore_lock(sdb->semid);
      sdb->comp->status.task[itask].used = 0; /* We don't need the task anymore */
      semaphore_release(sdb->semid);
    }

    exit (0);
  } else if (waiter_pid == -1) {
    log_slave_task(&sdb->comp->status.task[itask],L_WARNING,"Fork failed for task waiter");
    semaphore_lock(sdb->semid);
    sdb->comp->status.task[itask].used = 0; /* We don't need the task anymore */
    semaphore_release(sdb->semid);
    exit (1);
  } else {
    // in this "else" waiter_pid actually contains the PID of the waiter process
  } 
}


void zerocmd (char *cmd) {
  /* this functions zeros all the spaces of a cmd so it can be later parsed */
  while (*cmd != 0) {
    if (isspace ((int)*cmd))
      *cmd = 0;
    cmd++;
  }
}

void usage (void) {
  fprintf (stderr,"Valid options:\n"
           "\t-a <hour:minute> to use autoenable\n"
           "\t-n <nprocs> to set the maximum number of CPUs\n"
           "\t-f to force continuing if shared memory already exists\n"
           "\t-l <loglevel> From 0 to 3 (0=errors,1=warnings,2=info,3=debug).\n\t\tDefaults to 2. Each level logs all the previous levels\n"
           "\t-o log on screen instead of on files\n"
           "\t-c <config_file> use this config file\n"
           "\t-v version information\n"
           "\t-h prints this help\n");
}

void slave_get_options (int *argc,char ***argv, int *force, struct slave_database *sdb) {
  int opt;
  char *hour,*min;

  while ((opt = getopt (*argc,*argv,"a:n:fl:c:ohv")) != -1) {
    switch (opt) {
    case 'a':
      sdb->limits.autoenable.flags |= AEF_ACTIVE;
      hour = optarg;
      if ((min = strchr (hour,':')) == NULL) {
        usage ();
        exit (1);
      }
      *min = '\0';
      min++;
      sdb->limits.autoenable.h = atoi (hour) % 24;
      sdb->limits.autoenable.m = atoi (min) % 60;
      printf ("Autoenable time from command line: %02i:%02i\n",sdb->limits.autoenable.h,sdb->limits.autoenable.m);
      break;
    case 'n':
      sdb->limits.nmaxcpus = atoi (optarg);
      sdb->flags |= SDBF_SETMAXCPUS;
      break;
    case 'c':
      strncpy(sdb->conf,optarg,PATH_MAX-1);
      printf ("Reading config file from: '%s'\n",sdb->conf);
      break;
    case 'f':
      *force = 1;
      fprintf (stderr,"WARNING: Forcing usage of pre-existing shared memory (-f). Do not do this unless you really know what it means.\n");
      break;
    case 'l':
      loglevel = atoi (optarg);
      printf ("Logging level set to: %i (%s)\n",loglevel,log_level_str(loglevel));
      break;
    case 'o':
      logonscreen = 1;
      printf ("Logging on screen.\n");
      break;
    case 'v':
      show_version (*argv);
      kill (0,SIGINT);
    case '?':
    case 'h':
      usage();
      kill (0,SIGINT);
    }
  }
}

void slave_set_limits (struct slave_database *sdb) {
  if (sdb->limits.autoenable.flags & AEF_ACTIVE) {
    sdb->comp->limits.autoenable.flags = sdb->limits.autoenable.flags;
    sdb->comp->limits.autoenable.h = sdb->limits.autoenable.h % 24;
    sdb->comp->limits.autoenable.m = sdb->limits.autoenable.m % 60;
    log_slave_computer (L_INFO,"Setting autoenable time to %i:%02i",
                        sdb->comp->limits.autoenable.h, sdb->comp->limits.autoenable.m);
  }
  if (sdb->flags & SDBF_SETMAXCPUS) {
    sdb->comp->limits.nmaxcpus = (sdb->limits.nmaxcpus > sdb->comp->limits.nmaxcpus) ?
                                 sdb->comp->limits.nmaxcpus : sdb->limits.nmaxcpus;
    log_slave_computer (L_INFO,"Setting maximum number of CPUs to %i",sdb->comp->limits.nmaxcpus);
  }
}

void
slave_exit (int rc) {
  // Slave's clean exit procedure
  
}
