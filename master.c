/* $Id: master.c,v 1.18 2001/08/29 08:20:57 jorge Exp $ */

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
#include "libdrqueue.h"

struct database *wdb;		/* whole database */
int sfd;			/* socket file descriptor */
int icomp;			/* index to accepted computer, local to every child */


int main (int argc, char *argv[])
{
  int csfd;			/* child sfd, the socket once accepted the connection */
  int shmid;			/* shared memory id */
  int force = 0;		/* force even if shmem already exists */
  int opt;

  fprintf (stderr,"Master at: %i\n",(int)getpid());
  log_master (L_INFO,"Starting...");
  set_signal_handlers ();

  while ((opt = getopt (argc,argv,"fl:oh")) != -1) {
    switch (opt) {
    case 'f':
      force = 1;
      break;
    case 'l':
      loglevel = atoi (optarg);
      printf ("Logging level set to: %i\n",loglevel);
      break;
    case 'o':
      logonscreen = 1;
      printf ("Logging on screen.\n");
      break;
    case '?':
    case 'h':
      usage();
      exit (1);
    }
  }

  shmid = get_shared_memory (force);
  wdb = attach_shared_memory (shmid);
  wdb->shmid = shmid;
  wdb->semid = get_semaphores (force);

  database_init(wdb);

  if (fork() == 0) {
    fprintf (stderr,"Consistency checks at: %i\n",(int)getpid());
    /* Create the consistency checks process */
/*      strcpy (argv[0],"DrQueue - Consistency checks"); */
    set_signal_handlers_child_cchecks ();
    master_consistency_checks (wdb);
    exit (0);
  }

  if ((sfd = get_socket(MASTERPORT)) == -1) {
    kill(0,SIGINT);
  }

  printf ("Waiting for connections...\n");
  while (1) {
    if ((csfd = accept_socket (sfd,wdb,&icomp)) != -1) {
      if (fork() == 0) {
	/* Create a connection handler */
	fflush(stderr);
	set_signal_handlers_child_conn_handler ();
	close (sfd);
	set_alarm ();
	handle_request_master (csfd,wdb,icomp);
	close (csfd);
	exit (0);
      } else {
	close (csfd);
	if (csfd > 4)
	  printf ("Growing !! csfd: %i\n",csfd);
      }
    }
  }

  exit (0);
}

int get_shared_memory (int force)
{
  key_t key;
  int shmid;
  int shmflg;

  if ((key = ftok ("./master",'Z')) == -1) {
    perror ("ftok");
    exit (1);
  }
  
  if (force) {
    shmflg = IPC_CREAT|0600;
  } else {
    shmflg = IPC_EXCL|IPC_CREAT|0600;
  }

  if ((shmid = shmget (key,sizeof(struct database), shmflg)) == -1) {
    perror ("shmget");
    if (!force)
      fprintf (stderr,"Try with option -f (if you are sure that no other master is running)\n");
    exit (1);
  }

  return shmid;
}

int get_semaphores (int force)
{
  key_t key;
  int semid;
  struct sembuf op;
  int semflg;

  if ((key = ftok ("master",'Z')) == -1) {
    perror ("ftok");
    kill (0,SIGINT);
  }

  if (force) {
    semflg = IPC_CREAT|0600;
  } else {
    semflg = IPC_EXCL|IPC_CREAT|0600;
  }

  if ((semid = semget (key,1,semflg)) == -1) {
    perror ("semget");
    if (!force)
      fprintf (stderr,"Try with option -f (if you are sure that no other master is running)\n");
    kill (0,SIGINT);
  }
  if (semctl (semid,0,SETVAL,1) == -1) {
    perror ("semctl SETVAL -> 1");
    kill (0,SIGINT);
  }
  if (semctl (semid,0,GETVAL) == 0) {
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    if (semop(semid,&op,1) == -1) {
      perror ("semaphore_release");
      kill(0,SIGINT);
    }
  }

  fprintf (stderr,"semval: %i semid: %i\n",semctl (semid,0,GETVAL),semid);

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
    printf ("Child arrived ! %i\n",(int)child_pid); 
  }
  log_master (L_INFO,"Cleaning...");

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

  fprintf (stderr,"PID,Signal that caused death: %i,%i\n",(int)getpid(),signal);

  exit (1);
}

void set_alarm (void)
{
  alarm (MAXTIMECONNECTION);
}

void sigalarm_handler (int signal, siginfo_t *info, void *data)
{
  if (icomp != -1)
    log_master_computer (&wdb->computer[icomp],L_WARNING,"Connection time exceeded");
  else
    log_master (L_WARNING,"Connection time exceeded");
  exit (1);
}

void sigpipe_handler (int signal, siginfo_t *info, void *data)
{
  char *msg = "Broken connection while reading or writing (SIGPIPE)";

  if (icomp != -1)
    log_master_computer (&wdb->computer[icomp],L_WARNING,msg);
  else
    log_master (L_WARNING,msg);
  exit (1);
}

void sigsegv_handler (int signal, siginfo_t *info, void *data)
{
  char *msg = "Segmentation fault... too bad";

  if (icomp != -1)
    log_master_computer (&wdb->computer[icomp],L_ERROR,msg);
  else
    log_master (L_ERROR,msg);
  exit (1);
}

void master_consistency_checks (struct database *wdb)
{
  uint32_t i;

  while (1) {
    check_lastconn_times (wdb);

    for (i=0;i<MAXJOBS;i++) {
      if (wdb->job[i].used) {
	job_update_info (wdb,i);
      }
    }

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
	log_master_computer (&wdb->computer[i],L_INFO,"Maximum time without connecting exceeded. Deleting");
	semaphore_lock(wdb->semid);
	wdb->computer[i].used = 0;
	semaphore_release(wdb->semid);
      }
    }
  }
}

void usage (void)
{
  fprintf (stderr,"Valid options:\n"
	   "\t-f to force continuing if shared memory already exists\n"
	   "\t-l <loglevel> From 0 to 3 (0=errors,1=warnings,2=info,3=debug).\n\t\tDefaults to 1. Each level logs all the previous levels\n"
	   "\t-o log on screen instead of on files\n"
	   "\t-h prints this help\n");
}

