// 
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
// 
// $Id$
//

#ifndef _COMMUNICATIONS_H_
#define _COMMUNICATIONS_H_

#include "request.h"
#include "job.h"
#include "database.h"

#ifdef COMM_REPORT
extern long int bsent;		/* Bytes sent */
extern long int brecv;		/* Bytes received */
#endif


int get_socket (short port);
int accept_socket (int sfd,struct database *wdb,struct sockaddr_in *addr);
int accept_socket_slave (int sfd);
int connect_to_master (void);
int connect_to_slave (char *slave);

int recv_request (int sfd, struct request *request);
int send_request (int sfd, struct request *request,int who);
int send_computer (int sfd, struct computer *computer);
int recv_computer (int sfd, struct computer *computer);
int recv_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo);
int send_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo);
int recv_computer_status (int sfd, struct computer_status *status);
int send_computer_status (int sfd, struct computer_status *status);
int recv_computer_limits (int sfd, struct computer_limits *cl);
int send_computer_limits (int sfd, struct computer_limits *cl);
int recv_job (int sfd, struct job *job);
int send_job (int sfd, struct job *job);
int recv_task (int sfd, struct task *task);
int send_task (int sfd, struct task *task);
int recv_frame_info (int sfd, struct frame_info *fi);
int send_frame_info (int sfd, struct frame_info *fi);
int recv_autoenable (int sfd, struct autoenable *ae);
int send_autoenable (int sfd, struct autoenable *ae);
int send_blocked_host (int sdf, struct blocked_host *bh);
int recv_blocked_host (int sdf, struct blocked_host *bh);

int write_32b (int sfd, void *data);
int write_16b (int sfd, void *data);
int read_32b (int sfd, void *data);
int read_16b (int sfd, void *data);

int dr_read (int fd, void *buf, uint32_t len);
int dr_write (int fd, void *buf, uint32_t len);


#endif /* _COMMUNICATIONS_H_ */
