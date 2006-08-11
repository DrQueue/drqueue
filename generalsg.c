// 
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA      02111-1307
// USA
// 

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "generalsg.h"
#include "libdrqueue.h"

char *generalsg_create (struct generalsgi *info)
{
        /* This function creates the general render script based on the information given */
        /* Returns a pointer to a string containing the path of the just created file */
        /* Returns NULL on failure and sets drerrno */
        FILE *f;
        FILE *etc_general_sg;           /* The general script generator configuration file */
        int fd_etc_general_sg,fd_f;
        static char drqueue_filename[BUFFERLEN];
        char fn_etc_general_sg[BUFFERLEN]; /* File name pointing to DRQUEUE_ETC/general.sg */
        char buf[BUFFERLEN];
        int size;
        char *p;                        /* Scene filename without path */
        char drqueue_scriptdir[MAXCMDLEN];

		//  For debugging what info is getting set
/*
    fprintf(stderr,"DEBUG:: Entering generalsg_create\n");
    fprintf(stderr,"\tOWNER_ID: %d\n",info->uid_owner);
    fprintf(stderr,"\tOWNER_GRP_ID: %d\n",info->gid_owner);
    fprintf(stderr,"\tSCRIPT: %s\n",info->script);
    fprintf(stderr,"\tSCRIPT PATH: %s\n",info->scriptdir);
    fprintf(stderr,"\tDRQ_SCRIPTDIR: %s\n",info->drqueue_scriptdir);
*/

#ifdef __CYGWIN
        cygwin_conv_to_posix_path(info->drqueue_scriptdir, drqueue_scriptdir);
#else
        strncpy(drqueue_scriptdir,info->drqueue_scriptdir,MAXCMDLEN-1);
#endif

        p = strrchr(drqueue_scriptdir,'/');
        p = ( p ) ? p+1 : drqueue_scriptdir;
        snprintf(drqueue_filename,BUFFERLEN-1,"%s/%s.%lX",info->drqueue_scriptdir,p,(unsigned long int)time(NULL));
     // fprintf(stderr,"DEBUG:: DRQ_FILENAME: %s\n",drqueue_filename);

        if ((f = fopen (drqueue_filename, "a")) == NULL) {
                if (errno == ENOENT) {
                        /* If its because the directory does not exist we try creating it first */
                        if (mkdir (info->drqueue_scriptdir,0775) == -1) {
                                drerrno = DRE_COULDNOTCREATE;
                                return NULL;
                        } else if ((f = fopen (drqueue_filename, "a")) == NULL) {
                                drerrno = DRE_COULDNOTCREATE;
                                return NULL;
                        }
                } else {
                        drerrno = DRE_COULDNOTCREATE;
                        return NULL;
                }
        }

        fchmod (fileno(f),0777);

        /* So now we have the file open and so we must write to it */
        fprintf(f,"#!/usr/bin/env tcsh\n\n");
        fprintf(f,"set DRQUEUE_OWNER_UID=%d\n",info->uid_owner);
        fprintf(f,"set DRQUEUE_OWNER_GID=%d\n",info->gid_owner);

				// For when I've got the script path stored correctly
        // fprintf(f,"set DRQUEUE_SCRIPT=%s/%s\n",info->scriptdir,info->script);
        fprintf(f,"set DRQUEUE_SCRIPT=%s\n",info->script);

        fflush (f);

        snprintf(fn_etc_general_sg,BUFFERLEN-1,"%s/general.sg",getenv("DRQUEUE_ETC"));
        if ((etc_general_sg = fopen (fn_etc_general_sg,"r")) == NULL) {
                fprintf(f,"\necho -------------------------------------------------\n");
                fprintf(f,"echo ATTENTION ! There was a problem opening: %s\n",fn_etc_general_sg);
                fprintf(f,"echo Please correct the problem\n");
                fprintf(f,"echo -------------------------------------------------\n");
                fprintf(f,"\n\n");
        } else {
                fd_etc_general_sg = fileno (etc_general_sg);
                fd_f = fileno (f);

            // fprintf(stderr,"DEBUG: Template Script: %s\n",fn_etc_general_sg);
                while ((size = read (fd_etc_general_sg,buf,BUFFERLEN)) != 0) {
                        write (fd_f,buf,size);
                }
                fclose(etc_general_sg);
        }

        fclose(f);

        return drqueue_filename;
}


char *generalsg_default_script_path (void)
{
        static char buf[BUFFERLEN];
        char *p;

        if (!(p = getenv("DRQUEUE_TMP"))) {
                return ("/drqueue_tmp/not/set/");
        }

        if (p[strlen(p)-1] == '/')
                snprintf (buf,BUFFERLEN-1,"%s",p);
        else
                snprintf (buf,BUFFERLEN-1,"%s/",p);

        return buf;
}
// 
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA      02111-1307
// USA
// 

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "generalsg.h"
#include "libdrqueue.h"

