/* $Id: slave.c,v 1.10 2001/07/05 10:53:24 jorge Exp $ */

#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "slave.h"
#include "computer.h"
#include "logger.h"
#include "request.h"
#include "communications.h"
#include "semaphores.h"

struct slave_database sdb;	/* slave database */

int main (int argc,char *argv[])
{

  log_slave_computer (L_INFO,"Starting...");
  set_signal_handlers ();

  sdb.shmid = get_shared_memory_slave ();
  sdb.comp = attach_shared_memory_slave (sdb.shmid);
  sdb.semid = get_semaphores_slave ();

  init_tasks (sdb.comp->status.task);

  init_computer_status (&sdb.comp->status);
  get_hwinfo (&sdb.comp->hwinfo);
  report_hwinfo (&sdb.comp->hwinfo);

  register_slave (sdb.comp);

  if (fork() == 0) {
    /* Create the listening process */
/*      strcpy (argv[0],"DrQueue - slave -> listening process"); */
/*      argv[0] = "DrQueue - slave -> listening process"; */
    set_signal_handlers_child_listening ();
    slave_listening_process (&sdb);
    exit (0);
  }

  while (1) {
    get_computer_status (&sdb.comp->status);
    report_computer_status (&sdb.comp->status);

    while (computer_available(sdb.comp)) {
      printf ("Computer available !\n");
      if (request_job_available(&sdb)) {
	launch_task(&sdb);
      } else {
	break;			/* The while */
      }
    } /* WARNING could be in this loop forever if no care is taken !! */

    update_computer_status (sdb.comp); /* sends the computer status to the master */
    sleep (SLAVEDELAY);
  }

  exit (0);
}

void set_signal_handlers (void)
{
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
  sigaction (SIGCLD, &ignore, NULL);
}

void clean_out (int signal, siginfo_t *info, void *data)
{

  kill (0,SIGINT);
  log_slave_computer (L_INFO,"Cleaning...");

  if (semctl (sdb.semid,0,IPC_RMID,NULL) == -1) {
    perror ("semid");
  }
  if (shmctl (sdb.shmid,IPC_RMID,NULL) == -1) {
    perror ("shmid");
  }

  exit (0);
}


int get_shared_memory_slave (void)
{
  key_t key;
  int shmid;

  if ((key = ftok ("./slave",'A')) == -1) {
    perror ("ftok");
    exit (1);
  }
  
  if ((shmid = shmget (key,sizeof(struct computer), IPC_EXCL|IPC_CREAT|0600)) == -1) {
    perror ("shmget");
    exit (1);
  }

  return shmid;
}

int get_semaphores_slave (void)
{
  key_t key;
  int semid;

  if ((key = ftok ("slave",'A')) == -1) {
    perror ("ftok");
    kill (0,SIGINT);
  }

  if ((semid = semget (key,1, IPC_EXCL|IPC_CREAT|0600)) == -1) {
    perror ("semget");
    kill (0,SIGINT);
  }
  if (semctl (semid,0,SETVAL,1) == -1) {
    perror ("semctl SETVAL -> 1");
    kill (0,SIGINT);
  }

  return semid;
}

void *attach_shared_memory_slave (int shmid)
{
  void *rv;			/* return value */

  if ((rv = shmat (shmid,0,0)) == (void *)-1) {
    perror ("shmat");
    exit (1);
  }

  return rv;
}

void set_signal_handlers_child_listening (void)
{
  struct sigaction action_dfl;

  action_dfl.sa_sigaction = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
  action_dfl.sa_flags = SA_SIGINFO;
  sigaction (SIGINT, &action_dfl, NULL);
  sigaction (SIGTERM, &action_dfl, NULL);
}

