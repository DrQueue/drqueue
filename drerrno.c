/* $Id: drerrno.c,v 1.2 2001/08/06 12:33:50 jorge Exp $ */

#include "drerrno.h"
#include "constants.h"

char *drerrno_str (void)
{
  char *msg;

  switch (drerrno) {
  case DRE_NODRMAENV:
    msg = "DRQUEUE_MASTER not defined on environment";
    break;
  case DRE_NOTRESOLV:
    msg = "Could not resolv hostname";
    break;
  case DRE_NOSOCKET:
    msg = "Could not open socket";
    break;
  case DRE_NOCONNECT:
    msg = "Could not connect to master";
    break;
  case DRE_ANSWERNOTLISTED:
    msg = "Received request answer value not listed";
    break;
  case DRE_ANSWERNOTRIGHT:
    msg = "Received request answer value not appropiate for this query";
    break;
  case DRE_ERRORSENDING:
    msg = "Error sending through socket";
    break;
  default:
    msg = "drerrno NOT listed !!";
  }

  return msg;
}

