/* $Id: communications.h,v 1.1 2001/05/07 15:35:04 jorge Exp $ */

#ifndef _COMMUNICATIONS_H_
#define _COMMUNICATIONS_H_

#include "database.h"
#include "request.h"

int get_socket (short port);
int accept_socket (int sfd,struct database *wdb,int *index);
int connect_to_master (void);

void recv_request (int sfd, struct request *request,int who);
void send_request (int sfd, struct request *request,int who);
void recv_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo,int who);
void send_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo,int who);

#endif /* _COMMUNICATIONS_H_ */
