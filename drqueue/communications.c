/* $Id: communications.c,v 1.1 2001/05/07 15:35:04 jorge Exp $ */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>

#include "communications.h"
#include "database.h"
#include "semaphores.h"
#include "logger.h"

int get_socket (short port)
{
  int sfd;
  struct sockaddr_in addr;

  sfd = socket (PF_INET,SOCK_STREAM,0);
  if (sfd == -1) {
    perror ("socket");
    exit (1);
  }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind (sfd,(struct sockaddr *)&addr,sizeof (addr)) == -1) {
    perror ("bind");
    exit (1);
  }
  listen (sfd,MAXLISTEN);

  return sfd;
}

int accept_socket (int sfd,struct database *wdb,int *index)
{
  /* This function not just accepts the socket but also updates */
  /* the lastconn time of the client if this exists */
  int fd;
  struct sockaddr_in addr;
  int len;

  if ((fd = accept (sfd,(struct sockaddr *)&addr,&len)) != -1) {
    *index = computer_index_addr (wdb,addr.sin_addr);
  } else {
    log_master ("Child: Error in accept. Exiting...");
    exit (1);
  }

  return fd;
}

int connect_to_master (void)
{
  /* To be used by a slave ! */
  /* Connects to the master and returns the socket fd */
  int sfd;
  char *master;
  struct sockaddr_in addr;
  struct hostent *hostinfo;

  if ((master = getenv ("DRQUEUE_MASTER")) == NULL) {
    log_slave_computer ("Error: Environment variable DRQUEUE_MASTER not defined");
    kill(0,SIGINT);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(MASTERPORT);		/* Whatever */
  hostinfo = gethostbyname (master);
  if (hostinfo == NULL) {
    log_slave_computer ("Error: DRQUEUE_MASTER points to a non-valid address");
    kill(0,SIGINT);
  }
  addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;

  sfd = socket (PF_INET,SOCK_STREAM,0);
  if (sfd == -1) {
    log_slave_computer ("Error: Could not open socket");
    kill(0,SIGINT);
  }

  if (connect (sfd,(struct sockaddr *)&addr,sizeof (addr)) == -1) {
    log_slave_computer ("Error: Could not connect to the master");
    kill(0,SIGINT);
  }

  return sfd;
}

void recv_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo,int who)
{
  int r;
  int bleft;
  void *buf;

  buf = hwinfo;
  bleft = sizeof (struct computer_hwinfo);
  while ((r = read (sfd,buf,bleft)) < bleft) {
    bleft -= r;
    buf += r;

    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      if (who == MASTER) {
	log_master ("Error receiving computer_hwinfo");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error receiving computer_hwinfo");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: recv_computer_hwinfo: who value not valid !\n");
	exit (1);
      }
    }
  }
  /* Now we should have the computer hardware info with the values in */
  /* network byte order, so we put them in host byte order */
  hwinfo->procspeed = ntohl (hwinfo->procspeed);
  hwinfo->numproc = ntohs (hwinfo->numproc);
  hwinfo->speedindex = ntohl (hwinfo->speedindex);
}

void send_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo,int who)
{
  struct computer_hwinfo bswapped;
  int w;
  int bleft;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,hwinfo,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.procspeed = ntohl (bswapped.procspeed);
  bswapped.numproc = ntohs (bswapped.numproc);
  bswapped.speedindex = ntohl (bswapped.speedindex);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      if (who == MASTER) {
	log_master ("Error sending computer hardware info");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error sending computer hardware info");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: send_computer_hwinfo: who value not valid !\n");
	exit (1);
      }
    }
  }
}

void recv_request (int sfd, struct request *request,int who)
{
  int r;			/* bytes read */
  int bleft;			/* bytes left for reading */
  void *buf = request;

  bleft = sizeof (struct request);
  while ((r = read(sfd,buf,bleft)) < bleft) {
    bleft -= r;
    buf += r;

    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      if (who == MASTER) {
	log_master ("Error receiving request");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error receiving request");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: recv_request: who value not valid !\n");
	exit (1);
      }
    }
  }
  /* Byte order ! */
  request->data_s = ntohs (request->data_s);
}

void send_request (int sfd, struct request *request,int who)
{
  int w;
  int bleft;
  void *buf = request;

  request->data_s = htons (request->data_s);
  bleft = sizeof (struct request);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      if (who == MASTER) {
	log_master ("Error sending request");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error sending request");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: send_request: who value not valid !\n");
	exit (1);
      }
    }
  }
}


