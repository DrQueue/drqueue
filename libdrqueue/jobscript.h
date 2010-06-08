//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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

#ifndef _JOBSCRIPT_H_
#define _JOBSCRIPT_H_

#ifdef __CPLUSPLUS
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>

#pragma pack(push,1)

typedef enum {
  JOBSCRIPT_TCSH,
  JOBSCRIPT_PYTHON
} jobscript_type;

struct jobscript_info {
  uint16_t type;
  FILE *file;
  char filename[PATH_MAX];
};

#pragma pack(pop)

#define JS_MAX_VAR_NAME 48
#define JS_MAX_VAR_VALUE 256

struct jobscript_info *jobscript_new (jobscript_type type,char *filename);

int jobscript_check_pointer (struct jobscript_info *ji);
int jobscript_write_heading (struct jobscript_info *ji);
int jobscript_set_variable_int (struct jobscript_info *ji,char *name,int64_t value);
int jobscript_set_variable (struct jobscript_info *ji,char *name,char *value);
int jobscript_template_write (struct jobscript_info *ji,char *template_file_name);
int jobscript_close (struct jobscript_info *ji);

int jobscript_tcsh_check_pointer (struct jobscript_info *ji);
int jobscript_tcsh_write_heading (struct jobscript_info *ji);
int jobscript_tcsh_set_variable (struct jobscript_info *ji,char *name,char *value);

int jobscript_python_check_pointer (struct jobscript_info *ji);
int jobscript_python_write_heading (struct jobscript_info *ji);
int jobscript_python_set_variable (struct jobscript_info *ji,char *name,char *value);

#ifdef __CPLUSPLUS
}
#endif

#endif 	    /* !_JOBSCRIPT_H_ */
