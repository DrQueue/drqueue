/* $Id: request.c,v 1.2 2001/05/09 10:53:08 jorge Exp $ */
/* For the differences between data in big endian and little endian */
/* I transmit everything in network byte order */

#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#include "request.h"
#include "database.h"
#include "logger.h"
#include "communications.h"
#include "semaphores.h"

void handle_request_master (int sfd,struct database *wdb,int icomp)
{
  struct request request;

  recv_request (sfd,&request,MASTER);
  switch (request.type) {
  case R_R_REGISTER:
    log_master ("Info: Registration request");
    handle_r_r_register (sfd,wdb,icomp);
    break;
  case R_R_UCSTATUS:
    log_master ("Info: Update computer status request");
    handle_r_r_ucstatus (sfd,wdb,icomp);
    break;
  default:
    log_master ("Warning: Unknown request");
  }
  if (icomp != -1) {
    semaphore_lock (wdb->semid);
    /* set the time of the last connection */
    time(&wdb->computer[icomp].lastconn);
    semaphore_release (wdb->semid);
  }
}

void handle_request_slave (int sfd,struct database *wdb,int icomp)
{
  struct request request;

  recv_request (sfd,&request,SLAVE);
}

void handle_r_r_register (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of packages */
  struct request answer;
  struct computer_hwinfo hwinfo;
  int index;

  if (icomp != -1) {
    log_master ("Info: Already registered computer requesting registration");
    answer.type = R_A_REGISTER;
    answer.data_s = RERR_ALREADY;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }

  semaphore_lock(wdb->semid);	/* I put the lock here so no race condition can appear... */
  if ((index = computer_index_free(wdb)) == -1) {
    /* No space left on database */
    log_master ("Warning: No space left for computer");
    answer.type = R_A_REGISTER;
    answer.data_s = RERR_NOSPACE;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }
  wdb->computer[index].used = 1;
  time(&wdb->computer[index].lastconn);
  semaphore_release(wdb->semid);

  /* No errors, we (master) can receive the hwinfo from the remote */
  /* computer to be registered */
  answer.type = R_A_REGISTER;
  answer.data_s = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);
  
  recv_computer_hwinfo (sfd, &hwinfo, MASTER);
  semaphore_lock(wdb->semid);
  memcpy (&wdb->computer[index].hwinfo, &hwinfo, sizeof(hwinfo));
  semaphore_release(wdb->semid);

  report_hwinfo(&wdb->computer[index].hwinfo);
}

void update_computer_status (struct computer *computer)
{
  /* The slave calls this function to update the information about */
  /* his own status on the master */
  struct request req;
  int sfd;

  sfd = connect_to_master ();

  req.type = R_R_UCSTATUS;
  req.slave = 1;

  send_request (sfd,&req,SLAVE);
  recv_request (sfd,&req,SLAVE);
  if (req.type == R_A_UCSTATUS) {
    switch (req.data_s) {
    case RERR_NOERROR:
      send_computer_status (sfd,&computer->status,SLAVE);
      break;
    case RERR_NOREGIS:
      log_slave_computer ("Error: Computer not registered");
      break;
    default:
      log_slave_computer ("Error: Error code on listed on answer to R_R_UCSTATUS");
      kill (0,SIGINT);
    }
  } else {
    log_slave_computer ("Error: Not apropiate answer to request R_R_UCSTATUS");
    kill (0,SIGINT);
  }
  close (sfd);
}

void register_slave (struct computer *computer)
{
  /* The slave calls this function to register himself on the master */
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
      log_slave_computer ("Error: Already registered");
      kill (0,SIGINT);
      break;
    case RERR_NOSPACE:
      log_slave_computer ("Error: No space on database");
      kill (0,SIGINT);
      break;
    default:
      log_slave_computer ("Error: Error code on listed on answer to R_R_REGISTER");
      kill (0,SIGINT);
    }
  } else {
    log_slave_computer ("Error: Not apropiate answer to request R_R_REGISTER");
    kill (0,SIGINT);
  }

  close (sfd);
}

void handle_r_r_ucstatus (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of packages */
  struct request answer;
  struct computer_status status;

  if (icomp == -1) {
    log_master ("Info: Not registered computer requesting update of computer status");
    answer.type = R_A_UCSTATUS;
    answer.data_s = RERR_NOREGIS;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }

  /* No errors, we (master) can receive the status from the remote */
  /* computer already registered */
  answer.type = R_A_UCSTATUS;
  answer.data_s = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);
  
  recv_computer_status (sfd, &status, MASTER);
  semaphore_lock(wdb->semid);
  memcpy (&wdb->computer[icomp].status, &status, sizeof(status));
  semaphore_release(wdb->semid);

  report_computer_status (&wdb->computer[icomp].status);
}




