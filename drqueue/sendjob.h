/* $Id: sendjob.h,v 1.5 2004/10/09 05:21:28 jorge Exp $ */

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
