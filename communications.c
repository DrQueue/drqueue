/* $Id: communications.c,v 1.36 2001/10/04 08:17:25 jorge Exp $ */

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
#include <string.h>

#include "communications.h"
#include "database.h"
#include "semaphores.h"
#include "logger.h"
#include "job.h"
#include "drerrno.h"
#include "task.h"

#ifdef COMM_REPORT
long int bsent;			/* Bytes sent */
long int brecv;			/* Bytes received */
#endif

int get_socket (short port)
{
  int sfd;
  struct sockaddr_in addr;
  int opt = 1;

  sfd = socket (PF_INET,SOCK_STREAM,0);
  if (sfd == -1) {
    perror ("socket");
    kill (0,SIGINT);
  } else {
    if (setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,(int *)&opt,sizeof(opt)) == -1) {
      perror ("setsockopt");
    }
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

int accept_socket (int sfd,struct database *wdb,int *index,struct sockaddr_in *addr)
{
  /* This function not just accepts the socket but also updates */
  /* the lastconn time of the client if this exists */
  int fd;
  int len = sizeof (struct sockaddr_in);

  if ((fd = accept (sfd,(struct sockaddr *)addr,&len)) != -1) {
    *index = computer_index_addr (wdb,addr->sin_addr);
  } else {
    log_master (L_ERROR,"Accepting connection.");
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
    log_slave_computer (L_ERROR,"Accepting connection.");
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
    drerrno = DRE_NOTRESOLV;
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

int connect_to_slave (char *slave)
{
  /* Connects to the slave and returns the socket fd */
  int sfd;
  struct sockaddr_in addr;
  struct hostent *hostinfo;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(SLAVEPORT); /* Whatever */
  hostinfo = gethostbyname (slave);
  if (hostinfo == NULL) {
    drerrno = DRE_NOTRESOLV;
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
    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      if (who == MASTER) {
	log_master (L_ERROR,"Receiving computer_hwinfo");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer (L_ERROR,"Receiving computer_hwinfo");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: recv_computer_hwinfo: who value not valid !\n");
	exit (1);
      }
    }
    bleft -= r;
    buf += r;
#ifdef COMM_REPORT
    brecv += r;
#endif
  }
#ifdef COMM_REPORT
  brecv += r;
#endif

  /* Now we should have the computer hardware info with the values in */
  /* network byte order, so we put them in host byte order */
  hwinfo->id = ntohl (hwinfo->id);
  hwinfo->procspeed = ntohl (hwinfo->procspeed);
  hwinfo->ncpus = ntohs (hwinfo->ncpus);
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
  bswapped.id = htonl (bswapped.id);
  bswapped.procspeed = htonl (bswapped.procspeed);
  bswapped.ncpus = htons (bswapped.ncpus);
  bswapped.speedindex = htonl (bswapped.speedindex);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      if (who == MASTER) {
	log_master (L_ERROR,"Sending computer hardware info");
	exit (1);
      } else if (who == SLAVE) {
	log_slave_computer (L_ERROR,"Sending computer hardware info");
	kill(0,SIGINT);
      } else {
	fprintf (stderr,"ERROR: send_computer_hwinfo: who value not valid !\n");
	exit (1);
      }
    }
    bleft -= w;
    buf += w;
#ifdef COMM_REPORT
    bsent += w;
#endif
  }
#ifdef COMM_REPORT
  bsent += w;
#endif
}

int recv_request (int sfd, struct request *request)
{
  /* Returns 0 on failure */
  int r;			/* bytes read */
  int bleft;			/* bytes left for reading */
  void *buf = request;

  bleft = sizeof (struct request);
  while ((r = read(sfd,buf,bleft)) < bleft) {
    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      drerrno = DRE_ERRORRECEIVING;
      return 0;
    }
    bleft -= r;
    buf += r;
#ifdef COMM_REPORT
    brecv += r;
#endif
  }
#ifdef COMM_REPORT
  brecv += r;
#endif

  /* Byte order ! */
  request->data = ntohl (request->data);
  
  return 1;
}

