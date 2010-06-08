//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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

#ifndef _MAYASG_H_
#define _MAYASG_H_

#include "constants.h"
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#ifdef __CPLUSPLUS
extern "C" {
#endif

#pragma pack(push,1)

  struct mayasgi {  /* Maya Script Generator Info */
    char renderdir[BUFFERLEN];
    char projectdir[BUFFERLEN];
    char scene[BUFFERLEN];
    char image[BUFFERLEN];
    char scriptdir[BUFFERLEN];
    char file_owner[BUFFERLEN];
    char camera[BUFFERLEN];
    uint32_t res_x,res_y;  /* Resolution of the frame */
    char format[BUFFERLEN];
    uint8_t renderer;  // 0 if we should render with Maya software
    	    	       // 1 if we should render with Mental Ray
    	    	       // 2 if we should render with RenderMan
    	    	       // 3 if we should render with the renderer specified in the scene file
    char precommand[BUFFERLEN]; // allows a prerender command (mel script) to be executed before the render
    char postcommand[BUFFERLEN]; // allows a postrender command (mel script to be executed after the render
  };

#pragma pack(pop)

  char *mayasg_create (struct mayasgi *info);

  char *mayasg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif

#endif /* _MAYASG_H_ */
