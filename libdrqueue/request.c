//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
// Copyright (C) 2010 Andreas Schroeder
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

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "request.h"
#include "libdrqueue.h"

// ONGOING:
// * check r_r_uclimits

/* For the differences between data in big endian and little endian */
/* I transmit everything in network byte order */

int phantom[2];

void handle_request_master (int sfd,struct database *wdb,int icomp,struct sockaddr_in *addr) {
  struct request request;

  if (!recv_request (sfd,&request)) {
    log_auto (L_WARNING,"Error receiving request (handle_request_master)");
    // Log the problem and finish with this handler process
    exit(1);
  }

  // Update the time here, because it could not reach the end of the
  // switch if the handler decides to exit this process. (all
  // connection handling runs as different processes for every single
  // connection. Master forked before accepting this connection and
  // this is the child.)
  if ((icomp != -1) && (request.who == SLAVE)) {
    semaphore_lock (wdb->semid);
    /* set the time of the last connection */
    wdb->computer[icomp].lastconn = time(NULL);
    semaphore_release (wdb->semid);
  }

  switch (request.type) {
  case R_R_REGISTER:
    log_auto (L_DEBUG,"Registration of new slave request");
    icomp = handle_r_r_register (sfd,wdb,icomp,addr);
    break;
  case R_R_UCSTATUS:
    log_auto (L_DEBUG,"Update computer status request");
    handle_r_r_ucstatus (sfd,wdb,icomp);
    break;
  case R_R_REGISJOB:
    log_auto (L_DEBUG,"Registration of new job request");
    handle_r_r_regisjob (sfd,wdb);
    request_all_slaves_job_available(wdb);
    break;
  case R_R_AVAILJOB:
    log_auto (L_DEBUG,"Request available job");
    handle_r_r_availjob (sfd,wdb,icomp);
    break;
  case R_R_TASKFINI:
    log_auto (L_DEBUG,"Request task finished");
    handle_r_r_taskfini (sfd,wdb,icomp);
    request_all_slaves_job_available(wdb);
    break;
  case R_R_LISTJOBS:
    log_auto (L_DEBUG,"Request list of jobs");
    handle_r_r_listjobs (sfd,wdb,icomp);
    break;
  case R_R_LISTCOMP:
    log_auto (L_DEBUG,"Request list of computers");
    handle_r_r_listcomp (sfd,wdb,icomp);
    break;
  case R_R_DELETJOB:
    log_auto (L_DEBUG,"Request job deletion");
    handle_r_r_deletjob (sfd,wdb,icomp,&request);
    break;
  case R_R_STOPJOB:
    log_auto (L_DEBUG,"Request job stop");
    handle_r_r_stopjob (sfd,wdb,icomp,&request);
    break;
  case R_R_CONTJOB:
    log_auto (L_DEBUG,"Request job continue");
    handle_r_r_contjob (sfd,wdb,icomp,&request);
    request_all_slaves_job_available(wdb);
    break;
  case R_R_HSTOPJOB:
    log_auto (L_DEBUG,"Request job hard stop");
    handle_r_r_hstopjob (sfd,wdb,icomp,&request);
    break;
  case R_R_RERUNJOB:
    log_auto (L_DEBUG,"Request job rerun");
    handle_r_r_rerunjob (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBXFER:
    log_auto (L_DEBUG,"Request job transfer");
    handle_r_r_jobxfer (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBXFERFI:
    log_auto (L_DEBUG,"Request frame info transfer");
    handle_r_r_jobxferfi (sfd,wdb,icomp,&request);
    break;
  case R_R_COMPXFER:
    log_auto (L_DEBUG,"Request computer transfer");
    handle_r_r_compxfer (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBFWAIT:
    log_auto (L_DEBUG,"Request job frame set to waiting"); // Requeue (do we need another function ?)
    handle_r_r_jobfwait (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBFKILL:
    log_auto (L_DEBUG,"Request job frame kill");
    handle_r_r_jobfkill (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBFFINI:
    log_auto (L_DEBUG,"Request job frame finished");
    handle_r_r_jobffini (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBFKFIN:
    log_auto (L_DEBUG,"Request job frame kill and finished");
    handle_r_r_jobfkfin (sfd,wdb,icomp,&request);
    break;
  case R_R_UCLIMITS:
    log_auto (L_DEBUG,"Update computer limits request");
    handle_r_r_uclimits (sfd,wdb,icomp,&request);
    request_all_slaves_job_available(wdb);
    break;
  case R_R_SLAVEXIT:
    log_auto (L_DEBUG,"Slave exiting");
    handle_r_r_slavexit (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBSESUP:
    log_auto (L_DEBUG,"Update job SES");
    handle_r_r_jobsesup (sfd,wdb,icomp,&request);
    request_all_slaves_job_available(wdb);
    break;
  case R_R_JOBLNMCS:
    log_auto (L_DEBUG,"Set job limits nmaxcpus");
    handle_r_r_joblnmcs (sfd,wdb,icomp,&request);
    request_all_slaves_job_available(wdb);
    break;
  case R_R_JOBLNMCCS:
    log_auto (L_DEBUG,"Set job limits nmaxcpuscomputer");
    handle_r_r_joblnmccs (sfd,wdb,icomp,&request);
    request_all_slaves_job_available(wdb);
    break;
  case R_R_JOBPRIUP:
    log_auto (L_DEBUG,"Update priority");
    handle_r_r_jobpriup (sfd,wdb,icomp,&request);
    request_all_slaves_job_available(wdb);
    break;
  case R_R_JOBFINFO:
    log_auto (L_DEBUG,"Frame info");
    handle_r_r_jobfinfo (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBFRSTRQD:
    log_auto (L_DEBUG,"Frame reset requeued");
    handle_r_r_jobfrstrqd (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBBLKHOST:
    log_auto (L_DEBUG,"Block host add");
    handle_r_r_jobblkhost (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBDELBLKHOST:
    log_auto (L_DEBUG,"Blocked host delete");
    handle_r_r_jobdelblkhost (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBLSTBLKHOST:
    log_auto (L_DEBUG,"Blocked hosts list");
    handle_r_r_joblstblkhost (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBLMS:
    log_auto (L_DEBUG,"Request job limit memory set");
    handle_r_r_joblms (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBLPS:
    log_auto (L_DEBUG,"Request job limit pool set");
    handle_r_r_joblps (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBENVVARS:
    log_auto (L_DEBUG,"Request job environment variables");
    handle_r_r_jobenvvars (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBBLKHOSTNAME:
    log_auto (L_DEBUG,"Block host add by name");
    handle_r_r_jobblkhostname (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBUNBLKHOSTNAME:
    log_auto (L_DEBUG,"Unblock host by name");
    handle_r_r_jobunblkhostname (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBNAME:
    log_auto (L_DEBUG,"Requested job name");
    handle_r_r_jobname (sfd,wdb,icomp,&request);
    break;
  default:
    log_auto (L_WARNING,"Unknown request");
  }

  close (sfd);
  // The handler process finishes gracefully
  exit (0);
}

void handle_request_slave (int sfd,struct slave_database *sdb) {
  /* this function should only be called from the slave connection handler */
  struct request request;

  if (!recv_request (sfd,&request)) {
    log_auto (L_ERROR,"handle_request_slave(): Error receiving request. (%s)",strerror(drerrno_system));
    // Log the problem and finish with this handler process
    exit(1);
  }

  switch (request.type) {
  case RS_R_KILLTASK:
    log_auto (L_DEBUG,"Request kill task");
    handle_rs_r_killtask (sfd,sdb,&request);
    break;
  case RS_R_SETNMAXCPUS:
    log_auto (L_DEBUG,"Request set limits maximum number of usable cpus");
    handle_rs_r_setnmaxcpus (sfd,sdb,&request);
    break;
  case RS_R_SETENABLED:
    log_auto (L_DEBUG,"Request set limits enabled");
    handle_rs_r_setenabled (sfd,sdb,&request);
    break;
  case RS_R_SETMAXFREELOADCPU:
    log_auto (L_DEBUG,"Request set limits maximum free load for cpu");
    handle_rs_r_setmaxfreeloadcpu (sfd,sdb,&request);
    break;
  case RS_R_SETAUTOENABLE:
    log_auto (L_DEBUG,"Request set autoenable info");
    handle_rs_r_setautoenable (sfd,sdb,&request);
    break;
  case RS_R_JOBAVAILABLE:
    log_auto (L_DEBUG,"Request master has an available job");
    write(phantom[1],"D",1);
    break;
  case RS_R_LIMITSPOOLADD:
    log_auto (L_DEBUG,"Request add pool");
    handle_rs_r_limitspooladd (sfd,sdb,&request);
    break;
  case RS_R_LIMITSPOOLREMOVE:
    log_auto (L_DEBUG,"Request remove pool");
    handle_rs_r_limitspoolremove (sfd,sdb,&request);
    break;
  default:
    log_auto (L_WARNING,"Unknown request received");
  }
  close (sfd);
  // The handler process finishes gracefully
  exit (0);
}

int handle_r_r_register (int sfd,struct database *wdb,int icomp,struct sockaddr_in *addr) {
  /* The master handles this type of requests */
  struct request answer;
  struct computer_hwinfo hwinfo;
  int index = -1;  /* CHECK THIS: the index in the computer is an uint32_t. */
  char *name;
  char *dot;
  struct hostent *host;

  log_auto (L_DEBUG3,"handle_r_r_register(): >Entering...");

  if ((host = gethostbyaddr ((const void *)&addr->sin_addr.s_addr,sizeof (struct in_addr),AF_INET)) == NULL) {
    log_auto (L_INFO,"handle_r_r_register(). Using IP address as host name because '%s' could not be resolved",inet_ntoa(addr->sin_addr));
    name=inet_ntoa(addr->sin_addr);
  } else {
    if ((dot = strchr (host->h_name,'.')) != NULL)
      *dot = '\0';
    name = (char*)host->h_name;
  }

  semaphore_lock(wdb->semid); /* I put the lock here so no race condition can appear... */

  if (icomp != -1) {
    /*   semaphore_release(wdb->semid); */
    log_auto (L_WARNING,"Already registered computer requesting registration (%s)",name);
    log_auto (L_WARNING,"Registering again !! (%s)",name);
    /*   answer.type = R_R_REGISTER; */
    /*   answer.data = RERR_ALREADY; */
    /*   if (!send_request (sfd,&answer,MASTER)) { */
    /*    log_auto (L_ERROR,"Sending request handle_r_r_register.RERR_ALREADY to host : '%s'",name); */
    /*   } */
    /*   return -1; */
    index = icomp;
  } else {
    if ((index = computer_index_free(wdb)) == -1) {
      semaphore_release(wdb->semid);
      /* No space left on database */
      log_auto (L_ERROR,"No space left for computer: '%s'",name);
      answer.type = R_R_REGISTER;
      answer.data = RERR_NOSPACE;
      if (!send_request (sfd,&answer,MASTER)) {
        log_auto (L_ERROR,"Sending request handle_r_r_register.RERR_NOSPACE to host : '%s'",name);
      }
      return -1;
    }
  }

  computer_init(&wdb->computer[index]);
  wdb->computer[index].used = 1;
  wdb->computer[index].lastconn = time(NULL);

  /* No errors, we (master) can receive the hwinfo from the remote */
  /* computer to be registered */
  answer.type = R_R_REGISTER;
  answer.data = RERR_NOERROR;
  if (!send_request (sfd,&answer,MASTER)) {
    computer_free(&wdb->computer[index]);
    semaphore_release(wdb->semid);
    log_auto (L_ERROR,"Sending request (handle_r_r_register) to host : '%s'",name);
    return -1;
  }

  /* Now send to the computer it's id, it's position on the master */
  answer.data = (uint32_t) index;
  if (!send_request (sfd,&answer,MASTER)) {
    computer_free (&wdb->computer[index]);
    semaphore_release(wdb->semid);
    log_auto (L_ERROR,"Sending request (handle_r_r_register)");
    return -1;
  }

  if (!recv_computer_hwinfo (sfd, &hwinfo)) {
    computer_free (&wdb->computer[index]);
    semaphore_release(wdb->semid);
    log_auto (L_ERROR,"Receiving computer hardware info (handle_r_r_register)");
    return -1;
  }

  memcpy (&wdb->computer[index].hwinfo, &hwinfo, sizeof(hwinfo));
  strncpy(wdb->computer[index].hwinfo.name,name,MAXNAMELEN); /* We substitute the name that the computer sent */
  /* with the name that we obtained resolving it's ip */

  semaphore_release(wdb->semid);

  log_auto (L_INFO,"handle_r_r_register(): Computer %s registered with id %i.",wdb->computer[index].hwinfo.name,index);
  log_auto (L_DEBUG3,"handle_r_r_register(): <Exiting...");

  return index;
}

void update_computer_status (struct slave_database *sdb) {
  /* The slave calls this function to update the information about */
  /* his own status on the master */
  struct request req;
  struct computer_status status;
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    log_auto(L_ERROR,"Could not connect to master: %s",drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_UCSTATUS;
  if (!send_request (sfd,&req,SLAVE)) {
    log_auto (L_WARNING,"Error sending request (update_computer_status): %s",drerrno_str());
    close (sfd);
    kill (0,SIGINT);
    return;
  }
  if (!recv_request (sfd,&req)) {
    log_auto (L_WARNING,"Error receiving request (update_computer_status): %s",drerrno_str());
    close (sfd);
    kill (0,SIGINT);
    return;
  }

  if (req.type == R_R_UCSTATUS) {
    switch (req.data) {
    case RERR_NOERROR:
      semaphore_lock(sdb->semid);
      memcpy(&status,&sdb->comp->status,sizeof(status));
      semaphore_release(sdb->semid);
      send_computer_status (sfd,&status);
      break;
    case RERR_NOREGIS:
      log_auto (L_ERROR,"Computer not registered");
      close (sfd);
      kill (0,SIGINT);
      break;
    default:
      log_auto (L_ERROR,"Error code not listed on answer to R_R_UCSTATUS");
      break;
    }
  } else {
    log_auto (L_ERROR,"Not appropiate answer to request R_R_UCSTATUS");
  }
  close (sfd);
}

void register_slave (struct computer *computer) {
  /* The slave calls this function to register himself on the master */
  struct request req;
  int sfd;

  log_auto (L_DEBUG,"Entering register_slave");

  if ((sfd = connect_to_master ()) == -1) {
    log_auto(L_ERROR,drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_REGISTER;

  if (!send_request (sfd,&req,SLAVE)) {
    log_auto (L_ERROR,"Sending request (register_slave)");
    kill (0,SIGINT);
  }
  if (!recv_request (sfd,&req)) {
    log_auto (L_ERROR,"Receiving request (register_slave)");
    kill (0,SIGINT);
  }

  if (req.type == R_R_REGISTER) {
    switch (req.data) {
    case RERR_NOERROR:
      if (!recv_request (sfd,&req)) {
        log_auto (L_ERROR,"Receiving request (register_slave)");
        kill (0,SIGINT);
      }
      computer->hwinfo.id = req.data;
      computer->used = 1;
      if (!send_computer_hwinfo (sfd,&computer->hwinfo)) {
        log_auto (L_ERROR,"Sending computer hardware info (register_slave)");
        kill (0,SIGINT);
      }
      break;
    case RERR_ALREADY:
      log_auto (L_ERROR,"Already registered");
      kill (0,SIGINT);
      break;
    case RERR_NOSPACE:
      log_auto (L_ERROR,"No space on database");
      kill (0,SIGINT);
      break;
    default:
      log_auto (L_ERROR,"Error code not listed on answer to R_R_REGISTER");
      kill (0,SIGINT);
      break;
    }
  } else {
    log_auto (L_ERROR,"Not appropiate answer to request R_R_REGISTER");
    kill (0,SIGINT);
  }

  close (sfd);
}

void handle_r_r_ucstatus (int sfd,struct database *wdb,int icomp) {
  /* The master handles this type of packages */
  struct request answer;
  struct computer_status status;

  if (icomp == -1) {
    log_auto (L_WARNING,"Not registered computer requesting update of computer status");
    answer.type = R_R_UCSTATUS;
    answer.data = RERR_NOREGIS;
    if (!send_request (sfd,&answer,MASTER)) {
      log_auto (L_WARNING,"Error receiving request (handle_r_r_ucstatus)");
    }
    return;
  }

  /* No errors, we (master) can receive the status from the remote */
  /* computer already registered */
  answer.type = R_R_UCSTATUS;
  answer.data = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);

  computer_status_init(&status);
  if (!recv_computer_status (sfd, &status))
    return;   /* Do not update in case of failure */

  semaphore_lock(wdb->semid);
  memcpy (&wdb->computer[icomp].status, &status, sizeof(status));
  semaphore_release(wdb->semid);

  /*  report_computer_status (&wdb->computer[icomp].status); */
}

int register_job (struct job *job) {
  /* returns 0 on failure */
  struct request req;
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    fprintf(stderr,"ERROR: %s\n",drerrno_str());
    return 0;
  }

  req.type = R_R_REGISJOB;
  if (!send_request (sfd,&req,CLIENT)) {
    fprintf(stderr,"ERROR: sending request (register_job)\n");
    close (sfd);
    return 0;
  }
  if (!recv_request (sfd,&req)) {
    fprintf(stderr,"ERROR: receiving request (register_job)\n");
    close (sfd);
    return 0;
  }

  if (req.type == R_R_REGISJOB) {
    switch (req.data) {
    case RERR_NOERROR:
      if (!send_job (sfd,job)) {
        fprintf (stderr,"ERROR: Job couldn't be sent: %s\n",drerrno_str());
        close (sfd);
        return 0;
      }
      break;
    case RERR_ALREADY:
      fprintf (stderr,"ERROR: Already registered\n");
      close (sfd);
      return 0;
    case RERR_NOSPACE:
      fprintf (stderr,"ERROR: No space on database\n");
      close (sfd);
      return 0;
    default:
      fprintf (stderr,"ERROR: Error code not listed on answer to R_R_REGISJOB\n");
      close (sfd);
      return 0;
    }
  } else {
    fprintf (stderr,"ERROR: Not appropiate answer to request R_R_REGISJOB\n");
    close (sfd);
    return 0;
  }

  // We need to receive the jobid that has been assigned.
  if (!recv_request (sfd,&req)) {
    fprintf(stderr,"ERROR: receiving request (register_job)\n");
    close (sfd);
    return 0;
  }
  job->id = req.data;

  close (sfd);
  return 1;
}

void handle_r_r_regisjob (int sfd,struct database *wdb) {
  /* The master handles this type of request */
  struct request answer;
  struct job job;
  int index;

  /* TO DO */
  /* Check if the job is already registered ! Or not ? */

  job_init (&job);

  semaphore_lock(wdb->semid);
  if ((index = job_index_free(wdb)) == -1) {
    /* No space left on database */
    semaphore_release(wdb->semid);
    log_auto (L_WARNING,"No space left for job");
    answer.type = R_R_REGISJOB;
    answer.data = RERR_NOSPACE;
    send_request (sfd,&answer,MASTER);
    return;
  }
  wdb->job[index].used = 1;
  semaphore_release(wdb->semid);

  /* Debug */
  log_auto(L_DEBUG,"Job index %i free",index);

  /* No errors, we (master) can receive the job from the remote */
  /* computer to be registered */
  answer.type = R_R_REGISJOB;
  answer.data = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);
  if (!recv_job (sfd, &job)) {
    log_auto (L_ERROR,"Receiving job (handle_r_r_regisjob)");
    semaphore_lock(wdb->semid);
    job_init (&wdb->job[index]); /* We unassign the reserved space for that job */
    semaphore_release(wdb->semid);
    return;
  }

  //job_fix_received_invalid (&job); // Is this needed at all ?

  // We send the job first to avoid race conditions with "submit job stopped"
  job_init_registered (wdb,index,&job);

  // Send job index
  answer.type = R_R_REGISJOB;
  answer.data = index;
  if (!send_request (sfd,&answer,MASTER)) {
    log_auto (L_ERROR,"Sending job index");
    return;
  }
}


void handle_r_r_availjob (int sfd,struct database *wdb,int icomp) {
  /* The master handles this type of packages */
  struct request answer;
  uint32_t ijob = 0,i,j;
  uint16_t itask;
  uint32_t iframe;
  struct tpol pol[MAXJOBS];
  int equal_pols;
  int counter;

  log_auto (L_DEBUG,"Entering handle_r_r_availjob");

  if (icomp == -1) {
    log_auto (L_WARNING,"Not registered computer requesting available job");
    answer.type = R_R_AVAILJOB;
    answer.data = RERR_NOREGIS;
    if (!send_request (sfd,&answer,MASTER)) {
      log_auto (L_WARNING,"Error sending request (handle_r_r_availjob)");
    }
    return;
  }

  log_auto (L_DEBUG,"Creating priority ordered list of jobs");
  for (i=0;i<MAXJOBS;i++) {
    pol[i].index = i;
    pol[i].pri = wdb->job[i].priority;
    pol[i].submit_time = wdb->job[i].submit_time;
  }
  qsort ((void*)pol,MAXJOBS,sizeof(struct tpol),priority_job_compare);

  // Compare with previous pol
  equal_pols = 1;
  for (i=0;i<MAXJOBS;i++) {
    if ((pol[i].index != wdb->lb.pol[i].index)
        || (pol[i].pri != wdb->lb.pol[i].pri)) {
      equal_pols = 0;
      break;
    }
  }

  if (equal_pols == 0) {
    log_auto(L_DEBUG2,"handle_r_r_availjob(): equal_pols == 0");
    // If they changed we save the new one
    for (i=0;i<MAXJOBS;i++) {
      wdb->lb.pol[i].index = pol[i].index;
      wdb->lb.pol[i].pri = pol[i].pri;
      wdb->lb.pol[i].submit_time = pol[i].submit_time;
    }
    // Then we search for the first available job
    counter = 0;
    for (i=0;i<MAXJOBS;i++) {
      ijob = pol[i].index;
      if (pol[i].pri >= wdb->lb.last_priority) {
        for (j=wdb->lb.next_i;j<MAXJOBS;j++) {
          ijob = pol[j].index;
          if (job_available(wdb,ijob,&iframe,icomp)) {
	    logger_computer = &wdb->computer[icomp];
	    logger_job = &wdb->job[ijob];
            log_auto(L_INFO,"handle_r_r_availjob(): Frame %i assigned",job_frame_index_to_number(&wdb->job[ijob],iframe));
            wdb->lb.next_i = j+1;
            break;
          }
        }
        i = j;
        break;
      }
      /* ATENTION job_available sets the available frame as FS_ASSIGNED !! */
      /* We need to set it back to FS_WAITING if something fails */
      if (job_available(wdb,ijob,&iframe,icomp)) {
	logger_computer = &wdb->computer[icomp];
	logger_job = &wdb->job[ijob];
        log_auto(L_INFO,"Frame %i assigned",job_frame_index_to_number(&wdb->job[ijob],iframe));
        wdb->lb.next_i = i+1;
        break;
      }
    }
  } else {
    // Pols are equal
    log_auto(L_DEBUG2,"handle_r_r_availjob() equal_pols == 1");
    for (i=wdb->lb.next_i;i<MAXJOBS;i++) {
      if ((i+1) >= MAXJOBS) {
        for (i=0;i<MAXJOBS;i++) {
          ijob = pol[i].index;
          /* ATENTION job_available sets the available frame as FS_ASSIGNED !! */
          /* We need to set it back to FS_WAITING if something fails */
          if (job_available(wdb,ijob,&iframe,icomp)) {
	    logger_computer = &wdb->computer[icomp];
	    logger_job = &wdb->job[ijob];
            log_auto(L_INFO,"handle_r_r_availjob(): Frame %i assigned",job_frame_index_to_number(&wdb->job[ijob],iframe));
            wdb->lb.next_i = i+1;
            break;
          }
        }
        break;
      }
      if (pol[i].pri >= wdb->lb.last_priority) {
        ijob = pol[i].index;
        /* ATENTION job_available sets the available frame as FS_ASSIGNED !! */
        /* We need to set it back to FS_WAITING if something fails */
        if (job_available(wdb,ijob,&iframe,icomp)) {
	  logger_computer = &wdb->computer[icomp];
	  logger_job = &wdb->job[ijob];
          log_auto(L_INFO,"handle_r_r_availjob(): Frame %i assigned",job_frame_index_to_number(&wdb->job[ijob],iframe));
          wdb->lb.next_i = i+1;
          break;
        }
      } else {
        for (i=0;i<MAXJOBS;i++) {
          ijob = pol[i].index;
          /* ATENTION job_available sets the available frame as FS_ASSIGNED !! */
          /* We need to set it back to FS_WAITING if something fails */
          if (job_available(wdb,ijob,&iframe,icomp)) {
	    logger_computer = &wdb->computer[icomp];
	    logger_job = &wdb->job[ijob];
            log_auto(L_INFO,"Frame %i assigned",job_frame_index_to_number(&wdb->job[ijob],iframe));
            wdb->lb.next_i = i+1;
            break;
          }
        }
        break;
      }
    }
  }

  if (i>=MAXJOBS) {
    // No available jobs from the last assigned one to the end
    // Lets look before.
    for (i=0;i<MAXJOBS;i++) {
      ijob = pol[i].index;
      /* ATENTION job_available sets the available frame as FS_ASSIGNED !! */
      /* We need to set it back to FS_WAITING if something fails */
      if (job_available(wdb,ijob,&iframe,icomp)) {
	logger_computer = &wdb->computer[icomp];
	logger_job = &wdb->job[ijob];
        log_auto(L_INFO,"Frame %i assigned",job_frame_index_to_number(&wdb->job[ijob],iframe));
        wdb->lb.next_i = i+1;
        break;
      }
    }
  }

  logger_computer = NULL;
  logger_job = NULL;

  if (i>=MAXJOBS) {
    // No available one neither before.
    wdb->lb.next_i = 0;
    log_auto(L_DEBUG,"No available job");
    answer.type = R_R_AVAILJOB;
    answer.data = RERR_NOAVJOB;
    if (!send_request (sfd,&answer,MASTER)) {
      log_auto(L_WARNING,"Error sending request (handle_r_r_availjob)");
    }
    return;
  } else {
    wdb->lb.last_priority = pol[i].pri;
  }

  log_auto (L_DEBUG,"Available job (%i) on frame %i assigned. Sending RERR_NOERROR",ijob,iframe);

  /* ijob is now the index to the first available job */
  answer.type = R_R_AVAILJOB;
  answer.data = RERR_NOERROR;
  if (!send_request (sfd,&answer,MASTER)) {
    log_auto(L_WARNING,"Error sending request (handle_r_r_availjob)");
    job_frame_waiting (wdb,ijob,iframe); /* The reserved frame must be set back to waiting */
    exit (0);
  }

  log_auto (L_DEBUG,"Receiving task availability");
  /* Now we receive if there is a task structure available */
  if (!recv_request (sfd,&answer)) {
    log_auto(L_WARNING,"Error receiving request (handle_r_r_availjob)");
    job_frame_waiting (wdb,ijob,iframe);
    exit (0);
  }

  if (answer.type == R_R_AVAILJOB) {
    switch (answer.data) {
    case RERR_NOERROR:
      /* We continue processing the matter */
      log_auto(L_DEBUG,"Task space available");
      break;
    case RERR_NOSPACE:
      log_auto(L_WARNING,"No space for task");
      job_frame_waiting (wdb,ijob,iframe);
      exit (0);
    default:
      log_auto(L_ERROR,"Error code not listed expecting task error code");
      job_frame_waiting (wdb,ijob,iframe);
      exit(0);
    }
  } else {
    log_auto (L_ERROR,"Not appropiate answer, expecting task error code");
    job_frame_waiting (wdb,ijob,iframe);
    exit(0);
  }
  /* If there is a task structure available (we are here) then we receive the index of that task */
  if (!recv_request (sfd,&answer)) {
    log_auto(L_WARNING,"Error receiving request (handle_r_r_availjob)");
    job_frame_waiting (wdb,ijob,iframe);
    exit (0);
  }
  if (answer.type == R_R_AVAILJOB) {
    itask = (uint16_t) answer.data;
    log_auto(L_DEBUG,"Task index %i on computer %i",itask,icomp);
  } else {
    log_auto (L_ERROR,"Not appropiate answer, expecting task index");
    job_frame_waiting (wdb,ijob,iframe);
    exit (0);
  }

  log_auto (L_DEBUG,"Updating structures to be sent");

  semaphore_lock(wdb->semid);
  job_update_assigned (wdb,ijob,iframe,icomp,itask);
  computer_update_assigned (wdb,ijob,iframe,icomp,itask);
  semaphore_release(wdb->semid);

  log_auto (L_DEBUG,"Sending updated task");
  if (!send_task (sfd,&wdb->computer[icomp].status.task[itask])) {
    log_auto (L_ERROR,"Could not send updated task information to the slave: %s",drerrno_str());
    job_frame_waiting (wdb,ijob,iframe);
  }
}


int
request_job_available (struct slave_database *sdb, uint16_t *itask) {
  /* The slave requests the master an available job, and in case */
  /* of finding it the master stores the info into *job and fills the task record */
  /* except those fields that cannot be filled until the task is running (pid, etc.) */

  /* This function SETS *itask local to this process */
  /* This function returns 0 if there is no job available */

  struct request req;
  int sfd;
  struct task ttask;  /* Temporary task structure */

  if ((sfd = connect_to_master ()) == -1) {
    log_auto(L_ERROR,"request_job_available(): could not connect to master. (%s)",strerror(drerrno_system));
    return 0;
  }

  req.type = R_R_AVAILJOB;

  if (!send_request (sfd,&req,SLAVE)) {
    log_auto (L_ERROR,"Sending request for available job: %s",drerrno_str());
    return 0;
  }
  if (!recv_request (sfd,&req)) {
    log_auto (L_ERROR,"Receiving request for available job: %s",drerrno_str());
    return 0;
  }

  if (req.type == R_R_AVAILJOB) {
    switch (req.data) {
    case RERR_NOERROR:
      /* We continue processing the matter */
      log_auto(L_DEBUG,"Available job");
      break;
    case RERR_NOAVJOB:
      log_auto(L_DEBUG,"No available job");
      close (sfd);
      return 0;
      break;
    case RERR_NOREGIS:
      log_auto(L_ERROR,"Computer not registered. Requesting available job.");
      close (sfd);
      return 0;
      break;
    default:
      log_auto(L_ERROR,"Error code not listed on answer to R_R_AVAILJOB");
      close (sfd);
      return 0;
    }
  } else {
    log_auto (L_ERROR,"Not appropiate answer to request R_R_REGISJOB");
    close (sfd);
    return 0;
  }

  if (((*itask) = task_available (sdb)) == (uint16_t)-1) {
    /* No task structure available */
    log_auto(L_WARNING,"No task available for job");
    req.type = R_R_AVAILJOB;
    req.data = RERR_NOSPACE;
    if (!send_request(sfd,&req,SLAVE)) {
      log_auto (L_ERROR,"Sending request (request_job_available) : %s",drerrno_str());
    }
    close (sfd);  /* Finish */
    return 0;
  }

  log_auto (L_DEBUG,"There is an available task. Sending RERR_NOERROR");
  /* We've got an available task */
  req.type = R_R_AVAILJOB;
  req.data = RERR_NOERROR;
  if (!send_request(sfd,&req,SLAVE)) {
    log_auto (L_ERROR,"Sending request (request_job_available) : %s",drerrno_str());
	semaphore_lock(sdb->semid);
	task_init(&sdb->comp->status.task[*itask]);
	semaphore_release(sdb->semid);
    //kill (0,SIGINT);
    close (sfd);
    return 0;
  }

  log_auto (L_DEBUG,"Sending index to task.");
  /* So then we send the index */
  req.data = *itask;
  if (!send_request(sfd,&req,SLAVE)) {
    log_auto (L_ERROR,"Sending request (request_job_available) : %s",drerrno_str());
	semaphore_lock(sdb->semid);
	task_init(&sdb->comp->status.task[*itask]);
	semaphore_release(sdb->semid);
    //kill (0,SIGINT);
    close (sfd);
    return 0;
  }

  log_auto (L_DEBUG,"Receiving the task");
  /* Then we receive the task */
  if (!recv_task(sfd,&ttask)) {
    log_auto (L_ERROR,"Receiving the task (request_job_available) : %s",drerrno_str());
	semaphore_lock(sdb->semid);
	task_init(&sdb->comp->status.task[*itask]);
	semaphore_release(sdb->semid);
    //kill (0,SIGINT);
    close (sfd);
    return 0;
  }

  /* The we update the computer structure to reflect the new assigned task */
  /* that is not yet runnning so pid == 0 */
  semaphore_lock(sdb->semid);
  memcpy(&sdb->comp->status.task[*itask],&ttask,sizeof(ttask));
  sdb->comp->status.task[*itask].status = TASKSTATUS_LOADING; 
  sdb->comp->status.ntasks = computer_ntasks (sdb->comp);
  sdb->comp->status.nrunning = computer_nrunning (sdb->comp);
  semaphore_release(sdb->semid);

  close (sfd);
  return 1;
}

void request_task_finished (struct slave_database *sdb, uint16_t itask) {
  /* This function is called non-blocked */
  /* This function is called from inside a slave launcher process */
  /* It sends the information to the master about a finished task */
  /* FIXME: It doesn't check if the computer is not registered, should we ? */
  /* We are just sending a finished task, so it does not really matter if the computer */
  /* is not registered anymore */
  struct request req;
  int sfd;

  log_auto(L_DEBUG,"Entering request_task_finished");

  if ((sfd = connect_to_master ()) == -1) {
    log_auto(L_ERROR,"Connecting to master (request_task_finished) : %s",drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_TASKFINI;

  if (!send_request (sfd,&req,SLAVE_LAUNCHER)) {
    log_auto (L_ERROR,"sending request (request_task_finished) : %s", drerrno_str());
    kill (0,SIGINT);
  }

  if (!recv_request (sfd,&req)) {
    log_auto (L_ERROR,"receiving request (request_task_finished) : %s", drerrno_str());
    kill (0,SIGINT);
  }

  if (req.type == R_R_TASKFINI) {
    switch (req.data) {
    case RERR_NOERROR:
      /* We continue processing the matter */
      log_auto(L_DEBUG,"Master ready to receive the task");
      break;
    case RERR_NOREGIS:
      log_auto(L_ERROR,"Job not registered");
      close (sfd);
      return;
    case RERR_NOTINRA:
      log_auto(L_ERROR,"Frame out of range");
      close (sfd);
      return;
    default:
      log_auto(L_ERROR,"Error code not listed on answer to R_R_TASKFINI (%i)",req.data);
      close (sfd);
      return;
    }
  } else {
    log_auto (L_ERROR,"Not appropiate answer to request R_R_TASKFINI");
    close (sfd);
    return;
  }

  /* So the master is ready to receive the task */
  /* Then we send the task */
  if (!send_task (sfd,&sdb->comp->status.task[itask])) {
    /* We should retry, but really there should be no errors here */
    log_auto (L_ERROR,"Sending task on request_task_finished : %s",drerrno_str());
    close (sfd);
    return;
  }

  close (sfd);
  log_auto(L_DEBUG,"Finished task information has been sent to master correctly.");
  log_auto(L_DEBUG,"Exiting request_task_finished");
}

void handle_r_r_taskfini (int sfd,struct database *wdb,int icomp) {
  /* The master handles this type of requests */
  /* This funtion is called by the master, non-blocked */
  struct request answer;
  struct task task;
  struct frame_info *fi;

  log_auto (L_DEBUG,"Entering handle_r_r_taskfini");

  if (icomp == -1) {
    /* We log and continue */
    log_auto (L_WARNING,"Not registered computer requesting task finished");
  }

  /* Alway send RERR_NOERR */
  answer.type = R_R_TASKFINI;
  answer.data = RERR_NOERROR;
  if (!send_request (sfd,&answer,MASTER)) {
    log_auto (L_ERROR,"Receiving request (handle_r_r_taskfini)");
    return;
  }

  /* Receive task */
  if (!recv_task(sfd,&task)) {
    log_auto (L_ERROR,"Receiving task (handle_r_r_taskfini)");
    return;
  }

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,task.ijob)) {
    semaphore_release(wdb->semid);
    log_auto (L_WARNING,"ijob not correct in handle_r_r_taskfini");
    return;
  }

  if (strcmp(task.jobname,wdb->job[task.ijob].name) != 0) {
    semaphore_release(wdb->semid);
    log_auto (L_WARNING,"frame finished of non-existing job");
    return;
  }

  /* Once we have the task struct we need to update the information */
  /* on the job struct */
  if ((fi = attach_frame_shared_memory(wdb->job[task.ijob].fishmid)) == (struct frame_info *)-1) {
    job_delete(&wdb->job[task.ijob]);
    semaphore_release(wdb->semid);
    log_auto (L_ERROR,"Couldn't attach frame shared memory on r_r_taskfini. Deleting job.");
    return;
  }

  // In any case
  if (wdb->job[task.ijob].nprocs)
    wdb->job[task.ijob].nprocs--;

  if (job_frame_number_correct (&wdb->job[task.ijob],task.frame)) {
    log_auto (L_DEBUG,"Frame number correct");
    /* Frame is in range */
    task.frame = job_frame_number_to_index (&wdb->job[task.ijob],task.frame);/* frame converted to index frame */
    /* Now we should check the exit code to act accordingly */
    if (DR_WIFEXITED(task.exitstatus)) {
      if (fi[task.frame].flags & FF_REQUEUE) {
        fi[task.frame].status = FS_WAITING;
        fi[task.frame].start_time = 0;
        fi[task.frame].requeued++;
        wdb->job[task.ijob].fleft++;
      } else {
        fi[task.frame].status = FS_FINISHED;
        fi[task.frame].exitcode = DR_WEXITSTATUS(task.exitstatus);
        fi[task.frame].end_time = time(NULL);
        wdb->job[task.ijob].fdone++;
      }
    } else {
      /* Process exited abnormally either killed by us or by itself (SIGSEGV) */
      if (DR_WIFSIGNALED(task.exitstatus)) {
        int sig = DR_WTERMSIG(task.exitstatus);
        log_auto (L_DEBUG,"Signaled with %i",sig);
        if ((sig == SIGTERM) || (sig == SIGINT) || (sig == SIGKILL)) {
          /* Somebody killed the process, so it should be retried */
          log_auto (L_INFO,"Retrying frame %i",job_frame_index_to_number (&wdb->job[task.ijob],task.frame));
          switch (fi[task.frame].status) {
          case FS_WAITING:
            break;
          case FS_ASSIGNED:
            if (wdb->job[task.ijob].autoRequeue) {
              fi[task.frame].status = FS_WAITING;
              fi[task.frame].start_time = 0;
              fi[task.frame].requeued++;
              wdb->job[task.ijob].fleft++;
            } else {
              fi[task.frame].status = FS_ERROR;
              fi[task.frame].end_time = time(NULL);
              wdb->job[task.ijob].ffailed++;
            }
            break;
          case FS_ERROR:
          case FS_FINISHED:
            break;
          }
          fi[task.frame].start_time = 0;
          fi[task.frame].end_time = 0;
        } else {
          log_auto (L_INFO,"Frame %i died - signal %i",job_frame_index_to_number (&wdb->job[task.ijob],task.frame), sig);
          fi[task.frame].status = FS_ERROR;
          fi[task.frame].end_time = time(NULL);
          wdb->job[task.ijob].ffailed++;
        }
      } else {
        /* This  must be WIFSTOPPED, but I'm not sure */
        if (fi[task.frame].status == FS_ASSIGNED) {
          log_auto (L_INFO,"Frame %i died without signal.",job_frame_index_to_number (&wdb->job[task.ijob],task.frame));
          fi[task.frame].status = FS_ERROR;
          fi[task.frame].end_time = time (NULL);
          wdb->job[task.ijob].ffailed++;
        } else {
          // TODO: complete this.
          // log_auto ( /*<FILLME>*/ ,L_INFO,"Frame %i died without signal but was not assigned.",
          //   job_frame_index_to_number (&wdb->job[task.ijob],task.frame));
        }
      }
    }
  } else {
    log_auto (L_ERROR,"frame out of range in handle_r_r_taskfini");
  }
  semaphore_release(wdb->semid);

  detach_frame_shared_memory(fi);

  log_auto (L_DEBUG,"Exiting handle_r_r_taskfini");
}

void handle_r_r_listjobs (int sfd,struct database *wdb,int icomp) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  struct request answer;
  int i;

  /* FIXME : This function does not use semaphores */

  log_auto (L_DEBUG,"Entering handle_r_r_listjobs");

  /* We send the number of active jobs */
  answer.type = R_R_LISTJOBS;
  answer.data = job_njobs_masterdb (wdb);

  if (!send_request (sfd,&answer,MASTER)) {
    log_auto (L_ERROR,"Error receiving request (handle_r_r_listjobs)");
    return;
  }

  for (i=0;i<MAXJOBS;i++) {
    if (wdb->job[i].used) {
      if (!send_job (sfd,&wdb->job[i])) {
        log_auto (L_WARNING,"Error sending job on handling r_r_listjobs");
        return;
      }
    }
  }

  log_auto (L_DEBUG,"Exiting handle_r_r_listjobs");
}

void handle_r_r_listcomp (int sfd,struct database *wdb,int icomp) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  struct request answer;
  struct computer *computer;
  int i;

  log_auto (L_DEBUG3,"Entering handle_r_r_listcomp");

  semaphore_lock(wdb->semid);
  answer.type = R_R_LISTCOMP;
  answer.data = computer_ncomputers_masterdb (wdb);
  computer = malloc (sizeof(struct computer) * MAXCOMPUTERS);
  if (!computer) {
    // FIXME: log
    return;
  }
  memset (computer,0,sizeof(struct computer) * MAXCOMPUTERS);
  memcpy (computer,wdb->computer,sizeof(struct computer) * MAXCOMPUTERS);

  // We attach shared memory and copy it
  for (i=0;i<MAXCOMPUTERS;i++) {
    if (computer[i].used) {
      if (!computer_attach(&computer[i])) {
	      //fprintf (stderr,"%i,used,NOTATTACH (%s)",i,strerror(drerrno_system));
        log_auto (L_WARNING,"Could not locally attach computer pool's shared memory.");
        //computer[i].limits.npools = 0;
      }
    }
  }
  semaphore_release(wdb->semid);

  if (!send_request (sfd,&answer,MASTER)) {
    log_auto (L_ERROR,"Receiving request (handle_r_r_listcomp) : %s",drerrno_str());
    return;
  }

  for (i=0;i<MAXCOMPUTERS;i++) {
    if (computer[i].used) {
      log_auto(L_DEBUG,"Send computer");
      if (!send_computer (sfd,&computer[i],1)) {
        log_auto (L_ERROR,"Sending computer (handle_r_r_listcomp) : %s",drerrno_str());
        break;
      }
    }
  }

  for (i=0;i<MAXCOMPUTERS;i++) {
    if (computer[i].used) {
      computer_detach (&computer[i]);
    }
  }

  free(computer);

  log_auto (L_DEBUG3,"Exiting handle_r_r_listcomp");
}

int request_job_delete (uint32_t ijob, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_DELETJOB;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_deletjob (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  int i;
  uint32_t ijob;
  struct frame_info *fi;
  struct job job;  /* Temporary job for log removal */
  int nframes;

  ijob = req->data;

  log_auto (L_DEBUG,"Entering handle_r_r_deletjob");

  semaphore_lock (wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    log_auto (L_INFO,"Request for deletion of an unused job");
    return;
  }

  memcpy (&job,&wdb->job[ijob],sizeof(job));

  fi = attach_frame_shared_memory (wdb->job[ijob].fishmid);
  if (fi != (struct frame_info *)-1) {
    nframes = job_nframes (&wdb->job[ijob]);
    for (i=0;i<nframes;i++) {
      if (fi[i].status == FS_ASSIGNED) {
        /* FIXME: We use fi[i].icomp without checking it's value */
        request_slave_killtask (wdb->computer[fi[i].icomp].hwinfo.name,fi[i].itask,MASTER);
      }
    }
    detach_frame_shared_memory (fi);
  } else {
    log_auto (L_WARNING,"Could not attach frame shared memory on handle_r_r_deletjob. Deleting problematic job anyway.");
  }

  job_delete (&wdb->job[ijob]);

  semaphore_release (wdb->semid);

  /* Now we remove the logs directory */
  job_logs_remove (&job);

  log_auto (L_DEBUG,"Exiting handle_r_r_deletjob");
}

int request_slave_killtask (char *slave,uint16_t itask,uint16_t who) {
  /* This function is called by the master */
  /* It just sends a slave a request to kill a particular task */
  /* Returns 0 on failure */
  int sfd;
  struct request request;

  if ((sfd = connect_to_slave (slave)) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  request.type = RS_R_KILLTASK;
  request.data = itask;

  if (!send_request (sfd,&request,who)) {
    drerrno = DRE_ERRORWRITING;
    log_auto (L_ERROR,"resquest_slave_killtask to slave %s. %s",slave,drerrno_str());
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_rs_r_killtask (int sfd,struct slave_database *sdb,struct request *req) {
  /* This function is called by the slave unlocked */
  kill(-sdb->comp->status.task[req->data].pid,SIGINT);
}

int request_job_stop (uint32_t ijob, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_STOPJOB;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int request_job_continue (uint32_t ijob, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_CONTJOB;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int
request_job_envvars (uint32_t ijob, struct envvars *envvars, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBENVVARS;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    close (sfd);
    return 0;
  }

  if (!recv_envvars (sfd,envvars,1)) {
    close (sfd);
    return 0;
  }

  // TODO: envvars_check(envvars); to check for posible corrupted values.

  close (sfd);
  return 1;
}

void handle_r_r_jobenvvars (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  log_auto (L_DEBUG,"Entering handle_r_r_jobenvvars");

  uint32_t ijob;

  ijob = req->data;

  log_auto (L_DEBUG,"Requested environment variables of job: %i",ijob);

  if (!job_index_correct_master(wdb,ijob)) {
    log_auto (L_INFO,"Request for hard stopping of an unused job");
    close(sfd);
    return;
  }

  semaphore_lock (wdb->semid);

  if (wdb->job[ijob].used) {
    log_auto (L_DEBUG,"Sending %i environment variables",wdb->job[ijob].envvars.nvariables);
    if (!send_envvars(sfd,&wdb->job[ijob].envvars,1)) {
      close (sfd);
      log_auto (L_WARNING,"Error while sending the environment variables: %s",drerrno_str());
    }
  }

  semaphore_release (wdb->semid);

  log_auto (L_DEBUG,"Exiting handle_r_r_jobenvvars");
}

void handle_r_r_stopjob (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;

  ijob = req->data;

  if (ijob >= MAXJOBS)
    return;

  semaphore_lock (wdb->semid);

  if (wdb->job[ijob].used) {
    job_stop (&wdb->job[ijob]);
  }

  semaphore_release (wdb->semid);
}

void handle_r_r_contjob (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;

  ijob = req->data;

  if (ijob >= MAXJOBS)
    return;

  semaphore_lock (wdb->semid);

  if (wdb->job[ijob].used) {
    job_continue (&wdb->job[ijob]);
  }

  semaphore_release (wdb->semid);
}

void handle_r_r_hstopjob (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  int i;
  uint32_t ijob;
  struct frame_info *fi;
  int nframes;

  ijob = req->data;

  log_auto (L_DEBUG,"Entering handle_r_r_hstopjob");

  semaphore_lock (wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    log_auto (L_INFO,"Request for hard stopping of an unused job");
    return;
  }

  fi = attach_frame_shared_memory (wdb->job[ijob].fishmid);
  if (fi != (struct frame_info *)-1) {
    nframes = job_nframes (&wdb->job[ijob]);
    for (i=0;i<nframes;i++) {
      if (fi[i].status == FS_ASSIGNED) {
        request_slave_killtask (wdb->computer[fi[i].icomp].hwinfo.name,fi[i].itask,MASTER);
      }
    }
    detach_frame_shared_memory (fi);
    job_stop (&wdb->job[ijob]);
  } else {
    /* Couldn't attach the frame memory */
    log_auto (L_WARNING,"Could not attach frame shared memory in handle_r_r_hstopjob. Deleting problematic job.");
    job_delete(&wdb->job[ijob]);
  }

  semaphore_release (wdb->semid);

  log_auto (L_DEBUG,"Exiting handle_r_r_hstopjob");
}

int request_job_hstop (uint32_t ijob, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_HSTOPJOB;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int request_job_rerun (uint32_t ijob, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_RERUNJOB;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_rerunjob (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  int i;
  uint32_t ijob;
  struct frame_info *fi;
  int nframes;

  ijob = req->data;

  log_auto (L_DEBUG,"Entering handle_r_r_rerunjob");

  semaphore_lock (wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    log_auto (L_INFO,"Request for reruning of an unused job index");
    return;
  }

  fi = attach_frame_shared_memory (wdb->job[ijob].fishmid);
  if (fi != (struct frame_info *)-1) {
    nframes = job_nframes (&wdb->job[ijob]);
    for (i=0;i<nframes;i++) {
      if (fi[i].status == FS_ASSIGNED) {
        request_slave_killtask (wdb->computer[fi[i].icomp].hwinfo.name,fi[i].itask,MASTER);
      } else {
        fi[i].status = FS_WAITING;
        fi[i].start_time = 0;
        fi[i].requeued++;
      }
    }
    detach_frame_shared_memory (fi);
  } else {
    /* Couldn't attach the frame memory */
    log_auto (L_WARNING,"Could not attach frame shared memory in handle_r_r_rerunjob. Deleting problematic job.");
    job_delete(&wdb->job[ijob]);
  }

  semaphore_release (wdb->semid);

  log_auto (L_DEBUG,"Exiting handle_r_r_rerunjob");
}

int request_job_xfer (uint32_t ijob, struct job *job, uint16_t who) {
  /* This function can be called by anyone */
  struct request req;
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBXFER;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    close (sfd);
    return 0;
  }

  if (!recv_request (sfd,&req)) {
    close (sfd);
    return 0;
  }

  if (req.type == R_R_JOBXFER) {
    switch (req.data) {
    case RERR_NOERROR:
      /* We continue processing the matter */
      break;
    case RERR_NOREGIS:
      drerrno = DRE_NOTREGISTERED;
      close (sfd);
      return 0;
    default:
      drerrno = DRE_ANSWERNOTLISTED;
      close (sfd);
      return 0;
    }
  } else {
    drerrno = DRE_ANSWERNOTRIGHT;
    close (sfd);
    return 0;
  }

  if (!recv_job (sfd,job)) {
    drerrno = DRE_ERRORREADING;
    close (sfd);
    return 0;
  }

  drerrno = DRE_NOERROR;
  close (sfd);
  return 1;
}

void handle_r_r_jobxfer (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  struct job job;

  log_auto (L_DEBUG,"Entering handle_r_r_jobxfer");

  ijob = req->data;

  semaphore_lock(wdb->semid);
  if (!job_index_correct_master(wdb,ijob)) {
    semaphore_release(wdb->semid);
    log_auto (L_INFO,"Job asked to be transfered does not exist");
    req->type = R_R_JOBXFER;
    req->data = RERR_NOREGIS;
    send_request(sfd,req,MASTER);
    return;
  } else {
    /* The semaphore is still locked and the job index is right */
    /* We take a copy of the locked object and we send that */
    memcpy (&job,&wdb->job[ijob],sizeof (struct job));
  }
  semaphore_release(wdb->semid);

  req->type = R_R_JOBXFER;
  req->data = RERR_NOERROR;
  if (!send_request(sfd,req,MASTER))
    return;

  /* I send a copy because if I would send instead the original */
  /* I would need to block the execution during a network transfer */
  /* and that's too expensive */
  if (!send_job (sfd,&job)) {
    log_auto (L_ERROR,"Sending job (handle_r_r_jobxfer)");
  }

  log_auto (L_DEBUG,"Exiting handle_r_r_jobxfer");
}

int request_job_xferfi (uint32_t ijob, struct frame_info *fi, int nframes, uint16_t who) {
  /* fi must have been already allocated */
  /* This function can be called by anyone */
  struct request req;
  int sfd;
  int i;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBXFERFI;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    close (sfd);
    return 0;
  }

  if (!recv_request (sfd,&req)) {
    close (sfd);
    return 0;
  }

  if (req.type == R_R_JOBXFERFI) {
    switch (req.data) {
    case RERR_NOERROR:
      /* We continue processing the matter */
      break;
    case RERR_NOREGIS:
      drerrno = DRE_NOTREGISTERED;
      close (sfd);
      return 0;
    default:
      drerrno = DRE_ANSWERNOTLISTED;
      close (sfd);
      return 0;
    }
  } else {
    drerrno = DRE_ANSWERNOTRIGHT;
    close (sfd);
    return 0;
  }

  for (i=0;i<nframes;i++) {
    if (!recv_frame_info (sfd,fi)) {
      close (sfd);
      return 0;
    }
    fi++;
  }

  close (sfd);
  return 1;
}

void handle_r_r_jobxferfi (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  struct frame_info *fi,*fi_copy,*fi_start;
  int nframes;
  int i;

  log_auto (L_DEBUG,"Entering handle_r_r_jobxferfi");

  ijob = req->data;

  semaphore_lock(wdb->semid);
  if (!job_index_correct_master(wdb,ijob)) {
    req->type = R_R_JOBXFERFI;
    req->data = RERR_NOREGIS;
    send_request(sfd,req,MASTER);
    semaphore_release(wdb->semid);
    log_auto (L_INFO,"Job asked to be transfered frame info does not exist");
    return;
  }

  nframes = job_nframes (&wdb->job[ijob]);

  if (nframes) {
    if ((fi = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void*)-1) {
      job_delete(&wdb->job[ijob]);
      /* We send a not registered message because we have deleted the problematic job */
      req->type = R_R_JOBXFERFI;
      req->data = RERR_NOREGIS;
      send_request(sfd,req,MASTER);
      semaphore_release(wdb->semid);
      log_auto (L_WARNING,"Could not attach frame shared memory in handle_r_r_jobxferfi. Deleting problematic job.");
      return;
    }

    fi_start = fi_copy = (struct frame_info *) malloc (sizeof(struct frame_info) * nframes);
    if (!fi_copy) {
      semaphore_release(wdb->semid);
      log_auto (L_ERROR,"Allocating memory on handle_r_r_jobxferfi");
      return;   /* The lock should be released automatically and the end of the process */
    }
    memcpy(fi_copy,fi,sizeof(struct frame_info) * nframes);
    detach_frame_shared_memory(fi);
    semaphore_release(wdb->semid);

    /* We make a copy so we don't have the master locked during a network transfer */

    req->type = R_R_JOBXFERFI;
    req->data = RERR_NOERROR;
    if (!send_request(sfd,req,MASTER))
      return;

    log_auto (L_DEBUG,"Sending frame info");
    for (i=0;i<nframes;i++) {
      if (!send_frame_info (sfd,fi_copy)) {
        log_auto (L_ERROR,"Sending frame info");
        return;
      }
      fi_copy++;
    }

    free (fi_start);
  } else {
    semaphore_release(wdb->semid);
  }

  log_auto (L_DEBUG,"Exiting handle_r_r_jobxferfi");
}

void handle_r_r_jobfinfo (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  struct frame_info fi_copy,*fi;
  uint32_t iframe, frame;

  log_auto (L_DEBUG,"Entering handle_r_r_jobfinfo");

  ijob = req->data;

  semaphore_lock(wdb->semid);
  if (!job_index_correct_master(wdb,ijob)) {
    semaphore_release(wdb->semid);
    log_auto (L_INFO,"Job asked to be transfered frame info does not exist");
    return;
  }

  if (!recv_request(sfd,req)) {
    log_auto (L_ERROR,"Receiving request");
    return;
  }

  frame = req->data;

  if (!job_frame_number_correct(&wdb->job[ijob],frame)) {
    semaphore_release(wdb->semid);
    log_auto (L_INFO,"Frame asked to be transfered does not exist");
    return;
  }

  if ((fi = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void*)-1) {
    job_delete(&wdb->job[ijob]);
    /* We send a not registered message because we have deleted the problematic job */
    semaphore_release(wdb->semid);
    log_auto (L_WARNING,"Could not attach frame shared memory in handle_r_r_jobfrinfo. Deleting problematic job.");
    return;
  }

  iframe = job_frame_number_to_index (&wdb->job[ijob],frame);
  memcpy(&fi_copy,&fi[iframe],sizeof(struct frame_info));
  detach_frame_shared_memory(fi);
  semaphore_release(wdb->semid);

  /* We make a copy so we don't have the master locked during a network transfer */

  if (!send_frame_info (sfd,&fi_copy)) {
    log_auto (L_ERROR,"Sending frame info");
    return;
  }

  log_auto (L_DEBUG,"Exiting handle_r_r_jobfinfo");
}

int request_comp_xfer (uint32_t icomp, struct computer *comp, uint16_t who) {
  /* This function can be called by anyone */
  struct request req;
  int sfd;

  // Computer must be freed before receiving
  computer_free (comp);

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_COMPXFER;
  req.data = icomp;

  if (!send_request (sfd,&req,who)) {
    close (sfd);
    return 0;
  }

  if (!recv_request (sfd,&req)) {
    close (sfd);
    return 0;
  }

  if (req.type == R_R_COMPXFER) {
    switch (req.data) {
    case RERR_NOERROR:
      /* We continue processing the matter */
      break;
    case RERR_NOREGIS:
      drerrno = DRE_NOTREGISTERED;
      close (sfd);
      return 0;
    default:
      drerrno = DRE_ANSWERNOTLISTED;
      close (sfd);
      return 0;
    }
  } else {
    drerrno = DRE_ANSWERNOTRIGHT;
    close (sfd);
    return 0;
  }

  if (!recv_computer (sfd,comp)) {
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_compxfer (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t icomp2;  /* The id of the computer asked to be transfered */
  struct computer comp;

  log_auto (L_DEBUG,"Entering handle_r_r_compxfer");

  icomp2 = (uint32_t) req->data;

  semaphore_lock(wdb->semid);

  if (!computer_index_correct_master(wdb,icomp2)) {
    semaphore_release (wdb->semid);
    log_auto (L_INFO,"Computer asked to be transfered is not registered");
    req->type = R_R_COMPXFER;
    req->data = RERR_NOREGIS;
    send_request(sfd,req,MASTER);
    return;
  }

  memcpy(&comp,&wdb->computer[icomp2],sizeof(struct computer));
  if (!computer_attach(&comp)) {
    log_auto (L_WARNING,"Could not attach computer shared memory (handle_r_r_compxfer)");
    return;
  }
  semaphore_release(wdb->semid);

  req->type = R_R_COMPXFER;
  req->data = RERR_NOERROR;
  if (!send_request(sfd,req,MASTER)) {
    computer_detach (&comp);
    return;
  }

  log_auto (L_DEBUG,"Sending computer %i",icomp2);
  send_computer (sfd,&comp,1);
  computer_detach (&comp);
}

int request_job_frame_info (uint32_t ijob, uint32_t frame, struct frame_info *fi, uint16_t who) {
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBFINFO;
  req.data = ijob;
  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBFINFO;
  req.data = frame;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  if (!recv_frame_info(sfd,fi)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int request_job_delete_blocked_host (uint32_t ijob, uint32_t icomp, uint16_t who) {
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBDELBLKHOST;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBDELBLKHOST ;
  req.data = icomp;
  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }
  
  close (sfd);
  return 1;
}

int
request_job_block_host_by_name (uint32_t ijob, char *name, uint16_t who) {
  int sfd;
  struct request req;
  
  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBBLKHOSTNAME;
  req.data = ijob;
  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  if (!send_string(sfd,name)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }
  
  close (sfd);
  return 1;
}

int
request_job_unblock_host_by_name (uint32_t ijob, char *name, uint16_t who) {
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBUNBLKHOSTNAME;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  if (!send_string (sfd,name)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int
request_job_add_blocked_host (uint32_t ijob, uint32_t icomp, uint16_t who) {
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBBLKHOST;
  req.data = ijob;
  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBBLKHOST  ;
  req.data = icomp;
  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int request_job_frame_waiting (uint32_t ijob, uint32_t frame, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBFWAIT;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBFWAIT;
  req.data = frame;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_jobfwait (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  uint32_t frame;
  uint32_t iframe;
  uint32_t nframes;
  struct frame_info *fi;

  log_auto(L_DEBUG,"Entering handle_r_r_jobfwait");

  ijob = req->data;

  if (!recv_request (sfd,req)) {
    return;
  }

  frame = req->data;

  log_auto(L_DEBUG,"Requested job frame waiting for Job %i Frame %i ",ijob,frame);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    semaphore_release(wdb->semid);
    return;
  }

  nframes = job_nframes (&wdb->job[ijob]);

  if (!job_frame_number_correct(&wdb->job[ijob],frame)) {
    semaphore_release(wdb->semid);
    return;
  }

  nframes = job_nframes (&wdb->job[ijob]);
  iframe = job_frame_number_to_index (&wdb->job[ijob],frame);

  if ((fi = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void *)-1) {
    job_delete(&wdb->job[ijob]);
    semaphore_release(wdb->semid);
    log_auto (L_WARNING,"Could not attach frame shared memory in handle_r_r_jobfwait. Deleting problematic job.");
    return;
  }

  switch (fi[iframe].status) {
  case FS_WAITING:
    break;
  case FS_ASSIGNED:
    fi[iframe].flags |= FF_REQUEUE;
    break;
  case FS_ERROR:
  case FS_FINISHED:
    fi[iframe].status = FS_WAITING;
    fi[iframe].start_time = 0;
    fi[iframe].requeued++;
  }
  detach_frame_shared_memory (fi);

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobfwait");
}

int request_job_frame_kill (uint32_t ijob, uint32_t frame, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function requests a running frames (FS_ASSIGNED) and only running */
  /* to be killed. It is set to waiting again as part of the process of signaling */
  /* a frame. The master knows that the process has died because of a signal and */
  /* so it sets the frame again as waiting (requeued) */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBFKILL;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBFKILL;
  req.data = frame;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_jobfkill (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  uint32_t frame;
  uint32_t iframe;
  uint32_t nframes;
  struct frame_info *fi;

  log_auto(L_DEBUG,"Entering handle_r_r_jobfkill");

  ijob = req->data;

  if (!recv_request (sfd,req)) {
    return;
  }

  frame = req->data;

  log_auto(L_DEBUG,"Requested job frame kill for Job %i Frame %i ",ijob,frame);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob))
    return;

  nframes = job_nframes (&wdb->job[ijob]);

  if (!job_frame_number_correct(&wdb->job[ijob],frame))
    return;

  nframes = job_nframes (&wdb->job[ijob]);
  iframe = job_frame_number_to_index (&wdb->job[ijob],frame);

  if ((fi = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void *)-1) {
    job_delete(&wdb->job[ijob]);
    semaphore_release(wdb->semid);
    log_auto (L_WARNING,"Could not attach frame shared memory in handle_r_r_jobfkill. Deleting problematic job.");
    /* Read the next lines as comentary to this */
    log_auto (L_WARNING,"Atention, due to the previos problem attaching, some processors might continue "
                "running frames of an inexistent job.");
    /* So we need consistency checks in the slave to avoid that. We'll see how often that message appears */
    /* to see how bad we need those consistency checks. */
    return;
  }

  switch (fi[iframe].status) {
  case FS_WAITING:
    break;
  case FS_ASSIGNED:
    request_slave_killtask (wdb->computer[fi[iframe].icomp].hwinfo.name,fi[iframe].itask,MASTER);
    break;
  case FS_ERROR:
  case FS_FINISHED:
    break;
  }
  detach_frame_shared_memory (fi);

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobfkill");
}

int request_job_frame_finish (uint32_t ijob, uint32_t frame, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function asks for a frame to be set as finished, it only works */
  /* on FS_WAITING frames. */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBFFINI;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBFFINI;
  req.data = frame;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int request_job_frame_reset_requeued (uint32_t ijob, uint32_t frame, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function asks for a frame to be set as finished, it only works */
  /* on FS_WAITING frames. */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBFRSTRQD;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBFRSTRQD;
  req.data = frame;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_jobffini (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  uint32_t frame;
  uint32_t iframe;
  struct frame_info *fi;

  log_auto(L_DEBUG,"Entering handle_r_r_jobffini");

  ijob = req->data;

  if (!recv_request (sfd,req)) {
    return;
  }

  frame = req->data;

  log_auto(L_DEBUG,"Requested job frame finish for Job %i Frame %i ",ijob,frame);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob)) {
    return;
  }

  if (!job_frame_number_correct(&wdb->job[ijob],frame))
    return;

  iframe = job_frame_number_to_index (&wdb->job[ijob],frame);

  if ((fi = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void *)-1) {
    job_delete(&wdb->job[ijob]);
    semaphore_release(wdb->semid);
    log_auto (L_WARNING,"Could not attach frame shared memory in handle_r_r_jobffini. Deleting problematic job.");
    return;
  }

  switch (fi[iframe].status) {
  case FS_WAITING:
    fi[iframe].exitcode = 0;
    fi[iframe].end_time = time(NULL);
    wdb->job[ijob].fdone++;
    wdb->job[ijob].fdone--;
    fi[iframe].status = FS_FINISHED;
    break;
  case FS_ASSIGNED:
    fi[iframe].exitcode = 0;
    fi[iframe].end_time = time(NULL);
    wdb->job[ijob].fdone++;
    wdb->job[ijob].fleft--;
    fi[iframe].status = FS_FINISHED;
    break;
  case FS_ERROR:
  case FS_FINISHED:
    break;
  }
  detach_frame_shared_memory (fi);

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobffini");
}

void handle_r_r_jobdelblkhost (int sfd, struct database *wdb, int icomp, struct request *req) {
  uint32_t ijob;
  uint32_t ihost;
  struct blocked_host *nbh,*obh = NULL,*tnbh;
  int64_t nbhshmid;
  uint32_t i;

  log_auto(L_DEBUG,"Entering handle_r_r_jobdelblkhost");

  ijob = req->data;
  if (!job_index_correct_master(wdb,ijob))
    return;

  if (!recv_request(sfd,req)) {
    return;
  }
  ihost = req->data;

  semaphore_lock (wdb->semid);

  if (ihost >= wdb->job[ijob].nblocked) {
    // ihost out of range
    semaphore_release (wdb->semid);
    log_auto (L_INFO,"Host to be blocked out of range");
    return;
  }

  // If we reach this point means that there are computers on the block list
  if (wdb->job[ijob].nblocked > 1) { // There are more than one hosts on the list
    if ((obh = attach_blocked_host_shared_memory (wdb->job[ijob].bhshmid)) == (void *)-1) {
      return;
    }
    if ((nbhshmid = get_blocked_host_shared_memory (sizeof(struct blocked_host)*(wdb->job[ijob].nblocked-1))) == (int64_t)-1) {
      return;
    }
    if ((nbh = attach_blocked_host_shared_memory (nbhshmid)) == (void *)-1) {
      return;
    }
    tnbh = nbh;
    for (i=0; i < wdb->job[ijob].nblocked; i++) {
      if (i != ihost) {
        memcpy ((void*)tnbh,(void*)&obh[i],sizeof(struct blocked_host));
        tnbh++;
      } else {
        log_auto(L_INFO,"Deleted host %s from block list.",obh[i].name);
      }
    }
    // Once copied all but the removed host we can detach and remove the old shared memory
    detach_blocked_host_shared_memory (obh);
    detach_blocked_host_shared_memory (nbh);
    if (shmctl ((int)wdb->job[ijob].bhshmid,IPC_RMID,NULL) == -1) {
      log_auto(L_ERROR,"job_delete: shmctl (job->bhshmid,IPC_RMID,NULL) [Removing blocked hosts shared memory]");
    }
    wdb->job[ijob].bhshmid = nbhshmid;
    wdb->job[ijob].nblocked--;
  } else {
    // nblocked must be 1
    if ((obh = attach_blocked_host_shared_memory (wdb->job[ijob].bhshmid)) == (void *)-1) {
      return;
    }
    log_auto(L_INFO,"Deleted host %s from block list.",obh[0].name);
    detach_blocked_host_shared_memory (obh);
    if (shmctl ((int)wdb->job[ijob].bhshmid,IPC_RMID,NULL) == -1) {
      log_auto(L_ERROR, "job_delete: shmctl (job->bhshmid,IPC_RMID,NULL) [Removing blocked hosts shared memory]");
    }
    wdb->job[ijob].bhshmid = (int64_t)-1;
    wdb->job[ijob].nblocked = 0;
    wdb->job[ijob].blocked_host.ptr = NULL;
  }
  semaphore_release(wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobdelblkhost");
}

int
request_job_list_blocked_host (uint32_t ijob, struct blocked_host **bh, uint16_t *nblocked, uint16_t who) {
  int sfd;
  struct request req;
  struct blocked_host *tbh;
  int i;

  if (*bh != NULL) {
    // Free previous memory
    free (*bh);
    *bh = NULL;
  }

  *nblocked = 0;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBLSTBLKHOST;
  req.data = ijob;
  if (!send_request (sfd,&req,who)) {
    close (sfd);
    return 0;
  }

  if (!recv_request (sfd,&req)) {
    close (sfd);
    return 0;
  }

  *nblocked = (uint16_t)req.data;

    
  if ((*bh = (struct blocked_host *) malloc (sizeof (struct blocked_host)* (*nblocked))) == NULL) {
    // FIXME error handlind
    *nblocked = 0;
    return 0;
  }

  tbh = *bh;
  for (i=0;i<*nblocked;i++) {
    recv_blocked_host(sfd,tbh,1);
    tbh++;
  }
  
  close (sfd);
  return 1;
}

void handle_r_r_joblstblkhost (int sfd, struct database *wdb, int icomp, struct request *req) {
  uint32_t ijob;
  struct blocked_host *nbh = NULL;
  int i;

  log_auto(L_DEBUG,"Entering handle_r_r_joblstblkhost");

  ijob = req->data;

  if (!job_index_correct_master (wdb,ijob)) {
    return;
  }

  req->data = wdb->job[ijob].nblocked;
  if (!send_request(sfd,req,MASTER)) {
    return;
  }

  if (wdb->job[ijob].nblocked &&
      ((nbh = attach_blocked_host_shared_memory (wdb->job[ijob].bhshmid)) == (void *)-1)) {
    semaphore_release (wdb->semid);
    return;
  }

  for (i=0;i<wdb->job[ijob].nblocked;i++) {
    // log_auto (L_DEBUG,"Listing: %s",nbh[i].name);
    send_blocked_host (sfd,&nbh[i],1);
  }

  log_auto(L_DEBUG,"Exiting handle_r_r_joblstblkhost");
}

void handle_r_r_jobblkhost (int sfd, struct database *wdb, int icomp, struct request *req) {
  uint32_t ijob;
  uint32_t ihost;
  struct blocked_host *nbh,*obh = NULL;
  int64_t nbhshmid;
  int i;

  log_auto(L_DEBUG,"Entering handle_r_r_jobblkhost");

  ijob = req->data;

  if (!recv_request (sfd,req)) {
    return;
  }

  ihost = req->data;

  semaphore_lock (wdb->semid);

  if (!job_index_correct_master (wdb,ijob)) {
    semaphore_release (wdb->semid);
    return;
  }


  if (!wdb->computer[ihost].used) {
    semaphore_release (wdb->semid);
    return;
  }

  // log_auto(L_DEBUG,"Requested host (%i) block for job %i",ihost,ijob);
  // log_auto(L_DEBUG,"Already blocked: %i",wdb->job[ijob].nblocked);
  // If there are blocked hosts, attach the memory
  if (wdb->job[ijob].nblocked) {
    // log_auto(L_DEBUG,"About to attach old shared memory");
    if ((obh = attach_blocked_host_shared_memory (wdb->job[ijob].bhshmid)) == (void *)-1) {
      // log_auto(L_DEBUG,"Fail attach old shared memory");
      semaphore_release (wdb->semid);
      return;
    }
    // log_auto(L_DEBUG,"Success attach old shared memory");
    // Search for coincidence
    for (i = 0; i < wdb->job[ijob].nblocked; i++) {
      if (strcmp (obh[i].name,wdb->computer[ihost].hwinfo.name) == 0) {
        // log_auto(L_DEBUG,"Host already on old shared memory");
        // Host already on the list of blocked hosts
        semaphore_release (wdb->semid);
        return;
      }
    }
  }

  if ((nbhshmid = get_blocked_host_shared_memory (wdb->job[ijob].nblocked+1)) == (int64_t)-1) {
    semaphore_release (wdb->semid);
    return;
  }

  if ((nbh = attach_blocked_host_shared_memory (nbhshmid)) == (void *)-1) {
    semaphore_release (wdb->semid);
    return;
  }

  if (wdb->job[ijob].nblocked) {
    memcpy (nbh,obh,sizeof(struct blocked_host)*wdb->job[ijob].nblocked);
    // Once copied we can remove the previous list
    detach_blocked_host_shared_memory (obh);
    if (shmctl ((int)wdb->job[ijob].bhshmid,IPC_RMID,NULL) == -1) {
      log_auto(L_ERROR,"job_delete(): shmctl (job->bhshmid,IPC_RMID,NULL) [Removing blocked hosts shared memory]");
    }
  }

  wdb->job[ijob].bhshmid = nbhshmid;
  // Add to the end of the block list, we use the old nblocked value
  memcpy (nbh[wdb->job[ijob].nblocked].name,wdb->computer[ihost].hwinfo.name,MAXNAMELEN);
  wdb->job[ijob].nblocked++;

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobblkhost");
}

void handle_r_r_jobfrstrqd (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  uint32_t frame;
  uint32_t iframe;
  struct frame_info *fi;

  log_auto(L_DEBUG,"Entering handle_r_r_jobffini");

  ijob = req->data;

  if (!recv_request (sfd,req)) {
    return;
  }

  frame = req->data;

  log_auto(L_DEBUG,"Requested job frame finish for Job %i Frame %i ",ijob,frame);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob))
    return;


  if (!job_frame_number_correct(&wdb->job[ijob],frame))
    return;

  iframe = job_frame_number_to_index (&wdb->job[ijob],frame);

  if ((fi = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void *)-1) {
    job_delete(&wdb->job[ijob]);
    semaphore_release(wdb->semid);
    log_auto (L_WARNING,"Could not attach frame shared memory in handle_r_r_jobffini. Deleting problematic job.");
    return;
  }

  fi[iframe].requeued = 0;

  detach_frame_shared_memory (fi);

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobffini");
}

int request_job_frame_kill_finish (uint32_t ijob, uint32_t frame, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function asks for a frame to be set as finished, it only works */
  /* on FS_WAITING frames. */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBFKFIN;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBFKFIN;
  req.data = frame;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_jobfkfin (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  uint32_t frame;
  uint32_t iframe;
  uint32_t nframes;
  struct frame_info *fi;

  log_auto(L_DEBUG,"Entering handle_r_r_jobfkfin");

  ijob = req->data;

  if (!recv_request (sfd,req)) {
    return;
  }

  frame = req->data;

  log_auto(L_DEBUG,"Requested job frame kill and finish for Job %i Frame %i ",ijob,frame);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob))
    return;

  nframes = job_nframes (&wdb->job[ijob]);

  if (!job_frame_number_correct(&wdb->job[ijob],frame))
    return;

  nframes = job_nframes (&wdb->job[ijob]);
  iframe = job_frame_number_to_index (&wdb->job[ijob],frame);

  if ((fi = attach_frame_shared_memory (wdb->job[ijob].fishmid)) == (void *)-1) {
    job_delete(&wdb->job[ijob]);
    semaphore_release(wdb->semid);
    log_auto (L_WARNING,"Could not attach frame shared memory in handle_r_r_jobfkfin. Deleting problematic job.");
    return;
  }
  switch (fi[iframe].status) {
  case FS_WAITING:
    break;
  case FS_ASSIGNED:
    fi[iframe].status = FS_FINISHED;
    request_slave_killtask (wdb->computer[fi[iframe].icomp].hwinfo.name,fi[iframe].itask,MASTER);
    break;
  case FS_ERROR:
  case FS_FINISHED:
    break;
  }
  detach_frame_shared_memory (fi);

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobfkfin");
}


int request_slave_limits_nmaxcpus_set (char *slave, uint32_t nmaxcpus, uint16_t who) {
  int sfd;
  struct request req;

  if ((sfd = connect_to_slave (slave)) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = RS_R_SETNMAXCPUS;
  req.data = nmaxcpus;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    return 0;
  }

  close (sfd);
  return 1;
}

int request_slave_limits_enabled_set (char *slave, uint8_t enabled, uint16_t who) {
  int sfd;
  struct request req;

  if ((sfd = connect_to_slave (slave)) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = RS_R_SETENABLED;
  req.data = enabled;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    return 0;
  }
  
  close (sfd);
  return 1;
}

int request_slave_limits_autoenable_set (char *slave, uint32_t h, uint32_t m, unsigned char flags, uint16_t who) {
  int sfd;
  struct request req;

  if ((sfd = connect_to_slave (slave)) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = RS_R_SETAUTOENABLE;
  req.data = h;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    return 0;
  }

  req.type = RS_R_SETAUTOENABLE;
  req.data = m;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    return 0;
  }

  req.type = RS_R_SETAUTOENABLE;
  req.data = flags;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_rs_r_setnmaxcpus (int sfd,struct slave_database *sdb,struct request *req) {
  struct computer_limits limits;

  log_auto(L_DEBUG,"Entering handle_rs_r_setnmaxcpus");
  log_auto(L_DEBUG,"Received maximum cpus: %i",req->data);

  semaphore_lock(sdb->semid);

  log_auto(L_DEBUG,"Previous maximum cpus: %i",sdb->comp->limits.nmaxcpus);
  sdb->comp->limits.nmaxcpus = (req->data < sdb->comp->hwinfo.ncpus) ? req->data : sdb->comp->hwinfo.ncpus;
  log_auto(L_DEBUG,"Set maximum cpus: %i",sdb->comp->limits.nmaxcpus);
  memcpy (&limits,&sdb->comp->limits,sizeof(struct computer_limits));

  semaphore_release(sdb->semid);

  update_computer_limits (&limits);

  log_auto(L_DEBUG,"Exiting handle_rs_r_setnmaxcpus");
}

void handle_rs_r_setenabled (int sfd,struct slave_database *sdb,struct request *req) {
  struct computer_limits limits;

  log_auto(L_DEBUG,"Entering handle_rs_r_setenabled");
  log_auto(L_DEBUG,"Received enabled: %i",req->data);

  semaphore_lock(sdb->semid);

  log_auto(L_DEBUG,"Previous enabled value: %i",sdb->comp->limits.enabled);
  sdb->comp->limits.enabled = (uint8_t) req->data;
  log_auto(L_DEBUG,"Set enabled: %i",sdb->comp->limits.enabled);
  memcpy (&limits,&sdb->comp->limits,sizeof(struct computer_limits));

  semaphore_release(sdb->semid);

  update_computer_limits (&limits);

  log_auto(L_DEBUG,"Exiting handle_rs_r_setenabled");
}

void handle_rs_r_setautoenable (int sfd,struct slave_database *sdb,struct request *req) {
  struct computer_limits limits;
  uint32_t h,m;
  unsigned char flags;

  log_auto(L_DEBUG,"Entering handle_rs_r_setautoenable");

  h = req->data % 24;  /* Take care in case it exceeds the limits */

  if (!recv_request (sfd,req)) {
    log_auto (L_ERROR,"Receiving request (update_computer_limits) : %s",drerrno_str());
    kill (0,SIGINT);
  }

  m = req->data % 60;  /* Take care in case it exceeds the limits */

  log_auto(L_DEBUG,"Received autoenable hour: %i:%02i",h,m);

  if (!recv_request (sfd,req)) {
    log_auto (L_ERROR,"Receiving request (update_computer_limits) : %s",drerrno_str());
    kill (0,SIGINT);
  }

  flags = req->data;

  semaphore_lock(sdb->semid);

  log_auto(L_DEBUG,"Previous autoenable hour: %i:%02i",
                     sdb->comp->limits.autoenable.h,
                     sdb->comp->limits.autoenable.m);

  sdb->comp->limits.autoenable.flags = flags;
  sdb->comp->limits.autoenable.h = h;
  sdb->comp->limits.autoenable.m = m;
  sdb->comp->limits.autoenable.last = time(NULL) - AE_DELAY;

  log_auto(L_DEBUG,"Set autoenable hour: %i:%02i",
                     sdb->comp->limits.autoenable.h,
                     sdb->comp->limits.autoenable.m);

  memcpy (&limits,&sdb->comp->limits,sizeof(struct computer_limits));

  semaphore_release(sdb->semid);

  update_computer_limits (&limits);

  log_auto(L_DEBUG,"Exiting handle_rs_r_setnmaxcpus");
}

int
update_computer_limits (struct computer_limits *limits) {
  /* The slave calls this function to update the information about */
  /* its limits structure when it is changed */
  struct request req;
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    log_auto(L_ERROR,"Connecting to master (update_computer_limits) : %s",drerrno_str());
    drerrno = DRE_CONNMASTER;
    return 0;
  }

  req.type = R_R_UCLIMITS;
  if (!send_request (sfd,&req,SLAVE)) {
    log_auto (L_ERROR,"Sending request (update_computer_limits) : %s",strerror(drerrno_system));
    drerrno = DRE_COMMPROBLEM;
    return 0;
  }
  if (!recv_request (sfd,&req)) {
    log_auto (L_ERROR,"Receiving request (update_computer_limits) : %s",drerrno_str());
    drerrno = DRE_COMMPROBLEM;
    return 0;
  }

  if (req.type == R_R_UCLIMITS) {
    switch (req.data) {
    case RERR_NOERROR:
      if (!send_computer_limits (sfd,limits,0)) {
        log_auto (L_ERROR,"Sending computer limits (update_computer_limits) : %s",drerrno_str());
        log_auto (L_ERROR,"Sending computer limits (update_computer_limits) : %s (%s)",drerrno_str(),strerror(drerrno_system));
        drerrno = DRE_COMMPROBLEM;
        return 0;
      }
      break;
    case RERR_NOREGIS:
      log_auto (L_ERROR,"Computer not registered");
      drerrno = DRE_NOTREGISTERED;
      return 0;
      break;
    default:
      log_auto (L_ERROR,"Error code not listed on answer to R_R_UCLIMITS");
      drerrno = DRE_ANSWERNOTLISTED;
      return 0;
      break;
    }
  } else {
    log_auto (L_ERROR,"Not appropiate answer to request R_R_UCLIMITS");
    drerrno = DRE_ANSWERNOTRIGHT;
    return 0;
  }
  
  close (sfd);
  return 1;
}

void handle_r_r_uclimits (int sfd,struct database *wdb,int icomp, struct request *req) {
  /* The master handles this type of packages */
  struct computer_limits limits;

  log_auto (L_DEBUG,"Entering handle_r_r_uclimits");

  if (icomp == -1) {
    log_auto (L_WARNING,"Not registered computer requesting update of computer limits");
    req->type = R_R_UCLIMITS;
    req->data = RERR_NOREGIS;
    if (!send_request (sfd,req,MASTER)) {
      log_auto (L_ERROR,"Receiving request (handle_r_r_uclimits)");
    }
    exit (0);
  }

  /* No errors, we (master) can receive the status from the remote */
  /* computer already registered */
  req->type = R_R_UCLIMITS;
  req->data = RERR_NOERROR;
  if (!send_request (sfd,req,MASTER)) {
    log_auto (L_ERROR,"Receiving request (handle_r_r_uclimits)");
    exit (0);
  }

  computer_limits_init (&limits);
  if (!recv_computer_limits (sfd, &limits)) {
    log_auto (L_ERROR,"Receiving computer limits (handle_r_r_uclimits)");
    exit (0);
  }

  semaphore_lock(wdb->semid);
  computer_pool_free (&wdb->computer[icomp].limits);
  memcpy (&wdb->computer[icomp].limits, &limits, sizeof(limits));
  computer_pool_detach_shared_memory(&wdb->computer[icomp].limits);
  semaphore_release(wdb->semid);

  computer_pool_list (&limits);

  log_auto (L_DEBUG,"Exiting handle_r_r_uclimits");
}

void handle_rs_r_setmaxfreeloadcpu (int sfd,struct slave_database *sdb,struct request *req) {
  struct computer_limits limits;

  log_auto(L_DEBUG3,"handle_rs_r_setmaxfreeloadcpu(): >Entering...");
  log_auto(L_DEBUG2,"handle_rs_r_setmaxfreeloadcpu(): received maximum free load cpu: %u",req->data);

  log_auto(L_DEBUG2,"handle_rs_r_setmaxfreeloadcpu(): previous maximum free load cpu: %u",sdb->comp->limits.maxfreeloadcpu);

  semaphore_lock(sdb->semid);
  sdb->comp->limits.maxfreeloadcpu = (uint16_t)req->data;
  memcpy (&limits,&sdb->comp->limits,sizeof(struct computer_limits));
  semaphore_release(sdb->semid);

  log_auto(L_DEBUG,"handle_rs_r_setmaxfreeloadcpu(): set maximum free load cpu: %u",sdb->comp->limits.maxfreeloadcpu);

  update_computer_limits (&limits);
  log_auto(L_DEBUG3,"handle_rs_r_setmaxfreeloadcpu(): <Exiting...");
}

int request_slave_limits_maxfreeloadcpu_set (char *slave, uint32_t maxfreeloadcpu, uint16_t who) {
  int sfd;
  struct request req;

  if ((sfd = connect_to_slave (slave)) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = RS_R_SETMAXFREELOADCPU;
  req.data = maxfreeloadcpu;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    return 0;
  }

  close (sfd);
  return 1;
}


int request_slavexit (uint32_t icomp, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_SLAVEXIT;
  req.data = icomp;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_slavexit (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t icomp2;


  log_auto (L_DEBUG,"Entering handle_r_r_slavexit");

  icomp2 = req->data;

  semaphore_lock (wdb->semid);

  if (!computer_index_correct_master (wdb,icomp2)) {
    semaphore_release (wdb->semid);
    return;
  }
  if (wdb->computer[icomp2].hwinfo.id == (uint32_t)icomp) {
    log_auto (L_INFO,"Slave quitting: %s (%i)", wdb->computer[icomp2].hwinfo.name, icomp2);
    computer_free (&wdb->computer[icomp2]);
  }

  semaphore_release (wdb->semid);

  log_auto (L_DEBUG,"Exiting handle_r_r_slavexit");
}

int request_job_sesupdate (uint32_t ijob, uint32_t frame_start,uint32_t frame_end,
                           uint32_t frame_step, uint32_t block_size, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function sends the new frame_start,end and step for ijob */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBSESUP;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBSESUP;
  req.data = frame_start;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBSESUP;
  req.data = frame_end;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBSESUP;
  req.data = frame_step;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBSESUP;
  req.data = block_size;
  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_jobsesup (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  uint32_t nnframes;  /* Number of frames for the new job */
  uint32_t onframes;  /* Number of frames of the old job */
  struct job job;  /* Temporary job to calc nframes */
  struct job ojob;  /* Temporary old ojb info */
  struct frame_info *nfi,*ofi; /* new and old frame_info */
  uint32_t i;
  int64_t nfishmid;   /* New identifier for new shared frame info struct */
  int64_t ofishmid;   /* Old identifier */
  char cname[MAXNAMELEN]; /* Computer name */

  log_auto (L_DEBUG,"Entering handle_r_r_jobsesup");

  ijob = req->data;
  if (!recv_request(sfd,req))
    return;
  job.frame_start = req->data;
  if (!recv_request(sfd,req))
    return;
  job.frame_end = req->data;
  if (!recv_request(sfd,req))
    return;
  job.frame_step = req->data;
  if (!recv_request(sfd,req))
    return;
  job.block_size = req->data;

  semaphore_lock(wdb->semid);

  nnframes = job_nframes (&job);
  onframes = job_nframes (&wdb->job[ijob]);

  if (!job_index_correct_master (wdb,ijob)) {
    semaphore_release(wdb->semid);
    log_auto (L_INFO,"Job SES update received for non-existing job.");
    return;
  }

  if ((nfishmid = get_frame_shared_memory (nnframes)) == (int64_t)-1) {
    semaphore_release(wdb->semid);
    log_auto (L_ERROR,"Could not allocate memory for new SES. Could not proceed updating SES.");
    return;
  }

  if ((nfi = attach_frame_shared_memory (nfishmid)) == (void *)-1) {
    semaphore_release(wdb->semid);
    log_auto (L_ERROR,"Could not attach new frame shared memory. Could not proceed updating SES.");
    return;
  }

  if ((ofi = attach_frame_shared_memory(wdb->job[ijob].fishmid)) == (void *)-1) {
    semaphore_release(wdb->semid);
    log_auto (L_ERROR,"Could not attach old frame shared memory. Could not proceed updating SES");
    return;
  }

  /* We init the new frame info struct */
  for (i=0;i<nnframes;i++) {
    job_frame_info_init(&nfi[i]);
  }

  /* First we check the old frames that are inside the new range */
  for (i=0;i<nnframes;i++) {
    if (job_frame_number_correct(&wdb->job[ijob],
                                 job_frame_index_to_number(&job,i))) {
      /* If the frame number of the new job was part of the old job */
      /* then we must copy the contents of the frame info */

      memcpy(&nfi[i],&ofi[job_frame_number_to_index(&wdb->job[ijob],
                          job_frame_index_to_number(&job,i))],
             sizeof(struct frame_info));
    }
    /* The rest of frames are already initialized to WAITING */
  }

  /* We save the old job info for later use when killing frames */
  /* The only information we need is for passing from frame number to index and viceversa */
  /* so we only need SES info and also fishmid for deletion */
  ojob.frame_start = wdb->job[ijob].frame_start;
  ojob.frame_end = wdb->job[ijob].frame_end;
  ojob.frame_step = wdb->job[ijob].frame_step;
  ojob.block_size = wdb->job[ijob].block_size;

  /* Now everything can continue using the new frame info */
  wdb->job[ijob].frame_start = job.frame_start;
  wdb->job[ijob].frame_end = job.frame_end;
  wdb->job[ijob].frame_step = job.frame_step;
  wdb->job[ijob].block_size = job.block_size;

  ofishmid = wdb->job[ijob].fishmid;
  wdb->job[ijob].fishmid = nfishmid;

  detach_frame_shared_memory(nfi);

  /* Everything should continue now. Killing old out of range frames does not need to be locked */
  semaphore_release(wdb->semid);

  /* Now we check the old frames that are outside the new range */
  /* Nobody should be accesing this information now except this process */
  for (i=0;i<onframes;i++) {
    if (!job_frame_number_correct(&job,
                                  job_frame_index_to_number(&ojob,i))) {
      /* And we act accordingly to the status */
      /* We kill the frames that are running and are not inside the range */
      switch (ofi[i].status) {
      case FS_WAITING:
        break;
      case FS_ASSIGNED:
        semaphore_lock (wdb->semid);
        if (computer_index_correct_master(wdb,ofi[i].icomp)) {
          strncpy(cname,wdb->computer[ofi[i].icomp].hwinfo.name,MAXNAMELEN-1);
        }
        semaphore_release (wdb->semid);
        request_slave_killtask (cname,ofi[i].itask,MASTER);
        break;
      case FS_ERROR:
      case FS_FINISHED:
        break;
      }
    }
  }

  if (shmctl ((int)ofishmid,IPC_RMID,NULL) == -1) {
    /* We delete the old frame shared memory */
    /*    log_auto(job,L_ERROR,"job_delete: shmctl (job->fishmid,IPC_RMID,NULL) [Removing frame shared memory]"); */
  }

  detach_frame_shared_memory (ofi);

  log_auto (L_DEBUG,"Exiting handle_r_r_jobsesup");
}

int request_job_limits_memory_set (uint32_t ijob, uint32_t memory, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function request the master to set a new limit for job's nmaxcpus */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBLMS;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBLMS;
  req.data = memory;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int request_job_limits_pool_set (uint32_t ijob, char *pool, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function request the master to set a new limit for job's nmaxcpus */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBLPS;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    close (sfd);
    return 0;
  }

  if (!send_string (sfd,pool)) {
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_joblms (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  uint32_t memory;

  log_auto(L_DEBUG,"Entering handle_r_r_joblms");

  ijob = req->data;

  /* Second packet */
  if (!recv_request (sfd,req)) {
    return;
  }

  memory = req->data;

  log_auto(L_DEBUG,"Requested job (ijob:%u) limits memory set to: %u",ijob,memory);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob))
    return;

  wdb->job[ijob].limits.memory = memory;

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_joblms");
}

void handle_r_r_joblps (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  char *pool;

  log_auto(L_DEBUG,"Entering handle_r_r_joblps");

  ijob = req->data;

  if (!recv_string(sfd,&pool)) {
    return;
  }

  log_auto(L_DEBUG,"Requested job (ijob:%u) limits pool set to: %s",ijob,pool);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob))
    return;

  strncpy (wdb->job[ijob].limits.pool,pool,MAXNAMELEN-1);

  semaphore_release (wdb->semid);

  free (pool);

  log_auto(L_DEBUG,"Exiting handle_r_r_joblps");
}

int request_job_limits_nmaxcpus_set (uint32_t ijob, uint16_t nmaxcpus, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function request the master to set a new limit for job's nmaxcpus */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBLNMCS;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBLNMCS;
  req.data = nmaxcpus;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_joblnmcs (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  uint16_t nmaxcpus;

  log_auto(L_DEBUG,"Entering handle_r_r_joblnmcs");

  ijob = req->data;

  /* Second packet */
  if (!recv_request (sfd,req)) {
    return;
  }

  nmaxcpus = (uint16_t) req->data;

  log_auto(L_DEBUG,"Requested job (ijob:%u) limits nmaxcpus set to: %u",ijob,nmaxcpus);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob))
    return;

  wdb->job[ijob].limits.nmaxcpus = nmaxcpus;

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_joblnmcs");
}

int request_job_limits_nmaxcpuscomputer_set (uint32_t ijob, uint16_t nmaxcpuscomputer, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function request the master to set a new limit for job's nmaxcpus */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBLNMCCS;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBLNMCCS;
  req.data = nmaxcpuscomputer;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_joblnmccs (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* joblnmccs = job_limits_number_max_computer_cpus_set */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  uint16_t nmaxcpuscomputer;

  log_auto(L_DEBUG,"Entering handle_r_r_joblnmccs");

  ijob = req->data;

  /* Second packet */
  if (!recv_request (sfd,req)) {
    return;
  }

  nmaxcpuscomputer = (uint16_t) req->data;

  log_auto(L_DEBUG,"Requested job (ijob:%u) limits nmaxcpuscomputer set to: %u",ijob,nmaxcpuscomputer);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob))
    return;

  wdb->job[ijob].limits.nmaxcpuscomputer = nmaxcpuscomputer;

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_joblnmccs");
}

int request_job_priority_update (uint32_t ijob, uint32_t priority, uint16_t who) {
  /* On error returns 0, error otherwise drerrno is set to the error */
  /* This function request the master to set a priority for a job (identified by ijob) */
  int sfd;
  struct request req;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBPRIUP;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  req.type = R_R_JOBPRIUP;
  req.data = priority;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_jobpriup (int sfd,struct database *wdb,int icomp,struct request *req) {
  /* The master handles this type of packages */
  /* This function is called unlocked */
  uint32_t ijob;
  uint32_t priority;

  log_auto(L_DEBUG,"Entering handle_r_r_jobpriup");

  ijob = req->data;

  /* Second packet */
  if (!recv_request (sfd,req)) {
    return;
  }

  priority = req->data;

  log_auto(L_DEBUG,"Requested job (ijob:%u) priority set to: %u",ijob,priority);

  semaphore_lock(wdb->semid);

  if (!job_index_correct_master(wdb,ijob))
    return;

  wdb->job[ijob].priority = priority;

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobpriup");
}

void request_all_slaves_job_available (struct database *wdb) {
  /* This function checks for an available job. In case it exists it notifies all available slaves */
  struct tpol pol[MAXJOBS];
  uint32_t ijob = 0,i;
  uint32_t iframe;

  log_auto (L_DEBUG,"Entering request_all_slaves_job_available");

  log_auto (L_DEBUG,"Creating priority ordered list of jobs");

  for (i=0;i<MAXJOBS;i++) {
    pol[i].index = i;
    pol[i].pri = wdb->job[i].priority;
  }
  qsort ((void*)pol,MAXJOBS,sizeof(struct tpol),priority_job_compare);

  for (i=0;i<MAXJOBS;i++) {
    ijob = pol[i].index;
    if (job_available_no_icomp(wdb,ijob,&iframe)) {
      log_auto (L_DEBUG,"Job available (%i) for notification to slaves",ijob);
      break;
    }
  }

  if (i==MAXJOBS) {
    log_auto (L_DEBUG,"No job available for notification to slaves");
    return;
  }

  // FIXME: This should be an ordered list of computers
  for (i=0;i<MAXCOMPUTERS;i++) {
    if (wdb->computer[i].used) {
      //  if (computer_available(&wdb->computer[i])) {
      log_auto (L_DEBUG,"Notifying slave (%s)",wdb->computer[i].hwinfo.name);
      request_slave_job_available (wdb->computer[i].hwinfo.name,MASTER);
    }
  }

  log_auto (L_DEBUG,"Exiting request_all_slaves_job_available");
}

int request_slave_job_available (char *slave, uint16_t who) {
  /* This function is called by the master */
  /* It just sends a slave a notification that there is a job available */
  /* Returns 0 on failure */
  int sfd;
  struct request request;

  if ((sfd = connect_to_slave (slave)) == -1) {
    return 0;
  }

  request.type = RS_R_JOBAVAILABLE;
  request.data = 0;

  if (!send_request (sfd,&request,who)) {
    return 0;
  }

  close (sfd);
  return 1;
}

int request_slave_limits_pool_add (char *slave, char *pool, uint16_t who) {
  int sfd;
  struct request request;

  if ((sfd = connect_to_slave (slave)) == -1) {
    return 0;
  }

  request.type = RS_R_LIMITSPOOLADD;

  if (!send_request (sfd,&request,who)) {
    return 0;
  }

  if (!send_string(sfd,pool)) {
    return 0;
  }

  close (sfd);
  return 1;
}

int request_slave_limits_pool_remove (char *slave, char *pool, uint16_t who) {
  int sfd;
  struct request request;

  if ((sfd = connect_to_slave (slave)) == -1) {
    return 0;
  }

  request.type = RS_R_LIMITSPOOLREMOVE;

  if (!send_request (sfd,&request,who)) {
    return 0;
  }

  if (!send_string(sfd,pool)) {
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_rs_r_limitspooladd (int sfd,struct slave_database *sdb,struct request *req) {
  char *pool;

  log_auto(L_DEBUG,"Entering handle_rs_r_limitspooladd");

  if (!recv_string(sfd,&pool)) {
    return;
  }

  semaphore_lock(sdb->semid);

  log_auto(L_INFO,"New pool to list: %s",pool);
  computer_pool_add(&sdb->comp->limits,pool);

  semaphore_release(sdb->semid);

  update_computer_limits (&sdb->comp->limits);

  log_auto(L_DEBUG,"Exiting handle_rs_r_limitspooladd");
}

void handle_rs_r_limitspoolremove (int sfd,struct slave_database *sdb,struct request *req) {
  char *pool;

  log_auto(L_DEBUG,"Entering handle_rs_r_limitspoolremove");

  if (!recv_string(sfd,&pool)) {
    return;
  }

  semaphore_lock(sdb->semid);

  log_auto(L_INFO,"Pool to be removed from list: %s",pool);
  computer_pool_remove(&sdb->comp->limits,pool);

  semaphore_release(sdb->semid);

  update_computer_limits (&sdb->comp->limits);

  log_auto(L_DEBUG,"Exiting handle_rs_r_limitspooladd");
}

int
request_job_list (struct job **job, uint16_t who) {
  //
  // Returns the numbers of jobs that has copied into the returned pointer, -1 in case of error
  //
  struct request req;
  int sfd;
  struct job *tjob;
  int njobs;
  int i;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return -1;
  }

  req.type = R_R_LISTJOBS;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return -1;
  }

  if (!recv_request (sfd,&req)) {
    drerrno = DRE_ERRORREADING;
    close (sfd);
    return -1;
  }

  if (req.type == R_R_LISTJOBS) {
    njobs = req.data;
  } else {
    drerrno = DRE_ANSWERNOTRIGHT;
    close (sfd);
    return -1;
  }

  if (njobs) {
    if ((*job = (struct job *) malloc (sizeof (struct job) * njobs)) == NULL) {
      drerrno = DRE_NOMEMORY;
      close (sfd);
      return -1;
    }

    tjob = *job;
    for (i=0;i<njobs;i++) {
      job_init(tjob);
      if (!recv_job (sfd,tjob)) {
        fprintf (stderr,"ERROR: Receiving job (drqm_request_joblist)\n");
        break;
      }
      tjob++;
    }
  } else {
    *job = NULL;
  }

  close (sfd);

  return njobs;
}

int request_computer_list (struct computer **computer, uint16_t who) {
  struct request req;
  int sfd;
  struct computer *tcomputer;
  int ncomputers;
  int i;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"request_computer_list(): could not connect to master. (%s)",strerror(drerrno_system));
    drerrno = DRE_NOCONNECT;
    return -1;
  }

  req.type = R_R_LISTCOMP;

  if (!send_request (sfd,&req,who)) {
    log_auto (L_ERROR,"request_computer_list(): could not send request. (%s)",strerror(drerrno_system));
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return -1;
  }

  if (!recv_request (sfd,&req)) {
    log_auto (L_ERROR,"request_computer_list(): could not receive request reply. (%s)",strerror(drerrno_system));
    drerrno = DRE_ERRORREADING;
    close (sfd);
    return -1;
  }

  if (req.type == R_R_LISTCOMP) {
    ncomputers = req.data;
  } else {
    drerrno = DRE_ANSWERNOTRIGHT;
    log_auto (L_ERROR,"request_computer_list(): %s.",drerrno_str());
    close (sfd);
    return -1;
  }

  if (ncomputers) {
    if ((*computer = (struct computer *) malloc (sizeof (struct computer) * ncomputers)) == NULL) {
      drerrno_system = errno;
      log_auto (L_ERROR,"request_computer_list(): could not allocate memory. (%s)",strerror(drerrno_system));
      drerrno = DRE_NOMEMORY;
      close (sfd);
      return -1;
    }
    
    tcomputer = *computer;
    for (i=0;i<ncomputers;i++) {
      computer_init(tcomputer);
      if (!recv_computer (sfd,tcomputer)) {
        log_auto (L_ERROR,"request_computer_list(): problem while receiving computer on position %i. (%s)",
                  i,strerror(drerrno_system));
        break;
      }
      tcomputer++;
    }
  } else {
    *computer = NULL;
  }

  close (sfd);

  return ncomputers;
}

void
handle_r_r_jobblkhostname (int sfd, struct database *wdb, int icomp, struct request *req) {
  uint32_t ijob;
  uint32_t ihost;
  char *name;

  log_auto(L_DEBUG,"Entering handle_r_r_jobblkhostname");

  ijob = req->data;

  if (!recv_string(sfd,&name)) {
    // TODO: log
    return;
  }

  semaphore_lock (wdb->semid);

  if ((ihost = computer_index_name (wdb,name)) == (uint32_t)-1) {
    log_auto (L_WARNING,"Blocking a host that is not registered: %s",name);
    return;
  }

  if (!job_index_correct_master (wdb,ijob)) {
    semaphore_release (wdb->semid);
    return;
  }

  job_block_host_add_by_name (&wdb->job[ijob],name);

  // log_auto(L_DEBUG,"Requested host (%i) block for job %i",ihost,ijob);
  // log_auto(L_DEBUG,"Already blocked: %i",wdb->job[ijob].nblocked);
  // If there are blocked hosts, attach the memory

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobblkhost");
}

void
handle_r_r_jobunblkhostname (int sfd, struct database *wdb, int icomp, struct request *req) {
  uint32_t ijob;
  char *name;

  log_auto(L_DEBUG,"Entering handle_r_r_jobunblkhostname");

  ijob = req->data;

  if (!recv_string(sfd,&name)) {
    // TODO: log
    return;
  }

  semaphore_lock (wdb->semid);

  if (!job_index_correct_master (wdb,ijob)) {
    semaphore_release (wdb->semid);
    return;
  }

  job_block_host_remove_by_name (&wdb->job[ijob],name);

  // log_auto(L_DEBUG,"Requested host (%i) block for job %i",ihost,ijob);
  // log_auto(L_DEBUG,"Already blocked: %i",wdb->job[ijob].nblocked);
  // If there are blocked hosts, attach the memory

  semaphore_release (wdb->semid);

  log_auto(L_DEBUG,"Exiting handle_r_r_jobblkhost");
}

int
request_job_name (uint32_t ijob, char **jobname, uint16_t who) {
  struct request req;
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    drerrno = DRE_NOCONNECT;
    return 0;
  }

  req.type = R_R_JOBNAME;
  req.data = ijob;

  if (!send_request (sfd,&req,who)) {
    drerrno = DRE_ERRORWRITING;
    close (sfd);
    return 0;
  }

  if (!recv_string (sfd,jobname)) {
    drerrno = DRE_ERRORREADING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_jobname (int sfd, struct database *wdb, int icomp, struct request *req) {
  uint32_t ijob;

  ijob = req->data;

  if (!job_index_correct_master(wdb,ijob)) {
    // TODO: log it
    return;
  }

  if (!send_string (sfd,wdb->job[ijob].name)) {
    log_auto (L_ERROR,"handle_r_r_jobname(): sending job name. (%s)",strerror(drerrno_system));
    return;
  }

  return;
}