int send_request (int sfd, struct request *request,int who)
{
  int w;
  int bleft;
  void *buf = request;

  request->data = htonl (request->data);
  request->who = who;

  bleft = sizeof (struct request);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      drerrno = DRE_ERRORSENDING;
      return 0;
    }
#ifdef COMM_REPORT
    bsent += w;
#endif
  }
#ifdef COMM_REPORT
    bsent += w;
#endif

  drerrno = DRE_NOERROR;
  return 1;
}

int send_computer_status (int sfd, struct computer_status *status)
{
  struct computer_status bswapped;
  int w;
  int bleft;
  void *buf = &bswapped;
  uint16_t i;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,status,sizeof(bswapped));
  /* Prepare for sending */
  for (i=0;i<3;i++)
    bswapped.loadavg[i] = htons (bswapped.loadavg[i]);

  bswapped.ntasks = 0;
  for (i=0;i<MAXTASKS;i++) {
    if (bswapped.task[i].used)
      bswapped.ntasks++;
  }

  bswapped.ntasks = htons (bswapped.ntasks);

  bleft = sizeof (uint16_t) * 4; /* 3 loadvg + 1 ntasks */
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      drerrno = DRE_ERRORSENDING;
      return 0;
    }
#ifdef COMM_REPORT
    bsent += w;
#endif
  }
#ifdef COMM_REPORT
  bsent += w;
#endif

  /* NOTE : if task.ntasks is different from the real number of used==1 tasks then... there could be problems. */
  /* In case the ntasks < real.ntasks : the sending process will write on a closed socket. So it will fail and return 0 */
  /* If ntasks > real.ntasks : the receiving process will keep waiting for the rest of the tasks and then receive an */
  /*                           alarm signal. Or maybe it will notice that it's trying to read from a closed socket and so */
  /*                           will return error, again. */

  /* We just send the used tasks */
  for (i=0;i<MAXTASKS;i++) {
    if (status->task[i].used) {
      if (!send_task(sfd,&bswapped.task[i]))
	return 0;
    }
  }

  return 1;
}

int recv_computer_status (int sfd, struct computer_status *status)
{
  int r;
  int bleft;
  void *buf;
  int i;
  struct task task;

  buf = status;
  bleft = sizeof (uint16_t) * 4; /* 3 loadavg + 1 ntasks */
  while ((r = read (sfd,buf,bleft)) < bleft) {
    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      drerrno = DRE_ERRORRECEIVING;
      return 0;
    }
    bleft -= r;
    buf += r;
#ifdef COMM_REPORT
    brecv += r;
#endif
  }
#ifdef COMM_REPORT
  brecv += r;
#endif
  /* Now we should have the computer hardware info with the values in */
  /* network byte order, so we put them in host byte order */
  for (i=0;i<3;i++)
    status->loadavg[i] = ntohs (status->loadavg[i]);
  status->ntasks = ntohs (status->ntasks);

  for (i=0;i<status->ntasks;i++) {
    if (!recv_task(sfd,&task)) {
      return 0;
    }
    memcpy(&status->task[task.itask],&task,sizeof(task));
  }

  return 1;
}

void recv_job (int sfd, struct job *job,int who)
{
  int r;
  int bleft;
  void *buf;

  buf = job;			/* So when copying to buf we're really copying into job */
  bleft = sizeof (struct job);
  while ((r = read (sfd,buf,bleft)) < bleft) {
    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      switch (who) {
      case MASTER:
	log_master (L_ERROR,"Receiving job");
	exit (1);
      case SLAVE:
	log_slave_computer (L_ERROR,"Receiving job");
	kill(0,SIGINT);
      case CLIENT:
	fprintf (stderr,"ERROR: receiving request\n");
	exit (1);
      default:
	fprintf (stderr,"ERROR: recv_job: who value not valid !\n");
	exit (1);
      }
    }
    bleft -= r;
    buf += r;
#ifdef COMM_REPORT
    brecv += r;
#endif
  }
#ifdef COMM_REPORT
  brecv += r;
