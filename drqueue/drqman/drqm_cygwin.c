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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
// 
//
// $Id: drqm_cygwin.c 1141 2005-01-31 06:14:26Z kraken $
//

#ifdef __CYGWIN

#include <windows.h>
#include <commdlg.h>
//include <shell32.h>
#include <shlobj.h>
#include <objidl.h>
#ifndef OPENFILENAME_SIZE_VERSION_400
#define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
#endif


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

#endif // __CYGWIN
