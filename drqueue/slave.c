/* $Id: slave.c,v 1.5 2001/05/09 10:53:08 jorge Exp $ */

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

struct computer *comp;		/* Need to be on global scope because of signal handling */
int shmid;
int semid;

int main (int argc,char *argv[])
{

  log_slave_computer ("Starting...");
  set_signal_handlers ();

  shmid = get_shared_memory_slave ();
  comp = attach_shared_memory_slave (shmid);
  semid = get_semaphores_slave ();

  init_tasks (comp->status.task);

  init_computer_status (&comp->status);
  get_hwinfo (&comp->hwinfo);
  report_hwinfo (&comp->hwinfo);

  register_slave (comp);

  while (1) {
    get_computer_status (&comp->status);
    update_computer_status (comp);
    report_computer_status (&comp->status);

    if (computer_available(comp)) {
/*        launch_task (); */
    }

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

  if (semctl (semid,0,IPC_RMID,NULL) == -1) {
    perror ("semid");
  }
  if (shmctl (shmid,IPC_RMID,NULL) == -1) {
    perror ("shmid");
  }

  exit (0);
}


int get_shared_memory_slave (void)
{
  key_t key;
  int shmid;

  if ((key = ftok ("slave",'Z')) == -1) {
    perror ("ftok");
    exit (1);
  }
  
  if ((shmid = shmget (key,sizeof (struct computer), IPC_EXCL|IPC_CREAT|0600)) == -1) {
    perror ("shmget");
    exit (1);
  }

  return shmid;
}

int get_semaphores_slave (void)
{
  key_t key;
  int semid;

  if ((key = ftok ("slave",'Z')) == -1) {
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



