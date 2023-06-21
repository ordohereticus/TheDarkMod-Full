/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 915 $
 * $Date: 2007-04-19 16:10:27 -0400 (Thu, 19 Apr 2007) $
 * $Author: orbweaver $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: force.cpp 915 2007-04-19 20:10:27Z orbweaver $", init_version);

#include "../game_local.h"

CLASS_DECLARATION( idClass, idForce )
END_CLASS

idList<idForce*> idForce::forceList;

/*
================
idForce::idForce
================
*/
idForce::idForce( void ) {
	forceList.Append( this );
}

/*
================
idForce::~idForce
================
*/
idForce::~idForce( void ) {
	forceList.Remove( this );
}

/*
================
idForce::DeletePhysics
================
*/
void idForce::DeletePhysics( const idPhysics *phys ) {
	int i;

	for ( i = 0; i < forceList.Num(); i++ ) {
		forceList[i]->RemovePhysics( phys );
	}
}

/*
================
idForce::ClearForceList
================
*/
void idForce::ClearForceList( void ) {
	forceList.Clear();
}

/*
================
idForce::Evaluate
================
*/
void idForce::Evaluate( int time ) {
}

/*
================
idForce::RemovePhysics
================
*/
void idForce::RemovePhysics( const idPhysics *phys ) {
}
