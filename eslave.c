/* $Id: eslave.c,v 1.1 2001/04/25 15:55:23 jorge Exp $ */

#include "arch/computer_info.h"


main (int argc,char *argv[])
{
  struct computer_hwinfo hwinfo;

  get_hwinfo (&hwinfo);
  report_hwinfo (&hwinfo);
}
