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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
// $Id$
//

#include "envvars.h"
#include "drerrno.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

int envvars_init (struct envvars *envvars) {
  envvars->variables = NULL;
  envvars->nvariables = 0;
  envvars->evshmid = (int64_t)-1;

  drerrno = DRE_NOERROR;
  return 1;
}

int envvars_empty (struct envvars *envvars) {
  // empties the list of environment variables.
  if (envvars->evshmid != (int64_t)-1) {
    if (shmctl (envvars->evshmid,IPC_RMID,NULL) == -1) {
      envvars_init (envvars);
      drerrno = DRE_RMSHMEM;
      return 0;
    }
  }

  // everything should be fine, so default correct values need to be
  // assigned
  envvars_init (envvars);

  drerrno = DRE_NOERROR;
  return 1;
}

int envvars_attach (struct envvars *envvars) {
  // This function just tries to attach the segment identified by
  // evshmid to the data structure
  
  if (envvars->evshmid == (int64_t)-1) {
    // This could happen when trying to attach an empty list to
    // search for existing variables, like envvars_variable_find()
    //fprintf (stderr,"WARNING: envvars_attach() envvars shmid == -1 . Not attaching\n");
    drerrno = DRE_ATTACHSHMEM;
    return 0;
  }

  if (envvars->variables != NULL) {
    //
    // Already attached (?)
    //

    //fprintf (stderr,"WARNING: envvars_attach() variables segment seems to be already attached. Not attaching again.\n");
    //drerrno = DRE_ATTACHSHMEM;
    //return 0;

    // Other option.
    fprintf (stderr,"WARNING: envvars_attach() variables segment seems to be already attached. We proceed as requested.\n");
    shmdt (envvars->variables);
  }

  envvars->variables = (struct envvar *) shmat ((int)envvars->evshmid,0,0);

  if (envvars->variables == (struct envvar *)-1) {
    envvars->variables = NULL;
    drerrno = DRE_ATTACHSHMEM;
    return 0;
  }

#ifdef __DEBUG_ENVVARS
  fprintf (stderr,"++++ envvars_attach() ++++ variables == -1 (shmat)\n");
  envvars_dump_info(envvars);
  fprintf (stderr,"+++++++++ envvars_attach() EXIT\n");
#endif

  drerrno = DRE_NOERROR;
  return 1;
}

