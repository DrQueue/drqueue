//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//

#include "drerrno.h"
#include "constants.h"

int drerrno = DRE_NOERROR;
int drerrno_system = 0;

char *drerrno_str (void) {
  char *msg;

  switch (drerrno) {
  case DRE_NOERROR:
    msg = "No error";
    break;
  case DRE_ERROROPENING:
    msg = "Could not open file or directory";
    break;
  case DRE_NOTRESOLVE:
    msg = "Could not resolve hostname";
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
  case DRE_NOTCOMPLETE:
    msg = "Information not complete.";
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
    msg = "Directory for temporary files ($DRQUEUE_TMP) does not exist or is not writeable";
    break;
  case DRE_NODBDIR:
    msg = "Directory for database files ($DRQUEUE_DB) does not exist or is not writeable";
    break;
  case DRE_NOLOGDIR:
    msg = "Directory for log files ($DRQUEUE_LOGS) does not exist or is not writeable";
    break;
  case DRE_NOBINDIR:
    msg = "Directory for binary files ($DRQUEUE_BIN) does not exist";
    break;
  case DRE_NOETCDIR:
    msg = "Directory for configuration files ($DRQUEUE_ETC) does not exist";
    break;
  case DRE_RMSHMEM:
    msg = "Could not remove shared memory";
    break;
  case DRE_NOMEMORY:
    msg = "Not enough memory";
    break;
  case DRE_DTSHMEM:
    msg = "Could not detach shared memory";
    break;
  case DRE_CONNMASTER:
    msg = "Could not connect to master";
    break;
  case DRE_COMMPROBLEM:
    msg = "Communications error";
    break;
  default:
    msg = "drerrno NOT listed !!";
  }

  return msg;
}

