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
/* $Id$ */

#ifndef _COMMON_H_
#define _COMMON_H_

#if defined (__LINUX)
#include <stdint.h>
#elif defined (__IRIX)
#include <sys/types.h>
#elif defined (__OSX)
#include <stdint.h>
#elif defined (__FREEBSD)
#include <stdint.h>
#else
#error You need to define the OS, or OS defined not supported
#endif

#include "job.h"

#define VERSION  "0.51.4"

int common_environment_check (void);
void show_version (char **argv);
int rmdir_check_str (char *path);
int remove_dir (char *dir);
char *time_str (uint32_t nseconds);
void set_default_env(void);
void config_parse (char *cfg);

int common_date_check (void);

/* Mail notifications */
void mn_job_finished (struct job *job);

#endif /* _COMMON_H_ */

