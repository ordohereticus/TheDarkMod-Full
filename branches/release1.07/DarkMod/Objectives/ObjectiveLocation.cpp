/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4800 $
 * $Date: 2011-04-17 04:32:34 -0400 (Sun, 17 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ObjectiveLocation.cpp 4800 2011-04-17 08:32:34Z greebo $", init_version);

#include "ObjectiveLocation.h"
#include "ObjectiveComponent.h"
#include "MissionData.h"
#include "../StimResponse/StimResponseCollection.h"

/*===========================================================================
*
*CObjectiveLocation
*
*============================================================================*/
CLASS_DECLARATION( idEntity, CObjectiveLocation )
END_CLASS

CObjectiveLocation::CObjectiveLocation( void )
{
	m_Interval = 1000;
	m_TimeStamp = 0;

	m_EntsInBounds.Clear();
}

CObjectiveLocation::~CObjectiveLocation()
{
	// On destruction, deregister ourselves from each entity 
	for (int i = 0; i < m_EntsInBounds.Num(); ++i)
	{
		idEntity* entity = m_EntsInBounds[i].GetEntity();

		if (entity == NULL) continue; // probably already deleted

		entity->OnRemoveFromLocationEntity(this);
	}
}

void CObjectiveLocation::Spawn()
{
	m_Interval = static_cast<int>(1000.0f * spawnArgs.GetFloat( "interval", "1.0" ));
	m_TimeStamp = gameLocal.time;
	m_ObjectiveGroup = spawnArgs.GetString( "objective_group", "" );

	// Set the contents to a useless trigger so that the collision model will be loaded
	// FLASHLIGHT_TRIGGER seems to be the only one that doesn't do anything else we don't want
	GetPhysics()->SetContents( CONTENTS_FLASHLIGHT_TRIGGER );

	// SR CONTENTS_RESONSE FIX
	if( m_StimResponseColl->HasResponse() )
		GetPhysics()->SetContents( GetPhysics()->GetContents() | CONTENTS_RESPONSE );

	GetPhysics()->EnableClip();

	// get the clip model
	clipModel = new idClipModel( GetPhysics()->GetClipModel() );

	// remove the collision model from the physics object
	GetPhysics()->SetClipModel( NULL, 1.0f );

	BecomeActive( TH_THINK );
}

void CObjectiveLocation::Save( idSaveGame *savefile ) const
{
	savefile->WriteInt( m_Interval );
	savefile->WriteInt( m_TimeStamp );
	savefile->WriteString( m_ObjectiveGroup );

	savefile->WriteInt( m_EntsInBounds.Num() );
	for( int i=0;i < m_EntsInBounds.Num(); i++ )
	{
		m_EntsInBounds[i].Save(savefile);
	}

	savefile->WriteClipModel( clipModel );
}

void CObjectiveLocation::Restore( idRestoreGame *savefile )
{
	int num;

	savefile->ReadInt( m_Interval );
	savefile->ReadInt( m_TimeStamp );
	savefile->ReadString( m_ObjectiveGroup );

	m_EntsInBounds.Clear();
	savefile->ReadInt( num );
	m_EntsInBounds.SetNum( num );
	for (int i = 0; i < num; i++)
	{
		m_EntsInBounds[i].Restore(savefile);
	}

	savefile->ReadClipModel( clipModel );
}

void CObjectiveLocation::Think()
{
	idList< idEntityPtr<idEntity> > current;

	// only check on clock ticks
	if( (gameLocal.time - m_TimeStamp) < m_Interval )
		goto Quit;

	m_TimeStamp = gameLocal.time;

	if ( clipModel != NULL)
	{
		// angua: This is adapted from trigger_touch to allow more precise detection
		idBounds bounds;
		idClipModel *cm, *clipModelList[ MAX_GENTITIES ];

		bounds.FromTransformedBounds( clipModel->GetBounds(), clipModel->GetOrigin(), clipModel->GetAxis() );

		int numClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );
		for (int k = 0; k < numClipModels; k++ ) 
		{
			cm = clipModelList[ k ];

			if (!cm->IsTraceModel()) continue;

			idEntity *entity = cm->GetEntity();

			if (entity == NULL) continue;
			
			if ( !gameLocal.clip.ContentsModel( cm->GetOrigin(), cm, cm->GetAxis(), -1,
										clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis() ) ) 
			{
				continue;
			}

			if (entity->m_bIsObjective)
			{
				DM_LOG(LC_OBJECTIVES,LT_DEBUG)LOGSTRING("Objective location %s found entity %s during clock tick. \r", name.c_str(), entity->name.c_str() );
				current.Alloc() = entity;
			}
		}
	}
	else
	{
		// bounding box test
		int NumEnts(0);
		idEntity *Ents[MAX_GENTITIES];

		NumEnts = gameLocal.clip.EntitiesTouchingBounds(GetPhysics()->GetAbsBounds(), -1, Ents, MAX_GENTITIES);
		for( int i = 0; i < NumEnts; i++ )
		{
			if( Ents[i] && Ents[i]->m_bIsObjective )
			{
				DM_LOG(LC_OBJECTIVES,LT_DEBUG)LOGSTRING("Objective location %s found entity %s during clock tick. \r", name.c_str(), Ents[i]->name.c_str() );
				current.Alloc() = Ents[i];
			}
		}
	}
	
	// compare current list to previous clock tick list to generate added list
	for( int i = 0; i < current.Num(); i++ )
	{
		// Try to look up this entity in the existing list
		if( m_EntsInBounds.FindIndex(current[i]) == -1  && current[i].GetEntity() != NULL)
		{
			// Not found, call objectives system for all missing or added ents
			DM_LOG(LC_OBJECTIVES,LT_DEBUG)LOGSTRING("Objective entity %s entered objective location %s \r", current[i].GetEntity()->name.c_str(), name.c_str() );
			gameLocal.m_MissionData->MissionEvent( COMP_LOCATION, current[i].GetEntity(), this, true );

			// Mark this entity as "currently within location entity bounds", by adding ourselves to its list
			current[i].GetEntity()->OnAddToLocationEntity(this);	
		}
	}

	// compare again the other way to generate missing list
	for( int i = 0; i < m_EntsInBounds.Num(); i++ )
	{
		if (current.FindIndex(m_EntsInBounds[i]) == -1 && m_EntsInBounds[i].GetEntity() != NULL)
		{
			// not found in current, must be missing
			DM_LOG(LC_OBJECTIVES,LT_DEBUG)LOGSTRING("Objective entity %s left objective location %s \r", m_EntsInBounds[i].GetEntity()->name.c_str(), name.c_str() );
			gameLocal.m_MissionData->MissionEvent( COMP_LOCATION, m_EntsInBounds[i].GetEntity(), this, false );

			// Remove ourselves from this entity
			m_EntsInBounds[i].GetEntity()->OnRemoveFromLocationEntity(this);
		}
	}

	// Swap the "old" list with the updated one
	m_EntsInBounds.Swap(current);

Quit:
	idEntity::Think();
	return;
}

void CObjectiveLocation::OnEntityDestroyed(idEntity* ent)
{
	for (int i = 0; i < m_EntsInBounds.Num(); i++)
	{
		if (m_EntsInBounds[i].GetEntity() == ent)
		{
			// Handle this entity as if it were missing
			DM_LOG(LC_OBJECTIVES,LT_DEBUG)LOGSTRING("Objective entity %s in objective location %s was destroyed\r", ent->name.c_str(), name.c_str());
			gameLocal.m_MissionData->MissionEvent(COMP_LOCATION, ent, this, false);

			// greebo: No need to call OnRemoveFromLocationEntity(), as the entity is about to be destroyed anyway
			// calling it would only complicate the routine in the destructor
		}
	}
}
