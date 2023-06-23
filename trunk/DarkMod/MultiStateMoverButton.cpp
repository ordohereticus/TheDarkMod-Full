/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2340 $
 * $Date: 2008-05-15 13:28:07 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MultiStateMoverButton.cpp 2340 2008-05-15 17:28:07Z greebo $", init_version);

#include "DarkModGlobals.h"
#include "MultiStateMoverButton.h"
#include "MultiStateMover.h"

//===============================================================================
// CMultiStateMoverButton
//===============================================================================

CLASS_DECLARATION( CFrobButton, CMultiStateMoverButton )
	EVENT( EV_PostSpawn,				CMultiStateMoverButton::Event_PostSpawn)
END_CLASS

void CMultiStateMoverButton::Spawn()
{
	if (!spawnArgs.GetBool("ride", "0") && !spawnArgs.GetBool("fetch", "0"))
	{
		gameLocal.Warning("Elevator button %s has neither 'fetch' nor 'ride' spawnargs set. AI will not be able to use this button!", name.c_str());
	}

	PostEventMS(&EV_PostSpawn, 10);
}

void CMultiStateMoverButton::Event_PostSpawn()
{
	for (int i = 0; i < targets.Num(); i++)
	{
		idEntity* ent = targets[i].GetEntity();

		if (ent == NULL || !ent->IsType(CMultiStateMover::Type))
		{
			continue;
		}

		CMultiStateMover* elevator = static_cast<CMultiStateMover*>(ent);

		// Send the information about us to the elevator
		if (spawnArgs.GetBool("ride", "0"))
		{
			elevator->RegisterButton(this, BUTTON_TYPE_RIDE);
		}

		if (spawnArgs.GetBool("fetch", "0"))
		{
			elevator->RegisterButton(this, BUTTON_TYPE_FETCH);
		}
	}
}
