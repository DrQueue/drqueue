/* $Id: communications.c,v 1.50 2002/06/26 17:22:29 jorge Exp $ */

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

int accept_socket (int sfd,struct database *wdb,struct sockaddr_in *addr)
{
  /* This function not just accepts the socket but also updates */
  /* the lastconn time of the client if this exists */
  int fd;
  int len = sizeof (struct sockaddr_in);

  if ((fd = accept (sfd,(struct sockaddr *)addr,&len)) == -1) {
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
  /* or -1 in case of error */
  int sfd;
  char *master;
  struct sockaddr_in addr;
  struct hostent *hostinfo;

  if ((master = getenv ("DRQUEUE_MASTER")) == NULL) {
    drerrno = DRE_NOENVMASTER;
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

  drerrno = DRE_NOERROR;
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

int recv_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo)
{
  void *buf;

  buf = hwinfo;
  if (!dr_read (sfd,buf,sizeof(struct computer_hwinfo))) {
    return 0;
  }

  /* Now we should have the computer hardware info with the values in */
  /* network byte order, so we put them in host byte order */
  hwinfo->id = ntohl (hwinfo->id);
  hwinfo->procspeed = ntohl (hwinfo->procspeed);
  hwinfo->ncpus = ntohs (hwinfo->ncpus);
  hwinfo->speedindex = ntohl (hwinfo->speedindex);

  drerrno = DRE_NOERROR;
  return 1;
}

int send_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo)
{
  struct computer_hwinfo bswapped;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,hwinfo,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.id = htonl (bswapped.id);
  bswapped.procspeed = htonl (bswapped.procspeed);
  bswapped.ncpus = htons (bswapped.ncpus);
  bswapped.speedindex = htonl (bswapped.speedindex);

  if (!dr_write (sfd, buf, sizeof (struct computer_hwinfo))) {
    return 0;
  }
  
  drerrno = DRE_NOERROR;
  return 1;
}

int recv_request (int sfd, struct request *request)
{
  /* Returns 0 on failure */
  void *buf = request;

  if (!dr_read(sfd,buf,sizeof(struct request))) {
    return 0;
  }

  /* Byte order ! */
  request->data = ntohl (request->data);
  
  return 1;
}

int send_request (int sfd, struct request *request,int who)
{
  void *buf = request;

  request->data = htonl (request->data);
  request->who = who;

  if (!dr_write (sfd,buf,sizeof(struct request))) {
    return 0;
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int send_computer_status (int sfd, struct computer_status *status)
{
  struct computer_status bswapped;
  void *buf = &bswapped;
  uint16_t i;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,status,sizeof(struct computer_status));

  /* Prepare for sending */
  bswapped.loadavg[0] = htons(bswapped.loadavg[0]);
  bswapped.loadavg[1] = htons(bswapped.loadavg[1]);
  bswapped.loadavg[2] = htons(bswapped.loadavg[2]);

  /* Count the tasks. (Shouldn't be necessary) */
  bswapped.ntasks = 0;
  for (i=0;i<MAXTASKS;i++) {
    if (bswapped.task[i].used)
      bswapped.ntasks++;
  }

  bswapped.ntasks = htons (bswapped.ntasks);

  if (!dr_write(sfd,buf,sizeof(uint16_t) * 4)) {
    return 0;
  }
  
  /* We just send the used tasks */
  for (i=0;i<MAXTASKS;i++) {
    if (bswapped.task[i].used) {
      if (!send_task(sfd,&bswapped.task[i]))
	return 0;
    }
  }

  return 1;
}

int recv_computer_status (int sfd, struct computer_status *status)
{
  void *buf;
  int i;
  struct task task;

  computer_status_init (status);

  buf = status;
  if (!dr_read(sfd,buf,sizeof(uint16_t) * 4)) {
    return 0;
  }
  status->loadavg[0] = ntohs(status->loadavg[0]);
  status->loadavg[1] = ntohs(status->loadavg[1]);
  status->loadavg[2] = ntohs(status->loadavg[2]);
  status->ntasks = ntohs(status->ntasks);

  if (status->ntasks > MAXTASKS) {
    printf ("WARNING: ntasks > MAXTASKS (%i)\n",status->ntasks);
    status->ntasks = 0;
  }

  for (i=0;i<status->ntasks;i++) {
    if (!recv_task(sfd,&task)) {
      return 0;
    }
    memcpy(&status->task[task.itask],&task,sizeof(task));
  }

  return 1;
}

int recv_job (int sfd, struct job *job)
{
  int r;
  int bleft;
  void *buf;

  buf = job;			/* So when copying to buf we're really copying into job */
  bleft = sizeof (struct job);
  while ((r = read (sfd,buf,bleft)) < bleft) {
    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      drerrno = DRE_ERRORREADING;
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

  job->flags = ntohl (job->flags);

  /* Limits */
  job->limits.nmaxcpus = ntohs (job->limits.nmaxcpus);
  job->limits.nmaxcpuscomputer = ntohs (job->limits.nmaxcpuscomputer);
  job->limits.os_flags = ntohs (job->limits.os_flags);

  drerrno = DRE_NOERROR;
  return 1;
}

int send_job (int sfd, struct job *job)
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

  bswapped.flags = htonl (bswapped.flags);

  /* Limits */
  bswapped.limits.nmaxcpus = htons (bswapped.limits.nmaxcpus);
  bswapped.limits.nmaxcpuscomputer = htons (bswapped.limits.nmaxcpuscomputer);
  bswapped.limits.os_flags = htons (bswapped.limits.os_flags);

  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if no more bytes are written but they _SHOULD_ be */
      drerrno = DRE_ERRORWRITING;
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
      drerrno = DRE_ERRORREADING;
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
  task->itask = ntohs (task->itask);

  task->frame = ntohl (task->frame);
  task->frame_start = ntohl (task->frame_start);
  task->frame_end = ntohl (task->frame_end);
  task->frame_step = ntohl (task->frame_step);
  task->pid = ntohl (task->pid);
  task->exitstatus = ntohl (task->exitstatus);

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
  bswapped.itask = htons (bswapped.itask);
  bswapped.frame = htonl (bswapped.frame);
  bswapped.frame_start = htonl (bswapped.frame_start);
  bswapped.frame_end = htonl (bswapped.frame_end);
  bswapped.frame_step = htonl (bswapped.frame_step);
  bswapped.pid = htonl (bswapped.pid);
  bswapped.exitstatus = htonl (bswapped.exitstatus);
 
  bleft = sizeof (bswapped);
  while ((w = write(sfd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if no more bytes are written but they _SHOULD_ be */
      drerrno = DRE_ERRORWRITING;
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

int send_computer (int sfd, struct computer *computer)
{
  if (!send_computer_status (sfd,&computer->status)) {
    printf ("error send_computer_status\n");
    return 0;
  }
  if (!send_computer_hwinfo (sfd,&computer->hwinfo)) {
    printf ("error send_computer_hwinfo\n");
    return 0;
  }
  if (!send_computer_limits (sfd,&computer->limits)) {
    printf ("error send_computer_limits\n");
    return 0;
  }
  return 1;
}

int recv_computer (int sfd, struct computer *computer)
{
  if (!recv_computer_status (sfd,&computer->status)) {
    printf ("error recv_computer_status\n");
    return 0;
  }
  if (!recv_computer_hwinfo (sfd,&computer->hwinfo)) {
    printf ("error recv_computer_hwinfo\n");
    return 0;
  }
  if (!recv_computer_limits (sfd,&computer->limits)) {
    printf ("error recv_computer_limits\n");
    return 0;
  }
  return 1;
}

int recv_frame_info (int sfd, struct frame_info *fi)
{
  void *buf;

  buf = fi;
  if (!dr_read (sfd,buf,sizeof (struct frame_info))) {
    return 0;
  }

  fi->start_time = ntohl (fi->start_time);
  fi->end_time = ntohl (fi->end_time);
  fi->icomp = ntohl (fi->icomp);
  fi->itask = ntohs (fi->itask);

  return 1;
}

int send_frame_info (int sfd, struct frame_info *fi)
{
  struct frame_info bswapped;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,fi,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.start_time = htonl (bswapped.start_time);
  bswapped.end_time = htonl (bswapped.end_time);
  bswapped.icomp = htonl (bswapped.icomp);
  bswapped.itask = htons (bswapped.itask);

  if (!dr_write (sfd,buf,sizeof (struct frame_info))) {
    return 0;
  }

  return 1;
}

int recv_computer_limits (int sfd, struct computer_limits *cl)
{
  void *buf;

  buf = cl;
  if (!dr_read (sfd,buf,sizeof(struct computer_limits))) {
    return 0;
  }

  /* Swapping bytes */
  cl->nmaxcpus = ntohs (cl->nmaxcpus);
  cl->maxfreeloadcpu = ntohs (cl->maxfreeloadcpu);

  /* Autoenable stuff */
  cl->autoenable.last = ntohl (cl->autoenable.last);

  return 1;
}

int send_computer_limits (int sfd, struct computer_limits *cl)
{
  struct computer_limits bswapped;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,cl,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.nmaxcpus = htons (bswapped.nmaxcpus);
  bswapped.maxfreeloadcpu = htons (bswapped.maxfreeloadcpu);
  /* Autoenable stuff */
  bswapped.autoenable.last = htonl (bswapped.autoenable.last);

  if (!dr_write(sfd,buf,sizeof(struct computer_limits))) {
    return 0;
  }

  return 1;
}

int write_32b (int sfd, void *data)
{
  uint32_t bswapped;
  void *buf = &bswapped;

  bswapped = htonl (*(uint32_t *)data);
  if (!dr_write (sfd,buf,sizeof (uint32_t))) {
    return 0;
  }

  return 1;
}

int write_16b (int sfd, void *data)
{
  uint16_t bswapped;
  void *buf = &bswapped;

  bswapped = htons (*(uint16_t *)data);
  if (!dr_write (sfd,buf,sizeof (uint16_t))) {
    return 0;
  }

  return 1;
}

int read_32b (int sfd, void *data)
{
  void *buf;

  buf = data;
  if (!dr_read (sfd,buf,sizeof(uint32_t))) {
    return 0;
  }

  *(uint32_t *)data = ntohl (*((uint32_t *)data));

  return 1;
}

int read_16b (int sfd, void *data)
{
  void *buf;

  buf = data;
  if (!dr_read (sfd,buf,sizeof(uint16_t))) {
    return 0;
  }

  *(uint16_t *)data = ntohs (*((uint16_t *)data));

  return 1;
}

int send_autoenable (int sfd, struct autoenable *ae)
{
  struct autoenable bswapped;
  void *buf = &bswapped;
  
  /* We make a copy coz we need to modify the values */
  memcpy (buf,ae,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.last = htonl (bswapped.last);

  if (!dr_write(sfd,buf,sizeof(struct autoenable))) {
    return 0;
  }

  return 1;
}

int recv_autoenable (int sfd, struct autoenable *ae)
{
  if (!dr_read(sfd,ae,sizeof(struct autoenable))) {
    return 0;
  }

  /* Now we should have the autoenable info with the values in */
  /* network byte order, so we put them in host byte order */
  ae->last = ntohl (ae->last);

  return 1;
}

int dr_read (int fd, void *buf, uint32_t len)
{
  int r;
  int bleft;

  bleft = len;
  while ((r = read (fd,buf,bleft)) < bleft) {
    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      drerrno = DRE_ERRORREADING;
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

  return len;
}

int dr_write (int fd, void *buf, uint32_t len)
{
  int w;
  int bleft;

  bleft = len;
  while ((w = write(fd,buf,bleft)) < bleft) {
    bleft -= w;
    buf += w;
    if ((w == -1) || ((w == 0) && (bleft > 0))) {
      /* if w is error or if no more bytes are written but they _SHOULD_ be */
      drerrno = DRE_ERRORWRITING;
      return 0;
    }
#ifdef COMM_REPORT
    bsent += w;
#endif
  }
#ifdef COMM_REPORT
  bsent += w;
#endif

  return len;
}