void
envvars_dump_info (struct envvars *envvars) {
  // This fuction will try to dump the contents of envvars to stderr
  int i;

  fprintf (stderr,"envvars_dump_info() Starting...\n");
  fprintf (stderr,"variables=%p\n",(void*)envvars->variables);
  fprintf (stderr,"nvariables=%i\n",envvars->nvariables);
  fprintf (stderr,"evshmid=%lli\n",envvars->evshmid);

  if (envvars->evshmid != -1) {
    // There's a possible valid value on evshmid. Let's check
    struct envvar *temp;
    temp = (struct envvar *) shmat (envvars->evshmid,0,0);
    if ( temp != (struct envvar *)-1 ) {
      if (envvars->variables != NULL) {
        // variables had a non-null pointer. Is it valid ?
        // lets compare them
        if (envvars->nvariables) {
          // It supposedly stores nvariables
          if (memcmp (temp,envvars->variables,sizeof(struct envvar)*envvars->nvariables) == 0) {
            fprintf (stderr,"envvars_dump_info() The received envvars pointer cotained a VALID but not detached (that could be perfecty normal) shared memory segment\n");
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
    if (envvars->variables != NULL) {
      // evshmid is set to -1 but variables contains a wrong pointer.
      fprintf (stderr,"envvars_dump_info() evshmid equals -1 but variables is pointing to non-null (%p).\n",envvars->variables);
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

int envvars_detach (struct envvars *envvars) {

  if ((envvars->variables != NULL) && (shmdt (envvars->variables) != -1)) {
    // detached 
    envvars->variables = NULL;
    drerrno = DRE_NOERROR;
    return 1;
  }

#ifdef __DEBUG_ENVVARS
  perror ("envvars_detach");
  if (envvars->evshmid == (int64_t)-1) {
    fprintf (stderr,"WARNING: envvars_dettach() A shared memory segment with id == -1 was requested to be detached.\n");
  } else if (envvars->variables == NULL) { 
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

struct envvar *envvars_variable_find (struct envvars *envvars, char *name) {
  // Remember that you might need to detach shm after calling this
  // function. It is left attached, so the variable found can be
  // updated from outside this function
  struct envvar *result = NULL;
  int i;

  if (!envvars_attach(envvars)) {
#ifdef __DEBUG_ENVVARS
    fprintf (stderr,"NO WORRIES we were looking for the variable '%s' on some yet inexisting shm when found this problem:\n\t\
envvars_variable_find() could not attach variables. (%s)\n",name,drerrno_str());
#endif
    return result;
  }

  for (i = 0; i < envvars->nvariables; i++) {
    if (strncmp (envvars->variables[i].name,name,MAXNAMELEN) == 0) {
      result = &envvars->variables[i];
      return result;
    }
  }

  if (!result) {
    // Not found, so no need to change it.
    envvars_detach(envvars);
  }

  return result;
}

int64_t envvars_get_shared_memory (int size) {
  int64_t shmid;

  if ((shmid = (int64_t) shmget (IPC_PRIVATE,sizeof(struct envvar)*size, IPC_EXCL|IPC_CREAT|0600)) == (int64_t)-1) {
    perror ("shmget");
    drerrno = DRE_GETSHMEM;
    return shmid;
  }

  drerrno = DRE_NOERROR;
  return shmid;
}

int envvars_variable_add (struct envvars *envvars, char *name, char *value) {

#ifdef __DEBUG_ENVVARS
  fprintf (stderr,"envvars_variable_add() Starting...\n");
#endif

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
  int new_size = envvars->nvariables + 1;

  int64_t nshmid = envvars_get_shared_memory (new_size);
  if (nshmid == (int64_t)-1) {
    envvars_detach(envvars);
    fprintf (stderr,"ERROR: envvars_variable_add() could not get shared memory segment of size %i bytes\n",new_size*sizeof(struct envvar));
    return 0;
  }

  struct envvars new_envvars;
  envvars_init(&new_envvars);
  new_envvars.nvariables = new_size;
  new_envvars.evshmid = nshmid;
  if (!envvars_attach(&new_envvars)) {
    envvars_detach(envvars);
    envvars_empty(&new_envvars);
    fprintf (stderr,"ERROR: envvars_variable_add() could not attach newly allocated space. (%s)\n",drerrno_str());
    return 0;
  }


  // Copy old variables to new allocated memory
  if (envvars->nvariables > 0) {
    envvars_attach(envvars);
    //fprintf (stderr,"DEBUG: copying %i variables from old to new list.\n",envvars->nvariables);
    memcpy (new_envvars.variables,envvars->variables,sizeof(struct envvar)*envvars->nvariables);
    envvars_detach(envvars); // And we're done with old variables
    envvars_empty(envvars);  // surely
  }

  // Add new variable to new envvars
  // copy the variable that we were adding to the list with this call.
  strncpy (new_envvars.variables[new_size-1].name,name,MAXNAMELEN);
  strncpy (new_envvars.variables[new_size-1].value,value,MAXNAMELEN);

  // Copy new values to old structure
  // so the envvars pointer argument of the function gets updated
  envvars->nvariables = new_size;
  envvars->evshmid = nshmid;
  envvars->variables = new_envvars.variables;

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
    return envvars_empty (envvars);
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
    if (strncmp(envvars->variables[i].name,name,MAXNAMELEN) == 0) {
      continue;
    }
    memcpy (&new_envvars.variables[j],&envvars->variables[i],sizeof(struct envvar));
    j++;
  }

  // Delete old values
  envvars_detach(envvars);
  envvars_empty(envvars);

  // Copy new values to old structure
  envvars->nvariables = new_size;
  envvars->evshmid = nshmid;
  envvars->variables = new_envvars.variables;
  envvars_detach (envvars);

  drerrno = DRE_NOERROR;
  return 1;
}
