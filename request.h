/* $Id: request.h,v 1.2 2001/05/09 10:53:08 jorge Exp $ */
/* The request structure is not just used for the requests themselves */
/* but also for the answers to the requests */

#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <stdint.h>

#include "database.h"
#include "request_errors.h"
#include "request_codes.h"

struct request {
  unsigned char type;		/* Kind of request */
  unsigned char slave;		/* Is the requester a slave ? For updating lastconn time */
  uint16_t data_s;			/* Data number that might be needed for the request */
};

void handle_request_master (int sfd,struct database *wdb,int icomp);
void handle_request_slave (int sfd,struct database *wdb,int icomp);

void handle_r_r_register (int sfd,struct database *wdb,int icomp);
void handle_r_r_ucstatus (int sfd,struct database *wdb,int icomp);

void update_computer_status (struct computer *computer); /* The slave calls this function to update the */
                                                        /* information that the master has about him */
void register_slave (struct computer *computer);


#endif /* _REQUEST_H_ */






