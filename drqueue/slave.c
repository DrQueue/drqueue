/* $Id: slave.c,v 1.3 2001/05/02 16:12:33 jorge Exp $ */

#include <unistd.h>
#include <signal.h>

#include "slave.h"
#include "computer.h"
#include "logger.h"

struct computer comp;		/* Need to be on global scope because of signal handling */

int main (int argc,char *argv[])
{
  get_hwinfo (&comp.hwinfo);
  report_hwinfo (&comp.hwinfo);

  set_signal_handlers ();

  get_computerstatus (&comp.status);
  
  strcpy (comp.status.task[0].jobname,"Test");
  log_slave_task (&comp.status.task[0],"Another test");

  log_slave_computer ("Starting...");

  while (1)
    sleep (10);

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

  ignore.sa_handler = SIG_IGN;
  sigemptyset (&ignore.sa_mask);
  ignore.sa_flags = 0;
  sigaction (SIGHUP, &ignore, NULL);
}

void clean_out (int signal, siginfo_t *info, void *data)
{
  log_slave_computer ("Cleaning...");

  exit (1);
}
