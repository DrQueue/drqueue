//
// Copyright (C) 2007 Andreas Schroeder
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
// $Id: cinema4dsg.h 2089 2007-08-22 22:08:12Z kaazoo $
//

#ifndef _CINEMA4DSG_H_
#define _CINEMA4DSG_H_

#include "constants.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif

#pragma pack(push,1)

  struct cinema4dsgi {  /* Cinema4d Script Generator Info */
    char scene[BUFFERLEN];
    char scriptdir[BUFFERLEN];
 uint8_t kind;       // 1 if we should render animations
    				 // 2 if we should distribute one single image
  };

#pragma pack(pop)

  char *cinema4dsg_create (struct cinema4dsgi *info);
  char *cinema4dsg_default_script_path (void);

#ifdef __CPLUSPLUS
}
#endif

#endif /* _CINEMA4DSG_H_ */
