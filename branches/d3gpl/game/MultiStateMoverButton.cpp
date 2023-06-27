/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5066 $
 * $Date: 2011-12-03 14:49:11 -0500 (Sat, 03 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MultiStateMoverButton.cpp 5066 2011-12-03 19:49:11Z greebo $", init_version);

#include "DarkModGlobals.h"
#include "MultiStateMoverButton.h"
#include "MultiStateMover.h"

//===============================================================================
// CMultiStateMoverButton
//===============================================================================

const idEventDef EV_RegisterSelfWithElevator("MSMBRegisterSelfWithElevator", NULL);

CLASS_DECLARATION( CFrobButton, CMultiStateMoverButton )
	EVENT( EV_RegisterSelfWithElevator,		CMultiStateMoverButton::Event_RegisterSelfWithElevator)
END_CLASS

void CMultiStateMoverButton::Spawn()
{
	if (!spawnArgs.GetBool("ride", "0") && !spawnArgs.GetBool("fetch", "0"))
	{
		gameLocal.Warning("Elevator button %s has neither 'fetch' nor 'ride' spawnargs set. AI will not be able to use this button!", name.c_str());
	}

	PostEventMS(&EV_RegisterSelfWithElevator, 10);
}

void CMultiStateMoverButton::Event_RegisterSelfWithElevator()
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
