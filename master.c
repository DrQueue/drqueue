/* $Id: master.c,v 1.5 2001/06/05 12:19:45 jorge Exp $ */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <wait.h>
#include <time.h>
#include <stdlib.h>

#include "master.h"
#include "database.h"
#include "logger.h"
#include "communications.h"
#include "request.h"
#include "semaphores.h"
#include "drerrno.h"

struct database *wdb;		/* whole database */
int sfd;			/* socket file descriptor */
int icomp;			/* index to accepted computer, local to every child */


int main (int argc, char *argv[])
{
  int csfd;			/* child sfd, the socket once accepted the connection */
  int shmid;			/* shared memory id */

  fprintf (stderr,"Master at: %i\n",getpid());

  log_master ("Starting...");
  set_signal_handlers ();

  shmid = get_shared_memory ();
  wdb = attach_shared_memory (shmid);
  wdb->shmid = shmid;
  wdb->semid = get_semaphores ();

  database_init(wdb);

  if (fork() == 0) {
    fprintf (stderr,"Consistency checks at: %i\n",getpid());
    /* Create the consistency checks process */
/*      strcpy (argv[0],"DrQueue - Consistency checks"); */
    set_signal_handlers_child_cchecks ();
    master_consistency_checks (wdb);
    exit (0);
  }

  if ((sfd = get_socket(MASTERPORT)) == -1) {
    kill(0,SIGINT);
  }

  while (1) {
    printf ("Waiting for connections...\n");
    if ((csfd = accept_socket (sfd,wdb,&icomp)) != -1) {
      if (fork() == 0) {
	fprintf (stderr,"Child at: %i\n",getpid());
	fflush(stderr);
	/* Create a connection handler */
/*  	strcpy (argv[0],"DrQueue - Connection handler"); */
	set_signal_handlers_child_conn_handler ();
	close (sfd);
	set_alarm ();
	handle_request_master (csfd,wdb,icomp);
	close (csfd);
	exit (0);
      } else {
	close (csfd);
	printf ("csfd: %i\n",csfd);
      }
    }
  }

  exit (0);
}

int get_shared_memory (void)
{
  key_t key;
  int shmid;

  if ((key = ftok ("./master",'Z')) == -1) {
    perror ("ftok");
    exit (1);
  }
  
  if ((shmid = shmget (key,sizeof(struct database), IPC_EXCL|IPC_CREAT|0600)) == -1) {
    perror ("shmget");
    exit (1);
  }

  return shmid;
}

int get_semaphores (void)
{
  key_t key;
  int semid;

  if ((key = ftok ("master",'Z')) == -1) {
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

void *attach_shared_memory (int shmid)
{
  void *rv;			/* return value */

  if ((rv = shmat (shmid,0,0)) == (void *)-1) {
    perror ("shmat");
    exit (1);
  }

  return rv;
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


void set_signal_handlers_child_conn_handler (void)
{
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

void set_signal_handlers_child_cchecks (void)
{
  struct sigaction action_dfl;

  action_dfl.sa_sigaction = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
  action_dfl.sa_flags = SA_SIGINFO;
  sigaction (SIGINT, &action_dfl, NULL);
  sigaction (SIGTERM, &action_dfl, NULL);
  sigaction (SIGSEGV, &action_dfl, NULL);
}

void clean_out (int signal, siginfo_t *info, void *data)
{
  int rc;
  pid_t child_pid;
  int i;

  kill(0,SIGINT);		/* Kill all the children (Wow, I don't really want to do that...) */
  while ((child_pid = wait (&rc)) != -1) {
    printf ("Child arrived ! %i\n",child_pid); 
  }
  log_master ("Cleaning...");

  close (sfd);

  for (i=0;i<MAXJOBS;i++) {
    job_delete(&wdb->job[i]);
  }

  if (semctl (wdb->semid,0,IPC_RMID,NULL) == -1) {
    perror ("wdb->semid");
  }
  if (shmctl (wdb->shmid,IPC_RMID,NULL) == -1) {
    perror ("wdb->shmid");
  }

  fprintf (stderr,"PID,Signal: %i,%i\n",getpid(),signal);

  exit (1);
}

void set_alarm (void)
{
/*    alarm (MAXTIMECONNECTION); */
}

void sigalarm_handler (int signal, siginfo_t *info, void *data)
{
  if (icomp != -1)
    log_master_computer (&wdb->computer[icomp],"Connection time exceeded");
  else
    log_master ("Connection time exceeded");
  exit (1);
}

void sigpipe_handler (int signal, siginfo_t *info, void *data)
{
  if (icomp != -1)
    log_master_computer (&wdb->computer[icomp],"Broken connection while reading or writing");
  else
    log_master ("Broken connection while reading or writing");
  exit (1);
}

void sigsegv_handler (int signal, siginfo_t *info, void *data)
{
  if (icomp != -1)
    log_master_computer (&wdb->computer[icomp],"Segmentation fault... too bad");
  else
    log_master ("Segmentation fault... too bad");
  exit (1);
}

void master_consistency_checks (struct database *wdb)
{
  while (1) {
    check_lastconn_times (wdb);
    
    sleep (MASTERCCHECKSDELAY);
  }
}

void check_lastconn_times (struct database *wdb)
{
  int i;
  time_t now;

  time(&now);
  for (i=0;i<MAXCOMPUTERS;i++) {
    if (wdb->computer[i].used) {
      if ((now - wdb->computer[i].lastconn) > MAXTIMENOCONN) {
	log_master_computer (&wdb->computer[i],"Info: Maximum time without connecting exceeded. Deleting");
	semaphore_lock(wdb->semid);
	wdb->computer[i].used = 0;
	semaphore_release(wdb->semid);
      }
    }
  }
}



