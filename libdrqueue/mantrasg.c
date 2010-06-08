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

#include "mantrasg.h"
#include "libdrqueue.h"

char *mantrasg_create (struct mantrasgi *info) {
  /* This function creates the Mantra render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */

  static char filename[BUFFERLEN];
  char *p;   /* Scene filename without path */
  char scene[MAXCMDLEN];
  char renderdir[MAXCMDLEN];

  /* Check the parameters */
  if (!strlen(info->scene)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }
  if (!strlen(info->renderdir)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

#ifdef __CYGWIN
  cygwin_conv_to_posix_path(info->scene, scene);
  cygwin_conv_to_posix_path(info->renderdir, renderdir);
#else
  strncpy(scene,info->scene,MAXCMDLEN-1);
  strncpy(renderdir,info->renderdir,MAXCMDLEN-1);
#endif

  p = strrchr(scene,'/');
  p = ( p ) ? p+1 : scene;
  snprintf(filename,BUFFERLEN-1,"%s/%s.%lX",info->scriptdir,p,(unsigned long int)time(NULL));

  // TODO: Unified path handling
  struct jobscript_info *ji = jobscript_new (JOBSCRIPT_PYTHON, filename);

  jobscript_write_heading (ji);
  jobscript_set_variable (ji,"SCENE",scene);
  jobscript_set_variable (ji,"RENDERDIR",renderdir);
  jobscript_set_variable (ji,"RF_OWNER",info->file_owner);
  jobscript_set_variable_int (ji,"RAYTRACE",info->raytrace);
  jobscript_set_variable_int (ji,"ANTIALIAS",info->aaoff);
  
  if (info->custom_bucket) {
  	jobscript_set_variable (ji,"CUSTOM_BUCKET","yes");
  	jobscript_set_variable_int (ji,"BUCKETSIZE",info->bucketSize);
  }
  if (info->custom_lod) {
  	jobscript_set_variable (ji,"CUSTOM_LOD","yes");
  	jobscript_set_variable_int (ji,"LOD",info->LOD);
  }
  if (info->custom_varyaa) {
  	jobscript_set_variable (ji,"CUSTOM_VARYAA","yes");
  	jobscript_set_variable_int (ji,"VARYAA",info->varyAA);
  }
  if (info->custom_bDepth) {
  	jobscript_set_variable (ji,"CUSTOM_BDEPTH","yes");
  	jobscript_set_variable_int (ji,"BDEPTH",info->bDepth);
  }
  if (info->custom_zDepth) {
  	jobscript_set_variable (ji,"CUSTOM_ZDEPTH","yes");
  	jobscript_set_variable (ji,"ZDEPTH",info->zDepth);
  }
  if (info->custom_Cracks) {
  	jobscript_set_variable (ji,"CUSTOM_CRACKS","yes");
  	jobscript_set_variable_int (ji,"CRACKS",info->Cracks);
  }
  if (info->custom_Quality) {
  	jobscript_set_variable (ji,"CUSTOM_QUALITY","yes");
  	jobscript_set_variable_int (ji,"QUALITY",info->Quality);
  }
  if (info->custom_QFiner) {
  	jobscript_set_variable (ji,"CUSTOM_QFINER","yes");
  	jobscript_set_variable (ji,"QFINER",info->QFiner);
  }
  if (info->custom_SMultiplier) {
  	jobscript_set_variable (ji,"CUSTOM_SMULTIPLIER","yes");
  	jobscript_set_variable_int (ji,"SMULTIPLIER",info->SMultiplier);
  }
  if (info->custom_MPCache) {
  	jobscript_set_variable (ji,"CUSTOM_MPCACHE","yes");
  	jobscript_set_variable_int (ji,"MPCACHE",info->MPCache);
  }
  if (info->custom_MCache) {
  	jobscript_set_variable (ji,"CUSTOM_MCACHE","yes");
  	jobscript_set_variable_int (ji,"MCACHE",info->MCache);
  }
  if (info->custom_SMPolygon) {
  	jobscript_set_variable (ji,"CUSTOM_SMPOLYGON","yes");
  	jobscript_set_variable_int (ji,"SMPOLYGON",info->SMPolygon);
  }  
  if (info->custom_WH) {
  	jobscript_set_variable (ji,"CUSTOM_WH","yes");
  	jobscript_set_variable_int (ji,"WIDTH",info->Width);
  	jobscript_set_variable_int (ji,"HEIGHT",info->Height);
  }
  if (info->custom_Type) {
  	jobscript_set_variable (ji,"CUSTOM_TYPE","yes");
  	jobscript_set_variable (ji,"CTYPE",info->Type);
  }
    
  jobscript_template_write (ji,"3delight_sg.py");
  jobscript_close (ji);

  return filename;
}

char *mantrasg_default_script_path (void) {
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



