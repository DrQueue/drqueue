/* $Id: database.c,v 1.5 2001/10/29 16:25:09 jorge Exp $ */

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

int database_load (struct database *wdb)
{

}
