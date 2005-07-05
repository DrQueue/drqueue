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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
// USA
// 
//
// $Id$
//

#ifdef __CYGWIN

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <objidl.h>
#ifndef OPENFILENAME_SIZE_VERSION_400
#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
#endif

#include "drqm_common.h"
#include <sys/cygwin.h>

void cygwin_shell_execute(char *verb, char *path)
{
	SHELLEXECUTEINFO ShExecInfo;

				ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
				ShExecInfo.fMask = 0;
				ShExecInfo.hwnd = NULL;
				ShExecInfo.lpVerb = verb;
				ShExecInfo.lpFile = path;
				ShExecInfo.lpParameters = NULL;
				ShExecInfo.lpDirectory = NULL;
				ShExecInfo.nShow = SW_MAXIMIZE;
				ShExecInfo.hInstApp = NULL;

				ShellExecuteEx(&ShExecInfo);
} 

char *cygwin_dir_dialog(char *message)
{
	char *path; 
	char get_path[MAX_PATH];
	BROWSEINFO bi = { 0 };

	bi.lpszTitle = (message ? message : "Pick a Directory");
	bi.pszDisplayName = get_path;
	LPITEMIDLIST pidl = SHBrowseForFolder (&bi);
	if (!(path = malloc(MAX_PATH)))
		return (NULL);
	memset(path, 0, MAX_PATH);
	if (pidl != 0)
	{
		SHGetPathFromIDList(pidl, path); //can fail
	
		// free memory used
		LPMALLOC lpmalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&lpmalloc)))
		{
	lpmalloc->lpVtbl->Free(lpmalloc, pidl);
	lpmalloc->lpVtbl->Release(lpmalloc);
		}
	}
	//strncpy(path, get_path, MAX_PATH);
	return (path);
}

char *cygwin_file_dialog(char *fname, char *pat, char *message, char save)
{
	static char filenamebuffer[MAX_PATH];
	static OPENFILENAME wreq;

	memset(&wreq, 0, sizeof(wreq));
	wreq.lStructSize = OPENFILENAME_SIZE_VERSION_400; // needed for Win < Win2k
	wreq.lpstrFilter = pat;
	wreq.nFilterIndex = (pat) ? 1 : 0;
	wreq.lpstrFile = filenamebuffer;
	wreq.nMaxFile = MAX_PATH;
	wreq.lpstrTitle = message ? message : "Select the filename";
	if (fname)
		strncpy(filenamebuffer, fname, MAX_PATH);
	wreq.Flags = OFN_NOCHANGEDIR;
	if (save ? GetSaveFileName(&wreq) : GetOpenFileName(&wreq))
		return (wreq.lpstrFile);
	return (NULL);
}

char *conv_to_posix_path(char *win32_path)
{
	char *posix_path;
	
	if ((posix_path = malloc(MAXCMDLEN)) == NULL)
	return (NULL);
	cygwin_conv_to_posix_path(win32_path, posix_path);
	return (posix_path);
}

char *conv_to_win32_path(char *posix_path)
{
	char *win32_path;
	
	if ((win32_path = malloc(MAXCMDLEN)) == NULL)
	return(NULL);
	cygwin_conv_to_win32_path(posix_path, win32_path);
	return (win32_path);
}

#endif // __CYGWIN

