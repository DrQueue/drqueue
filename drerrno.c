/* $Id: drerrno.c,v 1.1 2001/05/28 14:21:31 jorge Exp $ */

#include "drerrno.h"
#include "constants.h"

char *drerrno_str (void)
{
  char *msg;

  switch (drerrno) {
  case DRE_NODRMAENV:
    msg = "DRQUEUE_MASTER not defined on environment";
    break;
  case DRE_DRMANONVA:
    msg = "DRQUEUE_MASTER pointing to a non-valid address";
    break;
  case DRE_NOSOCKET:
    msg = "Could not open socket";
    break;
  case DRE_NOCONNECT:
    msg = "Could not connect to master";
    break;
  default:
    msg = "drerrno NOT listed !!";
  }

  return msg;
}

