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

#include "turtlesg.h"
#include "libdrqueue.h"

#ifdef __CYGWIN
void cygwin_conv_to_posix_path(const char *path, char *posix_path);
#endif


char *turtlesg_create (struct turtlesgi *info) {
  /* This function creates the turtle render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */
  FILE *f;
  FILE *etc_turtle_sg;  /* The turtle script generator configuration file */
  int fd_etc_turtle_sg,fd_f;
  static char filename[BUFFERLEN];
  char fn_etc_turtle_sg[BUFFERLEN]; /* File name pointing to DRQUEUE_ETC/turtle.sg */
  char buf[BUFFERLEN];
  char image_arg[BUFFERLEN];
  int size;
  char *p;   /* Scene filename without path */
  char scene[MAXCMDLEN];
  char renderdir[MAXCMDLEN];
  char projectdir[MAXCMDLEN];
  char camera[MAXCMDLEN]; // -camera
  char resx[MAXCMDLEN]; // res X
  char resy[MAXCMDLEN]; // res Y

  /* Check the parameters */
  if ((!info->renderdir) || (!info->scene) || (!info->projectdir)
      || (!strlen(info->renderdir)) || (!strlen(info->scene)) || (!strlen(info->projectdir))) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

#ifdef __CYGWIN
  cygwin_conv_to_posix_path(info->scene, scene);
  cygwin_conv_to_posix_path(info->renderdir, renderdir);
  cygwin_conv_to_posix_path(info->projectdir, projectdir);
#else

  strncpy(scene,info->scene,MAXCMDLEN-1);
  strncpy(renderdir,info->renderdir,MAXCMDLEN-1);
  strncpy(projectdir,info->projectdir,MAXCMDLEN-1);
#endif

  strncpy(camera,info->camera,MAXCMDLEN-1);
  strncpy(resx,info->resx,MAXCMDLEN-1);
  strncpy(resy,info->resy,MAXCMDLEN-1);

  p = strrchr(scene,'/');
  p = ( p ) ? p+1 : scene;
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
  fprintf(f,"set DRQUEUE_RD=\"%s\"\n",info->renderdir);
  fprintf(f,"set DRQUEUE_PD=\"%s\"\n",info->projectdir);
  fprintf(f,"set DRQUEUE_SCENE=\"%s\"\n",info->scene);
  fprintf(f,"set RF_OWNER=%s\n",info->file_owner);
  if (strlen(info->format)) {
    fprintf(f,"set FFORMAT=%s\n",info->format);
  }
  if (strlen(info->resx)) {
    fprintf(f,"set RESX=%s\n",info->resx);
  }
  if (strlen(info->resy)) {
    fprintf(f,"set RESY=%s\n",info->resy);
  }
  if (strlen(info->camera)) {
    fprintf(f,"set CAMERA=%s\n",info->camera);
  }
  if (strlen(info->image)) {
    fprintf(f,"set DRQUEUE_IMAGE=%s\n",info->image);
  }
  if (info->usemaya70) {
    fprintf(f,"set USEMAYA70=1");
  }

  snprintf(fn_etc_turtle_sg,BUFFERLEN-1,"%s/turtle.sg",getenv("DRQUEUE_ETC"));

  fflush (f);

  if ((etc_turtle_sg = fopen (fn_etc_turtle_sg,"r")) == NULL) {
    fprintf(f,"\necho -------------------------------------------------\n");
    fprintf(f,"echo ATTENTION ! There was a problem opening: %s\n",fn_etc_turtle_sg);
    fprintf(f,"echo So the default configuration will be used\n");
    fprintf(f,"echo -------------------------------------------------\n");
    fprintf(f,"\n\n");
    fprintf(f,"Render -preRender $DRQUEUE_PRE -postRender $DRQUEUE_POST -s $DRQUEUE_FRAME -e $DRQUEUE_FRAME -rd $DRQUEUE_RD -proj $DRQUEUE_PD %s $DRQUEUE_SCENERender -s $DRQUEUE_FRAME -e $BLOCK $RESX $RESY $FFORMAT -rd $DRQUEUE_RD -proj $DRQUEUE_PD $USEMAYA65 $CIMAGE $CAMERA $DRQUEUE_SCENE\n\n",image_arg);
  } else {
    fd_etc_turtle_sg = fileno (etc_turtle_sg);
    fd_f = fileno (f);
    while ((size = read (fd_etc_turtle_sg,buf,BUFFERLEN)) != 0) {
      write (fd_f,buf,size);
    }
    fclose(etc_turtle_sg);
  }

  fclose(f);

  return filename;
}


char *turtlesg_default_script_path (void) {
  static char buf[BUFFERLEN];
  char *p;

  if (!(p = getenv("DRQUEUE_TMP"))) {
    return ("/drqueue_tmp/not/set/");
  }

  if (p[strlen(p)-1] == '/')
    snprintf (buf,BUFFERLEN-1,"%s",p);
  else
    snprintf (buf,BUFFERLEN-1,"%s/",p);

  return buf;
}



