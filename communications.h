/* $Id: communications.h,v 1.2 2001/05/09 10:53:08 jorge Exp $ */

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
void recv_computer_status (int sfd, struct computer_status *status,int who);
void send_computer_status (int sfd, struct computer_status *status,int who);

#endif /* _COMMUNICATIONS_H_ */