void set_signal_handlers_child_chandler (void)
{
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

void set_signal_handlers_child_launcher (void)
{
  struct sigaction action_dfl;

  action_dfl.sa_sigaction = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
  action_dfl.sa_flags = SA_SIGINFO;
  sigaction (SIGINT, &action_dfl, NULL);
  sigaction (SIGTERM, &action_dfl, NULL);
}

void slave_listening_process (struct slave_database *sdb)
{
  int sfd,csfd;

  if ((sfd = get_socket(SLAVEPORT)) == -1) {
    log_slave_computer (L_ERROR,"Unable to open socket");
    kill(0,SIGINT);
  }
  printf ("Waiting for connections...\n");
  while (1) {
    if ((csfd = accept_socket_slave (sfd)) != -1) {
      if (fork() == 0) {
	/* Create a connection handler */
	set_signal_handlers_child_chandler ();
	close (sfd);
	alarm (MAXTIMECONNECTION);
	handle_request_slave (csfd,sdb);
	close (csfd);
	exit (0);
      } else {
	/* Father */
	close (csfd);
	printf ("csfd: %i\n",csfd);
      }
    }
  }
}

void sigalarm_handler (int signal, siginfo_t *info, void *data)
{
  /* This is not an error because it only happens on a connection handler */
  log_slave_computer (L_WARNING,"Connection time exceeded");
  exit (1);
}

void sigpipe_handler (int signal, siginfo_t *info, void *data)
{
  /* This is not an error because it only happens on a connection handler */
  log_slave_computer (L_WARNING,"Broken connection while reading or writing");
  exit (1);
}

void launch_task (struct slave_database *sdb)
{
  /* Here we get the job ready in the process task structure */
  /* pointed by sdb->itask */
  int rc;
  pid_t task_pid;
  extern char **environ;

  if (fork() == 0) {
    /* This child reports the execution of the command itself */
    set_signal_handlers_child_launcher ();
    if ((task_pid = fork()) == 0) {
      /* This child execs the command */
      const char *new_argv[4];	/* from libc sources */
      new_argv[0] = SHELL_NAME;
      new_argv[1] = "-c";
      new_argv[2] = sdb->comp->status.task[sdb->itask].jobcmd;
      new_argv[3] = NULL;

      set_environment(sdb);

      execve(SHELL_PATH,(char*const*)new_argv,environ);

      exit(errno);		/* If we arrive here, something happened exec'ing */
    }
    
    /* Then we set the process as running */
    semaphore_lock(sdb->semid);
    sdb->comp->status.task[sdb->itask].status = TASKSTATUS_RUNNING;
    sdb->comp->status.task[sdb->itask].pid = task_pid;
    semaphore_release(sdb->semid);

    if (waitpid(task_pid,&rc,0) == -1) {
      /* Some problem exec'ing */
      log_slave_task(&sdb->comp->status.task[sdb->itask],L_ERROR,"Exec'ing cmdline (No child on launcher)");
    } else {
      /* We have to clean the task and send the info to the master */
      /* consider WIFSIGNALED(status), WTERMSIG(status), WEXITSTATUS(status) */
      /* we pass directly the status to the master and he decides what to do with the frame */
      log_slave_task(&sdb->comp->status.task[sdb->itask],L_INFO,"Frame finished");

      semaphore_lock(sdb->semid);
      sdb->comp->status.task[sdb->itask].exitstatus = rc; /* We set the exitstatus as the return code */
      semaphore_release(sdb->semid);

      request_task_finished (sdb);

      semaphore_lock(sdb->semid);
      sdb->comp->status.task[sdb->itask].used = 0; /* We don't need the task anymore */
      semaphore_release(sdb->semid);
    }

    exit (0);
  }
}

void set_environment (struct slave_database *sdb)
{
  char msg[BUFFERLEN];

  snprintf (msg,BUFFERLEN,"%04i",sdb->comp->status.task[sdb->itask].frame);
  if (setenv("FRAME",msg,1) == -1)
    printf ("ERROR\n");

  printf ("Environment: %s\n",getenv("FRAME"));
}










