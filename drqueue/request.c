/* $Id: request.c,v 1.3 2001/05/28 14:21:31 jorge Exp $ */
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
#include "slave.h"
#include "drerrno.h"

void handle_request_master (int sfd,struct database *wdb,int icomp)
{
  struct request request;

  recv_request (sfd,&request,MASTER);
  switch (request.type) {
  case R_R_REGISTER:
    log_master ("Info: Registration of new slave request");
    handle_r_r_register (sfd,wdb,icomp);
    break;
  case R_R_UCSTATUS:
    log_master ("Info: Update computer status request");
    handle_r_r_ucstatus (sfd,wdb,icomp);
    break;
  case R_R_REGISJOB:
    log_master ("Info: Registration of new job request");
    handle_r_r_regisjob (sfd,wdb);
    break;
  case R_R_AVAILJOB:
    log_master ("Info: Request of available job");
    handle_r_r_availjob (sfd,wdb,icomp);
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

void handle_request_slave (int sfd,struct slave_database *sdb)
{
  /* this function should only be called from the slave connection handler */
  struct request request;

  recv_request (sfd,&request,SLAVE_CHANDLER);
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
    answer.slave = 0;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }

  semaphore_lock(wdb->semid);	/* I put the lock here so no race condition can appear... */
  if ((index = computer_index_free(wdb)) == -1) {
    /* No space left on database */
    log_master ("Warning: No space left for computer");
    answer.type = R_A_REGISTER;
    answer.data_s = RERR_NOSPACE;
    answer.slave = 0;
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
  answer.slave = 0;
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
  char msg[BUFFERLEN];
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    snprintf(msg,BUFFERLEN-1,"ERROR: %s",drerrno_str());
    log_slave_computer(msg);
    kill(0,SIGINT);
  }

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
      log_slave_computer ("ERROR: Computer not registered");
      break;
    default:
      log_slave_computer ("ERROR: Error code not listed on answer to R_R_UCSTATUS");
      kill (0,SIGINT);
    }
  } else {
    log_slave_computer ("ERROR: Not appropiate answer to request R_R_UCSTATUS");
    kill (0,SIGINT);
  }
  close (sfd);
}

