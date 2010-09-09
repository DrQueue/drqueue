// 
// Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
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
%module (docstring=DOCSTRING) libdrqueue
%{
#include "libdrqueue.h"
%}

%include "typemaps.i"

%newobject request_computer_list;
%typemap(in,numinputs=0) struct computer **computer (struct computer *computer) {
    $1 = &computer;
}
%typemap(argout) struct computer **computer {
    if (result < 0) {
        PyErr_SetString(PyExc_IOError,drerrno_str());
        $result = NULL;
    } else {
        int i;
        PyObject *l = PyList_New(0);
		struct computer *c = malloc (sizeof(struct computer)*result);
        if (!c) {
			Py_DECREF(l);
            return PyErr_NoMemory();
		}
        struct computer *tc = c;
        memcpy (c,*$1,sizeof(struct computer)*result);
        for (i=0; i<result; i++) {
            PyObject *o = SWIG_NewPointerObj((void*)(tc), SWIGTYPE_p_computer, 1);
            PyList_Append(l,o);
            Py_DECREF(o);
            tc++;
        }
        //free (c);
        free (*$1);
        $result = l;
    }
}

%newobject request_job_list;
%typemap(in,numinputs=0) struct job **job (struct job *job) {
    $1 = &job;
}
%typemap(argout) struct job **job {
    if (result < 0) {
        PyErr_SetString(PyExc_IOError,drerrno_str());
        $result = NULL;
    } else {
        int i;
        PyObject *l = PyList_New(0);
        struct job *tj = *$1;
        for (i=0; i<result; i++) {
            struct job *j = (struct job *)malloc(sizeof(struct job));
            if (!j) {
				Py_DECREF(l);
                return PyErr_NoMemory();  
			}
            memcpy(j,($1[i]),sizeof(struct job));
            PyObject *o = SWIG_NewPointerObj((void*)(j), SWIGTYPE_p_job, 1);
            PyList_Append(l,o);
            Py_DECREF(o);
            tj++;
        }
        free(*$1);
        $result = l;
    }
}

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

typedef unsigned int time_t;
typedef unsigned short int uint16_t;
typedef unsigned long int uint32_t;
typedef unsigned char uint8_t;


// these methods generate new objects
%newobject *::request_job_list;
%newobject *::request_computer_list;


// JOB
%extend job {
    %newobject job;
    job ()
    {
        struct job *j;
        j = malloc (sizeof(struct job));
        if (!j)
            return (struct job *)PyErr_NoMemory();
        job_init (j);
        return j;
    }
    
    %delobject ~job;
    ~job ()
    {
        job_init(self);
        //free(self);
        job_frame_info_free (self);
		job_delete (self);
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
            PyErr_SetString(PyExc_IndexError,"No such variable");
            return NULL;
        }
    
        return variable->value;
    }
   
    %newobject request_frame_list;
    PyObject *request_frame_list (int who)
    {
        PyObject *l = PyList_New(0);
        int nframes = job_nframes(self);
        int i;
        if (nframes) {
            struct frame_info *fi = malloc (sizeof(struct frame_info) * nframes);
            if (!fi) {
				Py_DECREF(l);
                return PyErr_NoMemory();
            }
            if (!request_job_xferfi (self->id,fi,nframes,who)) {
                free(fi);
                PyErr_SetString(PyExc_IOError,drerrno_str());
                Py_DECREF(l);
                return NULL;
            }
            for (i=0; i<nframes; i++) {
                PyObject *o = SWIG_NewPointerObj((void*)(&fi[i]), SWIGTYPE_p_frame_info, 0);
                PyList_Append(l,o);
				Py_DECREF(o);
            }
            free (fi);
        }
        Py_INCREF(l);
        return l;
    }
    
    int job_frame_index_to_number (int index)
    {
        if ((index < 0) || (index >= job_nframes(self))) {
            PyErr_SetString(PyExc_IndexError,"frame index out of range");
            return -1;
        }
        
        return job_frame_index_to_number (self,index);
    }
    
    int request_stop (int who)
    {
        if (!request_job_stop (self->id,who)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
            return 0;
        }
        return 1;
    }
    
    int request_rerun (int who)
    {
        if (!request_job_rerun (self->id,who)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
            return 0;
        }
        return 1;
    }
    
    int request_hard_stop (int who)
    {
        if (!request_job_hstop (self->id,who))
        {
            PyErr_SetString(PyExc_IOError,drerrno_str());
            return 0;
        }
        return 1;
    }
    
    int request_delete (int who)
    {
        if (!request_job_delete (self->id,who)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
            return 0;
        }
        return 1;
    }
    
    int request_continue (int who)
    {
        if (!request_job_continue (self->id,who)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
            return 0;
        }
        return 1;
    }
    
    int send_to_queue (void)
    {
        if (!register_job (self)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
            return 0;
        }
        return 1;
    }
    
    int update (int who)
    {
        if (!request_job_xfer(self->id,self,who)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
            return 0;
        }
        return 1;
    }
}



