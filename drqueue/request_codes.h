/* $Id: request_codes.h,v 1.6 2001/07/19 09:05:53 jorge Exp $ */

#ifndef _REQUEST_CODES_H_
#define _REQUEST_CODES_H_

#define R_R_REGISTER 1		/* Request register computer */
#define R_A_REGISTER 2		/* Answer register computer */
#define R_R_UCSTATUS 3		/* Request to update computer status */
#define R_A_UCSTATUS 4		/* Answer to the previous request */
#define R_R_REGISJOB 5		/* Register a new job */
#define R_A_REGISJOB 6		/* Answer to register new job */
#define R_R_AVAILJOB 7		/* Request available job */
#define R_A_AVAILJOB 8		/* Answer to previous */
#define R_R_TASKFINI 9		/* Request task finished, the slave send this when finishes a task */
#define R_A_TASKFINI 10		/* Answer to the previous */
#define R_R_LISTJOBS 11		/* Request a list of current jobs in the queue */
#define R_A_LISTJOBS 12		/* Answer to the previous */
#define R_R_LISTCOMP 13		/* Request a list of current registered computers */
#define R_A_LISTCOMP 14		/* Answer to the previous */

#endif /* _REQUEST_CODES_H_ */
