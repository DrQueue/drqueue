/* $Id: common.c,v 1.2 2001/09/02 14:16:54 jorge Exp $ */

#include <stdlib.h>

#include "common.h"
#include "drerrno.h"

int common_environment_check (void)
{
  char *buf;

  if ((buf = getenv("DRQUEUE_MASTER")) == NULL) {
    drerrno = DRE_NOENVMASTER;
    return 0;
  }

  if ((buf = getenv("DRQUEUE_ROOT")) == NULL) {
    drerrno = DRE_NOENVROOT;
    return 0;
  }

  return 1;
}
