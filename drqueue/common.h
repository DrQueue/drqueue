/* $Id: common.h,v 1.14 2001/10/29 10:13:56 jorge Exp $ */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

#define VERSION "v_0-30"

int common_environment_check (void);
void show_version (char **argv);
int remove_dir (char *dir);

#endif /* _COMMON_H_ */
