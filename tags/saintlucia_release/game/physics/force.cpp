/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 12:53:28 -0400 (Tue, 16 Oct 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: force.cpp 1435 2007-10-16 16:53:28Z greebo $", init_version);

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
