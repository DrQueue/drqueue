/* $Id: request_codes.h,v 1.3 2001/05/28 14:21:31 jorge Exp $ */

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

#endif /* _REQUEST_CODES_H_ */
