/* $Id: common.h,v 1.22 2002/06/26 13:30:30 jorge Exp $ */

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
# else
#  error You need to define the OS, or OS defined not supported
# endif
#endif

#include "job.h"

#define VERSION "v_0-36"

int common_environment_check (void);
void show_version (char **argv);
int remove_dir (char *dir);
char *time_str (uint32_t nseconds);

int common_date_check (void);

/* Mail notifications */
void mn_job_finished (struct job *job);

#endif /* _COMMON_H_ */

