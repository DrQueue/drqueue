//
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA      02111-1307
// USA
//

#ifndef _GENERALSG_H_
#define _GENERALSG_H_

#include "constants.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif

  struct generalsgi {             /* general Script Generator Info */
    char drqueue_scriptdir[BUFFERLEN];
    char script[BUFFERLEN];
    // I'd like to get the script location path and store it.
    char *scriptdir;
    uid_t uid_owner;
    gid_t gid_owner;
  };

  char *generalsg_create (struct generalsgi *info);
  char *generalsg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif

#endif /* _GENERALSG_H_ */
