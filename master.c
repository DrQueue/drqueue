/* $Id: master.c,v 1.2 2001/05/07 15:35:04 jorge Exp $ */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <wait.h>

#include "master.h"
#include "database.h"
#include "logger.h"
#include "communications.h"
#include "request.h"

struct database *wdb;		/* whole database */
int sfd;			/* socket file descriptor */
int icomp;			/* index to accepted computer, local to every child */

int main (int argc, char *argv[])
{
  int csfd;			/* child sfd, the socket once accepted the connection */
  int shmid;			/* shared memory id */

  log_master ("Starting...");

  set_signal_handlers ();

  shmid = get_shared_memory ();
  wdb = attach_shared_memory (shmid);
  wdb->shmid = shmid;
  wdb->semid = get_semaphores ();

  sfd = get_socket(MASTERPORT);

  printf ("Pid: %i Gid: %i\n",getpid(),getpgid(0));
  printf ("%i %i\n",sizeof(*wdb),sizeof(struct database));
  printf ("%i %i\n",sizeof(int),sizeof(long int));
  printf ("%i %i\n",sizeof(struct job),sizeof(struct computer));

  while (1) {
    printf ("Waiting for connections...\n");
    if ((csfd = accept_socket (sfd,wdb,&icomp)) != -1) {
      if (fork() == 0) {
	strcpy (argv[0],"Connection handler");
	set_signal_handlers_child ();
	close (sfd);
	set_alarm ();
	handle_request_master (csfd,wdb,icomp);
	close (csfd);
	exit (0);
      }
    }
  }

  exit (0);
}

int get_shared_memory (void)
{
  key_t key;
  int shmid;

  if ((key = ftok ("master",'Z')) == -1) {
    perror ("ftok");
    exit (1);
  }
  
  if ((shmid = shmget (key,sizeof (struct database), IPC_CREAT | 0600)) == -1) {
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
    exit (1);
  }

  if ((semid = semget (key,1, IPC_CREAT | 0600)) == -1) {
    perror ("semget");
    exit (1);
  }
  semctl (semid,0,SETVAL,1);

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

  ignore.sa_handler = SIG_IGN;
  sigemptyset (&ignore.sa_mask);
  ignore.sa_flags = 0;
  sigaction (SIGHUP, &ignore, NULL);
  sigaction (SIGCLD, &ignore, NULL);
}


void set_signal_handlers_child (void)
{
  struct sigaction action_dfl;
  struct sigaction action_alarm;
  struct sigaction action_pipe;

  action_dfl.sa_sigaction = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
  action_dfl.sa_flags = SA_SIGINFO;
  sigaction (SIGINT, &action_dfl, NULL);
  action_alarm.sa_sigaction = sigalarm_handler;
  sigemptyset (&action_alarm.sa_mask);
  action_alarm.sa_flags = SA_SIGINFO;
  sigaction (SIGALRM, &action_alarm, NULL);
  action_pipe.sa_sigaction = sigpipe_handler;
  sigemptyset (&action_pipe.sa_mask);
  action_pipe.sa_flags = SA_SIGINFO;
  sigaction (SIGPIPE, &action_pipe, NULL);
}

void clean_out (int signal, siginfo_t *info, void *data)
{
  int rc;
  pid_t child_pid;

  kill(0,SIGINT);
  while ((child_pid = wait (&rc)) != -1) {
    printf ("Child arrived ! %i\n",child_pid); 
  }
  log_master ("Cleaning...");

  close (sfd);
  shmctl (wdb->shmid,IPC_RMID,NULL);
  shmctl (wdb->semid,IPC_RMID,NULL);

  exit (1);
}

void set_alarm (void)
{
  alarm (MAXTIMECONNECTION);
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







