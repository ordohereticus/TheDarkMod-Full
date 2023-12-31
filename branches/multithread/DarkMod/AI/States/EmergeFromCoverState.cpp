/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3431 $
 * $Date: 2009-05-07 22:54:31 -0400 (Thu, 07 May 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: EmergeFromCoverState.cpp 3431 2009-05-08 02:54:31Z greebo $", init_version);

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
