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

#include "pixiesg.h"
#include "libdrqueue.h"

char *pixiesg_create (struct pixiesgi *info) {
  /* This function creates the Pixie render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */

  static char filename[BUFFERLEN];
  char *p;   /* Scene filename without path */
  char scene[MAXCMDLEN];

  /* Check the parameters */
  if (!strlen(info->scene)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

#ifdef __CYGWIN
  cygwin_conv_to_posix_path(info->scene, scene);
#else
  strncpy(scene,info->scene,MAXCMDLEN-1);
#endif

  p = strrchr(scene,'/');
  p = ( p ) ? p+1 : scene;
  snprintf(filename,BUFFERLEN-1,"%s/%s.%lX",info->scriptdir,p,(unsigned long int)time(NULL));

  // TODO: Unified path handling
  struct jobscript_info *ji = jobscript_new (JOBSCRIPT_PYTHON, filename);

  jobscript_write_heading (ji);
  jobscript_set_variable (ji,"RIBFILE",scene);
  
  /*
  old options that may be worth to include:
  
  fprintf(f,"set CUSTOM_CROP=%u\n",info->custom_crop);
  if (info->custom_crop) {
   fprintf(f,"set CROP_XMIN=%u\n",info->xmin);
   fprintf(f,"set CROP_XMAX=%u\n",info->xmax);
   fprintf(f,"set CROP_YMIN=%u\n",info->ymin);
   fprintf(f,"set CROP_YMAX=%u\n",info->ymax);
  }
  fprintf(f,"set CUSTOM_SAMPLES=%u\n",info->custom_samples);
  if (info->custom_samples) {
   fprintf(f,"set XSAMPLES=%u\n",info->xsamples);
   fprintf(f,"set YSAMPLES=%u\n",info->ysamples);
  }
  fprintf(f,"set DISP_STATS=%u\n",info->disp_stats);
  fprintf(f,"set VERBOSE=%u\n",info->verbose);
  fprintf(f,"set CUSTOM_BEEP=%u\n",info->custom_beep);
  fprintf(f,"set CUSTOM_RADIOSITY=%u\n",info->custom_radiosity);
  if (info->custom_radiosity) {
   fprintf(f,"set RADIOSITY_SAMPLES=%u\n",info->radiosity_samples);
  }
  fprintf(f,"set CUSTOM_RAYSAMPLES=%u\n",info->custom_raysamples);
  if (info->custom_raysamples) {
   fprintf(f,"set RAYSAMPLES=%u\n",info->raysamples);
  }
  */
  
  jobscript_template_write (ji,"pixie_sg.py");
  jobscript_close (ji);

  return filename;
}

char *pixiesg_default_script_path (void) {
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




