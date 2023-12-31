/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 17:25:02 -0400 (Fri, 23 Mar 2007) $
 * $Author: sparhawk $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: force_constant.cpp 866 2007-03-23 21:25:02Z sparhawk $", init_version);

#include "../Game_local.h"

CLASS_DECLARATION( idForce, idForce_Constant )
END_CLASS

/*
================
idForce_Constant::idForce_Constant
================
*/
idForce_Constant::idForce_Constant( void ) {
	force		= vec3_zero;
	physics		= NULL;
	id			= 0;
	point		= vec3_zero;
}

/*
================
idForce_Constant::~idForce_Constant
================
*/
idForce_Constant::~idForce_Constant( void ) {
}

/*
================
idForce_Constant::Save
================
*/
void idForce_Constant::Save( idSaveGame *savefile ) const {
	savefile->WriteVec3( force );
	savefile->WriteInt( id );
	savefile->WriteVec3( point );
}

/*
================
idForce_Constant::Restore
================
*/
void idForce_Constant::Restore( idRestoreGame *savefile ) {
	// Owner needs to call SetPhysics!!
	savefile->ReadVec3( force );
	savefile->ReadInt( id );
	savefile->ReadVec3( point );
}

/*
================
idForce_Constant::SetPosition
================
*/
void idForce_Constant::SetPosition( idPhysics *physics, int id, const idVec3 &point ) {
	this->physics = physics;
	this->id = id;
	this->point = point;
}

/*
================
idForce_Constant::SetForce
================
*/
void idForce_Constant::SetForce( const idVec3 &force ) {
	this->force = force;
}

/*
================
idForce_Constant::SetPhysics
================
*/
void idForce_Constant::SetPhysics( idPhysics *physics ) {
	this->physics = physics;
}

/*
================
idForce_Constant::Evaluate
================
*/
void idForce_Constant::Evaluate( int time ) {
	idVec3 p;

	if ( !physics ) {
		return;
	}

	p = physics->GetOrigin( id ) + point * physics->GetAxis( id );

	physics->AddForce( id, p, force );
}

/*
================
idForce_Constant::RemovePhysics
================
*/
void idForce_Constant::RemovePhysics( const idPhysics *phys ) {
	if ( physics == phys ) {
		physics = NULL;
	}
}
