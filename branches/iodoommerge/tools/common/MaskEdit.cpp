/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5171 $ (Revision of last commit) 
 $Date: 2012-01-07 03:08:06 -0500 (Sat, 07 Jan 2012) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_engine.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: MaskEdit.cpp 5171 2012-01-07 08:08:06Z greebo $");

#define MASKEDIT_MAXINVALID	1024
typedef struct
{
	WNDPROC	mProc;
	char	mInvalid[MASKEDIT_MAXINVALID];
} rvGEMaskEdit;

/*
================
MaskEdit_WndProc

Prevents the invalid characters from being entered
================
*/
LRESULT CALLBACK MaskEdit_WndProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	rvGEMaskEdit* edit = (rvGEMaskEdit*)GetWindowLong ( hWnd, GWL_USERDATA );
	WNDPROC		  wndproc = edit->mProc;

	switch ( msg )
	{
		case WM_CHAR:
			if ( strchr ( edit->mInvalid, wParam ) )
			{
				return 0;
			}
			
			break;
			
		case WM_DESTROY:
			delete edit;
			SetWindowLong ( hWnd, GWL_WNDPROC, (LONG)wndproc );
			break;
	}

	return CallWindowProc ( wndproc, hWnd, msg, wParam, lParam );
}

/*
================
MaskEdit_Attach

Attaches the mask edit control to a normal edit control
================
*/
void MaskEdit_Attach ( HWND hWnd, const char* invalid )
{
	rvGEMaskEdit* edit = new rvGEMaskEdit;
	edit->mProc = (WNDPROC)GetWindowLong ( hWnd, GWL_WNDPROC );
	strcpy ( edit->mInvalid, invalid );
	SetWindowLong ( hWnd, GWL_USERDATA, (LONG)edit );
	SetWindowLong ( hWnd, GWL_WNDPROC, (LONG)MaskEdit_WndProc );
}

/*
================
NumberEdit_Attach

Allows editing of floating point numbers
================
*/
void NumberEdit_Attach ( HWND hWnd )
{
	static const char invalid[] = "`~!@#$%^&*()_+|=\\qwertyuiop[]asdfghjkl;'zxcvbnm,/QWERTYUIOP{}ASDFGHJKL:ZXCVBNM<>";
	MaskEdit_Attach ( hWnd, invalid );
}
