//
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
// Copyright (C) 2010 Andreas Schroeder
//
// This file is part of DrQueue
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

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "xsisg.h"
#include "libdrqueue.h"

char *xsisg_create (struct xsisgi *info) {
  /* This function creates the XSI render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */

  static char filename[BUFFERLEN];
  char *p;   /* Scene filename without path */
  char scene[MAXCMDLEN];
  char renderdir[MAXCMDLEN];
  char xsidir[MAXCMDLEN];

  /* Check the parameters */
  if (!strlen(info->scene)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }
  if (!strlen(info->renderdir)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }
  if (!strlen(info->xsiDir)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

#ifdef __CYGWIN
  cygwin_conv_to_posix_path(info->scene, scene);
  cygwin_conv_to_posix_path(info->renderdir, renderdir);
  cygwin_conv_to_posix_path(info->xsiDir, xsidir);
#else
  strncpy(scene,info->scene,MAXCMDLEN-1);
  strncpy(renderdir,info->renderdir,MAXCMDLEN-1);
  strncpy(xsidir,info->xsiDir,MAXCMDLEN-1);
#endif

  p = strrchr(scene,'/');
  p = ( p ) ? p+1 : scene;
  snprintf(filename,BUFFERLEN-1,"%s/%s.%lX",info->scriptdir,p,(unsigned long int)time(NULL));

  // TODO: Unified path handling
  struct jobscript_info *ji = jobscript_new (JOBSCRIPT_PYTHON, filename);

  jobscript_write_heading (ji);
  jobscript_set_variable (ji,"SCENE",scene);
  jobscript_set_variable (ji,"RENDERDIR",renderdir);
  jobscript_set_variable (ji,"XSI_DIR",xsidir);
  jobscript_set_variable (ji,"RF_OWNER",info->file_owner);
  jobscript_set_variable (ji,"DRQUEUE_PASS",info->xsipass);
  jobscript_set_variable_int (ji,"DRQUEUE_RUNSCRIPT",info->runScript);
  
  if (info->res_x > 0) {
    jobscript_set_variable_int (ji,"RESX",info->res_x);
  }
  if (info->res_y > 0) {
    jobscript_set_variable_int (ji,"RESY",info->res_y);
  }
  if (strlen(info->image)) {
  	jobscript_set_variable (ji,"DRQUEUE_IMAGE",info->image);
  }
  if (strlen(info->imageExt)) {
  	jobscript_set_variable (ji,"DRQUEUE_IMAGEEXT",info->imageExt);
  }
  if (info->skipFrames) {
  	jobscript_set_variable (ji,"DRQUEUE_SKIPFRAMES","on");
  } else {
  	jobscript_set_variable (ji,"DRQUEUE_SKIPFRAMES","off");
  }
  if (strlen(info->scriptRun)) {
  	jobscript_set_variable (ji,"DRQUEUE_SCRIPTRUN",info->scriptRun);
  }
  
  jobscript_template_write (ji,"xsi_sg.py");
  jobscript_close (ji);

  return filename;
}

char *xsisg_default_script_path (void) {
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
