/* $Id: drerrno.c,v 1.10 2001/11/15 14:30:56 jorge Exp $ */

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
  case DRE_ERROROPENING:
    msg = "Could not open file or directory";
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
  case DRE_ERRORWRITING:
    msg = "Could not write on file or socket";
    break;
  case DRE_ERRORREADING:
    msg = "Could not read from file or socket";
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
  case DRE_DIFFILEFORMAT:
    msg = "Different file format than expected";
    break;
  case DRE_DIFVERSION:
    msg = "Different version number than expected";
    break;
  case DRE_DIFJOBSIZE:
    msg = "Different number of jobs than expected";
    break;
  case DRE_GETSHMEM:
    msg = "Could not allocate shared memory";
    break;
  case DRE_ATTACHSHMEM:
    msg = "Could not attach shared memory";
    break;
  case DRE_NOTMPDIR:
    msg = "Directory for temporary files ($DRQUEUE_ROOT/tmp) does not exist or is not writeable";
    break;
  case DRE_NODBDIR:
    msg = "Directory for database files ($DRQUEUE_ROOT/db) does not exist or is not writeable";
    break;
  case DRE_NOLOGDIR:
    msg = "Directory for log files ($DRQUEUE_ROOT/logs) does not exist or is not writeable";
    break;
  case DRE_NOBINDIR:
    msg = "Directory for binary files ($DRQUEUE_ROOT/bin) does not exist";
    break;
  default:
    msg = "drerrno NOT listed !!";
  }

  return msg;
}

