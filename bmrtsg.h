/* $Id: bmrtsg.h,v 1.1 2003/12/18 20:39:41 jorge Exp $ */

#ifndef _BMRTSG_H_
#define _BMRTSG_H_

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
# else
#  error You need to define the OS, or OS defined not supported
# endif
#endif

#include "constants.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif 

struct bmrtsgi {		/* Bmrt Script Generator Info */
  char renderdir[BUFFERLEN];
  char scene[BUFFERLEN];
  char scriptdir[BUFFERLEN];
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
};

char *bmrtsg_create (struct bmrtsgi *info);
char *bmrtsg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif 

#endif /* _BMRTSG_H_ */
