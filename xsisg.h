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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
// 

#ifndef _XSISG_H_
#define _XSISG_H_

#include "constants.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif 

struct xsisgi {		/* XSI Script Generator Info */
	char xsiDir[BUFFERLEN];
	char scene[BUFFERLEN];
	char pass[BUFFERLEN];
	char renderdir[BUFFERLEN];
	char image[BUFFERLEN];
	char imageExt[BUFFERLEN];
	char scriptdir[BUFFERLEN];
	char file_owner[BUFFERLEN];
	int	 res_x,res_y;		/* Resolution of the frame */
	int  skipFrames; //skip rendered frames - 1 is true
	int	 runScript;	 //1 if we should run a script instead of rendering
	char scriptRun[BUFFERLEN]; //script to run
};

char *xsisg_create (struct xsisgi *info);

char *xsisg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif 

#endif /* _MENTALRAYSG_H_ */
