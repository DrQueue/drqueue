/* $Id$ */

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#  include <sys/types.h>
# else
#  ifdef __OSX
#   include <stdint.h>
#  else
#   ifdef __FREEBSD
#    include <stdint.h>
#   else
#    error You need to define the OS, or OS defined not supported
#   endif
#  endif
# endif
#endif

#include "job.h"

#define VERSION  "0.50rc6"

int common_environment_check (void);
void show_version (char **argv);
int rmdir_check_str (char *path);
int remove_dir (char *dir);
char *time_str (uint32_t nseconds);

int common_date_check (void);

/* Mail notifications */
void mn_job_finished (struct job *job);

#endif /* _COMMON_H_ */

