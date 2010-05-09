//
// Copyright (C) 2001,2002,2003,2004,2005 Jorge Daza Garcia-Blanes
// Copyright (C) 2010 Andreas Schroeder
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

#ifndef _SHAKESG_H_
#define _SHAKESG_H_

#include "constants.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif

#pragma pack(push,1)


  struct shakesgi {  // Shake script generator information
    char script[BUFFERLEN];    // This is the shake script, has nothing to do with the render script
    char scriptdir[BUFFERLEN];
  };

  char *shakesg_create (struct shakesgi *info);
  char *shakesg_default_script_path (void);

#pragma pack(pop)

#ifdef __CPLUSPLUS
}
#endif

#endif // _SHAKESG_H_
