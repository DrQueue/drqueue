/* $Id: communications.c,v 1.3 2001/05/28 14:21:31 jorge Exp $ */

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
#include "job.h"
#include "drerrno.h"

int get_socket (short port)
{
  int sfd;
  struct sockaddr_in addr;

  sfd = socket (PF_INET,SOCK_STREAM,0);
  if (sfd == -1) {
    perror ("socket");
    exit (1);
  } else {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sfd,(struct sockaddr *)&addr,sizeof (addr)) == -1) {
      perror ("bind");
      sfd = -1;
    } else {
      listen (sfd,MAXLISTEN);
    }
  }

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
    log_master ("Error: accepting connection.");
    exit (1);
  }

  return fd;
}

int accept_socket_slave (int sfd)
{
  int fd;
  struct sockaddr_in addr;
  int len;

  if ((fd = accept (sfd,(struct sockaddr *)&addr,&len)) == -1) {
    log_slave_computer ("Error: accepting connection.");
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
    drerrno = DRE_NODRMAENV;
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(MASTERPORT);		/* Whatever */
  hostinfo = gethostbyname (master);
  if (hostinfo == NULL) {
    drerrno = DRE_DRMANONVA;
    return -1;
  }
  addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;

  sfd = socket (PF_INET,SOCK_STREAM,0);
  if (sfd == -1) {
    drerrno = DRE_NOSOCKET;
    return -1;
  }

  if (connect (sfd,(struct sockaddr *)&addr,sizeof (addr)) == -1) {
    drerrno = DRE_NOCONNECT;
    return -1;
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
	log_master ("Error: receiving computer_hwinfo");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error: receiving computer_hwinfo");
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
  bswapped.procspeed = htonl (bswapped.procspeed);
  bswapped.numproc = htons (bswapped.numproc);
  bswapped.speedindex = htonl (bswapped.speedindex);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      if (who == MASTER) {
	log_master ("Error: sending computer hardware info");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error: sending computer hardware info");
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
      switch (who) {
      case MASTER:
	log_master ("Error: receiving request");
	exit (1);
	break;
      case SLAVE:
	log_slave_computer ("Error: receiving request");
	kill(0,SIGINT);
	break;
      case SLAVE_CHANDLER:
	log_slave_computer ("Error: connection handler: receiving request");
	exit (1);
      case CLIENT:
	fprintf (stderr,"ERROR: receiving request\n");
	exit (1);
      default:
	fprintf (stderr,"ERROR: recv_request: who value not valid !\n");
	kill(0,SIGINT);
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

  if (who == SLAVE)
    request->slave = 1;
  else
    request->slave = 0;

  bleft = sizeof (struct request);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      switch (who) {
      case MASTER:
	log_master ("Error sending request");
	exit (1);
      case SLAVE:
	log_slave_computer ("Error sending request");
	kill(0,SIGINT);
      case CLIENT:
	fprintf (stderr,"ERROR: sending request");
	exit (1);
      default:
	fprintf (stderr,"ERROR: send_request: who value not valid !\n");
	exit (1);
      }
    }
  }
}

void send_computer_status (int sfd, struct computer_status *status,int who)
{
  struct computer_status bswapped;
  int w;
  int bleft;
  void *buf = &bswapped;
  int i;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,status,sizeof(bswapped));
  /* Prepare for sending */
  for (i=0;i<3;i++)
    bswapped.loadavg[i] = htons (bswapped.loadavg[i]);
  bswapped.numtasks = htons (bswapped.numtasks);
  for (i=0;i<MAXTASKS;i++) {
    if (bswapped.task[i].used) {
      bswapped.task[i].jobindex = htons (bswapped.task[i].jobindex);
      bswapped.task[i].frame = htonl (bswapped.task[i].frame);
      bswapped.task[i].pid = htonl (bswapped.task[i].pid);
    }
  }

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      if (who == MASTER) {
	log_master ("Error sending computer status");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error sending computer status");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: send_computer_status: who value not valid !\n");
	exit (1);
      }
    }
  }
}