void register_slave (struct computer *computer)
{
  /* The slave calls this function to register himself on the master */
  struct request req;
  char msg[BUFFERLEN];
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    snprintf(msg,BUFFERLEN-1,"ERROR: %s",drerrno_str());
    log_slave_computer(msg);
    kill(0,SIGINT);
  }

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
      log_slave_computer ("ERROR: Already registered");
      kill (0,SIGINT);
      break;
    case RERR_NOSPACE:
      log_slave_computer ("ERROR: No space on database");
      kill (0,SIGINT);
      break;
    default:
      log_slave_computer ("ERROR: Error code not listed on answer to R_R_REGISTER");
      kill (0,SIGINT);
    }
  } else {
    log_slave_computer ("ERROR: Not appropiate answer to request R_R_REGISTER");
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
    answer.slave = 0;
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

void register_job (struct job *job)
{
  /* Some client (control, or whatever itl be called) calls */
  /* function to register a job on the master */
  struct request req;
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    fprintf(stderr,"ERROR: %s\n",drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_REGISJOB;
  req.slave = 0;

  send_request (sfd,&req,CLIENT);
  recv_request (sfd,&req,CLIENT);
  if (req.type == R_A_REGISJOB) {
    switch (req.data_s) {
    case RERR_NOERROR:
      send_job (sfd,job,CLIENT);
      break;
    case RERR_ALREADY:
      fprintf (stderr,"ERROR: Already registered\n");
      exit (1);
      break;
    case RERR_NOSPACE:
      fprintf (stderr,"ERROR: No space on database\n");
      exit (1);
      break;
    default:
      fprintf (stderr,"ERROR: Error code not listed on answer to R_R_REGISJOB\n");
      exit (1);
    }
  } else {
    fprintf (stderr,"ERROR: Not appropiate answer to request R_R_REGISJOB\n-");
    exit(1);
  }

  close (sfd);
}

void handle_r_r_regisjob (int sfd,struct database *wdb)
{
  /* The master handles this type of packages */
  struct request answer;
  struct job job;
  int index;
  int nframes;

  /* Check if the job is already registered ! Or not ? */

  semaphore_lock(wdb->semid);	/* I put the lock here so no race condition can appear... */
  if ((index = job_index_free(wdb)) == -1) {
    /* No space left on database */
    log_master ("Warning: No space left for job");
    answer.type = R_A_REGISJOB;
    answer.data_s = RERR_NOSPACE;
    answer.slave = 0;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }
  wdb->job[index].used = 1;
  semaphore_release(wdb->semid);

  /* No errors, we (master) can receive the job from the remote */
  /* computer to be registered */
  answer.type = R_A_REGISJOB;
  answer.data_s = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);
  
  recv_job (sfd, &job, MASTER);
  semaphore_lock(wdb->semid);
  memcpy (&wdb->job[index], &job, sizeof(job));
  wdb->job[index].status = JOBSTATUS_WAITING;
  wdb->job[index].used = 1;
  /* We allocate the memory for the frame_info */
  nframes = wdb->job[index].frame_end - wdb->job[index].frame_start;
  nframes = (nframes < 0) ? -nframes : nframes;
  nframes++;
  wdb->job[index].frame_info = (struct frame_info *) malloc (sizeof (struct frame_info) * nframes);
  if (wdb->job[index].frame_info == NULL) {
    log_master ("Warning: Could not allocate memory for frame info");
    wdb->job[index].used = 0;
  } else {
    job_init (&wdb->job[index]);
  }
  semaphore_release(wdb->semid);

  printf ("Index: %i\n",index);
  job_report(&wdb->job[index]);
}

void handle_r_r_availjob (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of packages */
  struct request answer;
  struct computer_status status;
  int ijob;
  int itask;
  int iframe;

  if (icomp == -1) {
    log_master ("Info: Not registered computer requesting available job");
    answer.type = R_A_AVAILJOB;
    answer.data_s = RERR_NOREGIS;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }

  for (ijob=0;ijob<MAXJOBS;ijob++) {
    if (job_available(&wdb->job[ijob],&iframe))
      break;
  }

  if (ijob==MAXJOBS) {
    answer.type = R_A_AVAILJOB;
    answer.data_s = RERR_NOAVJOB;
    send_request (sfd,&answer,MASTER);
    exit (0);
  } 

  /* ijob is now the index to the first available job */
  answer.type = R_A_AVAILJOB;
  answer.data_s = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);
  /* Now we receive if there is a task structure available */
  recv_request (sfd,&answer,MASTER);
  if (answer.type == R_A_AVAILJOB) {
    switch (answer.data_s) {
    case RERR_NOERROR:
      /* We continue processing the matter */
      break;
    case RERR_NOSPACE:
      log_master_computer(&wdb->computer[icomp],"Warning: No space for task");
      exit (0);
    default:
      log_master_computer(&wdb->computer[icomp],"ERROR: Error code not listed expecting task error code");
      exit(0);
    }
  } else {
    log_master_computer (&wdb->computer[icomp],"ERROR: Not appropiate answer, expecting task error code");
    exit(0);
  }
  /* If there is a task structure available (we are here) then we receive the index of that task */
  recv_request (sfd,&answer,MASTER);
  if (answer.type == R_A_AVAILJOB) {
    itask = answer.data_s;
  } else {
    log_master_computer (&wdb->computer[icomp],"ERROR: Not appropiate answer, expecting task index");
    exit (0);
  }

  semaphore_lock(wdb->semid);
  job_update_assigned (wdb,ijob);
  computer_update_assigned (&wdb->computer[icomp],itask,iframe);
  semaphore_release(wdb->semid);

  send_task (sfd,&wdb->computer[icomp].status.task[itask],MASTER);
}


int request_job_available (struct slave_database *sdb)
{
  /* Here we (slave) ask the master for an available job and in case */
  /* of finding it we store the info into *job, and fill the task record */
  /* except what cannot be filled until the proper execution of the task */
  struct request req;
  int sfd;
  char emsg[BUFFERLEN];

  if ((sfd = connect_to_master ()) == -1) {
    snprintf(emsg,BUFFERLEN-1,"ERROR: %s",drerrno_str());
    log_slave_computer(emsg);
    kill(0,SIGINT);
  }

  req.type = R_R_AVAILJOB;

  send_request (sfd,&req,SLAVE);
  recv_request (sfd,&req,SLAVE);
  if (req.type == R_A_AVAILJOB) {
    switch (req.data_s) {
    case RERR_NOERROR:
      /* We continue processing the matter */
      break;
    case RERR_NOAVJOB:
      log_slave_computer("Info: No available job");
      close (sfd);
      return 0;
      break;
    case RERR_NOREGIS:
      log_slave_computer("ERROR: Computer not registered");
      kill (0,SIGINT);
      break;
    default:
      log_slave_computer("ERROR: Error code not listed on answer to R_R_AVAILJOB");
      kill (0,SIGINT);
    }
  } else {
    fprintf (stderr,"ERROR: Not appropiate answer to request R_R_REGISJOB\n-");
    kill (0,SIGINT);
  }

  if ((sdb->itask = task_available (sdb->comp)) == -1) {
    /* No task structure available */
    log_slave_computer("Warning: No task available for job");
    req.type = R_A_AVAILJOB;
    req.data_s = RERR_NOSPACE;
    send_request(sfd,&req,SLAVE);
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}


