/* $Id: mayasg.h,v 1.2 2001/09/04 23:24:29 jorge Exp $ */

#ifndef _MAYASG_H_
#define _MAYASG_H_

#include "constants.h"

struct mayasgi {		/* Maya Script Generator Info */
  char project[BUFFERLEN];
  char scene[BUFFERLEN];
  char image[BUFFERLEN];
};

char *mayasg_create (struct mayasgi *info);

#endif /* _MAYASG_H_ */
