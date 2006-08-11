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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
//
// $Id$
//

#ifndef _DRQM_CYGWIN_H_
#define _DRQM_CYGWIN_H_

void cygwin_shell_execute(char *verb, char *path);
char *cygwin_file_dialog(char *fname, char *pat, char *message, char save);
char *cygwin_dir_dialog(char *message);
//char *conv_to_posix_path(char *win32_path);
//char *conv_to_win32_path(char *posix_path);

#endif /* _DRQM_CYGWIN_H */