#endif
  /* Now we should have the computer hardware info with the values in */
  /* network byte order, so we put them in host byte order */
  job->id = ntohl (job->id);
  job->nprocs = ntohs (job->nprocs);
  job->status = ntohs (job->status);

  /* Koj Stuff */
  job->koj = ntohs (job->koj);
  switch (job->koj) {
  case KOJ_GENERAL:
  case KOJ_MAYA:
    break;
  }

  job->frame_info = NULL;
  job->frame_start = ntohl (job->frame_start);
  job->frame_end = ntohl (job->frame_end);
  job->frame_step = ntohl (job->frame_step);
  job->frame_step = (job->frame_step == 0) ? 1 : job->frame_step; /* No 0 on step !! */
  job->avg_frame_time = ntohl (job->avg_frame_time);
  job->est_finish_time = ntohl (job->est_finish_time);
  job->fleft = ntohl (job->fleft);
  job->fdone = ntohl (job->fdone);
  job->ffailed = ntohl (job->ffailed);

  job->priority = ntohl (job->priority);
}

void send_job (int sfd, struct job *job,int who)
{
  /* This function _sets_ frame_info = NULL before sending */
  struct job bswapped;
  int w;
  int bleft;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,job,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.id = htonl (bswapped.id);
  bswapped.nprocs = htons (bswapped.nprocs);
  bswapped.status = htons (bswapped.status);

  /* Koj Stuff */
  switch (bswapped.koj) {
  case KOJ_GENERAL:
  case KOJ_MAYA:
    break;
  }
  bswapped.koj = htons (bswapped.koj);

  bswapped.frame_info = NULL;
  bswapped.frame_start = htonl (bswapped.frame_start);
  bswapped.frame_end = htonl (bswapped.frame_end);
  bswapped.frame_step = htonl (bswapped.frame_step);
  bswapped.avg_frame_time = htonl (bswapped.avg_frame_time);
  bswapped.est_finish_time = htonl (bswapped.est_finish_time);
  bswapped.fleft = htonl (bswapped.fleft);
  bswapped.fdone = htonl (bswapped.fdone);
  bswapped.ffailed = htonl (bswapped.ffailed);

  bswapped.priority = htonl (bswapped.priority);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if there are no more bytes are written but they _SHOULD_ be */
      switch (who) {
      case MASTER:
	log_master (L_ERROR,"Sending job");
	exit (1);
      case SLAVE:
	log_slave_computer (L_ERROR,"Sending job");
	kill(0,SIGINT);
      case CLIENT:
	fprintf (stderr,"ERROR: sending job\n");
	exit (1);
      default:
	fprintf (stderr,"ERROR: send_job: who value not valid !\n");
	kill(0,SIGINT);
      }
    }
#ifdef COMM_REPORT
    bsent += w;
#endif
  }
#ifdef COMM_REPORT
  bsent += w;
#endif
}

int recv_task (int sfd, struct task *task)
{
  int r;
  int bleft;
  void *buf;

  buf = task;			/* So when copying to buf we're really copying into job */
  bleft = sizeof (struct task);
  while ((r = read (sfd,buf,bleft)) < bleft) {
    bleft -= r;
    buf += r;

    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if w is error or if no more bytes are read but they _SHOULD_ be */
      drerrno = DRE_ERRORRECEIVING;
      return 0;
    }
    bleft -= r;
    buf += r;
#ifdef COMM_REPORT
    brecv += r;
#endif
  }
#ifdef COMM_REPORT
  brecv += r;
#endif
  /* Now we should have the task info with the values in */
  /* network byte order, so we put them in host byte order */
  task->ijob = ntohl (task->ijob);
  task->frame = ntohl (task->frame);
  task->pid = ntohl (task->pid);
  task->exitstatus = ntohl (task->exitstatus);
  task->itask = ntohs (task->itask);

  return 1;
}

int send_task (int sfd, struct task *task)
{
  struct task bswapped;
  int w;
  int bleft;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,task,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.ijob = htonl (bswapped.ijob);
  bswapped.frame = htonl (bswapped.frame);
  bswapped.pid = htonl (bswapped.pid);
  bswapped.exitstatus = htonl (bswapped.exitstatus);
  bswapped.itask = htons (bswapped.itask);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if no more bytes are written but they _SHOULD_ be */
      drerrno = DRE_ERRORSENDING;
      return 0;
    }
