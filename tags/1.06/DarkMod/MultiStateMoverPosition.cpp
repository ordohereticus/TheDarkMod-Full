/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2342 $
 * $Date: 2008-05-15 14:43:06 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MultiStateMoverPosition.cpp 2342 2008-05-15 18:43:06Z greebo $", init_version);

#include "MultiStateMoverPosition.h"
#include "MultiStateMoverButton.h"
#include "MultiStateMover.h"

CLASS_DECLARATION( idEntity, CMultiStateMoverPosition )
	EVENT( EV_PostSpawn,	CMultiStateMoverPosition::Event_PostSpawn )
END_CLASS

void CMultiStateMoverPosition::Spawn() 
{
	PostEventMS(&EV_PostSpawn, 4);
}

void CMultiStateMoverPosition::Event_PostSpawn()
{
	// Find all AAS obstacle entities among the targets
	for (int i = 0; i < targets.Num(); i++)
	{
		idEntity* target = targets[i].GetEntity();

		if (target != NULL && target->IsType(idFuncAASObstacle::Type))
		{
			// Allocate a new list element and call the operator=
			aasObstacleEntities.Alloc() = static_cast<idFuncAASObstacle*>(target);
		}
	}

	// Remove all AAS obstacle entities from our targets, they should not be blindly triggered
	for (int i = 0; i < aasObstacleEntities.Num(); i++)
	{
		RemoveTarget(aasObstacleEntities[i].GetEntity());
	}
}

void CMultiStateMoverPosition::SetMover(CMultiStateMover* newMover)
{
	mover = newMover;
}

CMultiStateMoverButton*	CMultiStateMoverPosition::GetFetchButton()
{
	CMultiStateMover* m = mover.GetEntity();
	if (m == NULL) return NULL;

	return m->GetButton(this, NULL, BUTTON_TYPE_FETCH);
}

CMultiStateMoverButton*	CMultiStateMoverPosition::GetRideButton(CMultiStateMoverPosition* toPosition)
{
	CMultiStateMover* m = mover.GetEntity();
	if (m == NULL) return NULL;

	return m->GetButton(toPosition, this, BUTTON_TYPE_RIDE);
}

void CMultiStateMoverPosition::Save(idSaveGame *savefile) const
{
	savefile->WriteInt(aasObstacleEntities.Num());
	for (int i = 0; i < aasObstacleEntities.Num(); i++)
	{
		aasObstacleEntities[i].Save(savefile);
	}

	mover.Save(savefile);
}

void CMultiStateMoverPosition::Restore(idRestoreGame *savefile)
{
	int num;
	savefile->ReadInt(num);
	aasObstacleEntities.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		aasObstacleEntities[i].Restore(savefile);
	}

	mover.Restore(savefile);
}

void CMultiStateMoverPosition::OnMultistateMoverArrive(CMultiStateMover* mover)
{
	if (mover == NULL) return;

	// First, check if we should trigger our targets
	if (spawnArgs.GetBool("always_trigger_targets", "1"))
	{
		ActivateTargets(mover);

		// Tell the idFuncAASObstacles to (re-)activate the AAS areas
		for (int i = 0; i < aasObstacleEntities.Num(); i++)
		{
			aasObstacleEntities[i].GetEntity()->SetAASState(false);
		}
	}

	// Run the mover event script
	RunMoverEventScript("call_on_arrive", mover);
}

void CMultiStateMoverPosition::OnMultistateMoverLeave(CMultiStateMover* mover)
{
	if (mover == NULL) return;

	// First, check if we should trigger our targets
	if (spawnArgs.GetBool("always_trigger_targets", "1"))
	{
		ActivateTargets(mover);

		// Handle the idFuncAASObstacles separately, tell them to deactivate the AAS areas
		for (int i = 0; i < aasObstacleEntities.Num(); i++)
		{
			aasObstacleEntities[i].GetEntity()->SetAASState(true);
		}
	}

	// Run the mover event script
	RunMoverEventScript("call_on_leave", mover);
}

void CMultiStateMoverPosition::RunMoverEventScript(const idStr& spawnArg, CMultiStateMover* mover)
{
	idStr scriptFuncName;
	if (!spawnArgs.GetString(spawnArg, "", scriptFuncName))
	{
		return; // no scriptfunction
	}

	// Script function signature is like this: void scriptobj::onMultiStateMover(entity mover)
	idThread* thread = CallScriptFunctionArgs(scriptFuncName, true, 0, "ee", this, mover);
	if (thread != NULL)
	{
		// greebo: Run the thread at once, the script result might be needed below.
		thread->Execute();
	}
}
