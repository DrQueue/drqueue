/* $Id: request.c,v 1.1 2001/05/07 15:35:04 jorge Exp $ */
/* For the differences between data in big endian and little endian */
/* I transmit everything in network byte order */

#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>

#include "request.h"
#include "database.h"
#include "logger.h"
#include "communications.h"

void handle_request_master (int sfd,struct database *wdb,int icomp)
{
  struct request request;

  recv_request (sfd,&request,MASTER);
  switch (request.type) {
  case R_R_REGISTER:
    log_master ("Info: Registration request");
    handle_r_r_register (sfd,wdb,icomp);
    break;
  default:
    log_master ("Warning: Unknown request");
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
  int index;

  if (icomp != -1) {
    log_master ("Info: Already registered computer requesting registration");
    answer.type = R_A_REGISTER;
    answer.data_s = RERR_ALREADY;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }

  if ((index = computer_index_free(wdb)) == -1) {
    /* No space left on database */
    log_master ("Warning: No space left for computer");
    answer.type = R_A_REGISTER;
    answer.data_s = RERR_NOSPACE;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }

  /* No errors, we (master) can receive the hwinfo from the remote */
  /* computer to be registered */
  answer.type = R_A_REGISTER;
  answer.data_s = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);

  recv_computer_hwinfo (sfd, &wdb->computer[index].hwinfo, MASTER);

  report_hwinfo(&wdb->computer[index].hwinfo);
}



