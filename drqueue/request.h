/* $Id: request.h,v 1.27 2001/10/04 08:19:52 jorge Exp $ */
/* The request structure is not just used for the requests themselves */
/* but also for the answers to the requests */

#ifndef _REQUEST_H_
#define _REQUEST_H_

#ifdef __LINUX
#include <stdint.h>
#else
# ifdef __IRIX
#include <sys/types.h>
# else
#  error You need to define the OS, or OS defined not supported
# endif
#endif

#include "job.h"
#include "database.h"
#include "request_errors.h"
#include "request_codes.h"
#include "slave.h"

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
void handle_r_r_stopjob  (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_contjob  (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_hstopjob (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobxfer (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobxferfi (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_compxfer (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobfwait (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobfkill (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobffini (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_jobfkfin (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_uclimits (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_slavexit (int sfd,struct database *wdb,int icomp,struct request *req);

/* sent TO MASTER */
void update_computer_status (struct slave_database *database); /* The slave calls this function to update the */
                                                        /* information that the master has about him */
void update_computer_limits (struct computer_limits *limits);

void register_slave (struct computer *computer);
int register_job (struct job *job);
int request_job_available (struct slave_database *sdb);
void request_task_finished (struct slave_database *sdb);
int request_job_delete (uint32_t ijob,int who);
int request_job_stop (uint32_t ijob, int who);
int request_job_hstop (uint32_t ijob, int who);
int request_job_continue (uint32_t ijob, int who);
int request_job_xfer (uint32_t ijob, struct job *job, int who);
int request_job_xferfi (uint32_t ijob, struct frame_info *fi, int nframes, int who);
int request_comp_xfer (uint32_t icomp, struct computer *comp, int who);
int request_job_frame_waiting (uint32_t ijob, uint32_t frame, int who);
int request_job_frame_kill (uint32_t ijob, uint32_t frame, int who);
int request_job_frame_finish (uint32_t ijob, uint32_t frame, int who);
int request_job_frame_kill_finish (uint32_t ijob, uint32_t frame, int who);
int request_slavexit (uint32_t icomp, int who);

/* sent TO SLAVE */
int request_slave_killtask (char *slave,uint16_t itask,int who);
int request_slave_limits_nmaxcpus_set (char *slave, uint32_t nmaxcpus, int who);
int request_slave_limits_maxfreeloadcpu_set (char *slave, uint32_t maxfreeloadcpu, int who);

/* handled by SLAVE */
void handle_rs_r_killtask (int sfd,struct slave_database *sdb,struct request *req);
void handle_rs_r_setnmaxcpus (int sfd,struct slave_database *sdb,struct request *req);
void handle_rs_r_setmaxfreeloadcpu (int sfd,struct slave_database *sdb,struct request *req);

#endif /* _REQUEST_H_ */






