/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 741 $
 * $Date: 2007-01-20 00:17:46 -0500 (Sat, 20 Jan 2007) $
 * $Author: sophisticatedzombie $
***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop
#include ".\idabsencemarkerentity.h"

//##################################################################
// Script events
//##################################################################

const idEventDef EV_GetNextReferencedKey( "getNextRefKey", "ss", 's' );
const idEventDef EV_GetReferencedKey( "getRefKey", "s", 's' );
const idEventDef EV_GetReferencedIntKey( "getRefIntKey", "s", 'f' );
const idEventDef EV_GetReferencedFloatKey( "getRefFloatKey", "s", 'f' );
const idEventDef EV_GetReferencedVectorKey( "getRefVectorKey", "s", 'v' );
const idEventDef EV_GetReferencedEntityKey( "getRefEntityKey", "s", 'e' );

CLASS_DECLARATION ( idEntity, idAbsenceMarkerEntity )
	// Methods to get spawn arg keys from the referenced ("missing") entities dictionary (using a copy thereof)
	EVENT( EV_GetNextReferencedKey,			idAbsenceMarkerEntity::Event_GetNextReferencedKey )
	EVENT( EV_GetReferencedKey,				idAbsenceMarkerEntity::Event_GetReferencedKey )
	EVENT( EV_GetReferencedIntKey,			idAbsenceMarkerEntity::Event_GetReferencedIntKey )
	EVENT( EV_GetReferencedFloatKey,			idAbsenceMarkerEntity::Event_GetReferencedFloatKey )
	EVENT( EV_GetReferencedVectorKey,			idAbsenceMarkerEntity::Event_GetReferencedVectorKey )
	EVENT( EV_GetReferencedEntityKey,			idAbsenceMarkerEntity::Event_GetReferencedEntityKey )
END_CLASS

//##################################################################
// Constructor, Destructor, and Init
//##################################################################

idAbsenceMarkerEntity::idAbsenceMarkerEntity(void)
{
	referenced_entityDefNumber = -1;
}

//-----------------------------------------------------------------------------------

idAbsenceMarkerEntity::~idAbsenceMarkerEntity(void)
{
	
}

//-----------------------------------------------------------------------------------

bool idAbsenceMarkerEntity::initAbsenceReference
(
	const idEntityPtr<idEntity>& absentEntity,
	idVec3 absentPosition,
	idMat3 absentOrientation
)
{
	idEntity* p_entity = absentEntity.GetEntity();
	if (p_entity == NULL)
	{
		return false;
	}

    
	referenced_entityDefNumber = p_entity->entityDefNumber;
	referenced_entityName = p_entity->name;

	// Fill with spawnargs of referenced entity
	referenced_spawnArgs = p_entity->spawnArgs;

	// We are on the same team as the referenced entity
	JoinTeam (p_entity);

	// Move to position where missing entity should have been
	// Importantly, we float up just a bit off of the surface that the item was sitting on, 
	// in case the items origin was embedded down in the surface a bit and therefore not visible.
	idVec3 markerOrg = absentPosition;
	markerOrg -= (GetPhysics()->GetGravityNormal() * 1.0);
	SetOrigin (markerOrg);

	if (p_entity->IsType(idItem::Type))
	{
		idItem* item;
		item = static_cast<idItem *>( p_entity );
		ownerTeam = item->ownerTeam;
	}

	// Done
	return true;
}

//##################################################################
// Script events to get referenced objects spawn args
//##################################################################

void idAbsenceMarkerEntity::Event_GetNextReferencedKey( const char *prefix, const char *lastMatch )
{
	const idKeyValue *kv;
	const idKeyValue *previous;

	if ( *lastMatch ) {
		previous = referenced_spawnArgs.FindKey( lastMatch );
	} else {
		previous = NULL;
	}

	kv = referenced_spawnArgs.MatchPrefix( prefix, previous );
	if ( !kv ) {
		idThread::ReturnString( "" );
	} else {
		idThread::ReturnString( kv->GetKey() );
	}

}

//--------------------------------------------------------------------------------------------

void idAbsenceMarkerEntity::Event_GetReferencedKey( const char *key )
{
	const char *value;

	referenced_spawnArgs.GetString( key, "", &value );
	idThread::ReturnString( value );

}

//--------------------------------------------------------------------------------------------

void idAbsenceMarkerEntity::Event_GetReferencedIntKey( const char *key )
{
	int value;

	referenced_spawnArgs.GetInt( key, "0", value );

	// scripts only support floats
	idThread::ReturnFloat( value );
}

//--------------------------------------------------------------------------------------------

void idAbsenceMarkerEntity::Event_GetReferencedFloatKey( const char *key )
{
	float value;

	referenced_spawnArgs.GetFloat( key, "0", value );
	idThread::ReturnFloat( value );
}

//--------------------------------------------------------------------------------------------

void idAbsenceMarkerEntity::Event_GetReferencedVectorKey( const char *key )
{
	idVec3 value;

	referenced_spawnArgs.GetVector( key, "0 0 0", value );
	idThread::ReturnVector( value );
}

//--------------------------------------------------------------------------------------------

void idAbsenceMarkerEntity::Event_GetReferencedEntityKey( const char *key )
{
	idEntity *ent;
	const char *entname;

	if ( !referenced_spawnArgs.GetString( key, NULL, &entname ) ) {
		idThread::ReturnEntity( NULL );
		return;
	}

	ent = gameLocal.FindEntity( entname );
	if ( !ent ) {
		gameLocal.Warning( "Couldn't find entity '%s' specified in '%s' key in entity '%s'", entname, key, name.c_str() );
	}

	idThread::ReturnEntity( ent );
}

//-----------------------------------------------------------------------------------