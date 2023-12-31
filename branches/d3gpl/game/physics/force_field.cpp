/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5121 $ (Revision of last commit) 
 $Date: 2011-12-11 14:12:26 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: Force_Field.cpp 5121 2011-12-11 19:12:26Z greebo $", init_version);

#include "../Game_local.h"

CLASS_DECLARATION( idForce, idForce_Field )
END_CLASS

/*
================
idForce_Field::idForce_Field
================
*/
idForce_Field::idForce_Field( void ) {
	type			= FORCEFIELD_UNIFORM;
	applyType		= FORCEFIELD_APPLY_FORCE;
	magnitude		= 0.0f;
	dir.Set( 0, 0, 1 );
	randomTorque	= 0.0f;
	playerOnly		= false;
	monsterOnly		= false;
	clipModel		= NULL;
}

/*
================
idForce_Field::~idForce_Field
================
*/
idForce_Field::~idForce_Field( void ) {
	if ( this->clipModel ) {
		delete this->clipModel;
	}
}

/*
================
idForce_Field::Save
================
*/
void idForce_Field::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( type );
	savefile->WriteInt( applyType);
	savefile->WriteFloat( magnitude );
	savefile->WriteVec3( dir );
	savefile->WriteFloat( randomTorque );
	savefile->WriteBool( playerOnly );
	savefile->WriteBool( monsterOnly );
	savefile->WriteClipModel( clipModel );
}

/*
================
idForce_Field::Restore
================
*/
void idForce_Field::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( (int &)type );
	savefile->ReadInt( (int &)applyType);
	savefile->ReadFloat( magnitude );
	savefile->ReadVec3( dir );
	savefile->ReadFloat( randomTorque );
	savefile->ReadBool( playerOnly );
	savefile->ReadBool( monsterOnly );
	savefile->ReadClipModel( clipModel );
}

/*
================
idForce_Field::SetClipModel
================
*/
void idForce_Field::SetClipModel( idClipModel *clipModel ) {
	if ( this->clipModel && clipModel != this->clipModel ) {
		delete this->clipModel;
	}
	this->clipModel = clipModel;
}

/*
================
idForce_Field::Uniform
================
*/
void idForce_Field::Uniform( const idVec3 &force ) {
	dir = force;
	magnitude = dir.Normalize();
	type = FORCEFIELD_UNIFORM;
}

/*
================
idForce_Field::Explosion
================
*/
void idForce_Field::Explosion( float force ) {
	magnitude = force;
	type = FORCEFIELD_EXPLOSION;
}

/*
================
idForce_Field::Implosion
================
*/
void idForce_Field::Implosion( float force ) {
	magnitude = force;
	type = FORCEFIELD_IMPLOSION;
}

/*
================
idForce_Field::RandomTorque
================
*/
void idForce_Field::RandomTorque( float force ) {
	randomTorque = force;
}

/*
================
idForce_Field::Evaluate
================
*/
void idForce_Field::Evaluate( int time ) {
	int numClipModels, i;
	idBounds bounds;
	idVec3 force, torque, angularVelocity;
	idClipModel *cm, *clipModelList[ MAX_GENTITIES ];

	assert( clipModel );

	bounds.FromTransformedBounds( clipModel->GetBounds(), clipModel->GetOrigin(), clipModel->GetAxis() );
	numClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );

	for ( i = 0; i < numClipModels; i++ ) {
		cm = clipModelList[ i ];

		if ( !cm->IsTraceModel() ) {
			continue;
		}

		idEntity *entity = cm->GetEntity();

		if ( !entity ) {
			continue;
		}
		
		idPhysics *physics = entity->GetPhysics();

		if ( playerOnly ) {
			if ( !physics->IsType( idPhysics_Player::Type ) ) {
				continue;
			}
		} else if ( monsterOnly ) {
			if ( !physics->IsType( idPhysics_Monster::Type ) ) {
				continue;
			}
		}

		if ( !gameLocal.clip.ContentsModel( cm->GetOrigin(), cm, cm->GetAxis(), -1,
									clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis() ) ) {
			continue;
		}

		switch( type ) {
			case FORCEFIELD_UNIFORM: {
				force = dir;
				break;
			}
			case FORCEFIELD_EXPLOSION: {
				force = cm->GetOrigin() - clipModel->GetOrigin();
				force.Normalize();
				break;
			}
			case FORCEFIELD_IMPLOSION: {
				force = clipModel->GetOrigin() - cm->GetOrigin();
				force.Normalize();
				break;
			}
			default: {
				gameLocal.Error( "idForce_Field: invalid type" );
				break;
			}
		}

		if ( randomTorque != 0.0f ) {
			torque[0] = gameLocal.random.CRandomFloat();
			torque[1] = gameLocal.random.CRandomFloat();
			torque[2] = gameLocal.random.CRandomFloat();
			if ( torque.Normalize() == 0.0f ) {
				torque[2] = 1.0f;
			}
		}

		switch( applyType ) {
			case FORCEFIELD_APPLY_FORCE: {
				if ( randomTorque != 0.0f ) {
					entity->AddForce( gameLocal.world, cm->GetId(), cm->GetOrigin() + torque.Cross( dir ) * randomTorque, dir * magnitude );
				}
				else {
					entity->AddForce( gameLocal.world, cm->GetId(), cm->GetOrigin(), force * magnitude );
				}
				break;
			}
			case FORCEFIELD_APPLY_VELOCITY: {
				physics->SetLinearVelocity( force * magnitude, cm->GetId() );
				if ( randomTorque != 0.0f ) {
					angularVelocity = physics->GetAngularVelocity( cm->GetId() );
					physics->SetAngularVelocity( 0.5f * (angularVelocity + torque * randomTorque), cm->GetId() );
				}
				break;
			}
			case FORCEFIELD_APPLY_IMPULSE: {
				if ( randomTorque != 0.0f ) {
					entity->ApplyImpulse( gameLocal.world, cm->GetId(), cm->GetOrigin() + torque.Cross( dir ) * randomTorque, dir * magnitude );
				}
				else {
					entity->ApplyImpulse( gameLocal.world, cm->GetId(), cm->GetOrigin(), force * magnitude );
				}
				break;
			}
			default: {
				gameLocal.Error( "idForce_Field: invalid apply type" );
				break;
			}
		}
	}
}
