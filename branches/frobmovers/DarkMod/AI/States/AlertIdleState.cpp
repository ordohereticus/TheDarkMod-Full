/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2475 $
 * $Date: 2008-06-14 02:37:18 -0400 (Sat, 14 Jun 2008) $
 * $Author: angua $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: AlertIdleState.cpp 2475 2008-06-14 06:37:18Z angua $", init_version);

#include "IdleState.h"
#include "AlertIdleState.h"
#include "../Memory.h"
#include "../Tasks/RandomHeadturnTask.h"
#include "../Tasks/PatrolTask.h"
#include "../Tasks/AnimalPatrolTask.h"
#include "../Tasks/SingleBarkTask.h"
#include "../Tasks/MoveToPositionTask.h"
#include "../Tasks/IdleAnimationTask.h"
#include "ObservantState.h"
#include "../Library.h"

namespace ai
{

// Get the name of this state
const idStr& AlertIdleState::GetName() const
{
	static idStr _name(STATE_ALERT_IDLE);
	return _name;
}

void AlertIdleState::Init(idAI* owner)
{
	// Init state class first
	// Note: we do not call IdleState::Init
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("AlertIdleState initialised.\r");
	assert(owner);

	_alertLevelDecreaseRate = 0.005f;

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;

	_startSitting = owner->spawnArgs.GetBool("sitting", "0");

	InitialiseMovement(owner);

	InitialiseCommunication(owner);

	// Initialise the animation state
	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle", 0);
	owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle", 0);

	// The sensory system does its Idle tasks
	owner->GetSubsystem(SubsysSenses)->ClearTasks();
	owner->GetSubsystem(SubsysSenses)->PushTask(RandomHeadturnTask::CreateInstance());
}

idStr AlertIdleState::GetInitialIdleBark(idAI* owner)
{
	Memory& memory = owner->GetMemory();

	// Decide what sound it is appropriate to play
	idStr soundName("");
	if (owner->m_lastAlertLevel < owner->thresh_3)
	{
		if (memory.alertClass == EAlertVisual)
		{
			soundName = "snd_alertdown0s";
		}
		else if (memory.alertClass == EAlertAudio)
		{
			soundName = "snd_alertdown0h";
		}
		else
		{
			soundName = "snd_alertdown0";
		}
	}

	return soundName;
}

StatePtr AlertIdleState::CreateInstance()
{
	return StatePtr(new AlertIdleState);
}

// Register this state with the StateLibrary
StateLibrary::Registrar alertIdleStateRegistrar(
	STATE_ALERT_IDLE, // Task Name
	StateLibrary::CreateInstanceFunc(&AlertIdleState::CreateInstance) // Instance creation callback
);

} // namespace ai
