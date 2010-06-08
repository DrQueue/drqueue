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

#ifndef _MANTRASG_H_
#define _MANTRASG_H_

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#  include <sys/types.h>
# else
#  ifdef __OSX
#  include <stdint.h>
#  else
#  ifdef __FREEBSD
#   include <stdint.h>
#  else
#   ifdef __CYGWIN
#   include <stdint.h>
#   else
#   error You need to define the OS, or OS defined not supported
#   endif
#  endif
#  endif
# endif
#endif

#include "constants.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif

#pragma pack(push,1)

  struct mantrasgi {  /* Aqsis Script Generator Info */
    char scene[BUFFERLEN];
    char renderdir[BUFFERLEN];
    char scriptdir[BUFFERLEN];
    char file_owner[BUFFERLEN];
    char custom_bucket;
    uint32_t bucketSize; /*,xmax,ymin,ymax;*/
    char custom_WH;
    uint32_t Width,Height;
    char custom_lod;
    uint32_t LOD;
    char custom_varyaa;
    float varyAA;
    char custom_samples;
    int  raytrace;
    int aaoff;

    char custom_bDepth;
    uint32_t bDepth;
    char custom_zDepth;
    char zDepth[BUFFERLEN];
    char custom_Cracks;
    uint32_t Cracks;

    char custom_Quality;
    uint32_t Quality;
    char custom_QFiner;
    char QFiner[BUFFERLEN];
    char custom_Type;
    char Type[BUFFERLEN];
    char custom_SMultiplier;
    uint32_t SMultiplier;

    char custom_MPCache;
    uint32_t MPCache;
    char custom_MCache;
    uint32_t MCache;
    char custom_SMPolygon;
    uint32_t SMPolygon;

    // char custom_Verbose;
    // char Verbose[BUFFERLEN];
  };

#pragma pack(pop)

  char *mantrasg_create (struct mantrasgi *info);
  char *mantrasg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif

#endif /* _AQSISSG_H_ */
