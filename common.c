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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "drerrno.h"
#include "constants.h"

int common_environment_check (void)
{
  /* This function checks the environment AND the directory structure */
  char *buf;
  struct stat s_stat;
  char dir_str[BUFFERLEN];

  if ((buf = getenv("DRQUEUE_MASTER")) == NULL) {
    drerrno = DRE_NOENVMASTER;
    return 0;
  }

  if ((buf = getenv("DRQUEUE_ROOT")) == NULL) {
    drerrno = DRE_NOENVROOT;
    return 0;
  }

#ifndef __CYGWIN
  snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_TMP"));
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOTMPDIR;
    return 0;
  }
  else
  {
    if ((!S_ISDIR(s_stat.st_mode)) || (!(S_IWOTH & s_stat.st_mode)))
    {
      drerrno = DRE_NOTMPDIR;
      return 0;
    }
  }
#endif
  snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_DB"));
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NODBDIR;
    return 0;
  } else if ((!S_ISDIR(s_stat.st_mode)) || (!(S_IWUSR & s_stat.st_mode))) {
    drerrno = DRE_NODBDIR;
    return 0;
  }

  snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_LOGS"));
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOLOGDIR;
    return 0;
  } else if ((!S_ISDIR(s_stat.st_mode)) || (!(S_IWUSR & s_stat.st_mode))) {
    drerrno = DRE_NOLOGDIR;
    return 0;
  }

  snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_BIN"));
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOBINDIR;
    return 0;
  }

  snprintf (dir_str,BUFFERLEN-1,"%s",getenv("DRQUEUE_ETC"));
  if (stat (dir_str,&s_stat) == -1) {
    drerrno = DRE_NOETCDIR;
    return 0;
  }

  return 1;
}

void show_version (char **argv)
{
  printf ("\nDistributed Rendering Queue\n");
  printf ("by Jorge Daza Garcia Blanes\n\n");
  printf ("%s version: %s\n",argv[0],VERSION);
  printf ("\n");
}

int rmdir_check_str (char *path)
{
	// This function should test a path's validity
	// So we don't pass a wrong path to remove_dir by mistake

	if (strstr(path,"/../") != NULL)
		return 1;
	if (strchr(path,'"') != NULL)
		return 1;	
	if (strchr(path,'\'') != NULL)
		return 1;

	return 0;
}

int remove_dir (char *dir)
{
  /* Removes a directory recursively */
  char cmd[BUFFERLEN];

	if (rmdir_check_str (dir)) // Check the path. If wrong don't remove
		return 0;

  snprintf (cmd,BUFFERLEN,"rm -fR '%s'",dir);

  return system (cmd);
}