/* COMPUTER LIMITS */
%extend computer_limits {
    %exception get_pool {
        $action
        if (!result || result == (void*)-1) {
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
        if (!pool)
            return (struct pool *)PyErr_NoMemory();
        
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
        if (!request_slave_limits_pool_add(computer_name,pool_name,who))
        {
            PyErr_SetString(PyExc_IOError,drerrno_str());
        }
    }

    void pool_remove (char *computer_name, char *pool_name, int who)
    {
        if (!request_slave_limits_pool_remove(computer_name,pool_name,who))
        {
            PyErr_SetString(PyExc_IOError,drerrno_str());
        }
    }
    
    void pool_list ()
    {
      computer_pool_list (self);
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

// struct pool
%extend pool {
    %newobject pool;
    pool (char *name)
    {
        struct pool *p;
        p = malloc (sizeof(struct pool));
        if (!p)
            return (struct pool *)PyErr_NoMemory();
        memset (p,0,sizeof(struct pool));
        strncpy (p->name,name,MAXNAMELEN-1);
        return p;
    }
    
    %delobject ~pool;
    ~pool ()
    {
        //free (self);
        computer_pool_free (self);
    }
}


// COMPUTER
%extend computer {
    %newobject computer;
    computer ()
    {
        struct computer *c;
        c = malloc (sizeof(struct computer));
        if (!c)
            return (struct computer *)PyErr_NoMemory();
        computer_init(c);
        return c;
    }
    
    %delobject computer;
    ~computer ()
    {
        //free (self);
        computer_free (self);
    }
    
    %newobject list_pools;
    PyObject *list_pools (void)
    {
        PyObject *l = PyList_New(0);
        int npools = self->limits.npools;
        
        if ((self->limits.pool.ptr = (struct pool *) computer_pool_attach_shared_memory(&self->limits)) == (void*)-1)
        {
            PyErr_SetString(PyExc_MemoryError,drerrno_str());
        }
        
        int i;
        for (i=0;i<npools;i++) {
            struct pool *pool_i = (struct pool *)malloc (sizeof(struct pool));
            if (!pool_i) {
				Py_DECREF(l);
                return PyErr_NoMemory();
            }
            memcpy (pool_i,&self->limits.pool.ptr[i],sizeof(struct pool));
            PyObject *o = SWIG_NewPointerObj((void*)(pool_i), SWIGTYPE_p_pool, 0);
            PyList_Append(l,o);
			Py_DECREF(o);
            free(pool_i);
        }
        
        computer_pool_detach_shared_memory (&self->limits);
        return l; 
    }


  PyObject *set_pools (PyObject *pool_list) {
    int i;
    if (!PyList_Check(pool_list)) {
      PyErr_SetString (PyExc_ValueError, "Expecting a list");
      return NULL;
    }
    int npools = PyList_Size(pool_list);
    PyObject *old_list = (PyObject*)computer_list_pools(self);
    if (!PyList_Check(old_list)) {
      PyErr_SetString (PyExc_ValueError, "Expecting a list");
      return NULL;
    }
    for (i=0;i<npools;i++) {
      PyObject *pool_obj = PyList_GetItem(pool_list,i);
      struct pool *tpool = NULL;
      SWIG_Python_ConvertPtr(pool_obj,(void **)&tpool, SWIGTYPE_p_pool, SWIG_POINTER_EXCEPTION | 0 );
      request_slave_limits_pool_add(self->hwinfo.name,tpool->name,CLIENT);
    }
    int onpools = PyList_Size(old_list);
    for (i=0;i<onpools;i++) {
      PyObject *pool_obj = PyList_GetItem(old_list,i);
      struct pool *tpool = NULL;
      SWIG_Python_ConvertPtr(pool_obj,(void **)&tpool, SWIGTYPE_p_pool, SWIG_POINTER_EXCEPTION | 0 );
      request_slave_limits_pool_remove(self->hwinfo.name,tpool->name,CLIENT);
    }
    PyObject *last_list = (PyObject*)computer_list_pools(self);
    return last_list;
  }

    void request_enable (int who)
    {
        if (!request_slave_limits_enabled_set (self->hwinfo.name,1,who)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
        }
    }
    
    void request_disable (int who)
    {
        if (!request_slave_limits_enabled_set (self->hwinfo.name,0,who)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
        }
    }
    
    void update (int who)
    {
        if (!request_comp_xfer(self->hwinfo.id,self,who)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
        }
    }

    void add_pool (char *pool_name, int who)
    {
        if (!request_slave_limits_pool_add(self->hwinfo.name,pool_name,who)) {
            PyErr_SetString(PyExc_IOError,drerrno_str());
        }
    }
    
    void remove_pool (char *pool_name, int who)
    {
        if (!request_slave_limits_pool_remove(self->hwinfo.name,
                                              pool_name,who))
        {
            PyErr_SetString(PyExc_IOError,drerrno_str());
        }
    }
}
