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

static bool versioned = RegisterVersionedFile("$Id: DebuggerBreakpoint.cpp 5171 2012-01-07 08:08:06Z greebo $");

#include "DebuggerApp.h"
#include "DebuggerBreakpoint.h"

int rvDebuggerBreakpoint::mNextID = 1;

rvDebuggerBreakpoint::rvDebuggerBreakpoint ( const char* filename, int linenumber, int id )
{
	mFilename = filename;
	mLineNumber = linenumber;
	mEnabled = true;
	
	if ( id == -1 )
	{	
		mID = mNextID++;
	}	
	else 
	{
		mID = id;
	}
}

rvDebuggerBreakpoint::rvDebuggerBreakpoint ( rvDebuggerBreakpoint& bp )
{
	mFilename = bp.mFilename;
	mEnabled = bp.mEnabled;
	mLineNumber = bp.mLineNumber;
}

rvDebuggerBreakpoint::~rvDebuggerBreakpoint ( void )
{
}