void mn_job_finished (struct job *job)
{
  FILE *mail;
  char command[BUFFERLEN];
  uint32_t total;
  char total_str[BUFFERLEN];

  snprintf (command,BUFFERLEN-1,"Mail -s 'Your job (%s) is finished' %s",job->name,job->email);

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

int common_date_check (void)
{
  time_t now;
  struct tm *tm_now;

  time(&now);
  tm_now = localtime (&now);
  
/*    fprintf (stderr,"Date: %i.%i.%u\n",tm_now->tm_mday,tm_now->tm_mon,tm_now->tm_year); */
  if (((tm_now->tm_mon >= 4) && (tm_now->tm_year == 102))
      || (tm_now->tm_year > 102))
    return 0;
  
  return 1;
}

void set_default_env(void)
{
	char *penv,renv[BUFFERLEN];

	if (!getenv("DRQUEUE_TMP")) {
		snprintf(renv,BUFFERLEN,"DRQUEUE_TMP=%s/tmp",getenv("DRQUEUE_ROOT"));
		penv = (char*) malloc (strlen(renv)+1);
		strncpy(penv,renv,strlen(renv)+1);
		putenv(penv);
	}

	if (!getenv("DRQUEUE_ETC")) {
		snprintf(renv,BUFFERLEN,"DRQUEUE_ETC=%s/etc",getenv("DRQUEUE_ROOT"));
		penv = (char*) malloc (strlen(renv)+1);
		strncpy(penv,renv,strlen(renv)+1);
		putenv(penv);
	}

	if (!getenv("DRQUEUE_BIN")) {
		snprintf(renv,BUFFERLEN,"DRQUEUE_BIN=%s/bin",getenv("DRQUEUE_ROOT"));
		penv = (char*) malloc (strlen(renv)+1);
		strncpy(penv,renv,strlen(renv)+1);
		putenv(penv);
	}

	if (!getenv("DRQUEUE_LOGS")) {
		snprintf(renv,BUFFERLEN,"DRQUEUE_LOGS=%s/logs",getenv("DRQUEUE_ROOT"));
		penv = (char*) malloc (strlen(renv)+1);
		strncpy(penv,renv,strlen(renv)+1);
		putenv(penv);
	}

	if (!getenv("DRQUEUE_DB")) {
		snprintf(renv,BUFFERLEN,"DRQUEUE_DB=%s/db",getenv("DRQUEUE_ROOT"));
		penv = (char*) malloc (strlen(renv)+1);
		strncpy(penv,renv,strlen(renv)+1);
		putenv(penv);
	}
}

void config_parse (char *cfg)
{
	FILE *f_conf;
	char buffer[BUFFERLEN];
	char *token;
	char renv[BUFFERLEN], *penv;

	if ((f_conf = fopen (cfg,"r")) == NULL) {
		fprintf (stderr,"Could not open config file using defaults\n");
		return;
	}

	while ((fgets (buffer,BUFFERLEN-1,f_conf)) != NULL) {
		if (buffer[0] == '#') {
			continue;
		}
		token = strtok(buffer,"=\n\r");
		if (strcmp(token,"logs") == 0) {
			if ((token = strtok (NULL,"=\n\r")) != NULL) {
				fprintf (stderr,"Logs on: '%s'\n",token);
				snprintf (renv,BUFFERLEN,"DRQUEUE_LOGS=%s",token);
				if ((penv = (char*) malloc (strlen (renv)+1)) == NULL) {
					fprintf (stderr,"ERROR allocating memory for DRQUEUE_LOGS.\n");
					exit (1);
				}
				strncpy(penv,renv,strlen(renv)+1);
				if (putenv (penv) != 0) {
					fprintf (stderr,"ERROR seting the environment: '%s'\n",penv);
				}
			} else {
				fprintf (stderr,"Warning parsing config file. No value for logs. Using default.\n");
			}
		} else if (strcmp(token,"tmp") == 0) {
			if ((token = strtok (NULL,"=\n\r")) != NULL) {
				fprintf (stderr,"Tmp on: '%s'\n",token);
				snprintf (renv,BUFFERLEN,"DRQUEUE_TMP=%s",token);
				if ((penv = (char*) malloc (strlen(renv)+1)) == NULL) {
					fprintf (stderr,"ERROR allocating memory for DRQUEUE_TMP.\n");
					exit (1);
				}
				strncpy(penv,renv,strlen(renv)+1);
				if (putenv (penv) != 0) {
					fprintf (stderr,"ERROR seting the environment: '%s'\n",penv);
				}
			} else {
				fprintf (stderr,"Warning parsing config file. No value for tmp. Using default.\n");
			}
		} else if (strcmp(token,"bin") == 0) {
			if ((token = strtok (NULL,"=\n\r")) != NULL) {
				fprintf (stderr,"Bin on: '%s'\n",token);
				snprintf (renv,BUFFERLEN,"DRQUEUE_BIN=%s",token);
				if ((penv = (char*) malloc (strlen(renv)+1)) == NULL) {
					fprintf (stderr,"ERROR allocating memory for DRQUEUE_BIN.\n");
					exit (1);
				}
				strncpy(penv,renv,strlen(renv)+1);
				if (putenv (penv) != 0) {
					fprintf (stderr,"ERROR seting the environment: '%s'\n",penv);
				}
			} else {
				fprintf (stderr,"Warning parsing config file. No value for bin. Using default.\n");
			}
		} else if (strcmp(token,"etc") == 0) {
			if ((token = strtok (NULL,"=\n\r")) != NULL) {
				fprintf (stderr,"Etc on: '%s'\n",token);
				snprintf (renv,BUFFERLEN,"DRQUEUE_ETC=%s",token);
				if ((penv = (char*) malloc (strlen(renv)+1)) == NULL) {
					fprintf (stderr,"ERROR allocating memory for DRQUEUE_ETC.\n");
					exit (1);
				}
				strncpy(penv,renv,strlen(renv)+1);
				if (putenv (penv) != 0) {
					fprintf (stderr,"ERROR seting the environment: '%s'\n",penv);
				}
			} else {
				fprintf (stderr,"Warning parsing config file. No value for etc. Using default.\n");
			}
		} else if (strcmp(token,"db") == 0) {
			if ((token = strtok (NULL,"=\n\r")) != NULL) {
				fprintf (stderr,"Db on: '%s'\n",token);
				snprintf (renv,BUFFERLEN,"DRQUEUE_DB=%s",token);
				if ((penv = (char*) malloc (strlen(renv)+1)) == NULL) {
					fprintf (stderr,"ERROR allocating memory for DRQUEUE_DB.\n");
					exit (1);
				}
				strncpy(penv,renv,strlen(renv)+1);
				if (putenv (penv) != 0) {
					fprintf (stderr,"ERROR seting the environment: '%s'\n",penv);
				}
			} else {
				fprintf (stderr,"Warning parsing config file. No value for db. Using default.\n");
			}
		} else if (strcmp(token,"pool") == 0) {
			if ((token = strtok (NULL,"=\n\r")) != NULL) {
				fprintf (stderr,"Pools are: '%s'\n",token);
				snprintf (renv,BUFFERLEN,"DRQUEUE_POOL=%s",token);
				if ((penv = (char*) malloc (strlen(renv)+1)) == NULL) {
					fprintf (stderr,"ERROR allocating memory for DRQUEUE_DB.\n");
					exit (1);
				}
				strncpy(penv,renv,strlen(renv)+1);
				if (putenv (penv) != 0) {
					fprintf (stderr,"ERROR seting the environment: '%s'\n",penv);
				}
			} else {
				fprintf (stderr,"Warning parsing config file. No value for pool. Using default.\n");
			}
		} else {
			fprintf (stderr,"ERROR parsing config file. Unknown token: '%s'\n",token);
			exit (1);
		}
	}
}
