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
/* $Id$ */

#ifndef _SENDJOB_H_
#define _SENDJOB_H_

#include <fstream>
#include "job.h"

#define TOJ_NONE				 0
#define TOJ_MAYA				 1
#define TOJ_BLENDER			 2
#define TOJ_BMRT				 3	/* BMRT koj */
#define TOJ_THREEDELIGHT 4	/* 3delight koj */
#define TOJ_PIXIE				 5	/* Pixie koj */
#define TOJ_MENTALRAY		 6
#define TOJ_LIGHTWAVE		 7	// Lightwave koj
#define TOJ_AFTEREFFECTS 8	// After Effects koj
#define TOJ_SHAKE				 9	// Shake koj
#define TOJ_AQSIS				 10 // Aqsis koj
#define TOJ_TERRAGEN		 11	 // Terragen koj
#define TOJ_NUKE				 12	 // Nuke koj
#define TOJ_MANTRA                               13      // Mantra koj

void presentation (void);

int RegisterMayaJobFromFile (std::ifstream &infile);
int RegisterMentalrayJobFromFile (std::ifstream &infile);
int RegisterBlenderJobFromFile (std::ifstream &infile);
int RegisterBmrtJobFromFile (std::ifstream &infile);
int RegisterThreedelightJobFromFile (std::ifstream &infile);
int RegisterPixieJobFromFile (std::ifstream &infile);
int RegisterLightwaveJobFromFile (std::ifstream &infile);
int RegisterAftereffectsJobFromFile (std::ifstream &infile);
int RegisterShakeJobFromFile (std::ifstream &infile);
int RegisterMantraJobFromFile (std::ifstream &infile);
int RegisterAqsisJobFromFile (std::ifstream &infile);
int RegisterTerragenJobFromFile (std::ifstream &infile);
int RegisterNukeJobFromFile (std::ifstream &infile);

int str2toj (char *str);

void cleanup (int signum);

#endif /* _SENDJOB_H_ */
