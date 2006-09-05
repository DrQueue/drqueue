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
// GNU General Public License for more details.http://i1.lelong.com.my/UserImages/Items/0511/06/jason0608@11.jpg
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
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

#include "mantrasg.h"
#include "libdrqueue.h"

char *mantrasg_create (struct mantrasgi *info) {
  /* This function creates the aqsis render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */
  FILE *f;
  FILE *etc_mantra_sg;   /* The aqsis script generator configuration file */
  int fd_etc_mantra_sg,fd_f;
  static char filename[BUFFERLEN];
  char fn_etc_mantra_sg[BUFFERLEN]; /* File name pointing to DRQUEUE_ETC/aqsis.sg */
  char buf[BUFFERLEN];
  int size;
  char *p;   /* Scene filename without path */
  char scene[MAXCMDLEN];
  char renderdir[MAXCMDLEN];

  /* Check the parameters */
  if ((!info->renderdir) || (!info->scene) || (!strlen(info->renderdir)) || (!strlen(info->scene))) {
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
  fprintf(f,"set DRQUEUE_SCENE=%s\n",info->scene);
  fprintf(f,"set DRQUEUE_RD=%s\n",info->renderdir);
  fprintf(f,"set RF_OWNER=%s\n",info->file_owner);
  fprintf(f,"set CUSTOM_BUCKET=%u\n",info->custom_bucket);
  if (info->custom_bucket) {
    fprintf (f,"set BUCKETSIZE=%u\n",info->bucketSize);
  }
  fprintf(f,"set CUSTOM_LOD=%u\n",info->custom_lod);
  if (info->custom_lod) {
    fprintf (f,"set LOD=%u\n",info->LOD);
  }
  fprintf(f,"set CUSTOM_VARYAA=%u\n",info->custom_varyaa);
  if (info->custom_varyaa) {
    fprintf (f,"set VARYAA=%.3f\n",info->varyAA);
  }
  fprintf(f,"set RAYTRACE=%u\n",info->raytrace);
  fprintf(f,"set ANTIALIAS=%u\n",info->aaoff);
  fprintf(f,"set CUSTOM_BDEPTH=%u\n",info->custom_bDepth);
  if (info->custom_bDepth) {
    fprintf (f,"set BDEPTH=%u\n",info->bDepth);
  }
  fprintf(f,"set CUSTOM_ZDEPTH=%u\n",info->custom_zDepth);
  if (info->custom_zDepth) {
    fprintf (f,"set ZDEPTH=%s\n",info->zDepth);
  }
  fprintf(f,"set CUSTOM_CRACKS=%u\n",info->custom_Cracks);
  if (info->custom_Cracks) {
    fprintf (f,"set CRACKS=%u\n",info->Cracks);
  }
  fprintf(f,"set CUSTOM_QUALITY=%u\n",info->custom_Quality);
  if (info->custom_Quality) {
    fprintf (f,"set QUALITY=%u\n",info->Quality);
  }
  fprintf(f,"set CUSTOM_QFINER=%u\n",info->custom_QFiner);
  if (info->custom_QFiner) {
    fprintf (f,"set QFINER=%s\n",info->QFiner);
  }
  fprintf(f,"set CUSTOM_SMULTIPLIER=%u\n",info->custom_SMultiplier);
  if (info->custom_SMultiplier) {
    fprintf (f,"set SMULTIPLIER=%u\n",info->SMultiplier);
  }
  fprintf(f,"set CUSTOM_MPCACHE=%u\n",info->custom_MPCache);
  if (info->custom_MPCache) {
    fprintf (f,"set MPCACHE=%u\n",info->MPCache);
  }
  fprintf(f,"set CUSTOM_MCACHE=%u\n",info->custom_MCache);
  if (info->custom_MCache) {
    fprintf (f,"set MCACHE=%u\n",info->MCache);
  }
  fprintf(f,"set CUSTOM_SMPOLYGON=%u\n",info->custom_SMPolygon);
  if (info->custom_SMPolygon) {
    fprintf (f,"set SMPOLYGON=%u\n",info->SMPolygon);
  }
  fprintf(f,"set CUSTOM_WH=%u\n",info->custom_WH);
  if (info->custom_WH) {
    fprintf (f,"set WIDTH=%u\n",info->Width);
    fprintf (f,"set HEIGHT=%u\n",info->Height);
  }
  fprintf(f,"set CUSTOM_TYPE=%u\n",info->custom_Type);
  if (info->custom_Type) {
    fprintf (f,"set TYPE=%s\n",info->Type);
  }

  snprintf(fn_etc_mantra_sg,BUFFERLEN-1,"%s/mantra.sg",getenv("DRQUEUE_ETC"));

  fflush (f);

  if ((etc_mantra_sg = fopen (fn_etc_mantra_sg,"r")) == NULL) {
    fprintf(f,"\necho -------------------------------------------------\n");
    fprintf(f,"echo ATTENTION ! There was a problem opening: %s\n",fn_etc_mantra_sg);
    fprintf(f,"echo Please correct the problem\n");
    fprintf(f,"echo -------------------------------------------------\n");
    fprintf(f,"\n\n");
  } else {
    fd_etc_mantra_sg = fileno (etc_mantra_sg);
    fd_f = fileno (f);
    while ((size = read (fd_etc_mantra_sg,buf,BUFFERLEN)) != 0) {
      write (fd_f,buf,size);
    }
    fclose(etc_mantra_sg);
  }

  fclose(f);

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