void recv_computer_status (int sfd, struct computer_status *status,int who)
{
  int r;
  int bleft;
  void *buf;
  int i;

  buf = status;
  bleft = sizeof (struct computer_status);
  while ((r = read (sfd,buf,bleft)) < bleft) {
    bleft -= r;
    buf += r;

    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      if (who == MASTER) {
	log_master ("Error receiving computer status");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error receiving computer status");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: recv_computer_status: who value not valid !\n");
	exit (1);
      }
    }
  }
  /* Now we should have the computer hardware info with the values in */
  /* network byte order, so we put them in host byte order */
  for (i=0;i<3;i++)
    status->loadavg[i] = ntohs (status->loadavg[i]);
  status->numtasks = ntohs (status->numtasks);
  for (i=0;i<MAXTASKS;i++) {
    if (status->task[i].used) {
      status->task[i].jobindex = ntohs (status->task[i].jobindex);
      status->task[i].frame = ntohl (status->task[i].frame);
      status->task[i].pid = ntohl (status->task[i].pid);
    }
  }
}

void recv_job (int sfd, struct job *job,int who)
{
  int r;
  int bleft;
  void *buf;

  buf = job;			/* So when copying to buf we're really copying into job */
  bleft = sizeof (struct job);
  while ((r = read (sfd,buf,bleft)) < bleft) {
    bleft -= r;
    buf += r;

    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      if (who == MASTER) {
	log_master ("Error: receiving job");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error: receiving job");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: recv_job: who value not valid !\n");
	exit (1);
      }
    }
  }
  /* Now we should have the computer hardware info with the values in */
  /* network byte order, so we put them in host byte order */
  job->status = ntohs (job->status);
  job->frame_start = ntohl (job->frame_start);
  job->frame_end = ntohl (job->frame_end);
}

void send_job (int sfd, struct job *job,int who)
{
  struct job bswapped;
  int w;
  int bleft;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,job,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.status = htons (bswapped.status);
  bswapped.frame_start = htonl (bswapped.frame_start);
  bswapped.frame_end = htonl (bswapped.frame_end);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      switch (who) {
      case MASTER:
	log_master ("Error: sending job");
	exit (1);
      case SLAVE:
	log_slave_computer ("Error: sending job");
	kill(0,SIGINT);
      case CLIENT:
	fprintf (stderr,"ERROR: sending job\n");
	exit (1);
      default:
	fprintf (stderr,"ERROR: send_job: who value not valid !\n");
	exit (1);
      }
    }
  }
}

void recv_task (int sfd, struct job *job,int who)
{
  int r;
  int bleft;
  void *buf;

  buf = job;			/* So when copying to buf we're really copying into job */
  bleft = sizeof (struct job);
  while ((r = read (sfd,buf,bleft)) < bleft) {
    bleft -= r;
    buf += r;

    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      if (who == MASTER) {
	log_master ("Error: receiving job");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer ("Error: receiving job");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: recv_job: who value not valid !\n");
	exit (1);
      }
    }
  }
  /* Now we should have the computer hardware info with the values in */
  /* network byte order, so we put them in host byte order */
  job->status = ntohs (job->status);
  job->frame_start = ntohl (job->frame_start);
  job->frame_end = ntohl (job->frame_end);
}

void send_job (int sfd, struct job *job,int who)
{
  struct job bswapped;
  int w;
  int bleft;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,job,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.status = htons (bswapped.status);
  bswapped.frame_start = htonl (bswapped.frame_start);
  bswapped.frame_end = htonl (bswapped.frame_end);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      switch (who) {
      case MASTER:
	log_master ("Error: sending job");
	exit (1);
      case SLAVE:
	log_slave_computer ("Error: sending job");
	kill(0,SIGINT);
      case CLIENT:
	fprintf (stderr,"ERROR: sending job\n");
	exit (1);
      default:
	fprintf (stderr,"ERROR: send_job: who value not valid !\n");
	exit (1);
      }
    }
  }
}



