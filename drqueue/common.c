/* $Id: common.c,v 1.1 2001/09/02 12:18:45 jorge Exp $ */

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
