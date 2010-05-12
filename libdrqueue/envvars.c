//
// Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
//
// This file is part of DrQueue
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

#include "envvars.h"
#include "drerrno.h"
#include "logger.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>

#include "pointer.h"

int
envvars_init (struct envvars *envvars) {
  envvars->variables.ptr = NULL;
  envvars->nvariables = 0;
  envvars->evshmid = (int64_t)-1;
  envvars->evsemid = (int64_t)-1;            // FIXME: init with a new semaphore if current invalid
  return 1;
}

int
envvars_empty (struct envvars *envvars) {
  // DEPRECATED
  log_auto (L_INFO,"envvars_empty(): usage deprecated");
  return envvars_free (envvars);
}

int
envvars_free (struct envvars *envvars) {
  // empties the list of environment variables.
  int rv = 1;
  drerrno = DRE_NOERROR;
  if (envvars->evshmid != (int64_t)-1) {
    if (shmctl ((int)envvars->evshmid,IPC_RMID,NULL) == -1) {
      drerrno_system = errno;
      drerrno = DRE_RMSHMEM;
      rv = 0;
    }
  }
  // everything should be fine, so default correct values need to be
  // assigned
  envvars_init (envvars);

  return rv;
}

int envvars_attach (struct envvars *envvars) {
  // This function just tries to attach the segment identified by
  // evshmid to the data structure
  
  drerrno = DRE_NOERROR;

  if (!envvars) {
    log_auto (L_WARNING,"envvars_attach(): received NULL pointer.");
    drerrno = DRE_ATTACHSHMEM;
    return 0;
  }

  if (envvars->evshmid == (int64_t)-1) {
    // This could happen when trying to attach an empty list to
    // search for existing variables, like envvars_variable_find()
    log_auto (L_DEBUG,"envvars_attach(): attempting to use an invalid identifier for environment variables. (%ji)",
              envvars->evshmid);
    drerrno = DRE_ATTACHSHMEM;
    return 0;
  }

  if (envvars->variables.ptr != NULL) {
    // Already attached (?)
    //
    log_auto (L_INFO,"envvars_attach(): envvars already attached (?). Detacching whatever was there.");
    if (shmdt (envvars->variables.ptr) == -1) {
      drerrno_system = errno;
      // Explanation: memory was freed and detached but the variable was not updated.
      log_auto (L_WARNING,"envvars_attach(): could not detach successfully. (%s)",strerror(drerrno_system));
    }
    envvars->variables.ptr = NULL;
  }

  if ((envvars->variables.ptr = (struct envvar *) shmat ((int)envvars->evshmid,0,0)) == (struct envvar *)-1) {
    drerrno_system = errno;
    log_auto (L_ERROR,"envvars_attach(): could not attach on 'shmat'. (%s)",strerror(drerrno_system));
    drerrno = DRE_ATTACHSHMEM;
    return 0;
  }

#ifdef __DEBUG_ENVVARS
  fprintf (stderr,"++++ envvars_attach() ++++ variables == -1 (shmat)\n");
  envvars_dump_info(envvars);
  fprintf (stderr,"+++++++++ envvars_attach() EXIT\n");
#endif

  return 1;
}

