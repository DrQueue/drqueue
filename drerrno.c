/* $Id: drerrno.c,v 1.8 2001/10/31 15:59:37 jorge Exp $ */

#include "drerrno.h"
#include "constants.h"

int drerrno = DRE_NOERROR;

char *drerrno_str (void)
{
  char *msg;

  switch (drerrno) {
  case DRE_NOERROR:
    msg = "No error";
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
  case DRE_NOTREGISTERED:
    msg = "Structure (job,computer,task...) not registered";
    break;
  case DRE_NOENVMASTER:
    msg = "Environment variable DRQUEUE_MASTER not set";
    break;
  case DRE_NOENVROOT:
    msg = "Environment variable DRQUEUE_ROOT not set";
    break;
  case DRE_COULDNOTCREATE:
    msg = "Could not create file or directory. Check permissions.";
    break;
  default:
    msg = "drerrno NOT listed !!";
  }

  return msg;
}

