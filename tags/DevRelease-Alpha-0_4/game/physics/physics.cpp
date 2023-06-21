/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 465 $
 * $Date: 2006-06-21 09:08:20 -0400 (Wed, 21 Jun 2006) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.2  2006/06/21 13:07:07  sparhawk
 * Added version tracking per cpp module
 *
 * Revision 1.1.1.1  2004/10/30 15:52:33  sparhawk
 * Initial release
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Source$  $Revision: 465 $   $Date: 2006-06-21 09:08:20 -0400 (Wed, 21 Jun 2006) $", init_version);

#include "../Game_local.h"

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
