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

#include "terragensg.h"
#include "libdrqueue.h"

#ifdef __CYGWIN
void cygwin_conv_to_posix_path(const char *path, char *posix_path);
#endif

char *terragensg_create (struct terragensgi *info) {
  /* This function creates the 3delight render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */

  static char filename[BUFFERLEN];
  char *p;   /* Scene filename without path */
  char scriptfile[MAXCMDLEN];
  char worldfile[MAXCMDLEN];
  char terrainfile[MAXCMDLEN];

  /* Check the parameters */
  if (!strlen(info->scriptfile)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

#ifdef __CYGWIN
  cygwin_conv_to_posix_path(info->scriptfile, scriptfile);
  cygwin_conv_to_posix_path(info->worldfile, worldfile);
  cygwin_conv_to_posix_path(info->terrainfile, terrainfile);
#else
  strncpy(scriptfile,info->scriptfile,MAXCMDLEN-1);
  strncpy(worldfile,info->worldfile,MAXCMDLEN-1);
  strncpy(terrainfile,info->terrainfile,MAXCMDLEN-1);
#endif

  p = strrchr(scriptfile,'/');
  p = ( p ) ? p+1 : scriptfile;
  snprintf(filename,BUFFERLEN-1,"%s/%s.%lX",info->scriptdir,p,(unsigned long int)time(NULL));

  // TODO: Unified path handling
  struct jobscript_info *ji = jobscript_new (JOBSCRIPT_PYTHON, filename);

  jobscript_write_heading (ji);
  jobscript_set_variable (ji,"SCENE",scriptfile);
  jobscript_set_variable (ji,"WORLDFILE",worldfile);
  jobscript_set_variable (ji,"TERRAINFILE",terrainfile);
  jobscript_set_variable (ji,"RF_OWNER",info->file_owner);
  
  if (strlen(info->format)) {
    jobscript_set_variable (ji,"FFORMAT",info->format);
  }
  if (info->res_x > 0) {
    jobscript_set_variable_int (ji,"RESX",info->res_x);
  }
  if (info->res_y > 0) {
    jobscript_set_variable_int (ji,"RESY",info->res_y);
  }
  if (strlen(info->camera)) {
    jobscript_set_variable (ji,"CAMERA",info->camera);
  }
    
  jobscript_template_write (ji,"terragen_sg.py");
  jobscript_close (ji);

  return filename;
}

char *terragensg_default_script_path (void) {
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



