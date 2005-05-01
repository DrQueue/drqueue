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

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "shakesg.h"
#include "libdrqueue.h"

#ifdef __CYGWIN
void cygwin_conv_to_posix_path(const char *path, char *posix_path);
#endif

char *shakesg_create (struct shakesgi *info)
{
  /* This function creates the shake render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */
  FILE *f;
  FILE *etc_shake_sg; 		/* The shake script generator configuration file */
  int fd_etc_shake_sg,fd_f;
  static char filename[BUFFERLEN];
  char fn_etc_shake_sg[BUFFERLEN]; /* File name pointing to DRQUEUE_ETC/shake.sg */
  char buf[BUFFERLEN];
	int size;
  char *p;			/* Script filename without path */
	char *script;

  /* Check the parameters */
  if (!strlen(info->script)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }
#ifdef __CYGWIN
  if ((script = malloc(MAXCMDLEN)) == NULL) return (NULL);
  cygwin_conv_to_posix_path(info->script, script);
#else
  script = info->script;
#endif

  p = strrchr(script,'/');
  p = ( p ) ? p+1 : script;
  snprintf(filename,BUFFERLEN-1,"%s/%s.%lX",info->scriptdir,p,(unsigned long int)time(NULL));

  if ((f = fopen (filename, "a")) == NULL) {
    if (errno == ENOENT) {
      /* If its because the directory does not exist we try creating it first */
      if (mkdir (info->scriptdir,0775) == -1) {
				drerrno = DRE_COULDNOTCREATE;
				return NULL;
      } else if ((f = fopen (filename, "a")) == NULL) {
				drerrno = DRE_COULDNOTCREATE;
				return NULL;
      }
    } else {
      drerrno = DRE_COULDNOTCREATE;
      return NULL;
    }
  }

  fchmod (fileno(f),0777);

  /* So now we have the file open and so we must write to it */
  fprintf(f,"#!/bin/tcsh\n\n");
  fprintf(f,"set DRQUEUE_SCRIPT=\"%s\"\n",info->script);

  snprintf(fn_etc_shake_sg,BUFFERLEN-1,"%s/shake.sg",getenv("DRQUEUE_ETC"));

  fflush (f);

  if ((etc_shake_sg = fopen (fn_etc_shake_sg,"r")) == NULL) {
    fprintf(f,"\necho -------------------------------------------------\n");
    fprintf(f,"echo ATTENTION ! There was a problem opening: %s\n",fn_etc_shake_sg);
    fprintf(f,"echo So the default configuration will be used\n");
    fprintf(f,"echo -------------------------------------------------\n");
    fprintf(f,"\n\n");
    fprintf(f,"shake -v -t $DRQUEUE_FRAME-${DRQUEUE_FRAME}x$DRQUEUE_STEPFRAME -exec $DRQUEUE_SCRIPT\n\n");
  } else {
    fd_etc_shake_sg = fileno (etc_shake_sg);
    fd_f = fileno (f);
    while ((size = read (fd_etc_shake_sg,buf,BUFFERLEN)) != 0) {
      write (fd_f,buf,size);
    }
    fclose(etc_shake_sg);
  }

  fclose(f);

  return filename;
}


char *shakesg_default_script_path (void)
{
  static char buf[BUFFERLEN];
  char *p;

  if (!(p = getenv("DRQUEUE_TMP"))) {
    return ("/drqueue_tmp/not/set/report/bug/please/");
  }

#ifdef __CYGWIN	 
  if (p[strlen(p)-1] == '\\')
		snprintf (buf,BUFFERLEN-1,"%s",p);
	else
		snprintf (buf,BUFFERLEN-1,"%s\\",p);
#else
  if (p[strlen(p)-1] == '/')
		snprintf (buf,BUFFERLEN-1,"%s",p);
	else
		snprintf (buf,BUFFERLEN-1,"%s/",p);
#endif


  return buf;
}
