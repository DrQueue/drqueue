/* $Id: mayasg.c,v 1.2 2001/09/04 23:24:42 jorge Exp $ */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mayasg.h"
#include "libdrqueue.h"

char *mayasg_create (struct mayasgi *info)
{
  /* This function creates the maya render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */
  FILE *f;
  static char filename[BUFFERLEN];
  char dir[BUFFERLEN];
  char image_arg[BUFFERLEN];
  char *root;
  
  /* Check the parameters */
  if ((!strlen(info->project)) || (!strlen(info->scene))) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

  if ((root = getenv ("DRQUEUE_ROOT")) == NULL) {
    drerrno = DRE_NOENVROOT;
    return NULL;
  }

  snprintf(dir,BUFFERLEN-1,"%s/tmp",root);
  snprintf(filename,BUFFERLEN-1,"%s/%s.%lX",dir,info->scene,time(NULL));

  if ((f = fopen (filename, "a")) == NULL) {
    if (errno == ENOENT) {
      /* If its because the directory does not exist we try creating it first */
      if (mkdir (dir,0775) == -1) {
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

  /* So now we have the file open and so we must write to it */
  fprintf(f,"#!/bin/tcsh\n\n");
  fprintf(f,"set PROJ=%s\n",info->project);
  fprintf(f,"set SCENE=%s\n",info->scene);
  if (strlen(info->image)) {
    fprintf(f,"set IMAGE=%s\n",info->image);
    snprintf(image_arg,BUFFERLEN-1,"-p $IMAGE");
  } else {
    image_arg[0] = 0;
  }
  fprintf(f,"\nsetupenv -ver 4.0 maya\n");
  fprintf(f,"Render -s $FRAME -e $FRAME -proj $PROJ %s $PROJ/scenes/$SCENE\n\n",image_arg);

  fclose(f);

  return filename;
}




