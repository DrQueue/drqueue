// 
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
// Copyright (C) 2007 Andreas Schroeder
//
// This file is part of DrQueue
// 
// DrQueue is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// DrQueue is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
//  


%define DOCSTRING
"The libdrqueue module allows the access to the libdrqueue library responsible
of all major operations that can be applied remotely to drqueue master and
slaves. Also provides access to all data structures of DrQueue."
%enddef
%module (docstring=DOCSTRING) drqueue

%{
#include "libdrqueue.h"
%}


// Tell SWIG to keep track of mappings between C/C++ structs/classes
%trackobjects;

// Specify the mark functions
%markfunc computer "Computer_markfunc";
%markfunc job "Job_markfunc";

// Specify the free functions
%freefunc computer "Computer_freefunc";
%freefunc job "Job_freefunc";


%include "typemaps.i"

%typemap(in,numinputs=0) struct computer **computer (struct computer *computer) {
	$1 = &computer;
}

// the typemap suggested from Gonzalo Garramuño:
%typemap(argout) struct computer** computer {
	if (result < 0) {    	rb_raise( rb_eIOError, drerrno_str() );
    	$result = Qnil;  	} else {    	int i;		/* Create a new ruby array to hold the result */		VALUE l = rb_ary_new();	    /* auxiliary variable to iterate through original array */	    struct computer** c = $1;	    for ( i=0; i < result; ++i, ++c) {			/* copy a single computer struct.  this assumes all members	        inside computer are copyable and not temporary pointers.	        with pointers you may need to use %tracking and do additional	        work - check Zoo/Animal tutorial in ruby manual of swig */
	         
	        struct computer* src = *c;			struct computer* dst = malloc(sizeof(struct computer));	      	memcpy(dst, src, sizeof(struct computer));	      	/* making the pointer owned, means that it will remain around	        until ruby holds no more references to it.  Ruby then         	will eventually call its freefunc to free the memory */      		
      		VALUE o = SWIG_NewPointerObj((void*)(dst), SWIGTYPE_p_computer, SWIG_POINTER_OWN);      		rb_ary_push(l,o);    	}		
		/* free orig. array of computers that drqueue returned        check drqueue api to see if they indeed need to be freed. */
                       	free(*$1);      	/* give array back to ruby as result of typemap */    	$result = l;  	}} 


%typemap(in,numinputs=0) struct job **job (struct job *job) {
	$1 = &job;
}

// simular to computer typemap:
%typemap(argout) struct job **job {
	if(result < 0) {    	rb_raise(rb_eIOError, drerrno_str());
    	$result = Qnil;  	} else {    	int i;		VALUE l = rb_ary_new();	    struct job ** j = $1;	    for(i=0; i < result; ++i, ++j) {
	    		         
	        struct job * src = *j;			struct job * dst = malloc(sizeof(struct job));	      	memcpy(dst, src, sizeof(struct job));      		
      		VALUE o = SWIG_NewPointerObj((void*)(dst), SWIGTYPE_p_job, SWIG_POINTER_OWN);      		rb_ary_push(l,o);    	}
                       	free(*$1);      	$result = l;  	}} 


%typemap(in,numinputs=0) struct frame_info **frame_info (struct frame_info *frame_info) {
	$1 = &frame_info;
}

%typemap(argout) struct frame_info **frame_info {
	if(result < 0) {    	rb_raise(rb_eIOError, drerrno_str());
    	$result = Qnil;  	} else {    	int i;		VALUE l = rb_ary_new();	    struct frame_info ** fi = $1;	    for(i=0; i < result; ++i, ++fi) {
	    		         
	        struct frame_info * src = *fi;			struct frame_info * dst = malloc(sizeof(struct frame_info));	      	memcpy(dst, src, sizeof(struct frame_info));      		
      		VALUE o = SWIG_NewPointerObj((void*)(dst), SWIGTYPE_p_frame_info, SWIG_POINTER_OWN);      		rb_ary_push(l,o);    	}
                       	free(*$1);      	$result = l;  	}}

%typemap(in,numinputs=0) struct pool **pool (struct pool *pool) {
	$1 = &pool;
}