char *generalsg_create (struct generalsgi *info)
{
        /* This function creates the general render script based on the information given */
        /* Returns a pointer to a string containing the path of the just created file */
        /* Returns NULL on failure and sets drerrno */
        FILE *f;
        FILE *etc_general_sg;           /* The general script generator configuration file */
        int fd_etc_general_sg,fd_f;
        static char drqueue_filename[BUFFERLEN];
        char fn_etc_general_sg[BUFFERLEN]; /* File name pointing to DRQUEUE_ETC/general.sg */
        char buf[BUFFERLEN];
        int size;
        char *p;                        /* Scene filename without path */
        char drqueue_scriptdir[MAXCMDLEN];

		//  For debugging what info is getting set
/*
    fprintf(stderr,"DEBUG:: Entering generalsg_create\n");
    fprintf(stderr,"\tOWNER_ID: %d\n",info->uid_owner);
    fprintf(stderr,"\tOWNER_GRP_ID: %d\n",info->gid_owner);
    fprintf(stderr,"\tSCRIPT: %s\n",info->script);
    fprintf(stderr,"\tSCRIPT PATH: %s\n",info->scriptdir);
    fprintf(stderr,"\tDRQ_SCRIPTDIR: %s\n",info->drqueue_scriptdir);
*/

#ifdef __CYGWIN
        cygwin_conv_to_posix_path(info->drqueue_scriptdir, drqueue_scriptdir);
#else
        strncpy(drqueue_scriptdir,info->drqueue_scriptdir,MAXCMDLEN-1);
#endif

        p = strrchr(drqueue_scriptdir,'/');
        p = ( p ) ? p+1 : drqueue_scriptdir;
        snprintf(drqueue_filename,BUFFERLEN-1,"%s/%s.%lX",info->drqueue_scriptdir,p,(unsigned long int)time(NULL));
     // fprintf(stderr,"DEBUG:: DRQ_FILENAME: %s\n",drqueue_filename);

        if ((f = fopen (drqueue_filename, "a")) == NULL) {
                if (errno == ENOENT) {
                        /* If its because the directory does not exist we try creating it first */
                        if (mkdir (info->drqueue_scriptdir,0775) == -1) {
                                drerrno = DRE_COULDNOTCREATE;
                                return NULL;
                        } else if ((f = fopen (drqueue_filename, "a")) == NULL) {
                                drerrno = DRE_COULDNOTCREATE;
                                return NULL;
                        }
                } else {
                        drerrno = DRE_COULDNOTCREATE;
                        return NULL;
                }
        }

        fchmod (fileno(f),0777);

        /* So now we have the file open and so we must write to it */
        fprintf(f,"#!/usr/bin/env tcsh\n\n");
        fprintf(f,"set DRQUEUE_OWNER_UID=%d\n",info->uid_owner);
        fprintf(f,"set DRQUEUE_OWNER_GID=%d\n",info->gid_owner);

				// For when I've got the script path stored correctly
        // fprintf(f,"set DRQUEUE_SCRIPT=%s/%s\n",info->scriptdir,info->script);
        fprintf(f,"set DRQUEUE_SCRIPT=%s\n",info->script);

        fflush (f);

        snprintf(fn_etc_general_sg,BUFFERLEN-1,"%s/general.sg",getenv("DRQUEUE_ETC"));
        if ((etc_general_sg = fopen (fn_etc_general_sg,"r")) == NULL) {
                fprintf(f,"\necho -------------------------------------------------\n");
                fprintf(f,"echo ATTENTION ! There was a problem opening: %s\n",fn_etc_general_sg);
                fprintf(f,"echo Please correct the problem\n");
                fprintf(f,"echo -------------------------------------------------\n");
                fprintf(f,"\n\n");
        } else {
                fd_etc_general_sg = fileno (etc_general_sg);
                fd_f = fileno (f);

            // fprintf(stderr,"DEBUG: Template Script: %s\n",fn_etc_general_sg);
                while ((size = read (fd_etc_general_sg,buf,BUFFERLEN)) != 0) {
                        write (fd_f,buf,size);
                }
                fclose(etc_general_sg);
        }

        fclose(f);

        return drqueue_filename;
}


char *generalsg_default_script_path (void)
{
        static char buf[BUFFERLEN];
        char *p;

        if (!(p = getenv("DRQUEUE_TMP"))) {
                return ("/drqueue_tmp/not/set/");
        }

        if (p[strlen(p)-1] == '/')
                snprintf (buf,BUFFERLEN-1,"%s",p);
        else
                snprintf (buf,BUFFERLEN-1,"%s/",p);

        return buf;
}
