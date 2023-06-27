/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5092 $
 * $Date: 2011-12-06 12:30:27 -0500 (Tue, 06 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef TDM_OBJECTIVE_LOCATION_H
#define TDM_OBJECTIVE_LOCATION_H

#include "../../idlib/precompiled.h"

// Helper entity for objective locations
class CObjectiveLocation : public idEntity
{
public:
	CLASS_PROTOTYPE( CObjectiveLocation );
	
	CObjectiveLocation();

	~CObjectiveLocation();

	void Think( void );
	void Spawn( void );

	// Called by ~idEntity to catch entity destructions
	void OnEntityDestroyed(idEntity* ent);

	void Save( idSaveGame *savefile ) const;
	void Restore( idRestoreGame *savefile );

protected:
	/**
	* Clock interval [seconds]
	**/
	int m_Interval;

	int m_TimeStamp;

	/**
	* List of entity names that intersected bounds in previous clock tick
	**/
	idList< idEntityPtr<idEntity> >	m_EntsInBounds;
	/**
	* Objective system: Location's objective group name for objective checks
	**/
	idStr		m_ObjectiveGroup;

private:
	idClipModel *		clipModel;

}; // CObjectiveLocation

#endif
