//
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
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
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "pointer.h"
#include "communications.h"
#include "database.h"
#include "semaphores.h"
#include "logger.h"
#include "job.h"
#include "drerrno.h"
#include "task.h"
#include "computer_pool.h"
#include "computer.h"
#include "common.h"

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

// ONGOING:
// * data size transmission
// * log messages moved to auto
// * perror messagges moved to log_auto
// * anydata_init_received
// * anydata_init_sending

int get_socket (uint16_t port) {
  int sfd;
  struct sockaddr_in addr;
  int opt = 1;

  sfd = socket (PF_INET,SOCK_STREAM,0);
  if (sfd == -1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"get_socket(): call to socket() could not create stream socket. Msg: %s",strerror(drerrno_system));
    return sfd;
    //kill (0,SIGINT);
  } else {
    if (setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,(int *)&opt,sizeof(opt)) == -1) {
      drerrno_system = errno;
      log_auto (L_ERROR,"get_socket(): call to setsockopt() failed. Msg: %s",strerror(drerrno_system));
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (sfd,(struct sockaddr *)&addr,sizeof (addr)) == -1) {
      drerrno_system = errno;
      log_auto (L_ERROR,"get_socket(): call to bind() failed. Msg: %s",strerror(drerrno_system));
      sfd = -1;
    } else {
      listen (sfd,MAXLISTEN);
    }
  }

  return sfd;
}

