/* $Id: drerrno.c,v 1.4 2001/08/22 09:02:48 jorge Exp $ */

#include "drerrno.h"
#include "constants.h"

int drerrno = DRE_NODRMAENV;

char *drerrno_str (void)
{
  char *msg;

  switch (drerrno) {
  case DRE_NOERROR:
    msg = "No error";
    break;
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
  case DRE_ERRORRECEIVING:
    msg = "Error receiving through socket";
    break;
  default:
    msg = "drerrno NOT listed !!";
  }

  return msg;
}

