/* $Id: common.c,v 1.6 2001/11/14 14:23:37 jorge Exp $ */

#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "drerrno.h"
#include "constants.h"

int common_environment_check (void)
{
  char *buf;

  if ((buf = getenv("DRQUEUE_MASTER")) == NULL) {
    drerrno = DRE_NOENVMASTER;
    return 0;
  }

  if ((buf = getenv("DRQUEUE_ROOT")) == NULL) {
    drerrno = DRE_NOENVROOT;
    return 0;
  }

  return 1;
}

void show_version (char **argv)
{
  printf ("\nDistributed Rendering Queue\n");
  printf ("by Triple-e VFX\n\n");
  printf ("%s version: %s\n",argv[0],VERSION);
  printf ("\n");
}

int remove_dir (char *dir)
{
  /* Removes a directory recursively */
  char cmd[BUFFERLEN];

  snprintf (cmd,BUFFERLEN,"rm -fR %s",dir);

  return system (cmd);
}

void mn_job_finished (struct job *job)
{
  FILE *mail;
  char command[BUFFERLEN];
  uint32_t total;
  char total_str[BUFFERLEN];

  snprintf (command,BUFFERLEN-1,"mail -s 'Your job (%s) is finished' %s",job->name,job->owner);

  if ((mail = popen (command,"w")) == NULL) {
    /* There was a problem creating the piped command */
    return;
  }

  total = job->avg_frame_time*job_nframes(job);
  snprintf (total_str,BUFFERLEN-1,"%s",time_str(total));

  fprintf (mail,"\n"
	   "Total number of frames: %i\n"
	   "The average time per frame was: %s\n"
	   "And the total rendering time: %s\n"
	   "\n.\n\n",job_nframes(job),time_str(job->avg_frame_time),total_str);

  pclose (mail);
}

char *time_str (uint32_t nseconds)
{
  static char msg[BUFFERLEN];

  if ((nseconds / 3600) > 0) {
    snprintf(msg,BUFFERLEN-1,"%i hours %i minutes %i seconds",
	     nseconds / 3600,
	     (nseconds % 3600) / 60,
	     (nseconds % 3600) % 60);
  } else if ((nseconds / 60) > 0) {
    snprintf(msg,BUFFERLEN-1,"%i minutes %i seconds",
	     nseconds / 60,
	     nseconds % 60);
  } else {
    snprintf(msg,BUFFERLEN-1,"%i seconds",nseconds);
  }

  return msg;
}
