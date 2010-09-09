//
// Copyright (C) 2008,2010 Andreas Schroeder
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

#include "3dsmaxsg.h"
#include "libdrqueue.h"

char *threedsmaxsg_create (struct threedsmaxsgi *info) {
  /* This function creates the 3dsmax render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */
  
  static char filename[BUFFERLEN];
  char *p;   /* Scene filename without path */  
  char scene[MAXCMDLEN];
  char image[MAXCMDLEN];
  struct jobscript_info *ji;

  /* Check the parameters */
  if (!strlen(info->scene)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

  dr_copy_path(scene, info->scene, MAXCMDLEN-1);
  dr_copy_path(image, info->image, MAXCMDLEN-1);

  p = strrchr(scene,'/');
  p = ( p ) ? p+1 : scene;
  snprintf(filename,BUFFERLEN-1,"%s/%s.%lX",info->scriptdir,p,(unsigned long int)time(NULL));

  // FIXME: Unified path handling
  ji = jobscript_new (JOBSCRIPT_PYTHON,filename);
  if(ji) {
    jobscript_write_heading (ji);
    jobscript_set_variable (ji,"SCENE",scene);
    jobscript_set_variable (ji,"DRQUEUE_IMAGE",image);
    jobscript_template_write (ji,"3dsmax_sg.py");
    jobscript_close (ji);
  } else {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

  return filename;
}

char *threedsmaxsg_default_script_path (void) {
  static char buf[BUFFERLEN];
  char *p;

  if ((p = getenv("DRQUEUE_TMP")) == NULL) {
    return ("/drqueue_tmp/not/set/report/bug/please/");
  }

  if (p[strlen(p)-1] == '/')
    snprintf (buf,BUFFERLEN-1,"%s",p);
  else
    snprintf (buf,BUFFERLEN-1,"%s/",p);

  return buf;
}
