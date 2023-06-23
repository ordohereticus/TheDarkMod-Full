/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3223 $
 * $Date: 2009-03-05 07:47:52 -0500 (Thu, 05 Mar 2009) $
 * $Author: angua $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: AlertIdleState.cpp 3223 2009-03-05 12:47:52Z angua $", init_version);

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

	InitialiseMovement(owner);
	InitialiseCommunication(owner);

	// Initialise the animation state
	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle", 0);
	owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle", 0);

	// The sensory system does its Idle tasks
	owner->GetSubsystem(SubsysSenses)->ClearTasks();
	owner->GetSubsystem(SubsysSenses)->PushTask(RandomHeadturnTask::CreateInstance());

	if (!owner->AI_bMeleeWeapDrawn)
	{
		owner->DrawWeapon();
	}

	// Let the AI update their weapons (make them nonsolid)
	owner->UpdateAttachmentContents(false);
}

idStr AlertIdleState::GetInitialIdleBark(idAI* owner)
{
	Memory& memory = owner->GetMemory();

	// Decide what sound it is appropriate to play
	idStr soundName("");
	if (owner->m_lastAlertLevel >= owner->thresh_1 && owner->m_lastAlertLevel < owner->thresh_3)
	{
		if (memory.alertClass == EAlertVisual && memory.alertType != EAlertTypeMissingItem)
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
