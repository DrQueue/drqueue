/* $Id: database.c,v 1.2 2001/05/30 15:11:47 jorge Exp $ */

#include "database.h"

void database_init (struct database *wdb)
{
  int i;

  for (i=0;i<MAXJOBS;i++) {
    job_init (&wdb->job[i]);
  }

  for (i=0;i<MAXCOMPUTERS;i++) {
    computer_init (&wdb->computer[i]);
  }
}
