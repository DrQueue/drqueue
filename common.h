/* $Id: common.h,v 1.13 2001/10/25 13:17:37 jorge Exp $ */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

#define VERSION "v_0-29"

int common_environment_check (void);
void show_version (char **argv);
int remove_dir (char *dir);

#endif /* _COMMON_H_ */
