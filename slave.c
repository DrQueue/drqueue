/* $Id: slave.c,v 1.7 2001/06/05 12:19:45 jorge Exp $ */

#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "slave.h"
#include "computer.h"
#include "logger.h"
#include "request.h"
#include "communications.h"

struct slave_database sdb;	/* slave database */

int main (int argc,char *argv[])
{

  log_slave_computer ("Starting...");
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
  log_slave_computer ("Cleaning...");

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


void slave_listening_process (struct slave_database *sdb)
{
  int sfd,csfd;

  if ((sfd = get_socket(SLAVEPORT)) == -1) {
    log_slave_computer ("Error: unable to open socket");
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
	close (sfd);
	printf ("csfd: %i\n",csfd);
      }
    }
  }
}

void sigalarm_handler (int signal, siginfo_t *info, void *data)
{
  log_slave_computer ("Connection time exceeded");
  exit (1);
}

void sigpipe_handler (int signal, siginfo_t *info, void *data)
{
  log_slave_computer ("Broken connection while reading or writing");
  exit (1);
}

void launch_task (struct slave_database *sdb)
{
  /* Ne need to get the possible task */
  
}












