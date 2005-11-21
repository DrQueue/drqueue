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
// $Id$
//

#include "envvars.h"
#include "drerrno.h"

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

int envvars_init (struct envvars *envvars)
{
	envvars->variables = NULL;
	envvars->nvariables = 0;
	envvars->evshmid = -1;

	drerrno = DRE_NOERROR;
	return 1;
}

int envvars_empty (struct envvars *envvars)
{
	if (envvars->evshmid != -1) {
		envvars_detach (envvars);
		if (shmctl (envvars->evshmid,IPC_RMID,NULL) == -1) {
			drerrno = DRE_RMSHMEM;
			return 0;
		}
		envvars_init (envvars);
	}

	drerrno = DRE_NOERROR;
	return 1;
}

int envvars_attach (struct envvars *envvars)
{
	if (envvars->evshmid == -1) {
		drerrno = DRE_ATTACHSHMEM;
		return 0;
	}		

	if (envvars->variables != NULL) {
		// Already attached
		drerrno = DRE_NOERROR;
		return 1;
	}

	envvars->variables = (struct envvar *) shmat (envvars->evshmid,0,0);
	if (envvars->variables == (struct envvar *)-1) {
		perror ("envvars_attach");
		drerrno = DRE_ATTACHSHMEM;
		return 0;
	}

	drerrno = DRE_NOERROR;
	return 1;
}

int envvars_detach (struct envvars *envvars)
{
	if ((envvars->evshmid != -1)
			&& (envvars->variables != NULL)
			&& (shmdt (envvars->variables) != -1))
		{
			envvars->variables = NULL;
			drerrno = DRE_NOERROR;
			return 1;
		}

	drerrno = DRE_DTSHMEM;
	return 0;
}

struct envvar *envvars_variable_find (struct envvars *envvars, char *name)
{
	// Remember that you might need to detach shm after calling this function
	struct envvar *result = NULL;
	int i;

	if (!envvars_attach(envvars)) {
		return result;
	}

	for (i = 0; i < envvars->nvariables; i++) {
		if (strncmp (envvars->variables[i].name,name,MAXNAMELEN) == 0) {
			result = &envvars->variables[i];
			return result;
		}
	}

	return result;
}

int envvars_get_shared_memory (int size)
{
	int shmid;
						
	if ((shmid = shmget (IPC_PRIVATE,sizeof(struct envvar)*size, IPC_EXCL|IPC_CREAT|0600)) == -1) {
		perror ("shmget");
		drerrno = DRE_GETSHMEM;
		return shmid;
	}

	drerrno = DRE_NOERROR;
	return shmid;
}

int envvars_variable_add (struct envvars *envvars, char *name, char *value)
{
	struct envvar *var = envvars_variable_find (envvars,name);

	if (var != NULL) {
		// If the variable already exists we change it's value
		strncpy (var->value,value,MAXNAMELEN);
		return 0;
	}

	int new_size = envvars->nvariables + 1;
	int nshmid = envvars_get_shared_memory (new_size);
	if (nshmid == -1) {
		fprintf (stderr,"envvars_variable_add : could not get shared memory\n");
		return 0;
	}
	struct envvars new_envvars;
	envvars_init(&new_envvars);
	new_envvars.nvariables = new_size;
	new_envvars.evshmid = nshmid;
	if (!envvars_attach(&new_envvars)) {
		return 0;
	}

	// Copy old variables to new allocated memory
	if (envvars->nvariables > 0)
		memcpy (new_envvars.variables,envvars->variables,sizeof(struct envvar) * envvars->nvariables);

	// Add new variable to new envvars
	strncpy (new_envvars.variables[envvars->nvariables].name,name,MAXNAMELEN);
	strncpy (new_envvars.variables[envvars->nvariables].value,value,MAXNAMELEN);
	new_envvars.nvariables = new_size;

	// Delete old values
	envvars_detach(envvars);
	envvars_empty(envvars);

	// Copy new values to old structure
	envvars->nvariables = new_size;
	envvars->evshmid = nshmid;

	drerrno = DRE_NOERROR;
	return 1;
}

int envvars_variable_delete (struct envvars *envvars, char *name)
{
	struct envvar *var = envvars_variable_find (envvars,name);

	if (!var) {
		// Trying to delete a non-existing variable
		envvars_detach (envvars);
		return 0;
	}

	int new_size = envvars->nvariables - 1;

	if (new_size == 0) {
		envvars_detach (envvars);
		return envvars_empty (envvars);
	}

	// New shared memory id
	int nshmid = envvars_get_shared_memory (new_size);
	if (nshmid == -1) {
		return 0;
	}
	struct envvars new_envvars;
	envvars_init(&new_envvars);
	new_envvars.nvariables = new_size;
	new_envvars.evshmid = nshmid;
	if (!envvars_attach(&new_envvars)) {
		return 0;
	}

	// Copy all but the deleted one to the new allocated space
	int i,j;
	for (i = 0,j = 0; i < envvars->nvariables; i++) {
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

	envvars_detach (envvars);

	drerrno = DRE_NOERROR;
	return 1;
}
