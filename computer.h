/* $Id: computer.h,v 1.1 2001/04/26 16:07:10 jorge Exp $ */

#ifndef _COMPUTER_H_
#define _COMPUTER_H_

#include "arch/computer_info.h"
#include "arch/computer_status.h"

struct computer {
  struct computer_hwinfo hwinfo;
  struct computer_status status;
};

#endif /* _COMPUTER_H_ */
