#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''type info here'''
#
# Copyright (C) 2010 Andreas Schroeder
#
# This file is part of DrQueue
#
# DrQueue is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# DrQueue is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
# USA
#

# might use: request, job, database, envvars


##ifdef COMM_REPORT
#extern long int bsent;  /* Bytes sent */
#extern long int brecv;  /* Bytes received */
##endif


#int get_socket (uint16_t port);
def get_socket(port):
  return 0

#int accept_socket_master (int sfd, struct database *wdb, struct sockaddr_in *addr);
def accept_socket_master(sfd, wdb, addr):
  return 0

#int accept_socket_slave (int sfd);
def accept_socket_slave(sfd):
  return 0

#int connect_to_master (void);
def connect_to_master():
  return 0

#int connect_to_slave (char *slave);
def connect_to_slave(slave):
  return 0

#int check_send_datasize (int sfd, uint32_t datasize);
def check_send_datasize(sfd, datasize):
  return 0

#int check_recv_datasize (int sfd, uint32_t datasize);
def check_recv_datasize(sfd, datasize):
  return 0

#int recv_request (int sfd, struct request *request);
def recv_request(sfd, request):
  return 0

#int send_request (int sfd, struct request *request, uint8_t who);
def send_request(sfd, request, who):
  return 0

#int send_computer (int sfd, struct computer *computer, uint8_t use_local_pools);
def send_computer(sfd, computer, use_local_pools):
  return 0

#int recv_computer (int sfd, struct computer *computer);
def recv_computer(sfd, computer):
  return 0

#int recv_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo);
def recv_computer_hwinfo(sfd, hwinfo):
  return 0

#int send_computer_hwinfo (int sfd, struct computer_hwinfo *hwinfo);
def send_computer_hwinfo(sfd, hwinfo):
  return 0

#int recv_computer_status (int sfd, struct computer_status *status);
def recv_computer_status(sfd, status):
  return 0

#int send_computer_status (int sfd, struct computer_status *status);
def send_computer_status(sfd, status):
  return 0

#int recv_computer_loadavg (int sfd, struct computer_status *status);
def recv_computer_loadavg(sfd, status):
  return 0

#int send_computer_loadavg (int sfd, struct computer_status *status);
def send_computer_loadavg(sfd, status):
  return 0

#int recv_computer_ntasks (int sfd, struct computer_status *status);
def recv_computer_ntasks(sfd, status):
  return 0

#int send_computer_ntasks (int sfd, struct computer_status *status);
def send_computer_ntasks(sfd, status):
  return 0

#int recv_computer_limits (int sfd, struct computer_limits *cl);
def recv_computer_limits(sfd, cl):
  return 0

#int send_computer_limits (int sfd, struct computer_limits *cl, uint8_t use_local_pools);
def send_computer_limits(sfd, cl, use_local_pools):
  return 0

#int recv_job (int sfd, struct job *job);
def recv_job(sfd, job):
  return 0

#int send_job (int sfd, struct job *job);
def send_job(sfd, job):
  return 0

#int recv_task (int sfd, struct task *task);
def recv_task(sfd, task):
  return 0

#int send_task (int sfd, struct task *task);
def send_task(sfd, task):
  return 0

#int recv_frame_info (int sfd, struct frame_info *fi);
def recv_frame_info(sfd, fi):
  return 0

#int send_frame_info (int sfd, struct frame_info *fi);
def send_frame_info(sfd, fi):
  return 0

#int recv_autoenable (int sfd, struct autoenable *ae);
def recv_autoenable(sfd, ae):
  return 0

#int send_autoenable (int sfd, struct autoenable *ae);
def send_autoenable(sfd, ae):
  return 0

#int send_blocked_host (int sdf, struct blocked_host *bh, int do_checksize);
def send_blocked_host(sfd, bh, do_checksize):
  return 0

#int recv_blocked_host (int sdf, struct blocked_host *bh, int do_checksize);
def recv_blocked_host(sfd, bh, do_checksize):
  return 0

#int send_computer_pools (int sfd, struct computer_limits *cl, uint8_t use_local_pools);
def send_computer_pools(sfd, cl, use_local_pools):
  return 0

#int recv_computer_pools (int sfd, struct computer_limits *cl);
def recv_computer_pools(sfd, cl):
  return 0

#int send_envvars (int sfd, struct envvars *envvars, int do_checksize);
def send_envvars(sfd, envvars, do_checksize):
  return 0

#int recv_envvars (int sfd, struct envvars *envvars, int do_checksize);
def recv_envvars(sfd, envvars, do_checksize):
  return 0
  
#int send_envvar (int sfd, struct envvar *var, int do_checksize);
def send_envvar(sfd, var, do_checksize):
  return 0

#int recv_envvar (int sfd, struct envvar *var, int do_checksize);
def recv_envvar(sfd, var, do_checksize):
  return 0

#int send_string (int sfd, char *str);
def send_string(sfd, str):
  return 0

#int recv_string (int sfd, char **str);
def recv_string(sfd, str):
  return 0

#int send_blocked_host_list (int sfd, struct blocked_host *bh, uint32_t size, int do_checksize);
def send_blocked_host_list(sfd, bh, size, do_checksize):
  return 0

#int recv_blocked_host_list (int sfd, struct blocked_host **bh, uint32_t *size, int do_checksize);
def recv_blocked_host_list(sfd, bh, size, do_checksize):
  return 0

#int network_initialize();
def network_initialize():
  return 0

#int network_shutdown();
def network_shutdown():
  return 0


