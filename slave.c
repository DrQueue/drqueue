/* $Id: slave.c,v 1.1 2001/04/26 14:20:55 jorge Exp $ */

#include "computer.h"

int main (int argc,char *argv[])
{
  struct computer comp;

  get_hwinfo (&comp.hwinfo);
  report_hwinfo (&comp.hwinfo);
  get_computerstatus (&comp.cstatus);

  exit (0);
}
