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

#include "../../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: EmergeFromCoverState.cpp 5121 2011-12-11 19:12:26Z greebo $", init_version);

#include "EmergeFromCoverState.h"
#include "../Memory.h"
#include "../Tasks/MoveToCoverTask.h"
#include "../Tasks/WaitTask.h"
#include "../Tasks/MoveToPositionTask.h"
#include "../Tasks/RandomHeadturnTask.h"
#include "LostTrackOfEnemyState.h"
#include "../Library.h"

namespace ai
{

// Get the name of this state
const idStr& EmergeFromCoverState::GetName() const
{
	static idStr _name(STATE_EMERGE_FROM_COVER);
	return _name;
}

void EmergeFromCoverState::Init(idAI* owner)
{
	// Init base class first
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("EmergeFromCoverState initialised.\r");
	assert(owner);

	// Shortcut reference
	Memory& memory = owner->GetMemory();
	
	// Fill the subsystems with their tasks
	owner->GetSubsystem(SubsysCommunication)->ClearTasks();
	owner->actionSubsystem->ClearTasks();

	owner->movementSubsystem->ClearTasks();
	owner->movementSubsystem->QueueTask(
		TaskPtr(new MoveToPositionTask(memory.positionBeforeTakingCover))
	);
}

// Gets called each time the mind is thinking
void EmergeFromCoverState::Think(idAI* owner)
{
	Memory& memory = owner->GetMemory();

	// Let the AI check its senses
	owner->PerformVisualScan();

	if (owner->AI_MOVE_DONE 
		&& !owner->AI_DEST_UNREACHABLE 
		&& (owner->GetPhysics()->GetOrigin() - memory.positionBeforeTakingCover).LengthFast() < 50)
	{
		// Reached position before taking cover, look for enemy
		// Turn to last visible enemy position
		owner->TurnToward(owner->lastVisibleEnemyPos);
		owner->Event_LookAtPosition(owner->lastVisibleEnemyPos,1);

		owner->GetMind()->SwitchState(STATE_LOST_TRACK_OF_ENEMY);
	}
	else if (owner->AI_DEST_UNREACHABLE)
	{
		// Can't move back to position before taking cover
		owner->GetMind()->SwitchState(STATE_LOST_TRACK_OF_ENEMY);
	}
}

StatePtr EmergeFromCoverState::CreateInstance()
{
	return StatePtr(new EmergeFromCoverState);
}

// Register this state with the StateLibrary
StateLibrary::Registrar emergeFromCoverStateRegistrar(
	STATE_EMERGE_FROM_COVER, // Task Name
	StateLibrary::CreateInstanceFunc(&EmergeFromCoverState::CreateInstance) // Instance creation callback
);

} // namespace ai
