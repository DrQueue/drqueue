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
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
// 
/* $Id$ */

#ifndef _SENDJOB_H_
#define _SENDJOB_H_

#include <fstream>
#include "job.h"

#define TOJ_NONE		0
#define TOJ_MAYA		1
#define	TOJ_BLENDER	2

void presentation (void);

int RegisterMayaJobFromFile (std::ifstream &infile);
int RegisterBlenderJobFromFile (std::ifstream &infile);

int str2toj (char *str);

void cleanup (int signum);

#endif /* _SENDJOB_H_ */
