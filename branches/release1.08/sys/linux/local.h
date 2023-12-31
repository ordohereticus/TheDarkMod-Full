/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5122 $ (Revision of last commit) 
 $Date: 2011-12-11 14:47:31 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/
#ifndef __LINUX_LOCAL_H__
#define __LINUX_LOCAL_H__

extern glconfig_t glConfig;

// glimp.cpp

//#define ID_ENABLE_DGA

#if defined( ID_ENABLE_DGA )
#include <X11/extensions/xf86dga.h>
#endif
#include <X11/extensions/xf86vmode.h>
#include <X11/XKBlib.h>

extern Display *dpy;
extern Window win;

// input.cpp
extern bool dga_found;
void Sys_XEvents();
void Sys_XUninstallGrabs();

#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask )

#ifndef ID_GL_HARDLINK
bool GLimp_dlopen();
void GLimp_dlclose();

void GLimp_BindLogging();
void GLimp_BindNative();
#endif

#endif
