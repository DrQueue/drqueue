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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
// 
// $Id: /drqueue/remote/trunk/request.h 2299 2005-05-13T01:01:43.946387Z jorge	$
// 
// The request structure is not just used for the requests themselves
// but also for the answers to the requests
// 

#ifndef _REQUEST_H_
#define _REQUEST_H_

#ifdef __LINUX
#include <stdint.h>
#elif defined (__IRIX)
#include <sys/types.h>
#elif defined (__OSX)
#include <stdint.h>
#elif defined (__FREEBSD)
#include <stdint.h>
#elif defined (__CYGWIN)
#include <stdint.h>
#else
#error You need to define the OS, or OS defined not supported
#endif

#include "job.h"
#include "database.h"
#include "request_errors.h"
#include "request_codes.h"
#include "slave.h"


#ifdef __CPLUSPLUS
extern "C" {
#endif 

struct request {
	unsigned char type;		/* Kind of request */
	unsigned char who;		/* who sends this request, a master, a client, a slave... (constants.h) */
	uint32_t data;		/* Data number that might be needed for the request */
};

void handle_request_master (int sfd,struct database *wdb,int icomp,struct sockaddr_in *addr);
void handle_request_slave (int sfd,struct slave_database *sdb);

/* handled by MASTER */
int handle_r_r_register (int sfd,struct database *wdb,int icomp,struct sockaddr_in *addr);
void handle_r_r_ucstatus (int sfd,struct database *wdb,int icomp);
void handle_r_r_regisjob (int sfd,struct database *wdb);
void handle_r_r_availjob (int sfd,struct database *wdb,int icomp);
void handle_r_r_taskfini (int sfd,struct database *wdb,int icomp);
void handle_r_r_listjobs (int sfd,struct database *wdb,int icomp);
void handle_r_r_listcomp (int sfd,struct database *wdb,int icomp);
void handle_r_r_deletjob (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_stopjob	 (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_contjob	 (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_hstopjob (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_rerunjob (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobxfer (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobxferfi (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_compxfer (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobfwait (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobfkill (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobffini (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobfkfin (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_uclimits (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_slavexit (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobsesup (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_joblnmcs (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_joblnmccs (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobpriup (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobfinfo (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobfrstrqd (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobblkhost (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobdelblkhost (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_joblstblkhost (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_joblms (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_joblps (int sfd,struct database *wdb,int icomp,struct request *req);

/* sent TO MASTER */
void update_computer_status (struct slave_database *database); /* The slave calls this function to update the */
																												/* information that the master has about him */
void update_computer_limits (struct computer_limits *limits);

void register_slave (struct computer *computer);
int request_comp_xfer (uint32_t icomp, struct computer *comp, int who);
int request_computer_list (struct computer **computer, int who);
int register_job (struct job *job);
int request_job_available (struct slave_database *sdb);
void request_task_finished (struct slave_database *sdb);
int request_job_list (struct job **job, int who);
int request_job_delete (uint32_t ijob,int who);
int request_job_stop (uint32_t ijob, int who);
int request_job_hstop (uint32_t ijob, int who);
int request_job_continue (uint32_t ijob, int who);
int request_job_rerun (uint32_t ijob, int who);
int request_job_xfer (uint32_t ijob, struct job *job, int who);
int request_job_xferfi (uint32_t ijob, struct frame_info *fi, int nframes, int who);
int request_job_frame_waiting (uint32_t ijob, uint32_t frame, int who);
int request_job_frame_kill (uint32_t ijob, uint32_t frame, int who);
int request_job_frame_finish (uint32_t ijob, uint32_t frame, int who);
int request_job_frame_kill_finish (uint32_t ijob, uint32_t frame, int who);
int request_job_frame_info (uint32_t ijob, uint32_t frame, struct frame_info *fi, int who);
int request_slavexit (uint32_t icomp, int who);
int request_job_sesupdate (uint32_t ijob, uint32_t frame_start,uint32_t frame_end,
													 uint32_t frame_step, uint32_t block_size, int who);
int request_job_limits_nmaxcpus_set (uint32_t ijob, uint16_t nmaxcpus, int who);
int request_job_limits_nmaxcpuscomputer_set (uint32_t ijob, uint16_t nmaxcpuscomputer, int who);
int request_job_limits_memory_set (uint32_t ijob, uint32_t memory, int who);
int request_job_limits_pool_set (uint32_t ijob, char *pool, int who);
int request_job_priority_update (uint32_t ijob, uint32_t priority, int who);
int request_job_frame_reset_requeued (uint32_t ijob, uint32_t frame, int who);
int request_job_add_blocked_host (uint32_t ijob, uint32_t icomp, int who);
int request_job_delete_blocked_host (uint32_t ijob, uint32_t icomp, int who);
int request_job_list_blocked_host (uint32_t ijob, struct blocked_host **bh, uint16_t *nblocked, int who);

/* sent TO SLAVE */
int request_slave_killtask (char *slave,uint16_t itask,int who);
int request_slave_limits_enabled_set (char *slave, uint8_t enabled, int who);
int request_slave_limits_nmaxcpus_set (char *slave, uint32_t nmaxcpus, int who);
int request_slave_limits_maxfreeloadcpu_set (char *slave, uint32_t maxfreeloadcpu, int who);
int request_slave_limits_autoenable_set (char *slave, uint32_t h, uint32_t m, unsigned char flags, int who);
int request_slave_limits_pool_add (char *slave, char *pool, int who);
int request_slave_limits_pool_remove (char *slave, char *pool, int who);
int request_slave_job_available (char *slave, int who);
void request_all_slaves_job_available (struct database *wdb);

/* handled by SLAVE */
void handle_rs_r_killtask (int sfd,struct slave_database *sdb,struct request *req);
void handle_rs_r_setnmaxcpus (int sfd,struct slave_database *sdb,struct request *req);
void handle_rs_r_setmaxfreeloadcpu (int sfd,struct slave_database *sdb,struct request *req);
void handle_rs_r_setautoenable (int sfd,struct slave_database *sdb,struct request *req);
void handle_rs_r_limitspooladd (int sfd,struct slave_database *sdb,struct request *req);
void handle_rs_r_limitspoolremove (int sfd,struct slave_database *sdb,struct request *req);
void handle_rs_r_setenabled (int sfd,struct slave_database *sdb,struct request *req);

#ifdef __CPLUSPLUS
}
#endif 

#endif /* _REQUEST_H_ */






