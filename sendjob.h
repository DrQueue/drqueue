/* $Id: sendjob.h,v 1.2 2001/07/04 10:13:59 jorge Exp $ */

#ifndef _SENDJOB_H_
#define _SENDJOB_H_

#include "job.h"

void presentation (void);
void jobinfo_get (struct job *job);
void input_get_word (char *res,int len,char *question);
void input_get_line (char *res,int len,char *question);
void input_get_uint32 (uint32_t *res,char *question);

void cleanup (int signum);

#endif /* _SENDJOB_H_ */