%typemap(argout) struct pool **pool {
	if(result < 0) {    	rb_raise(rb_eIOError, drerrno_str());
    	$result = Qnil;  	} else {    	int i;		VALUE l = rb_ary_new();	    struct pool ** p = $1;	    for(i=0; i < result; ++i, ++p) {
	    		         
	        struct pool * src = *p;			struct pool * dst = malloc(sizeof(struct pool));	      	memcpy(dst, src, sizeof(struct pool));      		
      		VALUE o = SWIG_NewPointerObj((void*)(dst), SWIGTYPE_p_pool, SWIG_POINTER_OWN);      		rb_ary_push(l,o);    	}
                       	free(*$1);      	$result = l;  	}}


%include "pointer.h"
%include "libdrqueue.h"
%include "computer.h"
%include "computer_info.h"
%include "computer_status.h"
%include "task.h"
%include "request.h"
%include "constants.h"
%include "job.h"
%include "envvars.h"
%include "common.h"
%include "computer_pool.h"

// for jobscript generators
%include "blendersg.h"
%include "mentalraysg.h"

// type mapppings
typedef unsigned int time_t;
typedef unsigned short int uint16_t;
typedef unsigned long int uint32_t;
typedef unsigned char uint8_t;


%newobject *::computer_pool_attach_shared_memory;
%newobject *::attach_frame_shared_memory;
%newobject *::attach_blocked_host_shared_memory;

