/* $Id: drqm_request.c,v 1.9 2001/08/28 15:36:28 jorge Exp $ */

#include <stdlib.h>
#include <unistd.h>

#include <libdrqueue.h>

#include "drqm_jobs.h"
#include "drqm_computers.h"
#include "drqm_request.h"

void drqm_request_joblist (struct info_drqm_jobs *info)
{
  /* This function is called non-blocked */
  /* This function is called from inside drqman */
  struct request req;
  int sfd;
  struct job *tjob;	
  int i;

  if ((sfd = connect_to_master ()) == -1) {
    fprintf(stderr,"%s\n",drerrno_str());
    return;
  }

  req.type = R_R_LISTJOBS;

  if (!send_request (sfd,&req,CLIENT))
    goto end;

  if (!recv_request (sfd,&req))
    goto end;

  if (req.type == R_A_LISTJOBS) {
    info->njobs = req.data;
  } else {
    fprintf (stderr,"ERROR: Not appropiate answer to request R_R_TASKFINI\n");
    goto end;			/* Should I use gotos ? It seems like a reasonable option for this case */
  }

  clean_joblist (info);
  if ((info->jobs = malloc (sizeof (struct job) * info->njobs)) == NULL) {
    fprintf (stderr,"Not enough memory for job structures\n");
    goto end;
  }
  tjob = info->jobs;
  for (i=0;i<info->njobs;i++) {
    recv_job (sfd,tjob,CLIENT);
    tjob++;
  }

 end:
  close (sfd);
}

void clean_joblist (struct info_drqm_jobs *info)
{
  if (info->jobs) {
    free (info->jobs);
    info->jobs = NULL;
  }
}

void drqm_request_computerlist (struct info_drqm_computers *info)
{
  /* This function is called non-blocked */
  /* This function is called from inside drqman */
  struct request req;
  int sfd;
  struct computer *tcomputer;	
  int i;

  if ((sfd = connect_to_master ()) == -1) {
    fprintf(stderr,"%s\n",drerrno_str());
    return;
  }

  req.type = R_R_LISTCOMP;

  if (!send_request (sfd,&req,CLIENT)) {
    fprintf(stderr,"%s\n",drerrno_str());
    goto end;
  }
  if (!recv_request (sfd,&req)) {
    fprintf(stderr,"%s\n",drerrno_str());
    goto end;
  }

  if (req.type == R_A_LISTCOMP) {
    info->ncomputers = req.data;
  } else {
    fprintf (stderr,"ERROR: Not appropiate answer to request R_R_TASKFINI\n");
    goto end;			/* Should I use gotos ? It seems like a reasonable option for this case */
  }

  clean_computerlist (info);
  if ((info->computers = malloc (sizeof (struct computer) * info->ncomputers)) == NULL) {
    fprintf (stderr,"Not enough memory for job structures\n");
    goto end;
  }
  tcomputer = info->computers;
  for (i=0;i<info->ncomputers;i++) {
    recv_computer (sfd,tcomputer,CLIENT);
    tcomputer++;
  }

 end:
  close (sfd);
}

void clean_computerlist (struct info_drqm_computers *info)
{
  if (info->computers) {
    free (info->computers);
    info->computers = NULL;
  }
}

void drqm_request_job_delete (struct info_drqm_jobs *info)
{
  /* This function sends the request to delete the job selected from the queue */
/*    int n; */
/*    char *buf; */

  if (info->njobs) {
/*      gtk_clist_get_text(GTK_CLIST(info->clist),info->row,0,&buf); */
/*      n = atoi (buf); */
    request_job_delete ((uint32_t)info->jobs[info->row].id,CLIENT);
  }
}

void drqm_request_job_stop (struct info_drqm_jobs *info)
{
  /* This function sends the request to stop the job selected from the queue */
  if (info->njobs) {
    request_job_stop ((uint32_t)info->jobs[info->row].id,CLIENT);
  }
}

void drqm_request_job_hstop (struct info_drqm_jobs *info)
{
  /* This function sends the request to hard stop the job selected from the queue */
  if (info->njobs) {
    request_job_hstop ((uint32_t)info->jobs[info->row].id,CLIENT);
  }
}

void drqm_request_job_continue (struct info_drqm_jobs *info)
{
  /* This function sends the request to continue the (stopped) job selected from the queue */
  if (info->njobs) {
    request_job_continue ((uint32_t)info->jobs[info->row].id,CLIENT);
  }
}



