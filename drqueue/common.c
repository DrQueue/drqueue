/* $Id: common.c,v 1.4 2001/10/25 13:17:36 jorge Exp $ */

#include <stdlib.h>

#include "common.h"
#include "drerrno.h"
#include "constants.h"

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

int remove_dir (char *dir)
{
  /* Removes a directory recursively */
  char cmd[BUFFERLEN];

  snprintf (cmd,BUFFERLEN,"rm -fR %s",dir);

  return system (cmd);
}
