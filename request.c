/* $Id: request.c,v 1.15 2001/07/20 08:27:59 jorge Exp $ */
/* For the differences between data in big endian and little endian */
/* I transmit everything in network byte order */

#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <wait.h>

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

  recv_request (sfd,&request,MASTER);
  switch (request.type) {
  case R_R_REGISTER:
    log_master (L_INFO,"Registration of new slave request");
    handle_r_r_register (sfd,wdb,icomp);
    break;
  case R_R_UCSTATUS:
    log_master (L_INFO,"Update computer status request");
    handle_r_r_ucstatus (sfd,wdb,icomp);
    break;
  case R_R_REGISJOB:
    log_master (L_INFO,"Registration of new job request");
    handle_r_r_regisjob (sfd,wdb);
    exit (0);			/* Because we don't want to update the lastconn time */
				/* The process that sends this request is not a slave */
  case R_R_AVAILJOB:
    log_master (L_INFO,"Request of available job");
    handle_r_r_availjob (sfd,wdb,icomp);
    break;
  case R_R_TASKFINI:
    log_master (L_INFO,"Request task finished");
    handle_r_r_taskfini (sfd,wdb,icomp);
    break;
  case R_R_LISTJOBS:
    log_master (L_INFO,"Request list of jobs");
    handle_r_r_listjobs (sfd,wdb,icomp);
    break;
  case R_R_LISTCOMP:
    log_master (L_INFO,"Request list of computers");
    handle_r_r_listcomp (sfd,wdb,icomp);
    break;
  default:
    log_master (L_WARNING,"Unknown request");
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
  /* The master handles this type of requests */
  struct request answer;
  struct computer_hwinfo hwinfo;
  int index;

  log_master (L_DEBUG,"Entering handle_r_r_register");

  if (icomp != -1) {
    log_master (L_INFO,"Already registered computer requesting registration");
    answer.type = R_A_REGISTER;
    answer.data_s = RERR_ALREADY;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }

  semaphore_lock(wdb->semid);	/* I put the lock here so no race condition can appear... */
  if ((index = computer_index_free(wdb)) == -1) {
    /* No space left on database */
    log_master (L_WARNING,"No space left for computer");
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
  int sfd;

  if ((sfd = connect_to_master ()) == -1) {
    log_slave_computer(L_ERROR,drerrno_str());
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
      log_slave_computer (L_ERROR,"Computer not registered");
      break;
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

  send_request (sfd,&req,SLAVE);
  recv_request (sfd,&req,SLAVE);
  if (req.type == R_A_REGISTER) {
    switch (req.data_s) {
    case RERR_NOERROR:
      send_computer_hwinfo (sfd,&computer->hwinfo,SLAVE);
      break;
    case RERR_ALREADY:
      log_slave_computer (L_ERROR,"Already registered");
      kill (0,SIGINT);
      break;
    case RERR_NOSPACE:
      log_slave_computer (L_ERROR,"No space on database");
      kill (0,SIGINT);
      break;
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

/*    report_computer_status (&wdb->computer[icomp].status); */
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
    fprintf (stderr,"ERROR: Not appropiate answer to request R_R_REGISJOB\n");
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
  char msg[BUFFERLEN];

  /* TO DO */
  /* Check if the job is already registered ! Or not ? */

  semaphore_lock(wdb->semid);	/* I put the lock here so no race condition can appear... */
  if ((index = job_index_free(wdb)) == -1) {
    /* No space left on database */
    log_master (L_WARNING,"No space left for job");
    answer.type = R_A_REGISJOB;
    answer.data_s = RERR_NOSPACE;
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
  answer.data_s = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);
  recv_job (sfd, &job, MASTER);

  job_init_registered (wdb,index,&job);

  job_report(&wdb->job[index]);
}

void handle_r_r_availjob (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of packages */
  struct request answer;
  uint32_t ijob;
  int itask;
  int iframe;
  char msg[BUFFERLEN];

  log_master (L_DEBUG,"Entering handle_r_r_availjob");

  if (icomp == -1) {
    log_master (L_WARNING,"Not registered computer requesting available job");
    answer.type = R_A_AVAILJOB;
    answer.data_s = RERR_NOREGIS;
    send_request (sfd,&answer,MASTER);
    exit (0);
  }

  for (ijob=0;ijob<MAXJOBS;ijob++) {
    if (job_available(wdb,ijob,&iframe)) {
      snprintf(msg,BUFFERLEN,"Frame %i assigned",iframe);
      log_master_job(&wdb->job[ijob],L_INFO,msg);
      break;
    }
  }

  if (ijob==MAXJOBS) {
    log_master(L_DEBUG,"No available job");
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
      log_master_computer(&wdb->computer[icomp],L_DEBUG,"Task space available");
      break;
    case RERR_NOSPACE:
      log_master_computer(&wdb->computer[icomp],L_WARNING,"No space for task");
      exit (0);
    default:
      log_master_computer(&wdb->computer[icomp],L_ERROR,"Error code not listed expecting task error code");
      exit(0);
    }
  } else {
    log_master_computer (&wdb->computer[icomp],L_ERROR,"Not appropiate answer, expecting task error code");
    exit(0);
  }
  /* If there is a task structure available (we are here) then we receive the index of that task */
  recv_request (sfd,&answer,MASTER);
  if (answer.type == R_A_AVAILJOB) {
    itask = answer.data_s;
    snprintf(msg,BUFFERLEN,"Task index %i on computer %i",itask,icomp);
    log_master_computer(&wdb->computer[icomp],L_DEBUG,msg);
  } else {
    log_master_computer (&wdb->computer[icomp],L_ERROR,"Not appropiate answer, expecting task index");
    exit (0);
  }

  semaphore_lock(wdb->semid);
  job_update_assigned (wdb,ijob,iframe,icomp,itask);
  computer_update_assigned (wdb,ijob,iframe,icomp,itask);
  semaphore_release(wdb->semid);

  job_update_info(wdb,ijob);

  send_task (sfd,&wdb->computer[icomp].status.task[itask],MASTER);
}


int request_job_available (struct slave_database *sdb)
{
  /* Here we (slave) ask the master for an available job and in case */
  /* of finding it we store the info into *job, and fill the task record */
  /* except what cannot be filled until the proper execution of the task */

  /* This funtion SETS sdb->itask local to this process */

  struct request req;
  int sfd;
  struct task ttask;		/* Temporary task structure */

  if ((sfd = connect_to_master ()) == -1) {
    log_slave_computer(L_ERROR,drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_AVAILJOB;

  send_request (sfd,&req,SLAVE);
  recv_request (sfd,&req,SLAVE);

  if (req.type == R_A_AVAILJOB) {
    switch (req.data_s) {
    case RERR_NOERROR: 
      /* We continue processing the matter */
      log_slave_computer(L_DEBUG,"Available job");
      break;
    case RERR_NOAVJOB:
      log_slave_computer(L_INFO,"No available job");
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
    req.data_s = RERR_NOSPACE;
    send_request(sfd,&req,SLAVE);
    close (sfd);		/* Finish */
    return 0;
  }

  /* We've got an available task */
  req.type = R_A_AVAILJOB;
  req.data_s = RERR_NOERROR;
  send_request(sfd,&req,SLAVE);

  /* So then we send the index */
  req.data_s = sdb->itask;
  send_request(sfd,&req,SLAVE);

  /* Then we receive the task */
  recv_task(sfd,&ttask,SLAVE);

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

  if ((sfd = connect_to_master ()) == -1) {
    log_slave_computer(L_ERROR,drerrno_str());
    kill(0,SIGINT);
  }

  req.type = R_R_TASKFINI;

  send_request (sfd,&req,SLAVE_LAUNCHER);
  recv_request (sfd,&req,SLAVE_LAUNCHER);

  if (req.type == R_A_TASKFINI) {
    switch (req.data_s) {
    case RERR_NOERROR: 
      /* We continue processing the matter */
      log_slave_computer(L_DEBUG,"Master ready to receive the task");
      break;
    case RERR_NOREGIS:
      log_slave_computer(L_ERROR,"Job not registered");
      exit (0);
      break;
    case RERR_NOTINRA:
      log_slave_computer(L_ERROR,"Frame out of range");
      exit (0);
      break;
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
  send_task (sfd,&sdb->comp->status.task[sdb->itask],SLAVE_LAUNCHER);

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

  if (icomp == -1) {
    /* We log and continue */
    log_master (L_WARNING,"Not registered computer requesting task finished");
  }

  /* Alway send RERR_NOERR */
  answer.type = R_A_TASKFINI;
  answer.data_s = RERR_NOERROR;
  send_request (sfd,&answer,MASTER);

  /* Receive task */
  recv_task(sfd,&task,MASTER);

  /* Once we have the task struct we need to update the information */
  /* on the job struct */
  semaphore_lock(wdb->semid);
  if (task.jobindex > MAXJOBS) { /* jobindex is always > 0 */
    /* task.jobindex out of range ! */
    log_master (L_ERROR,"jobindex out of range in handle_r_r_taskfini");
  }

  fi = attach_frame_shared_memory(wdb->job[task.jobindex].fishmid);
  if ((task.frame >= wdb->job[task.jobindex].frame_start)
      || (task.frame <= wdb->job[task.jobindex].frame_end)) {
    /* Frame is in range */
    task.frame -= wdb->job[task.jobindex].frame_start;
    /* Now we should check the exit code to act accordingly */
    if (WIFEXITED(task.exitstatus)) {
      fi[task.frame].status = FS_FINISHED;
      fi[task.frame].exitcode = WEXITSTATUS(task.exitstatus);
      time(&fi[task.frame].end_time);
    } else {
      /* Process exited abnormally either killed by us or by itself (SIGSEGV) */
      if (WIFSIGNALED(task.exitstatus)) {
	int sig = WTERMSIG(task.exitstatus);
	if ((sig == SIGTERM) || (sig == SIGINT) || (sig == SIGKILL)) {
	  /* Somebody killed the process, so it should be retried */
	  snprintf(msg,BUFFERLEN-1,"Retrying frame %i", task.frame + wdb->job[task.jobindex].frame_start);
	  log_master_job (&wdb->job[task.jobindex],L_INFO,msg);
	  fi[task.frame].status = FS_WAITING;
	} else {
	  snprintf(msg,BUFFERLEN-1,"Frame %i died signal not catched", task.frame + wdb->job[task.jobindex].frame_start);
	  log_master_job (&wdb->job[task.jobindex],L_INFO,msg);
	  fi[task.frame].status = FS_ERROR;
	  time(&fi[task.frame].end_time);
	}
      } else {
	/* This must be WIFSTOPPED, but I'm not sure */
	snprintf(msg,BUFFERLEN-1,"Frame %i died abnormally", task.frame + wdb->job[task.jobindex].frame_start);
	log_master_job (&wdb->job[task.jobindex],L_INFO,msg);
	fi[task.frame].status = FS_ERROR;
	time(&fi[task.frame].end_time);
      }
    }
  } else {
    log_master (L_ERROR,"frame out of range in handle_r_r_taskfini");
  }
  semaphore_release(wdb->semid);

  job_update_info(wdb,task.jobindex);
}

void handle_r_r_listjobs (int sfd,struct database *wdb,int icomp)
{
  /* The master handles this type of packages */
  /* This function is called unlocked */
  /* This function is called by the master */
  struct request answer;
  int i;

  answer.type = R_A_LISTJOBS;
  answer.data_s = job_njobs_masterdb (wdb);
  
  send_request (sfd,&answer,MASTER);
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
  answer.data_s = computer_ncomputers_masterdb (wdb);
  
  send_request (sfd,&answer,MASTER);
  for (i=0;i<MAXCOMPUTERS;i++) {
    if (wdb->computer[i].used) {
      send_computer (sfd,&wdb->computer[i],MASTER);
    }
  }
}

