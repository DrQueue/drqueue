/* $Id: blendersg.h,v 1.2 2003/12/18 20:39:41 jorge Exp $ */

#ifndef _BLENDERSG_H_
#define _BLENDERSG_H_

#include "constants.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif 

struct blendersgi {		/* Blender Script Generator Info */
  char scene[BUFFERLEN];
  char scriptdir[BUFFERLEN];
};

char *blendersg_create (struct blendersgi *info);

char *blendersg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif 

#endif /* _BLENDERSG_H_ */
