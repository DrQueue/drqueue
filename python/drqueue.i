// 
// Copyright (C) 2001,2002,2003,2004,2005 Jorge Daza Garcia-Blanes
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
// $Id: /drqueue/remote/trunk/jobfinfo.c 2334 2005-07-05T03:50:01.502018Z jorge  $
//
// This program returns the number of times a frame has been requeued.
// Useful for avoiding endless loops
//

%define DOCSTRING
"The drqueue module allows the access to the libdrqueue library responsible
of all major operations that can be applied remotely to drqueue master and
slaves. Also provides access to all data structures of DrQueue."
%enddef
%module (docstring=DOCSTRING) drqueue
%{
#include "libdrqueue.h"
%}

%include "libdrqueue.h"
%include "computer.h"
%include "computer_info.h"
%include "computer_status.h"
%include "task.h"
%include "request.h"
%include "constants.h"
%include "job.h"

typedef unsigned int time_t;
typedef unsigned short int uint16_t;
typedef unsigned long int uint32_t;
typedef unsigned char uint8_t;


%extend job {
	%exception job {
		$action
		if (!result) {
			PyErr_SetString(PyExc_MemoryError,"Not enough memory");
			return NULL;
		}
	}
	job ()
	{
		struct job *j;
		j = malloc (sizeof(struct job));
		if (j)
			job_init (j);
		return j;
	}

	~job ()
	{
		free (self);
	}
}

%pythoncode %{
def get_computer_list (who):
	computer_list = _drqueue.new_computerpp()
	ncomputers = _drqueue.request_computer_list (computer_list,who)
	result = []
	for i in range (ncomputers):
		result.append(_drqueue.get_computer_from_list(computer_list,i))
	_drqueue.free_computerpp(computer_list)
	return result
def get_job_list (who):
	job_list = _drqueue.new_jobpp()
	njobs = _drqueue.request_job_list (job_list,who)
	result = []
	for i in range (njobs):
		result.append(_drqueue.get_job_from_list(job_list,i))
	_drqueue.free_jobpp(job_list)
	return result
import copy
def get_job_frame_list (job,who):
	nframes = _drqueue.job_nframes (job)
	result = {}
	if nframes:
		frame_list = _drqueue.new_frame_info_p (job)
		_drqueue.request_job_xferfi (job.id,frame_list,nframes,who)
		for i in range(nframes):
			frame = _drqueue.get_frame_from_list(frame_list,i)
			frame_copy = copy.copy (frame)
			frame_number = _drqueue.job_frame_index_to_number (job,i)
			result[frame_number] = frame_copy
	return result	
%}

%inline %{
	struct frame_info *get_frame_from_list (struct frame_info *fi, int index)
	{
		return &fi[index];
	}
	struct frame_info *new_frame_info_p (struct job *job)
	{
		int nframes = job_nframes (job);
		struct frame_info *fi = NULL;

		if (nframes) {
			fi = malloc (sizeof(struct frame_info)*nframes);
		}
		return fi;
	}
	void free_frame_info_p (struct frame_info *p)
	{
		free (p);
	}
	struct job **new_jobpp () {
		struct job **r = malloc (sizeof (struct job **));
		return r;
	}
	void free_jobpp (struct job **p) {
		free (p);
	}
	struct job *get_job_from_list (struct job **job,int n) {
		return job[n];
	}

	struct computer **new_computerpp () {
		struct computer **r = malloc (sizeof (struct computer **));
		return r;
	}
	void free_computerpp (struct computer **p) {
		free (p);
	}
	struct computer *get_computer_from_list (struct computer **computer,int n) {
		return computer[n];
	}
%}

// COMPUTER
%extend computer {
	%exception computer {
		$action
		if (!result) {
			PyErr_SetString(PyExc_MemoryError,"Not enough memory");
			return NULL;
		}
	}
	computer ()
	{
		struct computer *c;
		c = malloc (sizeof(struct computer));
		if (c)
			computer_init(c);
		return c;
	}

	~computer ()
	{
		free (self);
	}
}

/* COMPUTER LIMITS */
%extend computer_limits {
	%exception get_pool {
		$action
		if (!result) {
			PyErr_SetString(PyExc_IndexError,"Index out of range");
			return NULL;
		}
	}
	%newobject get_pool;
	struct pool *get_pool (int n)
	{
		struct pool *pool;
		
		if (n >= self->npools) {
			return NULL;
		} else if ( n < 0 ) {
			return NULL;
		}

		pool = (struct pool *) malloc (sizeof (struct pool));
		if (self->npools) {
			if ((self->pool = (struct pool *) computer_pool_attach_shared_memory(self->poolshmid)) == (void*)-1) {
				perror ("Attaching");
				fprintf (stderr,"ERROR attaching memory %d shmid\n", self->poolshmid);
				return pool;
			}
		}
		memcpy(pool,&self->pool[n],sizeof(struct pool));

		computer_pool_detach_shared_memory (self->pool);

		return pool;
	}
}

/* COMPUTER STATUS */
%extend computer_status {

	%exception get_loadavg {
		$action
		if (result == (uint16_t)-1) {
			PyErr_SetString(PyExc_IndexError,"Index out of range");
			return NULL;
		}
	}
	uint16_t get_loadavg (int index)
	{
		if ((index < 0) || (index > 2)) {
			return -1;
		}

		return self->loadavg[index];
	}

	%exception get_task {
		$action
		if (!result) {
			PyErr_SetString(PyExc_IndexError,"Index out of range");
			return NULL;
		}
	}
	struct task *get_task (int index)
	{
		if ((index < 0) || (index >= MAXTASKS)) {
			return NULL;
		}
		return &self->task[index];
	}
}
