/* $Id: request.h,v 1.14 2001/08/08 12:59:46 jorge Exp $ */
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
  uint16_t data_s;		/* Data number that might be needed for the request */
};

void handle_request_master (int sfd,struct database *wdb,int icomp);
void handle_request_slave (int sfd,struct slave_database *sdb);

/* MASTER */
void handle_r_r_register (int sfd,struct database *wdb,int icomp);
void handle_r_r_ucstatus (int sfd,struct database *wdb,int icomp);
void handle_r_r_regisjob (int sfd,struct database *wdb);
void handle_r_r_availjob (int sfd,struct database *wdb,int icomp);
void handle_r_r_taskfini (int sfd,struct database *wdb,int icomp);
void handle_r_r_listjobs (int sfd,struct database *wdb,int icomp);
void handle_r_r_listcomp (int sfd,struct database *wdb,int icomp);
void handle_r_r_deletjob (int sfd,struct database *wdb,int icomp);
void handle_r_r_stopjob  (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_contjob  (int sfd,struct database *wdb,int icomp,struct request *req);
void handle_r_r_hstopjob (int sfd,struct database *wdb,int icomp,struct request *req);

/* TO MASTER */
void update_computer_status (struct computer *computer); /* The slave calls this function to update the */
                                                        /* information that the master has about him */
void register_slave (struct computer *computer);
int register_job (struct job *job);
int request_job_available (struct slave_database *sdb);
void request_task_finished (struct slave_database *sdb);
int request_job_delete (struct job *job,int who);
int request_job_stop (uint32_t ijob, int who);
int request_job_hstop (uint32_t ijob, int who);
int request_job_continue (uint32_t ijob, int who);


/* TO SLAVE */
int request_slave_killtask (char *slave,uint16_t itask);

/* SLAVE */
void handle_rs_r_killtask (int sfd,struct slave_database *sdb,struct request *req);

#endif /* _REQUEST_H_ */






