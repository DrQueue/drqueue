/* $Id: sendjob.h,v 1.4 2004/10/06 16:26:53 jorge Exp $ */

#ifndef _SENDJOB_H_
#define _SENDJOB_H_

#include <fstream>
#include "job.h"

void presentation (void);

int RegisterJobFromFile (std::ifstream &infile);

void cleanup (int signum);

#endif /* _SENDJOB_H_ */
