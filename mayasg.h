/* $Id: mayasg.h,v 1.6 2002/08/02 17:40:21 jorge Exp $ */

#ifndef _MAYASG_H_
#define _MAYASG_H_

#include "constants.h"

struct mayasgi {		/* Maya Script Generator Info */
  char renderdir[BUFFERLEN];
  char scene[BUFFERLEN];
  char image[BUFFERLEN];
  char scriptdir[BUFFERLEN];
  char file_owner[BUFFERLEN];
};

char *mayasg_create (struct mayasgi *info);
char *mayablocksg_create (struct mayasgi *info);

char *mayasg_default_script_path (void);

#endif /* _MAYASG_H_ */
