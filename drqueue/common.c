/* $Id: common.c,v 1.3 2001/09/05 15:17:38 jorge Exp $ */

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

void show_version (char **argv)
{
  printf ("\nDistributed Rendering Queue\n");
  printf ("by Triple-e VFX\n\n");
  printf ("%s version: %s\n",argv[0],VERSION);
  printf ("\n");
}

