/* $Id: communications.h,v 1.5 2001/07/19 09:03:20 jorge Exp $ */

#ifndef _COMMUNICATIONS_H_
#define _COMMUNICATIONS_H_

#include "request.h"
#include "job.h"
#include "database.h"

int get_socket (short port);
int accept_socket (int sfd,struct database *wdb,int *index);
int accept_socket_slave (int sfd);
int connect_to_master (void);

void recv_request (int sfd, struct request *request,int who);
void send_request (int sfd, struct request *request,int who);
void send_computer (int sfd, struct computer *computer,int who);
void recv_computer (int sfd, struct computer *computer,int who);
void recv_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo,int who);
void send_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo,int who);
void recv_computer_status (int sfd, struct computer_status *status,int who);
void send_computer_status (int sfd, struct computer_status *status,int who);
void recv_job (int sfd, struct job *job,int who);
void send_job (int sfd, struct job *job,int who);
void recv_task (int sfd, struct task *task,int who);
void send_task (int sfd, struct task *task,int who);


#endif /* _COMMUNICATIONS_H_ */
