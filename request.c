/* $Id: request.c,v 1.34 2001/08/31 14:06:23 jorge Exp $ */
/* For the differences between data in big endian and little endian */
/* I transmit everything in network byte order */

#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <wait.h>
#include <stdlib.h>

#include "request.h"
#include "database.h"
#include "logger.h"
#include "communications.h"
#include "semaphores.h"
#include "slave.h"
#include "drerrno.h"
#include "job.h"

void handle_request_master (int sfd,struct database *wdb,int icomp)
{
  struct request request;

  if (!recv_request (sfd,&request)) {
    log_master (L_WARNING,"Error receiving request (handle_request_master)");
    return;
  }
  switch (request.type) {
  case R_R_REGISTER:
    log_master (L_DEBUG,"Registration of new slave request");
    handle_r_r_register (sfd,wdb,icomp);
    break;
  case R_R_UCSTATUS:
    log_master (L_DEBUG,"Update computer status request");
    handle_r_r_ucstatus (sfd,wdb,icomp);
    break;
  case R_R_REGISJOB:
    log_master (L_DEBUG,"Registration of new job request");
    handle_r_r_regisjob (sfd,wdb);
    break;
  case R_R_AVAILJOB:
    log_master (L_DEBUG,"Request of available job");
    handle_r_r_availjob (sfd,wdb,icomp);
    break;
  case R_R_TASKFINI:
    log_master (L_DEBUG,"Request task finished");
    handle_r_r_taskfini (sfd,wdb,icomp);
    break;
  case R_R_LISTJOBS:
    log_master (L_DEBUG,"Request list of jobs");
    handle_r_r_listjobs (sfd,wdb,icomp);
    break;
  case R_R_LISTCOMP:
    log_master (L_DEBUG,"Request list of computers");
    handle_r_r_listcomp (sfd,wdb,icomp);
    break;
  case R_R_DELETJOB:
    log_master (L_DEBUG,"Request job deletion");
    handle_r_r_deletjob (sfd,wdb,icomp,&request);
    break;
  case R_R_STOPJOB:
    log_master (L_DEBUG,"Request job stop");
    handle_r_r_stopjob (sfd,wdb,icomp,&request);
    break;
  case R_R_CONTJOB:
    log_master (L_DEBUG,"Request job continue");
    handle_r_r_contjob (sfd,wdb,icomp,&request);
    break;
  case R_R_HSTOPJOB:
    log_master (L_DEBUG,"Request job hard stop");
    handle_r_r_hstopjob (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBXFER:
    log_master (L_DEBUG,"Request job transfer");
    handle_r_r_jobxfer (sfd,wdb,icomp,&request);
    break;
  case R_R_JOBXFERFI:
    log_master (L_DEBUG,"Request frame info transfer");
    handle_r_r_jobxferfi (sfd,wdb,icomp,&request);
    break;
  case R_R_COMPXFER:
    log_master (L_DEBUG,"Request computer transfer");
    handle_r_r_compxfer (sfd,wdb,icomp,&request);
    break;
  default:
    log_master (L_WARNING,"Unknown request");
  }
  if ((icomp != -1) && (request.who != CLIENT)) {
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

  if (!recv_request (sfd,&request)) {
    log_slave_computer (L_WARNING,"Error receiving request (handle_request_slave)");
    return;
  }

  switch (request.type) {
  case RS_R_KILLTASK:
    log_slave_computer (L_DEBUG,"Request kill task");
    handle_rs_r_killtask (sfd,sdb,&request);
    break;
  default:
    log_slave_computer (L_WARNING,"Unknown request received");
  }
}

void handle_r_r_register (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of requests */
  struct request answer;
  struct computer_hwinfo hwinfo;
  int index;

  log_master (L_DEBUG,"Entering handle_r_r_register");

  if (icomp != -1) {
    log_master (L_INFO,"Already registered computer requesting registration");
    answer.type = R_A_REGISTER;
    answer.data = RERR_ALREADY;
    if (!send_request (sfd,&answer,MASTER)) {
      log_master (L_WARNING,"Error sending request (handle_r_r_register)");
    }
    exit (0);
  }

  semaphore_lock(wdb->semid);	/* I put the lock here so no race condition can appear... */
  if ((index = computer_index_free(wdb)) == -1) {
    /* No space left on database */
    log_master (L_WARNING,"No space left for computer");
    answer.type = R_A_REGISTER;
    answer.data = RERR_NOSPACE;
    if (!send_request (sfd,&answer,MASTER)) {
      log_master (L_WARNING,"Error sending request (handle_r_r_register)");
    }
    exit (0);
  }
  wdb->computer[index].used = 1;
  time(&wdb->computer[index].lastconn);
  semaphore_release(wdb->semid);

  /* No errors, we (master) can receive the hwinfo from the remote */
  /* computer to be registered */
  answer.type = R_A_REGISTER;
  answer.data = RERR_NOERROR;
  if (!send_request (sfd,&answer,MASTER)) {
    log_master (L_WARNING,"Error sending request (handle_r_r_register)");
    exit (0);
  }
  
  recv_computer_hwinfo (sfd, &hwinfo, MASTER);
  semaphore_lock(wdb->semid);
  memcpy (&wdb->computer[index].hwinfo, &hwinfo, sizeof(hwinfo));
  wdb->computer[index].hwinfo.id = index;
  semaphore_release(wdb->semid);

  report_hwinfo(&wdb->computer[index].hwinfo);
}

void update_computer_status (struct computer *computer)
{
  /* The slave calls this function to update the information about */
  /* his own status on the master */
  struct request req;
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    log_slave_computer(L_ERROR,drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_UCSTATUS;
  if (!send_request (sfd,&req,SLAVE)) {
    log_slave_computer (L_WARNING,"Error sending request (update_computer_status)");
    kill(0,SIGINT);
  }
  if (!recv_request (sfd,&req)) {
    log_slave_computer (L_WARNING,"Error receiving request (update_computer_status)");
    kill(0,SIGINT);
  }

  if (req.type == R_A_UCSTATUS) {
    switch (req.data) {
    case RERR_NOERROR:
      send_computer_status (sfd,&computer->status,SLAVE);
      break;
    case RERR_NOREGIS:
      log_slave_computer (L_ERROR,"Computer not registered");
      kill (0,SIGINT);
    default:
      log_slave_computer (L_ERROR,"Error code not listed on answer to R_R_UCSTATUS");
      kill (0,SIGINT);
    }
  } else {
    log_slave_computer (L_ERROR,"Not appropiate answer to request R_R_UCSTATUS");
    kill (0,SIGINT);
  }
  close (sfd);
}

void register_slave (struct computer *computer)
{
  /* The slave calls this function to register himself on the master */
  struct request req;
  int sfd;

  log_slave_computer (L_DEBUG,"Entering register_slave");

  if ((sfd = connect_to_master ()) == -1) {
    log_slave_computer(L_ERROR,drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_REGISTER;

  if (!send_request (sfd,&req,SLAVE)) {
    log_slave_computer (L_WARNING,"Error sending request (register_slave)");
    kill (0,SIGINT);
  }
  if (!recv_request (sfd,&req)) {
    log_slave_computer (L_WARNING,"Error receiving request (register_slave)");
    kill (0,SIGINT);
  }

  if (req.type == R_A_REGISTER) {
    switch (req.data) {
    case RERR_NOERROR:
      send_computer_hwinfo (sfd,&computer->hwinfo,SLAVE);
      break;
    case RERR_ALREADY:
      log_slave_computer (L_ERROR,"Already registered");
      kill (0,SIGINT);
    case RERR_NOSPACE:
      log_slave_computer (L_ERROR,"No space on database");
      kill (0,SIGINT);
    default:
      log_slave_computer (L_ERROR,"Error code not listed on answer to R_R_REGISTER");
      kill (0,SIGINT);
    }
  } else {
    log_slave_computer (L_ERROR,"Not appropiate answer to request R_R_REGISTER");
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
    log_master (L_WARNING,"Not registered computer requesting update of computer status");
    answer.type = R_A_UCSTATUS;
    answer.data = RERR_NOREGIS;
    if (!send_request (sfd,&answer,MASTER)) {
      log_master (L_WARNING,"Error receiving request (handle_r_r_ucstatus)");
    }
    exit (0);
  }

  /* No errors, we (master) can receive the status from the remote */
  /* computer already registered */
  answer.type = R_A_UCSTATUS;
  answer.data = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);
  
  recv_computer_status (sfd, &status, MASTER);
  semaphore_lock(wdb->semid);
  memcpy (&wdb->computer[icomp].status, &status, sizeof(status));
  semaphore_release(wdb->semid);

/*    report_computer_status (&wdb->computer[icomp].status); */
}

int register_job (struct job *job)
{
  /* This function is called by drqman */
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

  if (req.type == R_A_REGISJOB) {
    switch (req.data) {
    case RERR_NOERROR:
      send_job (sfd,job,CLIENT);
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
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_regisjob (int sfd,struct database *wdb)
{
  /* The master handles this type of packages */
  struct request answer;
  struct job job;
  int index;
  char msg[BUFFERLEN];

  /* TO DO */
  /* Check if the job is already registered ! Or not ? */

  semaphore_lock(wdb->semid);	/* I put the lock here so no race condition can appear... */
  if ((index = job_index_free(wdb)) == -1) {
    /* No space left on database */
    log_master (L_WARNING,"No space left for job");
    answer.type = R_A_REGISJOB;
    answer.data = RERR_NOSPACE;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }
  wdb->job[index].used = 1;
  semaphore_release(wdb->semid);

  /* Debug */
  snprintf(msg,BUFFERLEN,"Job index %i free",index);
  log_master(L_DEBUG,msg);

  /* No errors, we (master) can receive the job from the remote */
  /* computer to be registered */
  answer.type = R_A_REGISJOB;
  answer.data = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);
  recv_job (sfd, &job, MASTER);

  job_init_registered (wdb,index,&job);

  job_report(&wdb->job[index]);
}


void handle_r_r_availjob (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of packages */
  struct request answer;
  uint32_t ijob,i;
  int itask;
  int iframe;
  char msg[BUFFERLEN];
  struct tpol pol[MAXJOBS];

  log_master (L_DEBUG,"Entering handle_r_r_availjob");

  if (icomp == -1) {
    log_master (L_WARNING,"Not registered computer requesting available job");
    answer.type = R_A_AVAILJOB;
    answer.data = RERR_NOREGIS;
    if (!send_request (sfd,&answer,MASTER)) {
      log_master (L_WARNING,"Error sending request (handle_r_r_availjob)");
    }
    exit (0);
  }

  for (i=0;i<MAXJOBS;i++) {
    pol[i].index = i;
    pol[i].pri = wdb->job[i].priority;
  }
  qsort ((void*)pol,MAXJOBS,sizeof(struct tpol),priority_job_compare);

  for (i=0;i<MAXJOBS;i++) {
    ijob = pol[i].index;
    /* ATENTION job_available sets the available frame as FS_ASSIGNED !! */
    /* We need to set it back to FS_WAITING if something fails */
    if (job_available(wdb,ijob,&iframe)) {
      snprintf(msg,BUFFERLEN,"Frame %i assigned",iframe);
      log_master_job(&wdb->job[ijob],L_INFO,msg);
      break;
    }
  }

  if (i==MAXJOBS) {
    log_master_computer(&wdb->computer[icomp],L_DEBUG,"No available job");
    answer.type = R_A_AVAILJOB;
    answer.data = RERR_NOAVJOB;
    if (!send_request (sfd,&answer,MASTER)) {
      log_master(L_WARNING,"Error sending request (handle_r_r_availjob)");
    }
    exit (0);
  } 

  log_master (L_DEBUG,"Available job. Sending request RERR_NOERROR");

  /* ijob is now the index to the first available job */
  answer.type = R_A_AVAILJOB;
  answer.data = RERR_NOERROR;
  if (!send_request (sfd,&answer,MASTER)) {
    log_master(L_WARNING,"Error sending request (handle_r_r_availjob)");
    job_frame_waiting (wdb,ijob,iframe); /* The reserved frame must be set back to waiting */
    exit (0);
  }

  log_master (L_DEBUG,"Receiving task availability");
  /* Now we receive if there is a task structure available */
  if (!recv_request (sfd,&answer)) {
    log_master(L_WARNING,"Error receiving request (handle_r_r_availjob)");
    job_frame_waiting (wdb,ijob,iframe);
    exit (0);
  }

  if (answer.type == R_A_AVAILJOB) {
    switch (answer.data) {
    case RERR_NOERROR:
      /* We continue processing the matter */
      log_master_computer(&wdb->computer[icomp],L_DEBUG,"Task space available");
      break;
    case RERR_NOSPACE:
      log_master_computer(&wdb->computer[icomp],L_WARNING,"No space for task");
      job_frame_waiting (wdb,ijob,iframe);
      exit (0);
    default:
      log_master_computer(&wdb->computer[icomp],L_ERROR,"Error code not listed expecting task error code");
      job_frame_waiting (wdb,ijob,iframe);
      exit(0);
    }
  } else {
    log_master_computer (&wdb->computer[icomp],L_ERROR,"Not appropiate answer, expecting task error code");
    job_frame_waiting (wdb,ijob,iframe);
    exit(0);
  }
  /* If there is a task structure available (we are here) then we receive the index of that task */
  if (!recv_request (sfd,&answer)) {
    log_master(L_WARNING,"Error receiving request (handle_r_r_availjob)");
    job_frame_waiting (wdb,ijob,iframe);
    exit (0);
  }
  if (answer.type == R_A_AVAILJOB) {
    itask = answer.data;
    snprintf(msg,BUFFERLEN,"Task index %i on computer %i",itask,icomp);
    log_master_computer(&wdb->computer[icomp],L_DEBUG,msg);
  } else {
    log_master_computer (&wdb->computer[icomp],L_ERROR,"Not appropiate answer, expecting task index");
    job_frame_waiting (wdb,ijob,iframe);
    exit (0);
  }

  log_master (L_DEBUG,"Updating structures to be sent");
  semaphore_lock(wdb->semid);
  job_update_assigned (wdb,ijob,iframe,icomp,itask);
  computer_update_assigned (wdb,ijob,iframe,icomp,itask);
  semaphore_release(wdb->semid);

  job_update_info(wdb,ijob);

  log_master_computer (&wdb->computer[icomp],L_DEBUG,"Sending updated task");
  if (!send_task (sfd,&wdb->computer[icomp].status.task[itask])) {
    log_master_computer (&wdb->computer[icomp],L_ERROR,drerrno_str());
    job_frame_waiting (wdb,ijob,iframe);
    exit (0);
  }    
}


int request_job_available (struct slave_database *sdb)
{
  /* Here we (slave) ask the master for an available job and in case */
  /* of finding it we store the info into *job, and fill the task record */
  /* except what cannot be filled until the proper execution of the task */

  /* This function SETS sdb->itask local to this process */
  /* This function returns 0 if there is no job available */

  struct request req;
  int sfd;
  struct task ttask;		/* Temporary task structure */

  log_slave_computer (L_DEBUG,"Entering request_job_available");

  if ((sfd = connect_to_master ()) == -1) {
    log_slave_computer(L_ERROR,drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_AVAILJOB;

  if (!send_request (sfd,&req,SLAVE)) {
    log_slave_computer (L_ERROR,drerrno_str());
    kill (0,SIGINT);
  }
  if (!recv_request (sfd,&req)) {
    log_slave_computer (L_ERROR,drerrno_str());
    kill (0,SIGINT);
  }

  if (req.type == R_A_AVAILJOB) {
    switch (req.data) {
    case RERR_NOERROR: 
      /* We continue processing the matter */
      log_slave_computer(L_DEBUG,"Available job");
      break;
    case RERR_NOAVJOB:
      log_slave_computer(L_DEBUG,"No available job");
      close (sfd);
      return 0;
    case RERR_NOREGIS:
      log_slave_computer(L_ERROR,"Computer not registered");
      kill (0,SIGINT);
      break;
    default:
      log_slave_computer(L_ERROR,"Error code not listed on answer to R_R_AVAILJOB");
      kill (0,SIGINT);
    }
  } else {
    fprintf (stderr,"ERROR: Not appropiate answer to request R_R_REGISJOB\n");
    kill (0,SIGINT);
  }

  if ((sdb->itask = task_available (sdb)) == -1) {
    /* No task structure available */
    log_slave_computer(L_WARNING,"No task available for job");
    req.type = R_A_AVAILJOB;
    req.data = RERR_NOSPACE;
    if (!send_request(sfd,&req,SLAVE)) {
      log_slave_computer (L_ERROR,drerrno_str());
      kill (0,SIGINT);
    }
    close (sfd);		/* Finish */
    return 0;
  }

  log_slave_computer (L_DEBUG,"There is an available task. Sending RERR_NOERROR");
  /* We've got an available task */
  req.type = R_A_AVAILJOB;
  req.data = RERR_NOERROR;
  if (!send_request(sfd,&req,SLAVE)) {
    log_slave_computer (L_ERROR,drerrno_str());
    kill (0,SIGINT);
  }

  log_slave_computer (L_DEBUG,"Sending index to task.");
  /* So then we send the index */
  req.data = sdb->itask;
  if (!send_request(sfd,&req,SLAVE)) {
    log_slave_computer (L_ERROR,drerrno_str());
    kill (0,SIGINT);
  }
  
  log_slave_computer (L_DEBUG,"Receiving the task");
  /* Then we receive the task */
  if (!recv_task(sfd,&ttask)) {
    log_slave_computer (L_ERROR,drerrno_str());
    kill (0,SIGINT);
  }

  /* The we update the computer structure to reflect the new assigned task */
  /* that is not yet runnning so pid == 0 */
  semaphore_lock(sdb->semid);
  memcpy(&sdb->comp->status.task[sdb->itask],&ttask,sizeof(ttask));
  sdb->comp->status.ntasks++;
  semaphore_release(sdb->semid);

  close (sfd);
  return 1;
}

void request_task_finished (struct slave_database *sdb)
{
  /* This function is called non-blocked */
  /* This function is called from inside a slave launcher process */
  /* It sends the information to the master about a finished task */
  /* FIXME: It doesn't check if the computer is not registered, should we ? */
  /* We are just sending a finished task, so it does not really matter if the computer */
  /* is not registered anymore */
  struct request req;
  int sfd;

  log_slave_computer(L_DEBUG,"Entering request_task_finished");

 begin:
  if ((sfd = connect_to_master ()) == -1) {
    log_slave_computer(L_ERROR,drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_TASKFINI;

  if (!send_request (sfd,&req,SLAVE_LAUNCHER)) {
    log_slave_computer (L_ERROR,"sending request (request_task_finished)");
    kill(0,SIGINT);
  }

  if (!recv_request (sfd,&req)) {
    log_slave_computer (L_ERROR,"receiving request (request_task_finished)");
    kill(0,SIGINT);
  }
    
  if (req.type == R_A_TASKFINI) {
    switch (req.data) {
    case RERR_NOERROR: 
      /* We continue processing the matter */
      log_slave_computer(L_DEBUG,"Master ready to receive the task");
      break;
    case RERR_NOREGIS:
      log_slave_computer(L_ERROR,"Job not registered");
      exit (0);
    case RERR_NOTINRA:
      log_slave_computer(L_ERROR,"Frame out of range");
      exit (0);
    default:
      log_slave_computer(L_ERROR,"Error code not listed on answer to R_R_TASKFINI");
      exit (0);
    }
  } else {
    fprintf (stderr,"ERROR: Not appropiate answer to request R_R_TASKFINI\n");
    exit (0);
  }

  /* So the master is ready to receive the task */
  /* Then we send the task */
  if (!send_task (sfd,&sdb->comp->status.task[sdb->itask])) {
    /* We should retry, but really there should be no errors here */
    log_slave_computer (L_WARNING,"Retrying R_R_TASKFINI because of error sending the task");
    close (sfd);
    goto begin;
  }

  close (sfd);
}

void handle_r_r_taskfini (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of requests */
  /* This funtion is called by the master, non-blocked */
  struct request answer;
  struct task task;
  struct frame_info *fi;
  char msg[BUFFERLEN];

  log_master (L_DEBUG,"Entering handle_r_r_taskfini");

  if (icomp == -1) {
    /* We log and continue */
    log_master (L_WARNING,"Not registered computer requesting task finished");
  }

  /* Alway send RERR_NOERR */
  answer.type = R_A_TASKFINI;
  answer.data = RERR_NOERROR;
  if (!send_request (sfd,&answer,MASTER)) {
    log_master (L_ERROR,"Error receiving request (handle_r_r_taskfini)");
    return;
  }

  /* Receive task */
  if (!recv_task(sfd,&task)) {
    log_master (L_ERROR,"Error receiving task (handle_r_r_taskfini)");
    return;
  }

  if (task.ijob >= MAXJOBS)
    return;

  semaphore_lock(wdb->semid);
  if ((!wdb->job[task.ijob].used) 
      || (strcmp(task.jobname,wdb->job[task.ijob].name) != 0)) {
    log_master (L_WARNING,"frame finished of non-existing job");
    return;
  }
  log_master (L_DEBUG,"Job index correct");

  /* Once we have the task struct we need to update the information */
  /* on the job struct */
  if (task.ijob > MAXJOBS) { /* ijob is always > 0 */
    /* task.ijob out of range ! */
    log_master (L_ERROR,"ijob out of range in handle_r_r_taskfini");
  }

  fi = attach_frame_shared_memory(wdb->job[task.ijob].fishmid);
  if (job_frame_number_correct (&wdb->job[task.ijob],task.frame)) {
    log_master (L_DEBUG,"Frame number correct");
    /* Frame is in range */
    task.frame = job_frame_number_to_index (&wdb->job[task.ijob],task.frame);/* frame converted to index frame */
    /* Now we should check the exit code to act accordingly */
    if (DR_WIFEXITED(task.exitstatus)) {
      fi[task.frame].status = FS_FINISHED;
      fi[task.frame].exitcode = DR_WEXITSTATUS(task.exitstatus);
      time(&fi[task.frame].end_time);
    } else {
      /* Process exited abnormally either killed by us or by itself (SIGSEGV) */
      if (DR_WIFSIGNALED(task.exitstatus)) {
	int sig = DR_WTERMSIG(task.exitstatus);
	snprintf(msg,BUFFERLEN-1,"Signaled with %i",sig);
	log_master_job (&wdb->job[task.ijob],L_DEBUG,msg);
	if ((sig == SIGTERM) || (sig == SIGINT) || (sig == SIGKILL)) {
	  /* Somebody killed the process, so it should be retried */
	  snprintf(msg,BUFFERLEN-1,"Retrying frame %i", job_frame_index_to_number (&wdb->job[task.ijob],task.frame));
	  log_master_job (&wdb->job[task.ijob],L_INFO,msg);
	  fi[task.frame].status = FS_WAITING;
	} else {
	  snprintf(msg,BUFFERLEN-1,"Frame %i died signal not catched", 
		   job_frame_index_to_number (&wdb->job[task.ijob],task.frame));
	  log_master_job (&wdb->job[task.ijob],L_INFO,msg);
	  fi[task.frame].status = FS_ERROR;
	  time(&fi[task.frame].end_time);
	}
      } else {
	/* This must be WIFSTOPPED, but I'm not sure */
	snprintf(msg,BUFFERLEN-1,"Frame %i died abnormally", 
		 job_frame_index_to_number (&wdb->job[task.ijob],task.frame));
	log_master_job (&wdb->job[task.ijob],L_INFO,msg);
	fi[task.frame].status = FS_ERROR;
	time(&fi[task.frame].end_time);
      }
    }
  } else {
    log_master (L_ERROR,"frame out of range in handle_r_r_taskfini");
  }
  semaphore_release(wdb->semid);
  
  log_master (L_DEBUG,"Everything right. Calling job_update_info.");
  job_update_info(wdb,task.ijob);
}

void handle_r_r_listjobs (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  struct request answer;
  int i;

  log_master (L_DEBUG,"Entering handle_r_r_listjobs");

  answer.type = R_A_LISTJOBS;
  answer.data = job_njobs_masterdb (wdb);
  
  if (!send_request (sfd,&answer,MASTER)) {
    log_master (L_ERROR,"Error receiving request (handle_r_r_listjobs)");
    return;
  }

  for (i=0;i<MAXJOBS;i++) {
    if (wdb->job[i].used) {
      send_job (sfd,&wdb->job[i],MASTER);
    }
  }
}

void handle_r_r_listcomp (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  struct request answer;
  int i;

  answer.type = R_A_LISTCOMP;
  answer.data = computer_ncomputers_masterdb (wdb);
  
  if (!send_request (sfd,&answer,MASTER)) {
    log_master (L_ERROR,"Error receiving request (handle_r_r_listcomp)");
    return;
  }

  for (i=0;i<MAXCOMPUTERS;i++) {
    if (wdb->computer[i].used) {
      send_computer (sfd,&wdb->computer[i],MASTER);
    }
  }
}

int request_job_delete (uint32_t ijob, int who)
{
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
    drerrno = DRE_ERRORSENDING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_deletjob (int sfd,struct database *wdb,int icomp,struct request *req)
{
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  int i;
  uint32_t ijob;
  struct frame_info *fi;
  int nframes;

  ijob = req->data;

  if (ijob >= MAXJOBS)
    return;

  semaphore_lock (wdb->semid);

  if (wdb->job[ijob].used) {
    fi = attach_frame_shared_memory (wdb->job[ijob].fishmid);
    nframes = job_nframes (&wdb->job[ijob]);
    for (i=0;i<nframes;i++) {
      if (fi[i].status == FS_ASSIGNED) {
	request_slave_killtask (wdb->computer[fi[i].icomp].hwinfo.name,fi[i].itask);
      }
    }
    detach_frame_shared_memory (fi);

    job_delete (&wdb->job[ijob]);
  } else {
    log_master (L_INFO,"Request for deletion of an unused job");
  }

  semaphore_release (wdb->semid);
}

int request_slave_killtask (char *slave,uint16_t itask)
{
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

  if (!send_request (sfd,&request,MASTER)) {
    drerrno = DRE_ERRORSENDING;
    return 0;
  }
  
  return 1;
}

void handle_rs_r_killtask (int sfd,struct slave_database *sdb,struct request *req)
{
  /* This function is called by the slave unlocked */
  kill(-sdb->comp->status.task[req->data].pid,SIGINT);
}

int request_job_stop (uint32_t ijob, int who)
{
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
    drerrno = DRE_ERRORSENDING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int request_job_continue (uint32_t ijob, int who)
{
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
    drerrno = DRE_ERRORSENDING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

void handle_r_r_stopjob (int sfd,struct database *wdb,int icomp,struct request *req)
{
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

void handle_r_r_contjob (int sfd,struct database *wdb,int icomp,struct request *req)
{
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

void handle_r_r_hstopjob (int sfd,struct database *wdb,int icomp,struct request *req)
{
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  int i;
  uint32_t ijob;
  struct frame_info *fi;
  int nframes;

  ijob = req->data;
  
  if (ijob >= MAXJOBS)
    return;

  semaphore_lock (wdb->semid);

  if (wdb->job[ijob].used) {
    fi = attach_frame_shared_memory (wdb->job[ijob].fishmid);
    nframes = job_nframes (&wdb->job[ijob]);
    for (i=0;i<nframes;i++) {
      if (fi[i].status == FS_ASSIGNED) {
	request_slave_killtask (wdb->computer[fi[i].icomp].hwinfo.name,fi[i].itask);
      }
    }
    detach_frame_shared_memory (fi);

    job_stop (&wdb->job[ijob]);
  }

  semaphore_release (wdb->semid);
}

int request_job_hstop (uint32_t ijob, int who)
{
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
    drerrno = DRE_ERRORSENDING;
    close (sfd);
    return 0;
  }

  close (sfd);
  return 1;
}

int request_job_xfer (uint32_t ijob, struct job *job, int who)
{
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

  if (req.type == R_A_JOBXFER) {
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
  
  recv_job (sfd,job,who);

  close (sfd);
  return 1;
}

void handle_r_r_jobxfer (int sfd,struct database *wdb,int icomp,struct request *req)
{
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;

  log_master (L_DEBUG,"Entering handle_r_r_jobxfer");

  ijob = req->data;
  
  if ((ijob >= MAXJOBS) || !wdb->job[ijob].used) {
    log_master (L_INFO,"Job asked to be transfered does not exist");
    req->type = R_A_JOBXFER;
    req->data = RERR_NOREGIS;
    if (!send_request(sfd,req,MASTER))
      return;
    return;
  }

  req->type = R_A_JOBXFER;
  req->data = RERR_NOERROR;
  if (!send_request(sfd,req,MASTER))
    return;

  log_master (L_DEBUG,"Sending job");
  send_job (sfd,&wdb->job[ijob],MASTER);
}

int request_job_xferfi (uint32_t ijob, struct frame_info *fi, int nframes, int who)
{
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

  if (req.type == R_A_JOBXFERFI) {
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

void handle_r_r_jobxferfi (int sfd,struct database *wdb,int icomp,struct request *req)
{
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t ijob;
  struct frame_info *fi;
  int nframes;
  int i;

  log_master (L_DEBUG,"Entering handle_r_r_jobxferfi");

  ijob = req->data;
  
  if ((ijob >= MAXJOBS) || !wdb->job[ijob].used) {
    log_master (L_INFO,"Job asked to be transfered frame info does not exist");
    req->type = R_A_JOBXFERFI;
    req->data = RERR_NOREGIS;
    if (!send_request(sfd,req,MASTER))
      return;
  }

  req->type = R_A_JOBXFERFI;
  req->data = RERR_NOERROR;
  if (!send_request(sfd,req,MASTER))
    return;

  fi = attach_frame_shared_memory (wdb->job[ijob].fishmid);
  nframes = job_nframes (&wdb->job[ijob]);

  log_master (L_DEBUG,"Sending frame info");
  for (i=0;i<nframes;i++) {
    if (!send_frame_info (sfd,fi)) {
      log_master (L_ERROR,"Sending frame info");
      return;
    }
    fi++;
  }

  detach_frame_shared_memory(fi);
}

int request_comp_xfer (uint32_t icomp, struct computer *comp, int who)
{
  /* This function can be called by anyone */
  struct request req;
  int sfd;

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

  if (req.type == R_A_COMPXFER) {
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
  
  recv_computer (sfd,comp,who);

  close (sfd);
  return 1;
}

void handle_r_r_compxfer (int sfd,struct database *wdb,int icomp,struct request *req)
{
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  uint32_t icomp2;		/* The id of the computer asked to be transfered */

  log_master (L_DEBUG,"Entering handle_r_r_compxfer");

  icomp2 = req->data;
  
  if ((icomp2 >= MAXCOMPUTERS) || !wdb->computer[icomp2].used) {
    log_master_computer(&wdb->computer[icomp],L_INFO,"Computer asked to be transfered is not registered");
    req->type = R_A_COMPXFER;
    req->data = RERR_NOREGIS;
    send_request(sfd,req,MASTER);
    return;
  }

  req->type = R_A_COMPXFER;
  req->data = RERR_NOERROR;
  if (!send_request(sfd,req,MASTER))
    return;

  log_master (L_DEBUG,"Sending computer");
  send_computer (sfd,&wdb->computer[icomp2],MASTER);
}


