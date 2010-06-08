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

#ifndef _TURTLESG_H_
#define _TURTLESG_H_

#include "constants.h"
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#ifdef __CPLUSPLUS
extern "C" {
#endif

#pragma pack(push,1)

  struct turtlesgi {  /* Maya Script Generator Info */
    char renderdir[BUFFERLEN]; // -imageOutputPath
    char projectdir[BUFFERLEN]; // -projectPath
    char scene[BUFFERLEN]; // -gemoetry
    char image[BUFFERLEN]; // -imageName
    char scriptdir[BUFFERLEN];
    char file_owner[BUFFERLEN];
    char camera[BUFFERLEN]; // -camera
    uint32_t res_x,res_y;  /* Resolution of the frame */
    //char res_x[BUFFERLEN];
    //char res_y[BUFFERLEN];
    char format[BUFFERLEN];
    uint8_t usemaya70; // 1 if we should render with Maya 7.0, 0 for Maya 6.5
    // char precommand[BUFFERLEN]; // allows a prerender command (mel script) to be executed before the render
    // char postcommand[BUFFERLEN]; // allows a postrender command (mel script to be executed after the render
  };

#pragma pack(pop)

  char *turtlesg_create (struct turtlesgi *info);

  char *turtlesg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif

#endif /* _MAYASG_H_ */
