/* $Id: mayasg.h,v 1.8 2003/12/18 20:39:41 jorge Exp $ */

#ifndef _MAYASG_H_
#define _MAYASG_H_

#include "constants.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif 

struct mayasgi {		/* Maya Script Generator Info */
  char renderdir[BUFFERLEN];
  char scene[BUFFERLEN];
  char image[BUFFERLEN];
  char scriptdir[BUFFERLEN];
  char file_owner[BUFFERLEN];
  char camera[BUFFERLEN];
  int  res_x,res_y;		/* Resolution of the frame */
  char format[BUFFERLEN];
};

char *mayasg_create (struct mayasgi *info);

char *mayasg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif 

#endif /* _MAYASG_H_ */