void
envvars_dump_info (struct envvars *envvars) {
  // This fuction will try to dump the contents of envvars to stderr
  int i;

  fprintf (stderr,"envvars_dump_info() Starting...\n");
  fprintf (stderr,"variables=%p\n",(void*)envvars->variables.ptr);
  fprintf (stderr,"nvariables=%i\n",envvars->nvariables);
  fprintf (stderr,"evshmid=%ji\n",(intmax_t)envvars->evshmid);

  if (envvars->evshmid != -1) {
    // There's a possible valid value on evshmid. Let's check
    struct envvar *temp;
    temp = (struct envvar *) shmat ((int)envvars->evshmid,0,0);
    if ( temp != (struct envvar *)-1 ) {
      if (envvars->variables.ptr != NULL) {
        // variables had a non-null pointer. Is it valid ?
        // lets compare them
        if (envvars->nvariables) {
          // It supposedly stores nvariables
          if (memcmp (temp,envvars->variables.ptr,sizeof(struct envvar)*envvars->nvariables) == 0) {
            log_auto (L_DEBUG,"envvars_dump_info() The received envvars pointer cotained a VALID but not detached (that could be perfecty normal) shared memory segment\n");
          } else {
            fprintf (stderr,"envvars_dump_info() The received envvars pointxer cotained a non detached shared memory segment\n");
            fprintf (stderr,"envvars_dump_info() But it did not match the real contents of it's evshmid\n");
          }
        } else {
          // It has pointer, but the size is zero. Because nvariables == 0
          fprintf (stderr,"envvars_dump_info() variables pointer NOT null, but nvariables equals 0.\n");
        }
      }

      // it worked. Let's dump variable names and values
      for (i=0;i<envvars->nvariables;i++) {
        fprintf (stderr,"-- Variable %i --\n",i);
        fprintf (stderr,"Name='%s'\n",temp[i].name);
        fprintf (stderr,"Value='%s'\n",temp[i].value);
      }
      
      // TODO: detach tests -> (shmdt (envvars->variables) != -1))
      
      if (shmdt(temp) == -1) {
        fprintf (stderr,"envvars_dump_info() failed at shmdt(temp)\n");
      }
    } else { // temp == -1
      // Thus the evshmid does not exist
      
    }
  } else {
    if (envvars->variables.ptr != NULL) {
      // evshmid is set to -1 but variables contains a wrong pointer.
      fprintf (stderr,"envvars_dump_info() evshmid equals -1 but variables is pointing to non-null (%p).\n",envvars->variables.ptr);
      if (envvars->nvariables != 0) {
        // and even nvariables was not properly initialized.
        fprintf (stderr,"envvars_dump_info() evshmid equals -1 and even nvariables is set to non-zero (%i).\n",envvars->nvariables);
      }
    } else {
      if (envvars->nvariables != 0) {
        // variables is set to NULL but nvariables is not consistent
        fprintf (stderr,"envvars_dump_info() evshmid==-1,variables==NULL ... BUT nvariables is set to non-zero (%i).\n",envvars->nvariables);
      } else {
        // Seems fine.
      }
    }
  }
  fprintf (stderr,"envvars_dump_info() Finished...\n");
}

int
envvars_detach (struct envvars *envvars) {

  if (envvars->variables.ptr != NULL) {
    if (shmdt (envvars->variables.ptr) != -1) {
      // detached 
      envvars->variables.ptr = NULL;
      drerrno = DRE_NOERROR;
      return 1;
    } else {
      drerrno_system = errno;
      drerrno = DRE_DTSHMEM;
      log_auto (L_ERROR,"envvars_detach(): could not detach memory allocated for environment variables. (%s)",
                strerror(drerrno_system));
    }
  }

#ifdef __DEBUG_ENVVARS
  if (envvars->evshmid == (int64_t)-1) {
    fprintf (stderr,"WARNING: envvars_dettach() A shared memory segment with id == -1 was requested to be detached.\n");
  } else if (envvars->variables.ptr == NULL) { 
    fprintf (stderr,"WARNING: envvars_dettach() A shared memory segment pointing to NULL was requested to be detached.\n");
  } else if (!envvars->nvariables) {
    // No need to warn
    fprintf (stderr,"FINE ! Everything looked cool, and even though we couldn't detach.\n");
  } else {
    fprintf (stderr,"WARNING: envvars_dettach() Requested memory segment could not be detached.\n");
    envvars_dump_info(envvars);;
  }
#endif

#ifdef __DEBUG_ENVVARS
  //envvars->variables = NULL;
  //
  fprintf (stderr,"++++ ++++ envvars_dettach() ++++ variables after null\n");
  envvars_dump_info(envvars);
  fprintf (stderr,"++++ ++++ envvars_dettach() EXITING\n");
#endif

  drerrno = DRE_DTSHMEM;
  return 0;
}

struct envvar *
envvars_variable_find (struct envvars *envvars, char *name) {
  // SHARED MEMORY IS LEFT ATTACHED AFTER CALLING THIS FUNCTION (If the variable is found)
  // Remember that you might need to detach shm after calling this
  // function. It is left attached, so the variable found can be
  // updated from outside this function
  struct envvar *result = NULL;
  int i;

  if (!envvars_attach(envvars)) {
    return result;
  }

  for (i = 0; i < envvars->nvariables; i++) {
    if (strncmp (envvars->variables.ptr[i].name,name,MAXNAMELEN) == 0) {
      result = &envvars->variables.ptr[i];
      return result;
    }
  }

  if (!result) {
    envvars_detach(envvars);
  }

  return result;
}

int64_t
envvars_get_shared_memory (int size) {
  // "size" is the number of envvars that we'd like to allocate
  // returns the shared memory identifier or -1 on error
  int64_t shmid;

  drerrno = DRE_NOERROR;

  if ((shmid = (int64_t) shmget (IPC_PRIVATE,sizeof(struct envvar)*size, IPC_EXCL|IPC_CREAT|0600)) == (int64_t)-1) {
    drerrno_system = errno;
    drerrno = DRE_GETSHMEM;
    log_auto (L_WARNING,"envvars_get_shared_memory(): could not allocate memory for %i environment variables. (%s)",
              size, strerror(drerrno_system));
  } else {
    log_auto (L_DEBUG2,"envvars_get_shared_memory(): allocated memory for %i environment variables.",size);
  }

  return shmid;
}