%delobject *::computer_pool_detach_shared_memory;
%delobject *::detach_frame_shared_memory;
%delobject *::detach_blocked_host_shared_memory;%header %{

static void Computer_markfunc(void* ptr) {
 	
 fprintf (stderr,"DEBUG: Computer_markfunc()\n");
 fflush(stderr);
  struct computer * computer = (struct computer *) ptr;
 
 void *cstatus = (void *) &computer->status;
 VALUE status_object = SWIG_RubyInstanceFor(cstatus);
 rb_gc_mark(status_object);
 
 void *chwinfo = (void *) &computer->hwinfo;
 VALUE hwinfo_object = SWIG_RubyInstanceFor(chwinfo);
 rb_gc_mark(hwinfo_object);
 
 void *climits = (void *) &computer->limits;
 VALUE limits_object = SWIG_RubyInstanceFor(climits);
 rb_gc_mark(limits_object);
 
 void *cpool = (void *) &computer->limits.pool.ptr;
 VALUE pool_object = SWIG_RubyInstanceFor(cpool);
 rb_gc_mark(pool_object);
 
 void *clocal_pool = (void *) &computer->limits.local_pool.ptr;
 VALUE local_pool_object = SWIG_RubyInstanceFor(clocal_pool);
 rb_gc_mark(local_pool_object);
 
 void *cautoenable = (void *) &computer->limits.autoenable;
 VALUE autoenable_object = SWIG_RubyInstanceFor(cautoenable);
 rb_gc_mark(autoenable_object);
 }

static void Job_markfunc(void* ptr) {
 	
 fprintf (stderr,"DEBUG: Job_markfunc()\n");
 fflush(stderr);
  struct job * job = (struct job *) ptr;
 
 void *jframe_info = (void *) &job->frame_info.ptr;
 VALUE frame_info_object = SWIG_RubyInstanceFor(jframe_info);
 rb_gc_mark(frame_info_object);
 
 void *jblocked_host = (void *) &job->blocked_host.ptr;
 VALUE blocked_host_object = SWIG_RubyInstanceFor(jblocked_host);
 rb_gc_mark(blocked_host_object);
 
 void *jlimits = (void *) &job->limits;
 VALUE limits_object = SWIG_RubyInstanceFor(jlimits);
 rb_gc_mark(limits_object);
 
 void *jkoji_general = (void *) &job->koji.general;
 VALUE koji_general_object = SWIG_RubyInstanceFor(jkoji_general);
 rb_gc_mark(koji_general_object);
 
 void *jkoji_maya = (void *) &job->koji.maya;
 VALUE koji_maya_object = SWIG_RubyInstanceFor(jkoji_maya);
 rb_gc_mark(koji_maya_object);
 
 void *jkoji_mentalray = (void *) &job->koji.mentalray;
 VALUE koji_mentalray_object = SWIG_RubyInstanceFor(jkoji_mentalray);
 rb_gc_mark(koji_mentalray_object);
 
 void *jkoji_blender = (void *) &job->koji.blender;
 VALUE koji_blender_object = SWIG_RubyInstanceFor(jkoji_blender);
 rb_gc_mark(koji_blender_object);
 
 void *jkoji_bmrt = (void *) &job->koji.bmrt;
 VALUE koji_bmrt_object = SWIG_RubyInstanceFor(jkoji_bmrt);
 rb_gc_mark(koji_bmrt_object);
 
 void *jkoji_pixie = (void *) &job->koji.pixie;
 VALUE koji_pixie_object = SWIG_RubyInstanceFor(jkoji_pixie);
 rb_gc_mark(koji_pixie_object);
 
 void *jkoji_threedelight = (void *) &job->koji.threedelight;
 VALUE koji_threedelight_object = SWIG_RubyInstanceFor(jkoji_threedelight);
 rb_gc_mark(koji_threedelight_object);
 
 void *jkoji_lightwave = (void *) &job->koji.lightwave;
 VALUE koji_lightwave_object = SWIG_RubyInstanceFor(jkoji_lightwave);
 rb_gc_mark(koji_lightwave_object);
 
 void *jkoji_nuke = (void *) &job->koji.nuke;
 VALUE koji_nuke_object = SWIG_RubyInstanceFor(jkoji_nuke);
 rb_gc_mark(koji_nuke_object);
 
 void *jkoji_terragen = (void *) &job->koji.terragen;
 VALUE koji_terragen_object = SWIG_RubyInstanceFor(jkoji_terragen);
 rb_gc_mark(koji_terragen_object);
 
 void *jkoji_aqsis = (void *) &job->koji.aqsis;
 VALUE koji_aqsis_object = SWIG_RubyInstanceFor(jkoji_aqsis);
 rb_gc_mark(koji_aqsis_object);
 
 void *jkoji_mantra = (void *) &job->koji.mantra;
 VALUE koji_mantra_object = SWIG_RubyInstanceFor(jkoji_mantra);
 rb_gc_mark(koji_mantra_object);
 
 void *jkoji_aftereffects = (void *) &job->koji.aftereffects;
 VALUE koji_aftereffects_object = SWIG_RubyInstanceFor(jkoji_aftereffects);
 rb_gc_mark(koji_aftereffects_object);
 
 void *jkoji_shake = (void *) &job->koji.shake;
 VALUE koji_shake_object = SWIG_RubyInstanceFor(jkoji_shake);
 rb_gc_mark(koji_shake_object);
 
 void *jkoji_turtle = (void *) &job->koji.turtle;
 VALUE koji_turtle_object = SWIG_RubyInstanceFor(jkoji_turtle);
 rb_gc_mark(koji_turtle_object);
 
 void *jkoji_xsi = (void *) &job->koji.xsi;
 VALUE koji_xsi_object = SWIG_RubyInstanceFor(jkoji_xsi);
 rb_gc_mark(koji_xsi_object);
 
 void *jenvvars = (void *) &job->envvars;
 VALUE envvars_object = SWIG_RubyInstanceFor(jenvvars);
 rb_gc_mark(envvars_object);
 }

static void Computer_freefunc(void* ptr) {
 
 fprintf (stderr,"DEBUG: Computer_freefunc()\n");	
 fflush(stderr);
  struct computer * computer = (struct computer *) ptr;
 
 void *cstatus = (void *) &computer->status; SWIG_RubyUnlinkObjects(cstatus); SWIG_RubyRemoveTracking(cstatus);
 
 void *chwinfo = (void *) &computer->hwinfo; SWIG_RubyUnlinkObjects(chwinfo); SWIG_RubyRemoveTracking(chwinfo);
 
 void *climits = (void *) &computer->limits; SWIG_RubyUnlinkObjects(climits); SWIG_RubyRemoveTracking(climits);
 
 void *cpool = (void *) &computer->limits.pool.ptr; SWIG_RubyUnlinkObjects(cpool); SWIG_RubyRemoveTracking(cpool);
 
 void *clocal_pool = (void *) &computer->limits.local_pool.ptr; SWIG_RubyUnlinkObjects(clocal_pool); SWIG_RubyRemoveTracking(clocal_pool);
 
 void *cautoenable = (void *) &computer->limits.autoenable; SWIG_RubyUnlinkObjects(cautoenable); SWIG_RubyRemoveTracking(cautoenable);
 
 SWIG_RubyRemoveTracking(ptr);
 
 fprintf (stderr,"DEBUG: computer_detach(computer)\n");	
 fflush(stderr);
 
 computer_detach(computer);
 
 fprintf (stderr,"DEBUG: free(computer)\n");	
 fflush(stderr);
   // Now free the computer object free(computer);
 }

static void Job_freefunc(void* ptr) {
 
 fprintf (stderr,"DEBUG: Job_freefunc()\n");	
 fflush(stderr);
 
 struct job * job = (struct job *) ptr;
 
 void *jframe_info = (void *) &job->frame_info.ptr;
 SWIG_RubyUnlinkObjects(jframe_info); SWIG_RubyRemoveTracking(jframe_info);
 
 void *jblocked_host = (void *) &job->blocked_host.ptr;
 SWIG_RubyUnlinkObjects(jblocked_host); SWIG_RubyRemoveTracking(jblocked_host);
 
 void *jlimits = (void *) &job->limits;
 SWIG_RubyUnlinkObjects(jlimits); SWIG_RubyRemoveTracking(jlimits);
 
 void *jkoji_general = (void *) &job->koji.general;
 SWIG_RubyUnlinkObjects(jkoji_general); SWIG_RubyRemoveTracking(jkoji_general);
 
 void *jkoji_maya = (void *) &job->koji.maya;
 SWIG_RubyUnlinkObjects(jkoji_maya); SWIG_RubyRemoveTracking(jkoji_maya);
 
 void *jkoji_mentalray = (void *) &job->koji.mentalray;
 SWIG_RubyUnlinkObjects(jkoji_mentalray); SWIG_RubyRemoveTracking(jkoji_mentalray);
 
 void *jkoji_blender = (void *) &job->koji.blender;
 SWIG_RubyUnlinkObjects(jkoji_blender); SWIG_RubyRemoveTracking(jkoji_blender);
 
 void *jkoji_bmrt = (void *) &job->koji.bmrt;
 SWIG_RubyUnlinkObjects(jkoji_bmrt); SWIG_RubyRemoveTracking(jkoji_bmrt);
 
 void *jkoji_pixie = (void *) &job->koji.pixie;
 SWIG_RubyUnlinkObjects(jkoji_pixie); SWIG_RubyRemoveTracking(jkoji_pixie);
 
 void *jkoji_threedelight = (void *) &job->koji.threedelight;
 SWIG_RubyUnlinkObjects(jkoji_threedelight); SWIG_RubyRemoveTracking(jkoji_threedelight);
 
 void *jkoji_lightwave = (void *) &job->koji.lightwave;
 SWIG_RubyUnlinkObjects(jkoji_lightwave); SWIG_RubyRemoveTracking(jkoji_lightwave);
 
 void *jkoji_nuke = (void *) &job->koji.nuke;
 SWIG_RubyUnlinkObjects(jkoji_nuke); SWIG_RubyRemoveTracking(jkoji_nuke);
 
 void *jkoji_terragen = (void *) &job->koji.terragen;
 SWIG_RubyUnlinkObjects(jkoji_terragen); SWIG_RubyRemoveTracking(jkoji_terragen);
 
 void *jkoji_aqsis = (void *) &job->koji.aqsis;
 SWIG_RubyUnlinkObjects(jkoji_aqsis); SWIG_RubyRemoveTracking(jkoji_aqsis);
 
 void *jkoji_mantra = (void *) &job->koji.mantra;
 SWIG_RubyUnlinkObjects(jkoji_mantra); SWIG_RubyRemoveTracking(jkoji_mantra);
 
 void *jkoji_aftereffects = (void *) &job->koji.aftereffects;
 SWIG_RubyUnlinkObjects(jkoji_aftereffects); SWIG_RubyRemoveTracking(jkoji_aftereffects);
 
 void *jkoji_shake = (void *) &job->koji.shake;
 SWIG_RubyUnlinkObjects(jkoji_shake); SWIG_RubyRemoveTracking(jkoji_shake);
 
 void *jkoji_turtle = (void *) &job->koji.turtle;
 SWIG_RubyUnlinkObjects(jkoji_turtle); SWIG_RubyRemoveTracking(jkoji_turtle);
 
 void *jkoji_xsi = (void *) &job->koji.xsi;
 SWIG_RubyUnlinkObjects(jkoji_xsi); SWIG_RubyRemoveTracking(jkoji_xsi);
 
 void *jenvvars = (void *) &job->envvars;
 SWIG_RubyUnlinkObjects(jenvvars); SWIG_RubyRemoveTracking(jenvvars);
  
 SWIG_RubyRemoveTracking(ptr);
   // Now free the job object free(job);
 }

%}


