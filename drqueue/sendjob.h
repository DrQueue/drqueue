/* $Id: sendjob.h,v 1.3 2002/09/22 19:10:24 jorge Exp $ */

#ifndef _SENDJOB_H_
#define _SENDJOB_H_

#include "job.h"

void presentation (void);

int RegisterJobFromFile (ifstream &infile);

void cleanup (int signum);

#endif /* _SENDJOB_H_ */
