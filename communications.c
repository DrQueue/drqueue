//
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
// $Id$
//

#include <sys/types.h>
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

#ifdef LIBWRAP
#include <tcpd.h>
#include <syslog.h>
int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;
#endif /* LIBWRAP */

#ifdef COMM_REPORT
long int bsent;   /* Bytes sent */
long int brecv;   /* Bytes received */
#endif

int get_socket (short port) {
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

int accept_socket (int sfd,struct database *wdb,struct sockaddr_in *addr) {
  int fd;
  socklen_t len = sizeof (struct sockaddr_in);

  if ((fd = accept (sfd,(struct sockaddr *)addr,&len)) == -1) {
    log_master (L_ERROR,"Accepting connection.");
    exit (1);
  }

#ifdef LIBWRAP
  /* Check whether logins are denied from this host. */
  {
    struct request_info req;

    request_init(&req, RQ_DAEMON, "drqueue-master", RQ_FILE, fd, 0);
    fromhost(&req);

    if (!hosts_access(&req)) {
      close(fd);
      log_master (L_WARNING,"Connection refused by tcp wrapper.");
      return (-1);
    }
  }
#endif /* LIBWRAP */

  return fd;
}

int accept_socket_slave (int sfd) {
  int fd;
  struct sockaddr_in addr;
  socklen_t len = sizeof (struct sockaddr_in);

  if ((fd = accept (sfd,(struct sockaddr *)&addr,&len)) == -1) {
    perror ("accept");
    log_slave_computer (L_ERROR,"Accepting connection.");
    exit (1);
  }

#ifdef LIBWRAP
  /* Check whether logins are denied from this host. */
  {
    printf ("cleb\n");
    struct request_info req;

    request_init(&req, RQ_DAEMON, "drqueue-slave", RQ_FILE, fd, 0);
    fromhost(&req);

    if (!hosts_access(&req)) {
      close(fd);
      log_slave_computer (L_WARNING,"Connection refused by tcp wrapper.");
      return (-1);
    }
  }
#endif /* LIBWRAP */


  return fd;
}


int connect_to_master (void) {
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
  addr.sin_port = htons(MASTERPORT);  /* Whatever */
  hostinfo = gethostbyname (master);
  if (hostinfo == NULL) {
    drerrno = DRE_NOTRESOLVE;
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

int connect_to_slave (char *slave) {
  /* Connects to the slave and returns the socket fd */
  int sfd;
  struct sockaddr_in addr;
  struct hostent *hostinfo;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(SLAVEPORT); /* Whatever */
  hostinfo = gethostbyname (slave);
  if (hostinfo == NULL) {
    drerrno = DRE_NOTRESOLVE;
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

int recv_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo) {
  void *buf;

  buf = hwinfo;
  if (!dr_read (sfd,(char *)buf,sizeof(struct computer_hwinfo))) {
    return 0;
  }

  /* Now we should have the computer hardware info with the values in */
  /* network byte order, so we put them in host byte order */
  hwinfo->id = ntohl (hwinfo->id);
  hwinfo->procspeed = ntohl (hwinfo->procspeed);
  hwinfo->ncpus = ntohs (hwinfo->ncpus);
  hwinfo->speedindex = ntohl (hwinfo->speedindex);
  hwinfo->memory = ntohl (hwinfo->memory);

  drerrno = DRE_NOERROR;
  return 1;
}

int send_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo) {
  struct computer_hwinfo bswapped;
  void *buf = &bswapped;

  /* We make a copy coz we need to modify the values */
  memcpy (buf,hwinfo,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.id = htonl (bswapped.id);
  bswapped.procspeed = htonl (bswapped.procspeed);
  bswapped.ncpus = htons (bswapped.ncpus);
  bswapped.speedindex = htonl (bswapped.speedindex);
  bswapped.memory = htonl (bswapped.memory);

  if (!dr_write (sfd, (char *) buf, sizeof (struct computer_hwinfo))) {
    return 0;
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int recv_request (int sfd, struct request *request) {
  /* Returns 0 on failure */
  void *buf = request;

  if (!dr_read(sfd,(char *)buf,sizeof(struct request))) {
    return 0;
  }

  /* Byte order ! */
  request->data = ntohl (request->data);

  return 1;
}

int send_request (int sfd, struct request *request,uint8_t who) {
  void *buf = request;

  request->data = htonl (request->data);
  request->who = who;

  if (!dr_write (sfd,(char*)buf,sizeof(struct request))) {
    return 0;
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int send_computer_status (int sfd, struct computer_status *status) {
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
  // FIXME: Remove this ?
  bswapped.ntasks = 0;
  for (i=0;i<MAXTASKS;i++) {
    if (bswapped.task[i].used)
      bswapped.ntasks++;
  }

  bswapped.ntasks = htons (bswapped.ntasks);

  if (!dr_write(sfd,(char*)buf,sizeof(uint16_t) * 4)) { // Send the first 4 uint16_t that are loadavg and ntasks
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

int recv_computer_status (int sfd, struct computer_status *status) {
  void *buf;
  int i;
  struct task task;

  computer_status_init (status);

  buf = status;
  if (!dr_read(sfd,(char*)buf,sizeof(uint16_t) * 4)) {
    return 0;
  }
  status->loadavg[0] = ntohs(status->loadavg[0]);
  status->loadavg[1] = ntohs(status->loadavg[1]);
  status->loadavg[2] = ntohs(status->loadavg[2]);
  status->ntasks = ntohs(status->ntasks);

  if (status->ntasks > MAXTASKS) {
    fprintf (stderr,"WARNING: ntasks > MAXTASKS (%i)\n",status->ntasks);
    status->ntasks = 0;
  }

  for (i=0;i<status->ntasks;i++) {
    if (!recv_task(sfd,&task)) {
      fprintf (stderr,"ERROR: receiving task\n");
      return 0;
    }
    memcpy(&status->task[task.itask],&task,sizeof(task));
  }

  return 1;
}

int recv_envvar (int sfd, struct envvar *var) {
  if (!dr_read (sfd,(char *)var,sizeof (struct envvar))) {
    return 0;
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int send_envvar (int sfd, struct envvar *var) {
  if (!dr_write (sfd,(char *)var,sizeof (struct envvar))) {
    return 0;
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int send_envvars (int sfd, struct envvars *envvars) {
  if (!write_16b (sfd,(char *)&envvars->nvariables)) {
    return 0;
  }

  envvars_attach (envvars);
  int i;
  for (i = 0; i < envvars->nvariables; i++) {
    if (!send_envvar (sfd,&envvars->variables[i])) {
      return 0;
    }
  }
  envvars_detach (envvars);

  drerrno = DRE_NOERROR;
  return 1;
}

int recv_envvars (int sfd, struct envvars *envvars) {
  uint16_t nvariables;

  if (!envvars_empty (envvars)) {
    return 0;
  }

  if (!read_16b (sfd,&nvariables)) {
    return 0;
  }

  int i;
  struct envvar var;
  for (i = 0; i < nvariables; i++) {
    if (!recv_envvar (sfd, &var)) {
      return 0;
    }
    envvars_variable_add (envvars,var.name,var.value);
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int recv_job (int sfd, struct job *job) {
  if (!dr_read(sfd,(char*)job,sizeof (struct job))) {
    return 0;
  }

  envvars_init (&job->envvars);

  // Environment variables
  if (!recv_envvars (sfd,&job->envvars)) {
    return 0;
  }

  /* Now we should have the job info with the values in */
  /* network byte order, so we put them in host byte order */
  job->id = ntohl (job->id);
  job->nprocs = ntohs (job->nprocs);
  job->status = ntohs (job->status);

  // Frame info
  job->fishmid = -1;

  // Blocked hosts
  job->blocked_host = NULL;
  job->bhshmid = -1;
  job->nblocked = 0;

  /* Koj Stuff */
  job->koj = ntohs (job->koj);
  switch (job->koj) {
  case KOJ_GENERAL:
    break;
  case KOJ_MAYA:
    break;
  case KOJ_MENTALRAY:
  case KOJ_BLENDER:
  case KOJ_LIGHTWAVE:
  case KOJ_TERRAGEN:
  case KOJ_NUKE:
  case KOJ_AFTEREFFECTS:
  case KOJ_SHAKE:
    break;
  case KOJ_BMRT:
    job->koji.bmrt.xmin = ntohl (job->koji.bmrt.xmin);
    job->koji.bmrt.xmax = ntohl (job->koji.bmrt.xmax);
    job->koji.bmrt.ymin = ntohl (job->koji.bmrt.ymin);
    job->koji.bmrt.ymax = ntohl (job->koji.bmrt.ymax);
    job->koji.bmrt.xsamples = ntohl (job->koji.bmrt.xsamples);
    job->koji.bmrt.ysamples = ntohl (job->koji.bmrt.ysamples);
    job->koji.bmrt.radiosity_samples = ntohl (job->koji.bmrt.radiosity_samples);
    job->koji.bmrt.raysamples = ntohl (job->koji.bmrt.raysamples);
    break;
  case KOJ_3DELIGHT:
  case KOJ_PIXIE:
  case KOJ_XSI:
    break;
  case KOJ_TURTLE:
    job->koji.turtle.resx = ntohl (job->koji.turtle.resx);
    job->koji.turtle.resy = ntohl (job->koji.turtle.resy);
    break;
  }

  job->frame_info = NULL;
  job->frame_start = ntohl (job->frame_start);
  job->frame_end = ntohl (job->frame_end);
  job->frame_step = ntohl (job->frame_step);
  job->frame_step = (job->frame_step == 0) ? 1 : job->frame_step; /* No 0 on step !! */
  job->block_size = ntohl (job->block_size);
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
  job->limits.memory = ntohl (job->limits.memory);

  drerrno = DRE_NOERROR;
  return 1;
}

int send_job (int sfd, struct job *job) {
  /* This function _sets_ frame_info = NULL before sending */
  struct job bswapped;
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
    break;
  case KOJ_MAYA:
    break;
  case KOJ_MENTALRAY:
  case KOJ_BLENDER:
  case KOJ_LIGHTWAVE:
  case KOJ_TERRAGEN:
  case KOJ_NUKE:
  case KOJ_AFTEREFFECTS:
  case KOJ_SHAKE:
    break;
  case KOJ_BMRT:
    bswapped.koji.bmrt.xmin = htonl (bswapped.koji.bmrt.xmin);
    bswapped.koji.bmrt.xmax = htonl (bswapped.koji.bmrt.xmax);
    bswapped.koji.bmrt.ymin = htonl (bswapped.koji.bmrt.ymin);
    bswapped.koji.bmrt.ymax = htonl (bswapped.koji.bmrt.ymax);
    bswapped.koji.bmrt.xsamples = htonl (bswapped.koji.bmrt.xsamples);
    bswapped.koji.bmrt.ysamples = htonl (bswapped.koji.bmrt.ysamples);
    bswapped.koji.bmrt.radiosity_samples = htonl (bswapped.koji.bmrt.radiosity_samples);
    bswapped.koji.bmrt.raysamples = htonl (bswapped.koji.bmrt.raysamples);
    break;
  case KOJ_3DELIGHT:
  case KOJ_PIXIE:
  case KOJ_XSI:
    break;
  case KOJ_TURTLE:
    bswapped.koji.turtle.resx = htonl (bswapped.koji.turtle.resx);
    bswapped.koji.turtle.resy = htonl (bswapped.koji.turtle.resy);
    break;
  }
  bswapped.koj = htons (bswapped.koj);

  bswapped.frame_info = NULL;
  bswapped.frame_start = htonl (bswapped.frame_start);
  bswapped.frame_end = htonl (bswapped.frame_end);
  bswapped.frame_step = htonl (bswapped.frame_step);
  bswapped.block_size = htonl (bswapped.block_size);
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
  bswapped.limits.memory = htonl (bswapped.limits.memory);

  if (!dr_write (sfd,(char*)buf,sizeof(bswapped))) {
    fprintf (stderr,"ERROR: Failed to write job to sfd\n");
    return 0;
  }

  // Environment variables
  if (!send_envvars (sfd,&bswapped.envvars)) {
    fprintf (stderr,"ERROR: Failed to send Environment\n");
    return 0;
  }

  return 1;
}

int recv_task (int sfd, struct task *task) {
  void *buf;

  buf = task;   /* So when copying to buf we're really copying into job */
  if (!dr_read(sfd,(char*)buf,sizeof(struct task))) {
    return 0;
  }

  /* Now we should have the task info with the values in */
  /* network byte order, so we put them in host byte order */
  task->ijob = ntohl (task->ijob);
  task->icomp = ntohl (task->icomp);
  task->itask = ntohs (task->itask);

  task->frame = ntohl (task->frame);
  task->frame_start = ntohl (task->frame_start);
  task->frame_end = ntohl (task->frame_end);
  task->frame_step = ntohl (task->frame_step);
  task->block_size = ntohl (task->block_size);
  task->pid = ntohl (task->pid);
  task->exitstatus = ntohl (task->exitstatus);

  return 1;
}

int send_task (int sfd, struct task *task) {
  struct task bswapped;
  void *buf = &bswapped;

  /* We make a copy coz we need to modify the values */
  memcpy (buf,task,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.ijob = htonl (bswapped.ijob);
  bswapped.icomp = htonl (bswapped.icomp);
  bswapped.itask = htons (bswapped.itask);
  bswapped.frame = htonl (bswapped.frame);
  bswapped.frame_start = htonl (bswapped.frame_start);
  bswapped.frame_end = htonl (bswapped.frame_end);
  bswapped.frame_step = htonl (bswapped.frame_step);
  bswapped.block_size = htonl (bswapped.block_size);
  bswapped.pid = htonl (bswapped.pid);
  bswapped.exitstatus = htonl (bswapped.exitstatus);

  if (!dr_write (sfd,(char*)buf,sizeof(struct task))) {
    return 0;
  }

  return 1;
}

int send_computer (int sfd, struct computer *computer, uint8_t attached) {
  if (!send_computer_status (sfd,&computer->status)) {
    printf ("error send_computer_status\n");
    return 0;
  }
  if (!send_computer_hwinfo (sfd,&computer->hwinfo)) {
    printf ("error send_computer_hwinfo\n");
    return 0;
  }
  if (!send_computer_limits (sfd,&computer->limits,attached)) {
    printf ("error send_computer_limits\n");
    return 0;
  }

  return 1;
}

int recv_computer (int sfd, struct computer *computer) {
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

int recv_frame_info (int sfd, struct frame_info *fi) {
  void *buf;

  buf = fi;
  if (!dr_read (sfd,(char*)buf,sizeof (struct frame_info))) {
    return 0;
  }

  fi->start_time = ntohl (fi->start_time);
  fi->end_time = ntohl (fi->end_time);
  fi->icomp = ntohl (fi->icomp);
  fi->itask = ntohs (fi->itask);
  fi->requeued = ntohs (fi->requeued);
  fi->flags = ntohs (fi->flags);

  return 1;
}

int send_frame_info (int sfd, struct frame_info *fi) {
  struct frame_info bswapped;
  void *buf = &bswapped;

  /* We make a copy coz we need to modify the values */
  memcpy (buf,fi,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.start_time = htonl (bswapped.start_time);
  bswapped.end_time = htonl (bswapped.end_time);
  bswapped.icomp = htonl (bswapped.icomp);
  bswapped.itask = htons (bswapped.itask);
  bswapped.requeued = htons (bswapped.requeued);
  bswapped.flags = htons (bswapped.flags);

  if (!dr_write (sfd,(char*)buf,sizeof (struct frame_info))) {
    return 0;
  }

  return 1;
}

int send_string (int sfd, char *str) {
  uint16_t len,lensw;

  len = strlen (str)+1;
  lensw = htons (len);
  if (!dr_write (sfd,(char*)&lensw,sizeof (lensw)))
    return 0;

  if (!dr_write (sfd,(char*)str,len))
    return 0;

  return 1;
}

int recv_string (int sfd, char **str) {
  uint16_t len;

  if (!dr_read (sfd,(char*)&len,sizeof(len)))
    return 0;

  len = ntohs (len);

  *str = (char *) malloc (len);
  if (!dr_read (sfd,*str,len))
    return 0;

  return 1;
}

int send_computer_pools (int sfd, struct computer_limits *cl, uint8_t attached) {
  int i;
  uint16_t npools;
  struct pool *pool;

  // fprintf (stderr,"Send npools: %u\n",cl->npools);
  npools = htons (cl->npools);
  if (!dr_write (sfd,(char*)&npools,sizeof(npools))) {
    return 0;
  }

  if (cl->npools) {
    if (attached) {
      pool = cl->pool;
    } else {
      if ((pool = (struct pool *) computer_pool_attach_shared_memory(cl->poolshmid)) == (void*)-1) {
        perror ("Attaching");
        fprintf (stderr,"ERROR attaching memory %d shmid\n", cl->poolshmid);
        return 0;
      }
    }

    for (i=0;i<cl->npools;i++) {
      if (!dr_write(sfd,(char*)&pool[i],sizeof(struct pool))) {
        return 0;
      }
    }

    if (!attached)
      computer_pool_detach_shared_memory (pool);
  }

  // fprintf (stderr,"communications.c\n");
  // computer_pool_list (cl);

  return 1;
}

int recv_computer_pools (int sfd, struct computer_limits *cl) {
  int i;
  uint16_t npools;
  struct pool pool;

  if (!dr_read (sfd,(char*)&npools,sizeof(npools))) {
    return 0;
  }
  npools = ntohs (npools);
  // fprintf (stderr,"Recv npools: %u\n",npools);

  computer_pool_free (cl);
  for (i=0;i<npools;i++) {
    if (!dr_read(sfd,(char*)&pool,sizeof(pool))) {
      return 0;
    }
    computer_pool_add (cl,pool.name);
  }

  // computer_pool_list (cl);

  return 1;
}

int recv_computer_limits (int sfd, struct computer_limits *cl) {
  void *buf;

  buf = cl;
  if (!dr_read (sfd,(char*)buf,sizeof(struct computer_limits))) {
    return 0;
  }

  /* Swapping bytes */
  cl->nmaxcpus = ntohs (cl->nmaxcpus);
  cl->maxfreeloadcpu = ntohs (cl->maxfreeloadcpu);

  /* Autoenable stuff */
  cl->autoenable.last = ntohl (cl->autoenable.last);

  // Pools
  cl->npools = 0;
  cl->poolshmid = -1;

  if (!recv_computer_pools (sfd,cl)) {
    return 0;
  }

  return 1;
}

int send_computer_limits (int sfd, struct computer_limits *cl, uint8_t attached) {
  struct computer_limits bswapped;
  void *buf = &bswapped;

  /* We make a copy coz we need to modify the values */
  memcpy (buf,cl,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.nmaxcpus = htons (bswapped.nmaxcpus);
  bswapped.maxfreeloadcpu = htons (bswapped.maxfreeloadcpu);
  /* Autoenable stuff */
  bswapped.autoenable.last = htonl (bswapped.autoenable.last);

  if (!dr_write(sfd,(char*)buf,sizeof(struct computer_limits))) {
    return 0;
  }

  // Pools
  if (!send_computer_pools(sfd,cl,attached)) {
    return 0;
  }

  return 1;
}

int write_32b (int sfd, void *data) {
  uint32_t bswapped;
  void *buf = &bswapped;

  bswapped = htonl (*(uint32_t *)data);
  if (!dr_write (sfd,(char*)buf,sizeof (uint32_t))) {
    return 0;
  }

  return 1;
}

int write_16b (int sfd, void *data) {
  uint16_t bswapped;
  void *buf = &bswapped;

  bswapped = htons (*(uint16_t *)data);
  if (!dr_write (sfd,(char*)buf,sizeof (uint16_t))) {
    return 0;
  }

  return 1;
}

int read_32b (int sfd, void *data) {
  void *buf;

  buf = data;
  if (!dr_read (sfd,(char*)buf,sizeof(uint32_t))) {
    return 0;
  }

  *(uint32_t *)data = ntohl (*((uint32_t *)data));

  return 1;
}

int read_16b (int sfd, void *data) {
  void *buf;

  buf = data;
  if (!dr_read (sfd,(char*)buf,sizeof(uint16_t))) {
    return 0;
  }

  *(uint16_t *)data = ntohs (*((uint16_t *)data));

  return 1;
}

int send_autoenable (int sfd, struct autoenable *ae) {
  struct autoenable bswapped;
  void *buf = &bswapped;

  /* We make a copy coz we need to modify the values */
  memcpy (buf,ae,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.last = htonl (bswapped.last);

  if (!dr_write(sfd,(char*)buf,sizeof(struct autoenable))) {
    return 0;
  }

  return 1;
}

int recv_autoenable (int sfd, struct autoenable *ae) {
  if (!dr_read(sfd,(char*)ae,sizeof(struct autoenable))) {
    return 0;
  }

  /* Now we should have the autoenable info with the values in */
  /* network byte order, so we put them in host byte order */
  ae->last = ntohl (ae->last);

  return 1;
}

int send_blocked_host (int sfd, struct blocked_host *bh) {
  if (!dr_write(sfd,(char*)bh,sizeof (struct blocked_host))) {
    return 0;
  }

  return 1;
}

int recv_blocked_host (int sfd, struct blocked_host *bh) {
  if (!dr_read (sfd,(char*)bh,sizeof (struct blocked_host))) {
    return 0;
  }

  return 1;
}

int dr_read (int fd, char *buf, uint32_t len) {
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

int dr_write (int fd, char *buf, uint32_t len) {
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