// JOB
%extend job {
	job ()
	{
		struct job *j;
		j = (struct job *)malloc (sizeof(struct job));
		if (!j) {
			rb_raise(rb_eNoMemError,"out of memory");
			return (VALUE)NULL;
		}
		job_init (j);
		return j;
	}

	~job ()
	{
		job_init(self);
		free (self);
	}	

	int environment_variable_add (char *name, char *value)
	{
		return envvars_variable_add (&self->envvars,name,value);
	}

	int environment_variable_delete (char *name)
	{
		return envvars_variable_delete (&self->envvars,name);
	}

	char *environment_variable_find (char *name)
	{
		struct envvar *variable;

		variable = envvars_variable_find (&self->envvars,name);
	
		if (!variable) {
			rb_raise(rb_eIndexError,"No such variable");
			return (VALUE)NULL;
		}

		return variable->value;
	}

	VALUE request_frame_list (int who)
	{
		VALUE l = rb_ary_new();
		int nframes = job_nframes(self);
		int i;
		if (nframes) {
			struct frame_info *fi = (struct frame_info *)malloc (sizeof(struct frame_info) * nframes);
			if (!fi) {
				rb_raise(rb_eNoMemError,"out of memory");
				return (VALUE)NULL;
			}
			if (!request_job_xferfi (self->id,fi,nframes,who)) {
				rb_raise(rb_eIOError,drerrno_str());
				return (VALUE)NULL;
			}
			for (i=0; i<nframes; i++) {
				//VALUE o = SWIG_NewPointerObj((void*)(&fi[i]), SWIGTYPE_p_frame_info, 0);
				VALUE o = SWIG_NewPointerObj((void*)(&fi[i]), SWIGTYPE_p_frame_info, SWIG_POINTER_OWN);
				rb_ary_push(l,o);
			}
		}
		return l;
	}

	int job_frame_index_to_number (int index)
	{
		if ((index < 0) || (index >= job_nframes(self))) {
			rb_raise(rb_eIndexError,"frame index out of range");
			return -1;
		}

		return job_frame_index_to_number (self,index);
	}

	int request_stop (int who)
	{
		if (!request_job_stop (self->id,who)) {
			rb_raise(rb_eIOError,drerrno_str());
			return 0;
		}
		return 1;
	}

	int request_rerun (int who)
	{
		if (!request_job_rerun (self->id,who)) {
			rb_raise(rb_eIOError,drerrno_str());
			return 0;
		}
		return 1;
	}

	int request_hard_stop (int who)
	{
		if (!request_job_hstop (self->id,who)) {
			rb_raise(rb_eIOError,drerrno_str());
			return 0;
		}
		return 1;
	}

	int request_delete (int who)
	{
		if (!request_job_delete (self->id,who)) {
			rb_raise(rb_eIOError,drerrno_str());
			return 0;
		}
		return 1;
	}

	int request_continue (int who)
	{
		if (!request_job_continue (self->id,who)) {
			rb_raise(rb_eIOError,drerrno_str());
			return 0;
		}
		return 1;
	}

	int send_to_queue (void)
	{
		if (!register_job (self)) {
			rb_raise(rb_eIOError,drerrno_str());
			return 0;
		}
		return 1;
	}

	int update (int who)
	{
		if (!request_job_xfer(self->id,self,who)) {
			rb_raise(rb_eIOError,drerrno_str());
			return 0;
		}
		return 1;
	}
	
	
	// Blender script file generation
	char *blendersg (char *scene, char *scriptdir, int blender)
	{	
		struct blendersgi *blend = (struct blendersgi *)malloc (sizeof(struct blendersgi));
    	if (!blend) {
 	     	rb_raise(rb_eNoMemError,"out of memory");
    	 	return NULL;
   		}	
		
		char *outfile = (char *)malloc(sizeof(char *));
		if (!outfile) {
 	     	rb_raise(rb_eNoMemError,"out of memory");
    	 	return NULL;
   		}
		
		memset (blend,0,sizeof(struct blendersgi));
		
		strncpy(blend->scene, scene, BUFFERLEN-1);
		strncpy(blend->scriptdir, scriptdir, BUFFERLEN-1);
		blend->blender = blender;
		
  		outfile = blendersg_create(blend);
  		
		if (!outfile) {
			rb_raise(rb_eException,"Problem creating script file");
      		return NULL;
		}
		
		return outfile;
	}
	
	
	// MentalRay script file generation
	char *mentalraysg (char *scene, char *scriptdir, char *renderdir, char *image, char *file_owner, char *camera, int res_x, int res_y, char *format, int mentalray)
	{	
		struct mentalraysgi *ment = (struct mentalraysgi *)malloc (sizeof(struct mentalraysgi));
    	if (!ment) {
 	     	rb_raise(rb_eNoMemError,"out of memory");
    	 	return NULL;
   		}	
		
		char *outfile = (char *)malloc(sizeof(char *));
		if (!outfile) {
 	     	rb_raise(rb_eNoMemError,"out of memory");
    	 	return NULL;
   		}
		
		memset (ment,0,sizeof(struct mentalraysgi));
		
		strncpy(ment->renderdir, renderdir, BUFFERLEN-1);
		strncpy(ment->scene, scene, BUFFERLEN-1);
		strncpy(ment->image, image, BUFFERLEN-1);
		strncpy(ment->scriptdir, scriptdir, BUFFERLEN-1);
		strncpy(ment->file_owner, file_owner, BUFFERLEN-1);
		strncpy(ment->camera, camera, BUFFERLEN-1);
		ment->res_x = res_x;
		ment->res_y = res_y;
		strncpy(ment->format, format, BUFFERLEN-1);
		ment->mentalray = mentalray;
		
  		outfile = mentalraysg_create(ment);
  		
		if (!outfile) {
			rb_raise(rb_eException,"Problem creating script file");
      		return NULL;
		}
		
		return outfile;
	}
	
}



