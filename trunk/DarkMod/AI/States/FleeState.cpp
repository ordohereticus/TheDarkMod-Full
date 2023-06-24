/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3440 $
 * $Date: 2009-05-12 12:48:35 -0400 (Tue, 12 May 2009) $
 * $Author: angua $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: FleeState.cpp 3440 2009-05-12 16:48:35Z angua $", init_version);

#include "FleeState.h"
#include "../Memory.h"
#include "../../AIComm_Message.h"
#include "../Library.h"
#include "../Tasks/WaitTask.h"
#include "../Tasks/FleeTask.h"
#include "../Tasks/RepeatedBarkTask.h"
#include "FleeDoneState.h"

namespace ai
{

// Get the name of this state
const idStr& FleeState::GetName() const
{
	static idStr _name(STATE_FLEE);
	return _name;
}

void FleeState::Init(idAI* owner)
{
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("FleeState initialised.\r");
	assert(owner);

	// Shortcut reference
	Memory& memory = owner->GetMemory();
	memory.fleeingDone = false;

	// Fill the subsystems with their tasks

	// The movement subsystem should wait half a second before starting to run
	owner->StopMove(MOVE_STATUS_DONE);
	owner->FaceEnemy();
	owner->movementSubsystem->ClearTasks();
	owner->movementSubsystem->PushTask(TaskPtr(new WaitTask(1000)));
	owner->movementSubsystem->QueueTask(FleeTask::CreateInstance());

	// The communication system cries for help
	owner->StopSound(SND_CHANNEL_VOICE, false);
/*	owner->GetSubsystem(SubsysCommunication)->ClearTasks();
	owner->GetSubsystem(SubsysCommunication)->PushTask(TaskPtr(new WaitTask(200)));*/// TODO_AI
	
	// Setup the message to be delivered each time
	CommMessagePtr message(new CommMessage(
		CommMessage::DetectedEnemy_CommType, 
		owner, NULL, // from this AI to anyone 
		owner->GetEnemy(),
		memory.alertPos
	));

/*	owner->GetSubsystem(SubsysCommunication)->PushTask(
		TaskPtr(new RepeatedBarkTask("snd_flee", 4000,8000, message))
	);*/

	// The sensory system 
	owner->senseSubsystem->ClearTasks();

	// No action
	owner->actionSubsystem->ClearTasks();

	// Play the surprised animation
	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Surprise", 5);
	owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Surprise", 5);
	owner->SetWaitState("surprise");
}

// Gets called each time the mind is thinking
void FleeState::Think(idAI* owner)
{
	// Shortcut reference
	Memory& memory = owner->GetMemory();

	if (memory.fleeingDone)
	{
		owner->GetMind()->SwitchState(STATE_FLEE_DONE);
	}
}


StatePtr FleeState::CreateInstance()
{
	return StatePtr(new FleeState);
}

// Register this state with the StateLibrary
StateLibrary::Registrar fleeStateRegistrar(
	STATE_FLEE, // Task Name
	StateLibrary::CreateInstanceFunc(&FleeState::CreateInstance) // Instance creation callback
);

} // namespace ai
