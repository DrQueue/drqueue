/* $Id: slave.c,v 1.4 2001/05/07 15:35:04 jorge Exp $ */

#include <unistd.h>
#include <signal.h>

#include "slave.h"
#include "computer.h"
#include "logger.h"
#include "request.h"
#include "communications.h"

struct computer comp;		/* Need to be on global scope because of signal handling */

int main (int argc,char *argv[])
{

  set_signal_handlers ();

  log_slave_computer ("Starting...");
  get_hwinfo (&comp.hwinfo);
  report_hwinfo (&comp.hwinfo);

  register_slave (&comp);

  get_computerstatus (&comp.status);

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

void register_slave (struct computer *computer)
{
  struct request req;
  int sfd;

  sfd = connect_to_master ();
  
  req.type = R_R_REGISTER;
  req.slave = 1;

  send_request (sfd,&req,SLAVE);
  recv_request (sfd,&req,SLAVE);
  if (req.type == R_A_REGISTER) {
    switch (req.data_s) {
    case RERR_NOERROR:
      send_computer_hwinfo (sfd,&computer->hwinfo,SLAVE);
      break;
    case RERR_ALREADY:
      log_slave_computer ("Already registered");
      kill (0,SIGINT);
      break;
    case RERR_NOSPACE:
      log_slave_computer ("No space on database");
      kill (0,SIGINT);
      break;
    default:
    }
  }

  close (sfd);
}