// COMPUTER LIMITS
%extend computer_limits {
	%exception get_pool {
		$action
		if ((!result) || (result == (void *)-1)) {
			rb_raise(rb_eIndexError,"Index out of range");
			return (VALUE)NULL;
		}
	}
	struct pool *get_pool (int n)
	{
		struct pool *pool;
		
		if (n >= self->npools) {
			return (VALUE)NULL;
		} else if ( n < 0 ) {
			return (VALUE)NULL;
		}

		pool = (struct pool *)malloc (sizeof (struct pool));
		if (!pool) {
			rb_raise(rb_eNoMemError,"out of memory");
			return (VALUE)NULL;
		}
		
		if (self->npools) {
			if ((self->pool.ptr = (struct pool *) computer_pool_attach_shared_memory(self)) == (void*)-1) {
				return pool;
			}
		}
		memcpy(pool,&self->pool.ptr[n],sizeof(struct pool));

		computer_pool_detach_shared_memory (self);

		return pool;
	}

  void pool_add (char *computer_name, char *pool_name, int who)
  {
		if (!request_slave_limits_pool_add(computer_name,pool_name,who)) {
			rb_raise(rb_eIOError,drerrno_str());
    }
  }

  void pool_remove (char *computer_name, char *pool_name, int who)
  {
		if (!request_slave_limits_pool_remove(computer_name,pool_name,who)) {
			rb_raise(rb_eIOError,drerrno_str());
    }
  }

  void pool_list ()
  {
    computer_pool_list (self);
  }
}

