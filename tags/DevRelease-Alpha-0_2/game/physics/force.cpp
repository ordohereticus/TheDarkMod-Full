/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 2 $
 * $Date: 2004-10-30 11:52:07 -0400 (Sat, 30 Oct 2004) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:33  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

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
