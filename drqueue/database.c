/* $Id: database.c,v 1.4 2001/09/20 10:51:32 jorge Exp $ */

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

#ifdef COMM_REPORT
  wdb->bsent = wdb->brecv = 0;
#endif

}
