/* $Id: database.c,v 1.3 2001/06/05 12:19:45 jorge Exp $ */

#include "database.h"
#include "computer.h"

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