// COMPUTER STATUS 
%extend computer_status {
	%exception get_loadavg {
		$action
		if (result == (uint16_t)-1) {
			rb_raise(rb_eIndexError,"Index out of range");
			return (VALUE)NULL;
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
			rb_raise(rb_eIndexError,"Index out of range");
			return (VALUE)NULL;
		}
	}
	struct task *get_task (int index)
	{
		if ((index < 0) || (index >= MAXTASKS)) {
			return (VALUE)NULL;
		}
		return &self->task[index];
	}
}

// struct pool
%extend pool {
  pool (char *name)
  {
    struct pool *p;
    p = (struct pool *)malloc (sizeof(struct pool));
    if (!p) {
      rb_raise(rb_eNoMemError,"out of memory");
      return (VALUE)NULL;
    }
    memset (p,0,sizeof(struct pool));
    strncpy (p->name,name,MAXNAMELEN-1);
    return p;
  }

  ~pool ()
  {
    free (self);
  }
}


// COMPUTER
%extend computer {
	
	computer ()
	{
		struct computer *c;
		c = (struct computer *)malloc (sizeof(struct computer));
		if (!c) {
			rb_raise(rb_eNoMemError,"out of memory");
			return (VALUE)NULL;
		}
		computer_init(c);
		return c;
	}

	~computer ()
	{
		free (self);
	}

  VALUE list_pools (void)
  {
    VALUE l = rb_ary_new();
	int npools = self->limits.npools;

    if ((self->limits.pool.ptr = (struct pool *) computer_pool_attach_shared_memory(&self->limits)) == (void*)-1) {
    	rb_raise(rb_eException,drerrno_str());
    	return (VALUE)NULL;
	}

    int i;
    for (i=0;i<npools;i++) {
      struct pool *pool_i = (struct pool *)malloc (sizeof(struct pool));
			if (!pool_i) {
				rb_raise(rb_eNoMemError,"out of memory");
				return (VALUE)NULL;
			}
      memcpy (pool_i,&self->limits.pool.ptr[i],sizeof(struct pool));
      //VALUE o = SWIG_NewPointerObj((void*)(pool_i), SWIGTYPE_p_pool, 0);
	  VALUE o = SWIG_NewPointerObj((void*)(pool_i), SWIGTYPE_p_pool, SWIG_POINTER_OWN);
	  rb_ary_push(l,o);
	  }

    computer_pool_detach_shared_memory (&self->limits);
	return l; 
  }

  VALUE set_pools (VALUE pool_list) {	
    int i;
    if (RARRAY(pool_list)->len >0) {	
      rb_raise(rb_eException,"Expecting a list");
      return (VALUE)NULL;
    }
    int npools = RARRAY(pool_list)->len;
    VALUE old_list = computer_list_pools(self);
    if (RARRAY(old_list)->len >0) {
      rb_raise(rb_eException,"Expecting a list");
      return (VALUE)NULL;
    }
    for (i=0;i<npools;i++) {
      VALUE pool_obj = rb_ary_entry(pool_list,i);
      struct pool *tpool = NULL;
      //SWIG_ConvertPtr(pool_obj,(void **)&tpool, SWIGTYPE_p_pool, SWIG_POINTER_EXCEPTION | 0 );
      SWIG_ConvertPtr(pool_obj,(void **)&tpool, SWIGTYPE_p_pool, SWIG_POINTER_EXCEPTION | SWIG_POINTER_OWN );
      request_slave_limits_pool_add(self->hwinfo.name,tpool->name,CLIENT);
    }
    int onpools = RARRAY(old_list)->len;
    for (i=0;i<onpools;i++) {
      VALUE pool_obj = rb_ary_entry(old_list,i);
      struct pool *tpool = NULL;
      //SWIG_ConvertPtr(pool_obj,(void **)&tpool, SWIGTYPE_p_pool, SWIG_POINTER_EXCEPTION | 0 );
      SWIG_ConvertPtr(pool_obj,(void **)&tpool, SWIGTYPE_p_pool, SWIG_POINTER_EXCEPTION | SWIG_POINTER_OWN );
      request_slave_limits_pool_remove(self->hwinfo.name,tpool->name,CLIENT);
    }
    VALUE last_list = computer_list_pools(self);
    return last_list;
  }

	void request_enable (int who)
	{
		if (!request_slave_limits_enabled_set (self->hwinfo.name,1,who)) {
			rb_raise(rb_eIOError,drerrno_str());
		}
	}

	void request_disable (int who)
	{
		if (!request_slave_limits_enabled_set (self->hwinfo.name,0,who)) {
			rb_raise(rb_eIOError,drerrno_str());
		}
	}

	void update (int who)
	{
		if (!request_comp_xfer(self->hwinfo.id,self,who)) {
			rb_raise(rb_eIOError,drerrno_str());
		}
	}

  void add_pool (char *pool_name, int who)
  {
    if (!request_slave_limits_pool_add(self->hwinfo.name,pool_name,who)) {
		  	rb_raise(rb_eIOError,drerrno_str());
    }
  }

  void remove_pool (char *pool_name, int who)
  {
    if (!request_slave_limits_pool_remove(self->hwinfo.name,pool_name,who)) {
		  	rb_raise(rb_eIOError,drerrno_str());
    }
  }
}
