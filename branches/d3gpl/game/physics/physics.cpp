/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5083 $
 * $Date: 2011-12-04 23:50:26 -0500 (Sun, 04 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: Physics.cpp 5083 2011-12-05 04:50:26Z greebo $", init_version);

#include "../game_local.h"

ABSTRACT_DECLARATION( idClass, idPhysics )
END_CLASS


/*
================
idPhysics::~idPhysics
================
*/
idPhysics::~idPhysics( void ) {
}

/*
================
idPhysics::Save
================
*/
void idPhysics::Save( idSaveGame *savefile ) const {
}

/*
================
idPhysics::Restore
================
*/
void idPhysics::Restore( idRestoreGame *savefile ) {
}

/*
================
idPhysics::SetClipBox
================
*/
void idPhysics::SetClipBox( const idBounds &bounds, float density ) {
	SetClipModel( new idClipModel( idTraceModel( bounds ) ), density );
}

/*
================
idPhysics::SnapTimeToPhysicsFrame
================
*/
int idPhysics::SnapTimeToPhysicsFrame( int t ) {
	int s;
	s = t + USERCMD_MSEC - 1;
	return ( s - s % USERCMD_MSEC );
}