#ifdef COMM_REPORT
    bsent += w;
#endif
  }
#ifdef COMM_REPORT
  bsent += w;
#endif

  return 1;
}

int send_computer (int sfd, struct computer *computer,int who)
{
  if (!send_computer_status (sfd,&computer->status)) {
    printf ("error send_computer_status\n");
    return 0;
  }
  send_computer_hwinfo (sfd,&computer->hwinfo,who);
  if (!send_computer_limits (sfd,&computer->limits)) {
    printf ("error send_computer_limits\n");
    return 0;
  }
  return 1;
}

int recv_computer (int sfd, struct computer *computer,int who)
{
  if (!recv_computer_status (sfd,&computer->status)) {
    printf ("error recv_computer_status\n");
    return 0;
  }
  recv_computer_hwinfo (sfd,&computer->hwinfo,who);
  if (!recv_computer_limits (sfd,&computer->limits)) {
    printf ("error recv_computer_limits\n");
    return 0;
  }
  return 1;
}

int recv_frame_info (int sfd, struct frame_info *fi)
{
  int r;
  int bleft;
  void *buf;

  buf = fi;
  bleft = sizeof (struct frame_info);
  while ((r = read (sfd,buf,bleft)) < bleft) {
    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if w is error or if no more bytes are read but they _SHOULD_ be */
      drerrno = DRE_ERRORRECEIVING;
      return 0;
    }
    bleft -= r;
    buf += r;
#ifdef COMM_REPORT
    brecv += r;
#endif
 }
#ifdef COMM_REPORT
  brecv += r;
#endif
  fi->start_time = ntohl (fi->start_time);
  fi->end_time = ntohl (fi->end_time);
  fi->icomp = ntohl (fi->icomp);
  fi->itask = ntohs (fi->itask);

  return 1;
}

int send_frame_info (int sfd, struct frame_info *fi)
{
  struct frame_info bswapped;
  int w;
  int bleft;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,fi,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.start_time = htonl (bswapped.start_time);
  bswapped.end_time = htonl (bswapped.end_time);
  bswapped.icomp = htonl (bswapped.icomp);
  bswapped.itask = htons (bswapped.itask);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if no more bytes are written but they _SHOULD_ be */
      drerrno = DRE_ERRORSENDING;
      return 0;
    }
#ifdef COMM_REPORT
    bsent += w;
#endif
  }
#ifdef COMM_REPORT
  bsent += w;
#endif

  return 1;
}

int recv_computer_limits (int sfd, struct computer_limits *cl)
{
  int r;
  int bleft;
  void *buf;

  buf = cl;
  bleft = sizeof (struct computer_limits);
  while ((r = read (sfd,buf,bleft)) < bleft) {
    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if w is error or if no more bytes are read but they _SHOULD_ be */
      drerrno = DRE_ERRORRECEIVING;
      return 0;
    }
    bleft -= r;
    buf += r;
#ifdef COMM_REPORT
    brecv += r;
#endif
  }
#ifdef COMM_REPORT
  brecv += r;
#endif
  cl->nmaxcpus = ntohs (cl->nmaxcpus);
  cl->maxfreeloadcpu = ntohs (cl->maxfreeloadcpu);

  return 1;
}

int send_computer_limits (int sfd, struct computer_limits *cl)
{
  struct computer_limits bswapped;
  int w;
  int bleft;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,cl,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.nmaxcpus = htons (bswapped.nmaxcpus);
  bswapped.maxfreeloadcpu = htons (bswapped.maxfreeloadcpu);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if no more bytes are written but they _SHOULD_ be */
      drerrno = DRE_ERRORSENDING;
      return 0;
    }
#ifdef COMM_REPORT
    bsent += w;
#endif
  }
#ifdef COMM_REPORT
  bsent += w;
#endif

  return 1;
}

