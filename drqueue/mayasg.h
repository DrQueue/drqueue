/* $Id: mayasg.h,v 1.1 2001/09/04 12:30:33 jorge Exp $ */

#ifndef _MAYASG_H_
#define _MAYASG_H_

#include "constants.h"

struct mayasg {
  char project[BUFFERLEN];
  char scene[BUFFERLEN];
  char image[BUFFERLEN];
};

char *mayasg_create (struct mayasg *info);

#endif /* _MAYASG_H_ */
