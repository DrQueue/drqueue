/* $Id: sendjob.c,v 1.7 2002/02/26 15:52:05 jorge Exp $ */
/* To set up a job from a terminal and send it to the master */
/* I'd like it to be curses based */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
# else
#  error You need to define the OS, or OS defined not supported
# endif
#endif

#include "sendjob.h"
#include "libdrqueue.h"

int main (int argc,char *argv[])
{
  struct job job;

  presentation();

  do {
    jobinfo_get(&job);
    job_report(&job);
    printf ("Is this information correct ? (y/n) ");
  } while (getchar() != 'y');

  register_job (&job);

  exit (0);
}

void presentation (void)
{
  printf ("DrQueue - by Digital Renaissance\n\n");
  printf ("The information you enter here will be sent to the master process\n");
}

void jobinfo_get (struct job *job)
{
  input_get_word (job->name,MAXNAMELEN,"Job name ? ");
  input_get_line (job->cmd,MAXCMDLEN,"Job command ? ");
  input_get_uint32 (&job->frame_start,"Start frame ? ");
  input_get_uint32 (&job->frame_end,"End frame ? ");
  input_get_uint32 (&job->priority,"Priority ? ");
  strncpy (job->owner,getlogin(),MAXNAMELEN-1);
  job->owner[MAXNAMELEN-1] = 0;
  job->status = JOBSTATUS_WAITING;
  job->frame_info = NULL;
}

void input_get_word (char *res,int len,char *question)
{
  char buf[len];
  int n;

  do {
    printf ("%s",question);
    fgets (buf,len,stdin);
    n = sscanf (buf,"%s",res);
  } while (n != 1);
  /* clean the buffer */
  while (strchr (buf,'\n') == NULL) {
    fgets (buf,len,stdin);
  }
}

void input_get_line (char *res,int len,char *question)
{
  char buf[len];

  printf ("%s",question);
  fgets (buf,len,stdin);

  strncpy(res,buf,len-1);
}


void input_get_uint32 (uint32_t *res,char *question)
{
  char buf[BUFFERLEN];
  int n;

  do {
    printf ("%s",question);
    fgets (buf,BUFFERLEN,stdin);
    n = sscanf (buf,"%i",res);
  } while (n != 1);
  /* clean the buffer */
  while (strchr (buf,'\n') == NULL) {
    fgets (buf,BUFFERLEN,stdin);
  }
}

void cleanup (int signum)
{
  exit(0);
}



