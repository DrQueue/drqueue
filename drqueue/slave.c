/* $Id: slave.c,v 1.2 2001/04/26 16:06:22 jorge Exp $ */

#include "computer.h"
#include "logger.h"

int main (int argc,char *argv[])
{
  struct computer comp;

  get_hwinfo (&comp.hwinfo);
  report_hwinfo (&comp.hwinfo);
  get_computerstatus (&comp.status);
  
  strcpy (comp.status.task[0].jobname,"Test");
  log_slave_task (&comp.status.task[0],"Another test");

  exit (0);
}