// FIXME: should be named accept_socket_master
int
accept_socket (int sfd,struct database *wdb,struct sockaddr_in *addr) {
  int fd;
  socklen_t len = sizeof (struct sockaddr_in);

  if ((fd = accept (sfd,(struct sockaddr *)addr,&len)) == -1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"accept_socket(): error accepting connection. Msg: %s",strerror(drerrno_system));
    return fd;
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

int
accept_socket_slave (int sfd) {
  int fd;
  struct sockaddr_in addr;
  socklen_t len = sizeof (struct sockaddr_in);

  if ((fd = accept (sfd,(struct sockaddr *)&addr,&len)) == -1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"accept_socket_slave(): error accepting connection. (%s)",strerror(drerrno_system));
    drerrno = DRE_COMMPROBLEM;
    return fd;
  }

#ifdef LIBWRAP
  /* Check whether logins are denied from this host. */
  {
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

  drerrno = DRE_NOERROR;

  if ((master = getenv ("DRQUEUE_MASTER")) == NULL) {
    drerrno_system = errno;
    drerrno = DRE_NOENVMASTER;
    return -1;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(MASTERPORT);  /* Whatever */
  hostinfo = gethostbyname (master);
  /* check if ip address of host could be resolved */
  if ( (hostinfo == NULL) || (hostinfo->h_addr == NULL) ) {
    drerrno_system = errno;
    drerrno = DRE_NOTRESOLVE;
    return -1;
  }
  addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;

  sfd = socket (PF_INET,SOCK_STREAM,0);
  if (sfd == -1) {
    drerrno_system = errno;
    drerrno = DRE_NOSOCKET;
    return -1;
  }

  if (connect (sfd,(struct sockaddr *)&addr,sizeof (addr)) == -1) {
    drerrno_system = errno;
    drerrno = DRE_NOCONNECT;
    return -1;
  }

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
    drerrno_system = errno;
    drerrno = DRE_NOTRESOLVE;
    return -1;
  }
  addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;

  sfd = socket (PF_INET,SOCK_STREAM,0);
  if (sfd == -1) {
    drerrno_system = errno;
    drerrno = DRE_NOSOCKET;
    return -1;
  }

  if (connect (sfd,(struct sockaddr *)&addr,sizeof (addr)) == -1) {
    drerrno_system = errno;
    drerrno = DRE_NOCONNECT;
    return -1;
  }

  return sfd;
}

int
check_recv_datasize (int sfd, uint32_t datasize) {
  uint32_t remotesize = 0;
  uint32_t localsize;

  if (!dr_read(sfd,(char*)&remotesize,sizeof(uint32_t))) {
    log_auto (L_ERROR,"check_recv_datasize(): communications problem. Could not read from socket or file. (%s)",strerror(errno));
    return 0;
  }
  remotesize = ntohl(remotesize);
  localsize = htonl(datasize);
  if (!dr_write(sfd,(char*)&localsize,sizeof(uint32_t))) {
    log_auto (L_ERROR,"check_recv_datasize(): communications problem. Could not write to socket or file. (%s)",strerror(errno));
    return 0;
  }

  if (datasize != remotesize) {
    log_auto (L_ERROR,"check_recv_datasize(): remote and local sizes for data are different.");
    log_auto (L_ERROR,"check_recv_datasize(): if you are using different versions of the software please, update older ones.");
    log_auto (L_ERROR,"check_recv_datasize(): if not this is probably a BUG. Check if it's has been reported.");
    return 0;
  }

  return 1;
}

int
check_send_datasize (int sfd, uint32_t datasize) {
  drerrno = DRE_NOERROR;
  uint32_t remotesize = 0;
  uint32_t localsize;
  
  log_auto(L_DEBUG3,"check_send_datasize():> Entering...");

  localsize = htonl(datasize);
  if (!dr_write(sfd,(char*)&localsize,sizeof(uint32_t))) {
    log_auto (L_ERROR,"check_recv_datasize(): communications problem. Could not write to socket or file. (%s)",strerror(errno));
    return 0;
  }
  if (!dr_read(sfd,(char*)&remotesize,sizeof(uint32_t))) {
    log_auto (L_ERROR,"check_recv_datasize(): communications problem. Could not read from socket or file. (%s)",strerror(errno));
    return 0;
  }
  remotesize = ntohl(remotesize);

  if (datasize != remotesize) {
    log_auto (L_ERROR,"check_recv_datasize(): remote and local sizes for data are different.");
    log_auto (L_ERROR,"check_recv_datasize(): if you are using different versions of the software please, update older ones.");
    log_auto (L_ERROR,"check_recv_datasize(): if not this is probably a BUG. Check if it's has been reported.");
    return 0;
  }

  log_auto(L_DEBUG3,"check_send_datasize():< Returning...");
  return 1;
}

int recv_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo) {
  void *buf;
  uint32_t datasize;
  
  datasize = sizeof(struct computer_hwinfo);
  if (!check_recv_datasize(sfd, datasize)) {
    log_auto (L_ERROR,"recv_computer_hwinfo(): different data sizes for 'struct computer_hwinfo'.");
    return 0;
  }

  buf = hwinfo;
  if (!dr_read (sfd,(char*)buf,datasize)) {
    log_auto (L_ERROR,"recv_computer_hwinfo(): could not read computer hardware information. (%s)",strerror(errno));
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
  uint32_t datasize;

  datasize = sizeof(bswapped);
  if (!check_send_datasize(sfd,datasize)) {
    log_auto (L_ERROR,"send_computer_hwinfo(): different data sizes for struct computer_hwinfo.");
    return 0;
  }

  /* We make a copy coz we need to modify the values */
  memcpy (buf,hwinfo,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.id = htonl (bswapped.id);
  bswapped.procspeed = htonl (bswapped.procspeed);
  bswapped.ncpus = htons (bswapped.ncpus);
  bswapped.speedindex = htonl (bswapped.speedindex);
  bswapped.memory = htonl (bswapped.memory);

  if (!dr_write (sfd, (char *) buf, datasize)) {
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
  uint16_t i;
  uint16_t ntasks;
  uint16_t nrunning;

  /* Count the tasks. (Shouldn't be necessary) */
  // FIXME: Remove this ?
  ntasks = 0;
  nrunning = 0;
  for (i=0;i<MAXTASKS;i++) {
    if (status->task[i].used) {
      ntasks++;
      if (status->task[i].status != TASKSTATUS_FINISHED) {
        nrunning++;
      }
    }
  }
  status->ntasks = ntasks;
  status->nrunning = nrunning;

  if (!send_computer_loadavg(sfd,status)) {
    return 0;
  }

  if (!send_computer_ntasks(sfd,status)) {
    return 0;
  }

  /* We just send the used tasks */
  for (i=0;i<MAXTASKS;i++) {
    if (status->task[i].used) {
      if (!send_task(sfd,&status->task[i]))
        return 0;
    }
  }

  return 1;
}

int send_computer_loadavg (int sfd, struct computer_status *status) {
  uint16_t nbo_loadavg[3]; // network byte ordered
  
  nbo_loadavg[0] = htons(status->loadavg[0]);
  nbo_loadavg[1] = htons(status->loadavg[1]);
  nbo_loadavg[2] = htons(status->loadavg[2]);
  if (!dr_write(sfd,(char*)nbo_loadavg,sizeof(uint16_t)*3)) {
    return 0;
  }
  return 1;
}

int recv_computer_loadavg (int sfd, struct computer_status *status) {
  uint16_t nbo_loadavg[3];

  if (!dr_read(sfd,(char*)nbo_loadavg,sizeof(uint16_t)*3)) {
    return 0;
  }
  status->loadavg[0] = ntohs(nbo_loadavg[0]);
  status->loadavg[1] = ntohs(nbo_loadavg[1]);
  status->loadavg[2] = ntohs(nbo_loadavg[2]);
  return 1;
}

int recv_computer_ntasks (int sfd, struct computer_status *status) {
  uint16_t ntasks;
  uint16_t nrunning;
  if (!dr_read(sfd,(char*)&ntasks,sizeof(uint16_t))) {
    return 0;
  }
  if (!dr_read(sfd,(char*)&nrunning,sizeof(uint16_t))) {
    return 0;
  }
  status->ntasks = ntohs(ntasks);
  status->nrunning = ntohs(nrunning);

  return 1;
}

int send_computer_ntasks (int sfd, struct computer_status *status) {
  uint16_t nbo_ntasks;   // network byte ordered
  uint16_t nbo_nrunning; // network byte ordered
  
  nbo_ntasks = htons(status->ntasks);
  nbo_nrunning = htons(status->nrunning);
  if (!dr_write(sfd,(char*)&nbo_ntasks,sizeof(uint16_t))) {
    return 0;
  }
  if (!dr_write(sfd,(char*)&nbo_nrunning,sizeof(uint16_t))) {
    return 0;
  }
  return 1;
}


int recv_computer_status (int sfd, struct computer_status *status) {
  void *buf;
  int i;
  struct task task;

  computer_status_init (status);

  buf = status;
  if (!recv_computer_loadavg (sfd,status)) {
    drerrno = DRE_ERRORREADING;
    fprintf (stderr,"ERROR: recv_computer_loadavg: %s\n",drerrno_str());
  }
  if (!recv_computer_ntasks (sfd,status)) {
    drerrno = DRE_ERRORREADING;
    fprintf (stderr,"ERROR: recv_computer_ntasks: %s\n",drerrno_str());
  }
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

int
recv_envvar (int sfd, struct envvar *var, int do_checksize) {
  // receives a single environment variable structure
  uint32_t datasize;

  datasize = sizeof (struct envvar);
  if (do_checksize && !check_recv_datasize(sfd, datasize)) {
    // TODO: log it
    return 0;
  }

  if (!dr_read (sfd,(char *)var, datasize)) {
    drerrno = DRE_ERRORREADING;
    return 0;
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int
send_envvar (int sfd, struct envvar *var, int do_checksize) {
  // sends a single environment variable structure
  uint32_t datasize;

  datasize = sizeof (struct envvar);
  if (do_checksize && !check_send_datasize(sfd,datasize)) {
    // TODO: log it
    return 0;
  }

  if (!dr_write (sfd,(char *)var, datasize)) {
    // TODO: log it
    return 0;
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int
send_envvars (int sfd, struct envvars *envvars, int do_checksize) {
  //fprintf (stderr,"DEBUG: send_envvars() we have %i environment variables available for the request\n",envvars->nvariables);
  uint16_t nvariables;

  nvariables = htons (envvars->nvariables);
  if (!dr_write (sfd,(char *)&nvariables,sizeof(nvariables))) {
    return 0;
  }

  //fprintf (stderr,"DEBUG: send_envvars() and just informed the client about that\n");

  if (envvars->nvariables) {
    envvars_attach (envvars);
    int i;
    for (i = 0; i < envvars->nvariables; i++) {
      if (!send_envvar (sfd,&(envvars->variables.ptr[i]),do_checksize)) {
        return 0;
      }
      //fprintf (stderr,"DEBUG: send_envvars() just sent (%s,%s)\n",envvars->variables[i].name,envvars->variables[i].value);
    }
    envvars_detach (envvars);
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int
recv_envvars (int sfd, struct envvars *envvars, int do_checksize) {
  // This function leaves envvars DETACHED
  uint16_t nvariables;

  if (!envvars) {
    return 0;
  }

  if (!envvars_free (envvars)) {
    fprintf (stderr,"ERROR: recv_envvars() error calling envvars_free(). (%s)\n",drerrno_str());
    return 0;
  }

  if (!dr_read(sfd,(char*)&nvariables,sizeof(nvariables))) {
    fprintf (stderr,"ERROR: recv_envvars() while receiving nvariables. (%s)\n",drerrno_str());
    return 0;
  }
  nvariables = ntohs (nvariables);
  
  int i;
  struct envvar var;
  if (nvariables) {
    //fprintf (stderr,"DEBUG: recv_envvars() we'll receive %i variables\n",nvariables);

    for (i = 0; i < nvariables; i++) {
      //fprintf (stderr,"DEBUG: recv_envvars() receive variable number %i\n",i);

      if (!recv_envvar (sfd, &var, do_checksize)) {
        fprintf (stderr,"ERROR: recv_envvars() while receiving a single variable. (%s)\n",drerrno_str());
        return 0;
      }

      envvars_variable_add (envvars,var.name,var.value);
      memset (&var,0,sizeof(struct envvar));
    }
  }

  drerrno = DRE_NOERROR;
  return 1;
}

int
recv_job (int sfd, struct job *job) {
  uint32_t datasize;

  datasize = sizeof (struct job);
  if (!check_recv_datasize(sfd,datasize)) {
    log_auto(L_ERROR,"recv_job(): error on datasize check. Local datasize: %u. (%s)",datasize,strerror(drerrno_system));
    return 0;
  }

  // We should empty all locally reserved structures
  job_delete (job);

  if (!dr_read(sfd,(char*)job,datasize)) {
    return 0;
  }
  
  job_fix_received_invalid (job);

  // Environment variables
  if (!recv_envvars (sfd,&job->envvars,1)) {
    fprintf (stderr,"ERROR: recv_job() there was an error while receiving envvars (%s)\n",drerrno_str());
    return 0;
  }

  job_bswap_from_network (job,job);

  drerrno = DRE_NOERROR;
  return 1;
}


int
send_job (int sfd, struct job *job) {
  /* This function _sets_ frame_info = NULL before sending */
  struct job bswapped;
  void *buf = &bswapped;
  uint32_t datasize;

  datasize = sizeof(struct job);
  if (!check_send_datasize(sfd,datasize)) {
    // TODO: log it
    fprintf (stderr,"datasize: %i",datasize);
    return 0;
  }

  /* Prepare for sending */
  job_bswap_to_network (job,&bswapped);

  if (!dr_write (sfd,(char*)buf,datasize)) {
    fprintf (stderr,"ERROR: Failed to write job to sfd\n");
    return 0;
  }

  // Environment variables
  if (!send_envvars (sfd,&job->envvars,1)) {
    fprintf (stderr,"ERROR: Failed to send Environment\n");
    return 0;
  }

  return 1;
}

int
recv_task (int sfd, struct task *task) {
  void *buf;
  uint32_t datasize;

  buf = task;   /* So when copying to buf we're really copying into
		   job */
  datasize = sizeof (*task);
  if (!check_recv_datasize(sfd,datasize)) {
    // TODO
    return 0;
  }

  task_init (task);
  if (!dr_read(sfd,(char*)buf,datasize)) {
    return 0;
  }

  /* Now we should have the task info with the values in */
  /* network byte order, so we put them in host byte order */
  task->ijob  = ntohl (task->ijob);
  task->icomp = ntohl (task->icomp);
  task->itask = ntohs (task->itask);

  task->frame       = ntohl (task->frame);
  task->frame_start = ntohl (task->frame_start);
  task->frame_end   = ntohl (task->frame_end);
  task->frame_step  = ntohl (task->frame_step);
  task->block_size  = ntohl (task->block_size);
  task->pid         = ntohl (task->pid);
  task->exitstatus  = ntohl (task->exitstatus);
  task->start_loading_time = dr_ntoh64 (task->start_loading_time);
  return 1;
}

int
send_task (int sfd, struct task *task) {
  struct task bswapped;
  void *buf = &bswapped;
  uint32_t datasize = sizeof (*task);

  if (!check_send_datasize(sfd,datasize)) {
    // TODO
    return 0;
  }

  /* We make a copy coz we need to modify the values */
  memcpy (buf,task,sizeof(bswapped));
  /* Prepare for sending */
  bswapped.ijob  = htonl (bswapped.ijob);
  bswapped.icomp = htonl (bswapped.icomp);
  bswapped.itask = htons (bswapped.itask);

  bswapped.frame       = htonl (bswapped.frame);
  bswapped.frame_start = htonl (bswapped.frame_start);
  bswapped.frame_end   = htonl (bswapped.frame_end);
  bswapped.frame_step  = htonl (bswapped.frame_step);
  bswapped.block_size  = htonl (bswapped.block_size);
  bswapped.pid         = htonl (bswapped.pid);
  bswapped.exitstatus  = htonl (bswapped.exitstatus);
  bswapped.start_loading_time = dr_hton64 (bswapped.start_loading_time);

  if (!dr_write (sfd,(char*)buf,datasize)) {
    return 0;
  }

  return 1;
}

int send_computer (int sfd, struct computer *computer, uint8_t use_local_pools) {
  if (!send_computer_status (sfd,&computer->status)) {
    printf ("error send_computer_status\n");
    return 0;
  }
  if (!send_computer_hwinfo (sfd,&computer->hwinfo)) {
    printf ("error send_computer_hwinfo\n");
    return 0;
  }
  if (!send_computer_limits (sfd,&computer->limits,use_local_pools)) {
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
  uint32_t datasize;

  len = strlen (str)+1;
  lensw = htons (len);
  datasize = sizeof (lensw);
  if (!check_send_datasize(sfd,datasize)) {
    return 0;
  }
  if (!dr_write (sfd,(char*)&lensw,datasize))
    return 0;

  datasize = (uint32_t)len;
  if (!check_send_datasize(sfd,datasize)) {
    return 0;
  }
  if (!dr_write (sfd,(char*)str,len)) {
    return 0;
  }

  return 1;
}

int recv_string (int sfd, char **str) {
  uint16_t len;
  uint32_t datasize;

  datasize = sizeof(len);
  if (!check_recv_datasize(sfd,datasize)) {
    return 0;
  }
  if (!dr_read (sfd,(char*)&len,datasize)) {
    return 0;
  }
  len = ntohs (len);
  
  datasize = (uint32_t)len;
  if (!check_recv_datasize(sfd,datasize)) {
    return 0;
  }
  *str = (char *) malloc (len);
  if (!dr_read (sfd,(char*)*str,len)) {
    return 0;
  }

  return 1;
}

int send_computer_pools (int sfd, struct computer_limits *cl, uint8_t use_local_pools) {
  uint16_t i;
  uint16_t npools;
  struct pool *pool;
  uint32_t datasize;

  log_auto (L_DEBUG3,"send_computer_pools() : Entering...");

  computer_pool_lock (cl);
  datasize = sizeof (npools);
  if (!check_send_datasize(sfd,datasize)) {
    log_auto (L_ERROR,"send_computer_pools(): different data size for npools (uint16_t). Local: %u",datasize); 
    return 0;
  }

  log_auto (L_DEBUG3,"send_computer_pools() : going to send '%u' pools",cl->npools);
  npools = htons (cl->npools);
  if (!dr_write (sfd,(char*)&npools,datasize)) {
    log_auto (L_ERROR,"send_computer_pools(): could not send npools. (%s)",strerror(drerrno_system));
    return 0;
  }

  datasize = sizeof(struct pool);
  if (cl->npools) {
    if (!use_local_pools) {
      log_auto (L_DEBUG3,"send_computer_pools(): NOT USING local pools");
      if ((pool = (struct pool *)computer_pool_attach_shared_memory(cl)) == (struct pool*)-1) {
        log_auto (L_ERROR,"send_computer_pools() : attaching pool shared memory '%ji' shmid. (%s)",cl->poolshmid,strerror(errno));
        return 0;
      }
    } else {
      log_auto (L_DEBUG3,"send_computer_pools(): USING local pools");
      pool = cl->local_pool.ptr;
    }
    log_auto (L_DEBUG2,"send_computer_pools(): npools = %u",cl->npools);
    for (i=0;i<cl->npools;i++) {
      log_auto (L_DEBUG3,"send_computer_pools(): previous to check_send_datasize(struct pool).");
      if (!check_send_datasize(sfd,datasize)) {
	log_auto (L_ERROR,"send_computer_pools() : different data sizes for struct pool.");
	return 0;
      }
      log_auto (L_DEBUG3,"send_computer_pools(): success. post check_send_datasize(struct pool).");
      log_auto (L_DEBUG3,"send_computer_pools(): previous send pool '%i' with name '%s'",i,pool[i].name);
      if (!dr_write(sfd,(char*)&pool[i],datasize)) { // write "sizeof(struct pool)" bytes
        log_auto (L_ERROR,"send_computer_pools() : error while sending pool number %i. (%s)",i,strerror(drerrno_system));
	if (!use_local_pools) {
	  computer_pool_detach_shared_memory (cl);
	}
        return 0;
      }
      log_auto (L_DEBUG3,"send_computer_pools(): success. post send pool '%i' with name '%s'",i,pool[i].name);
    }

    if (!use_local_pools) {
      computer_pool_detach_shared_memory (cl);
    }
  }

  log_auto (L_DEBUG2,"send_computer_pools(): following pool list has been sent:");

  log_auto (L_DEBUG3,"send_computer_pools() : Returning...");

  return 1;
}

int recv_computer_pools (int sfd, struct computer_limits *cl) {
  uint16_t i;
  uint16_t npools;
  struct pool pool;
  uint32_t datasize;
  struct computer_limits limits,old_cl;

  log_auto (L_DEBUG3,"recv_computer_pools(): >Entering...");

  datasize = sizeof(npools);
  if (!check_recv_datasize(sfd,datasize)) {
    log_auto (L_ERROR,"recv_computer_pools(): different npools (uint16_t) size.");
    return 0;
  }

  log_auto (L_DEBUG3,"recv_computer_pools() : _before_ receiving the number of pools");
  if (!dr_read (sfd,(char*)&npools,datasize)) {
    log_auto (L_ERROR,"recv_computer_pools(): error reading from file or socket. (%s)",strerror(drerrno_system));
    return 0;
  }
  npools = ntohs (npools);

  log_auto (L_DEBUG2,"recv_computer_pools(): received number of pools, that will be = %u",npools);

  memset (&limits,0,sizeof(struct computer_limits));
  computer_pool_init (&limits);
  datasize = sizeof(struct pool);
  for (i=0;i<npools;i++) {
    if (!check_recv_datasize(sfd,datasize)) {
      log_auto (L_ERROR,"recv_computer_pools(): different pools (struct pool) size. Local size: %u",datasize);
      return 0;
    }
    memset(&pool,0,sizeof(struct pool));
    if (!dr_read(sfd,(char*)&pool,datasize)) {
      log_auto (L_ERROR,"recv_computer_pools() : could not read pool from file or socket (pool number: %i). Msg: %s",
		i,strerror(drerrno_system));
      // FIXME: if we return, old data is still there.
      return 0;
    }
    log_auto (L_DEBUG3,"recv_computer_pools(): received pool name='%s'. Attempting to add...",pool.name);
    computer_pool_add (&limits,pool.name);
  }

  computer_pool_copy(cl,&old_cl);
  computer_pool_copy(&limits,cl);
  computer_pool_free(&old_cl);

  log_auto (L_DEBUG,"recv_computer_pools(): %i pools received successfully",npools);
  log_auto (L_DEBUG3,"recv_computer_pools(): <Exiting...");

  return 1;
}

int recv_computer_limits (int sfd, struct computer_limits *cl) {
  struct computer_limits limits;
  void *buf=&limits;
  uint32_t datasize;

  datasize = sizeof(struct computer_limits);
  memset (buf,0,datasize);
  computer_limits_init(&limits);
  if (!check_recv_datasize(sfd,datasize)) {
    log_auto (L_ERROR,"recv_computer_limits(): different data sizes for 'struct computer_limits'. Local size: %u",datasize);
    return 0;
  }

  // Receive limits struct
  if (!dr_read (sfd,(char*)buf,datasize)) {
    log_auto (L_ERROR,"recv_computer_limits(): error while receiving computer limits. (%s)",strerror(errno));
    return 0;
  }
  // Remove references to remote shared memory
  computer_limits_cleanup_received (&limits);

  // Receive pools
  if (!recv_computer_pools (sfd,&limits)) {
    log_auto (L_ERROR,"recv_computer_limits(): error while receiving computer pool list. (%s)",strerror(errno));
    return 0;
  }
  
  // Delete old pool data
  computer_pool_free(cl);
  // Put new one in place
  memcpy (cl,&limits,sizeof(limits));

  // Byteswapping
  cl->nmaxcpus = ntohs (cl->nmaxcpus);
  cl->maxfreeloadcpu = ntohs (cl->maxfreeloadcpu);
  cl->autoenable.last = ntohl (cl->autoenable.last);

  return 1;
}

int
send_computer_limits (int sfd, struct computer_limits *cl, uint8_t use_local_pools) {
  struct computer_limits bswapped;
  void *buf = &bswapped;
  uint32_t datasize;

  // Zero it
  memset (buf,0,sizeof(bswapped));

  /* We make a copy coz we need to modify the values */
  memcpy (buf,cl,sizeof(bswapped));

  /* Prepare for sending */
  bswapped.nmaxcpus = htons (bswapped.nmaxcpus);
  bswapped.maxfreeloadcpu = htons (bswapped.maxfreeloadcpu);

  /* Autoenable stuff */
  bswapped.autoenable.last = htonl (bswapped.autoenable.last);

  // Clean up shared memory info
  computer_limits_cleanup_to_send (&bswapped);
 
  datasize = sizeof (struct computer_limits);
  if (!check_send_datasize (sfd,datasize)) {
    log_auto (L_ERROR,"send_computer_limits(): different data sizes for 'struct computer_limits'. Local size: %u",datasize);
    return 0;
  }
  if (!dr_write(sfd,(char*)buf,datasize)) {
    log_auto (L_ERROR,"send_computer_limits(): error while sending main computer limits data. Msg: %s",strerror(drerrno_system));
    return 0;
  }

  // Pools
  if (!send_computer_pools(sfd,cl,use_local_pools)) {
    log_auto (L_ERROR,"send_computer_limits(): error while sending computer pools. Msg: %s",strerror(drerrno_system));
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

  // FIXME: check size
  if (!dr_read(sfd,(char*)ae,sizeof(struct autoenable))) {
    return 0;
  }

  /* Now we should have the autoenable info with the values in */
  /* network byte order, so we put them in host byte order */
  ae->last = ntohl (ae->last);

  return 1;
} 

int
send_blocked_host_list (int sfd, struct blocked_host *bh, uint32_t size, int do_checksize) {
  uint32_t datasize,bssize;
  struct blocked_host *tbh = bh;
  uint32_t i;

  datasize = sizeof (size);
  if (do_checksize && !check_send_datasize(sfd,datasize)) {
    log_auto (L_ERROR,"send_blocked_host_list(): data size mismatch.");
    return 0;
  }

  bssize = htonl(size);
  if (!dr_write(sfd,(char*)&bssize,datasize)) {
    return 0;
  }

  for (i=0;i<size;i++) {
    if (!send_blocked_host(sfd,tbh,do_checksize)) {
      return 0;
    }
    tbh++;
  }

  return 1;
}

int
recv_blocked_host_list (int sfd, struct blocked_host **bh, uint32_t *size, int do_checksize) {
  uint32_t datasize;
  struct blocked_host *tbh;
  uint32_t i;

  datasize = sizeof (*size);
  if (do_checksize && !check_recv_datasize(sfd,datasize)) {
    log_auto (L_ERROR,"recv_blocked_host_list(): size mismatch. (%s)",strerror(drerrno_system));
    return 0;
  }

  if (!dr_read(sfd,(char*)size,datasize)) {
    return 0;
  }
  *size = ntohl(*size);

  if (*size) {
    *bh = (struct blocked_host *) malloc (sizeof (struct blocked_host) * *size);
    if (!*bh) {
      drerrno_system = errno;
      return 0;
    }

    tbh = *bh;
    for (i=0;i<*size;i++) {
      if (!recv_blocked_host(sfd,tbh,do_checksize)) {
	log_auto (L_ERROR,"recv_blocked_host_list(): error receiving blocked host '%i'. (%s)",i,strerror(drerrno_system));
	return 0;
      }
      tbh++;
    }
  }

  return 1;
}

int
send_blocked_host (int sfd, struct blocked_host *bh, int do_checksize) {
  uint32_t datasize;
  datasize = sizeof (*bh);

  if (do_checksize && !check_send_datasize(sfd,datasize)) {
    // TODO: log
    return 0;
  }

  if (!dr_write(sfd,(char*)bh,datasize)) {
    return 0;
  }

  return 1;
}

int recv_blocked_host (int sfd, struct blocked_host *bh, int do_checksize) {
  uint32_t datasize;
  datasize = sizeof (*bh);

  if (do_checksize && !check_recv_datasize(sfd,datasize)) {
    // TODO: log
    return 0;
  }

  if (!dr_read (sfd,(char*)bh,datasize)) {
    return 0;
  }

  return 1;
}

int dr_read (int fd, char *buf, uint32_t len) {
  int r;
  int bleft;
  int total = 0;

  bleft = len;
  while ((r = read (fd,buf,bleft)) < bleft) {
    if ((r == -1) || ((r == 0) && (bleft > 0))) {
      /* if r is error or if there are no more bytes left on the socket but there _SHOULD_ be */
      drerrno_system = errno;
      drerrno = DRE_ERRORREADING;
      return 0;
    }
    bleft -= r;
    buf += r;
    total += r;
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
      drerrno_system = errno;
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
