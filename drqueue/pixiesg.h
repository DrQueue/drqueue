/* $Id: pixiesg.h,v 1.1 2004/03/09 18:54:29 jorge Exp $ */

#ifndef _PIXIESG_H_
#define _PIXIESG_H_

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
# else
#  ifdef __OSX
#   include <stdint.h>
#  else
#   error You need to define the OS, or OS defined not supported
#  endif
# endif
#endif

#include "constants.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif 

struct pixiesgi {		/* Pixie Script Generator Info */
  char renderdir[BUFFERLEN];
  char scene[BUFFERLEN];
  char scriptdir[BUFFERLEN];
/*
	char custom_crop;
	uint32_t xmin,xmax,ymin,ymax;
	char custom_samples;
	uint32_t xsamples,ysamples;
	char disp_stats;
	char verbose;
	char custom_beep;
	char custom_radiosity;
	uint32_t radiosity_samples;
	char custom_raysamples;
	uint32_t raysamples;
*/
};

char *pixiesg_create (struct pixiesgi *info);
char *pixiesg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif 

#endif /* _PIXIESG_H_ */
