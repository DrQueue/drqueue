/* $Id: task.c,v 1.1 2001/04/26 14:22:38 jorge Exp $ */

#include "task.h"

void init_tasks (struct t_task *task)
{
  int i;

  for (i=0;i < MAXTASKS; i++)
    task[i].used = 0;
}

