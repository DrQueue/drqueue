/* $Id: master.c,v 1.1 2001/05/02 16:12:33 jorge Exp $ */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "master.h"
#include "database.h"
#include "logger.h"

#include "computer.h"
#include "job.h"

struct database *wdb;		/* whole database */
int shmid;			/* shared memory id */
int semid;			/* semaphore id */
int sfd;			/* socket file descriptor */

int main (int argc, char *argv[])
{
  struct sembuf op;

  set_signal_handlers ();

  shmid = get_shared_memory ();
  semid = get_semaphores ();
  wdb = attach_shared_memory (shmid);

  sfd = get_socket();

  printf ("Pid: %i Gid: %i\n",getpid(),getpgid(0));
  printf ("%i %i\n",sizeof(*wdb),sizeof(struct database));
  printf ("%i %i\n",sizeof(int),sizeof(long int));
  printf ("%i %i\n",sizeof(struct job),sizeof(struct computer));
  if (fork() == 0) {
    set_signal_handlers_child ();

    while (1) {
      printf ("Pid: %i Gid: %i\n",getpid(),getpgid(0));
      op.sem_num = 0;
      op.sem_op = -1;
      op.sem_flg = 0;
      semop(semid,&op,1);
      sleep (1);
      op.sem_num = 0;
      op.sem_op = 1;
      op.sem_flg = 0;
      semop(semid,&op,1);
    }
  }

  while (1) {
    sleep (1);
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
}


void set_signal_handlers_child (void)
{
  struct sigaction action_dfl;

  action_dfl.sa_sigaction = (void *)SIG_DFL;
  sigemptyset (&action_dfl.sa_mask);
  action_dfl.sa_flags = SA_SIGINFO;
  sigaction (SIGINT, &action_dfl, NULL);
}

void clean_out (int signal, siginfo_t *info, void *data)
{
  int rc;
  pid_t child_pid;

  kill(0,SIGINT);
  child_pid = wait (&rc);
  printf ("Child arrived ! %i\n",child_pid); 
  log_master ("Cleaning...");

  shmctl (shmid,IPC_RMID,NULL);
  shmctl (semid,IPC_RMID,NULL);

  exit (1);
}

int get_socket (void)
{
  int sfd;
  struct sockaddr_in addr;

  sfd = socket (PF_INET,SOCK_STREAM,0);
  if (sfd == -1) {
    perror ("socket");
    exit (1);
  }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(MASTERPORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind (sfd,(struct sockaddr *)&addr,sizeof (addr)) == -1) {
    perror ("bind");
    exit (1);
  }
  listen (sfd,MAXLISTEN);

  return sfd;
}