int envvars_variable_add (struct envvars *envvars, char *name, char *value) {

#ifdef __DEBUG_ENVVARS
  fprintf (stderr,"envvars_variable_add() Starting...\n");
#endif
  if (!envvars || !name || !value) {
    return 0;
  }

  // Search for another one with the same name.
  struct envvar *var = envvars_variable_find (envvars,name);
  if (var != NULL) {
    // If the variable already exists UPDATE the value
    strncpy (var->value,value,MAXNAMELEN);
    envvars_detach (envvars); // If found, we have to detach after
                              // the update.
    return 1;
  }

  // New number of environment variables
  int16_t new_size = envvars->nvariables + 1;

  int64_t nshmid;
  if ((nshmid = envvars_get_shared_memory (new_size)) == (int64_t)-1) {
    log_auto (L_ERROR,"envvars_variable_add(): couldn't allocate memory for %i variables. (%s)",
              new_size,strerror(drerrno_system));
    if (!envvars_detach(envvars)) {
      envvars_free(envvars);
    }
    return 0;
  }

  struct envvars new_envvars;
  envvars_init(&new_envvars);
  new_envvars.nvariables = new_size;
  new_envvars.evshmid = nshmid;
  if (!envvars_attach(&new_envvars)) {
    log_auto (L_ERROR,"envvars_variable_add(): could not attach newly allocated space. (%s)",strerror(drerrno_system));
    envvars_detach(envvars);
    envvars_free(&new_envvars);
    return 0;
  }


  // Copy old variables to new allocated memory
  if (envvars->nvariables > 0) {
    envvars_attach(envvars);
    //fprintf (stderr,"DEBUG: copying %i variables from old to new list.\n",envvars->nvariables);
    memcpy (new_envvars.variables.ptr,envvars->variables.ptr,sizeof(struct envvar)*envvars->nvariables);
    envvars_detach(envvars); // And we're done with old variables
    envvars_free(envvars);  // surely
  }

  // Add new variable to new envvars
  // copy the variable that we were adding to the list with this call.
  strncpy (new_envvars.variables.ptr[new_size-1].name,name,MAXNAMELEN);
  strncpy (new_envvars.variables.ptr[new_size-1].value,value,MAXNAMELEN);

  // Copy new values to old structure
  // so the envvars pointer argument of the function gets updated
  envvars->nvariables = new_size;
  envvars->evshmid = nshmid;
  envvars->variables.ptr = new_envvars.variables.ptr;

  // We're done using new_envvars...
  envvars_detach(envvars);

#ifdef __DEBUG_ENVVARS  
  fprintf (stderr,"envvars_variable_add() Added variable (%s,%s) to evshmid (%lli).\n",name,value,nshmid);
  fprintf (stderr,"envvars_variable_add() Finish...\n");
#endif

  drerrno = DRE_NOERROR;
  return 1;
}

int envvars_variable_delete (struct envvars *envvars, char *name) {
  struct envvar *var = envvars_variable_find (envvars,name);

  if (!var) {
    // Trying to delete a non-existing variable
    return 1;
  }

  if (!envvars->nvariables) {
    // we should have returned previously but
    // just in case
    return 1;
  }

  int new_size = envvars->nvariables - 1;

  if (new_size == 0) {
    envvars_detach (envvars);
    return envvars_free (envvars);
  }

  // New shared memory id
  int64_t nshmid = envvars_get_shared_memory (new_size);
  if (nshmid == (int64_t)-1) {
    // TODO: Report
    return 0;
  }
  struct envvars new_envvars;
  envvars_init(&new_envvars);
  new_envvars.nvariables = new_size;
  new_envvars.evshmid = nshmid;
  if (!envvars_attach(&new_envvars)) {
    // TODO: Report
    return 0;
  }

  // Copy all but the deleted one to the new allocated space
  int i,j;
  for (i = 0,j = 0; i < envvars->nvariables; i++) {
    // FIXME: What would happen with two variables with the same name ?
    if (strncmp(envvars->variables.ptr[i].name,name,MAXNAMELEN) == 0) {
      continue;
    }
    memcpy (&new_envvars.variables.ptr[j],&envvars->variables.ptr[i],sizeof(struct envvar));
    j++;
  }

  // Delete old values
  envvars_detach(envvars);
  envvars_free(envvars);

  // Copy new values to old structure
  envvars->nvariables = new_size;
  envvars->evshmid = nshmid;
  envvars->variables.ptr = new_envvars.variables.ptr;
  envvars_detach (envvars);

  drerrno = DRE_NOERROR;
  return 1;
}
