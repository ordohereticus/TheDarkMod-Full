/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4991 $
 * $Date: 2011-10-11 13:49:38 -0400 (Tue, 11 Oct 2011) $
 * $Author: grayman $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: IdleState.cpp 4991 2011-10-11 17:49:38Z grayman $", init_version);

#include "IdleState.h"
#include "AlertIdleState.h"
#include "IdleSleepState.h"
#include "../Memory.h"
#include "../Tasks/RandomHeadturnTask.h"
#include "../Tasks/AnimalPatrolTask.h"
#include "../Tasks/SingleBarkTask.h"
#include "../Tasks/RepeatedBarkTask.h"
#include "../Tasks/MoveToPositionTask.h"
#include "../Tasks/IdleAnimationTask.h"
#include "../Tasks/WaitTask.h"
#include "ObservantState.h"
#include "../Library.h"

namespace ai
{


// Get the name of this state
const idStr& IdleState::GetName() const
{
	static idStr _name(STATE_IDLE);
	return _name;
}

bool IdleState::CheckAlertLevel(idAI* owner)
{
	if (owner->AI_AlertIndex > 0)
	{
		// Alert index is too high, switch to the higher State
		owner->GetMind()->PushState(owner->backboneStates[EObservant]);
		return false;
	}

	// Alert Index is matching, return OK
	return true;
}

void IdleState::Init(idAI* owner)
{
	// Init base class first
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("IdleState initialised.\r");
	assert(owner);

	// Memory shortcut
	Memory& memory = owner->GetMemory();
//	memory.alertClass = EAlertNone; // grayman #2603 - moved further down, otherwise we don't hear the correct rampdown bark
//	memory.alertType = EAlertTypeNone;

	_startSleeping = owner->spawnArgs.GetBool("sleeping", "0");
	_startSitting = owner->spawnArgs.GetBool("sitting", "0");
	
	if (owner->HasSeenEvidence() && owner->spawnArgs.GetBool("disable_alert_idle", "0") == false)
	{
		owner->GetMind()->SwitchState(STATE_ALERT_IDLE);
		return;
	}

	_alertLevelDecreaseRate = 0.01f;

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;

	owner->SheathWeapon();

	// Initialise the animation state
	if (_startSitting && memory.idlePosition == idVec3(idMath::INFINITY, idMath::INFINITY, idMath::INFINITY))
	{
		owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle", 0);
		owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle", 0);
		owner->SetAnimState(ANIMCHANNEL_HEAD, "Head_Idle", 0);

		owner->SitDown();
	}
	else if (owner->GetMoveType() == MOVETYPE_SIT)
	{
		owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle_Sit", 0);
		owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle_Sit", 0);
		owner->SetAnimState(ANIMCHANNEL_HEAD, "Head_Idle", 0);
	}
	else if (owner->GetMoveType() == MOVETYPE_SLEEP)
	{
		owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle_Sleep", 0);
		owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle_Sleep", 0);
		owner->SetAnimState(ANIMCHANNEL_HEAD, "Head_Idle_Sleep", 0);
	}
	else
	{
		owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle", 0);
		owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle", 0);
		owner->SetAnimState(ANIMCHANNEL_HEAD, "Head_Idle", 0);
	}

	owner->actionSubsystem->ClearTasks();
	owner->senseSubsystem->ClearTasks();

	// angua: drunken AIs won't do any head turning and idle anims for now
	if (owner->spawnArgs.GetBool("drunk", "0") == false)
	{
		// The action subsystem plays the idle anims (scratching, yawning...)
		owner->actionSubsystem->PushTask(IdleAnimationTask::CreateInstance());

		// The sensory system does its Idle tasks
		owner->senseSubsystem->PushTask(RandomHeadturnTask::CreateInstance());
	}

	InitialiseMovement(owner);

	if (_startSitting)
	{
		if (owner->spawnArgs.FindKey("sit_down_angle") != NULL)
		{
			owner->AI_SIT_DOWN_ANGLE = owner->spawnArgs.GetFloat("sit_down_angle", "0");
		}
		else
		{
			owner->AI_SIT_DOWN_ANGLE = memory.idleYaw;
		}

		owner->AI_SIT_DOWN_ANGLE = idMath::AngleNormalize180(owner->AI_SIT_DOWN_ANGLE);

		owner->AI_SIT_UP_ANGLE = memory.idleYaw;
	}	


	InitialiseCommunication(owner);
	memory.alertClass = EAlertNone;
	memory.alertType = EAlertTypeNone;

	int idleBarkIntervalMin = SEC2MS(owner->spawnArgs.GetInt("idle_bark_interval_min", "20"));
	int idleBarkIntervalMax = SEC2MS(owner->spawnArgs.GetInt("idle_bark_interval_max", "60"));
	// Push the regular patrol barking to the list too
	if (owner->spawnArgs.GetBool("drunk", "0"))
	{
		owner->commSubsystem->AddCommTask(
			CommunicationTaskPtr(new RepeatedBarkTask("snd_drunk", idleBarkIntervalMin, idleBarkIntervalMax))
		);
	}
	else
	{
		owner->commSubsystem->AddCommTask(
			CommunicationTaskPtr(new RepeatedBarkTask("snd_relaxed", idleBarkIntervalMin, idleBarkIntervalMax))
		);
	}

	// Let the AI update their weapons (make them nonsolid)
	owner->UpdateAttachmentContents(false);
}

// Gets called each time the mind is thinking
void IdleState::Think(idAI* owner)
{
	Memory& memory = owner->GetMemory();
	idStr waitState = owner->WaitState();

	if (_startSleeping && !owner->HasSeenEvidence() && owner->GetMoveType() == MOVETYPE_ANIM)
	{
		if (owner->ReachedPos(memory.idlePosition, MOVE_TO_POSITION) 
			&& owner->GetCurrentYaw() == memory.idleYaw)
		{
			owner->actionSubsystem->ClearTasks();
			owner->senseSubsystem->ClearTasks();

			owner->GetMind()->SwitchState(STATE_IDLE_SLEEP);
			return;
		}
	}
	else if (owner->GetMoveType() == MOVETYPE_SLEEP)
	{
		owner->GetMind()->SwitchState(STATE_IDLE_SLEEP);
		return;
	}
	else if (_startSitting && owner->GetMoveType() != MOVETYPE_SIT && waitState != "sit_down")
	{
		if (owner->ReachedPos(memory.idlePosition, MOVE_TO_POSITION) 
			&& owner->GetCurrentYaw() == memory.idleYaw)
		{
			owner->SitDown();
		}
	}

	UpdateAlertLevel();

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;

	// Let the AI check its senses
	owner->PerformVisualScan();
}

void IdleState::InitialiseMovement(idAI* owner)
{
	Memory& memory = owner->GetMemory();

	owner->AI_RUN = false;

	// The movement subsystem should start patrolling
	owner->movementSubsystem->ClearTasks();

	// angua: store the position at map start
	if (memory.idlePosition == idVec3(idMath::INFINITY, idMath::INFINITY, idMath::INFINITY))
	{
		// No idle position saved yet, take the current one
		memory.idlePosition = owner->GetPhysics()->GetOrigin();
		memory.idleYaw = owner->GetCurrentYaw();
	}

	if (owner->spawnArgs.GetBool("patrol", "1")) // grayman #2683 - only start patrol if you're supposed to 
	{
		owner->movementSubsystem->StartPatrol();
	}

	// Check if the owner has patrol routes set
	idPathCorner* path = memory.currentPath.GetEntity();
	idPathCorner* lastPath = memory.lastPath.GetEntity();

	if (path == NULL && lastPath == NULL)
	{
		// We don't have any patrol routes, so we're supposed to stand around
		// where the mapper has put us.
		
		if (owner->GetMoveType() == MOVETYPE_ANIM)
		{
			// angua: don't do this when we are sitting or sleeping
			// We already HAVE an idle position set, this means that we are
			// supposed to be there, let's move
			float startPosTolerance = owner->spawnArgs.GetFloat("startpos_tolerance", "-1");
			owner->movementSubsystem->PushTask(
				TaskPtr(new MoveToPositionTask(memory.idlePosition, memory.idleYaw, startPosTolerance))
			);
		}
	}
	
}

void IdleState::InitialiseCommunication(idAI* owner)
{
	Memory& memory = owner->GetMemory(); // grayman #2603 - only allow rampdown barks if the AI was searching

	if (memory.searchFlags & SRCH_WAS_SEARCHING)
	{
		memory.searchFlags = 0; // clear
		// Push a single bark to the communication subsystem first, it fires only once
		owner->commSubsystem->AddCommTask(
			CommunicationTaskPtr(new SingleBarkTask(GetInitialIdleBark(owner))));
	}
}


void IdleState::Save(idSaveGame* savefile) const
{
	State::Save(savefile);

	savefile->WriteBool(_startSitting);
	savefile->WriteBool(_startSleeping);
}

void IdleState::Restore(idRestoreGame* savefile)
{
	State::Restore(savefile);

	savefile->ReadBool(_startSitting);
	savefile->ReadBool(_startSleeping);
}

idStr IdleState::GetInitialIdleBark(idAI* owner)
{
	// greebo: Ported from ai_darkmod_base::task_Idle written by SZ

	Memory& memory = owner->GetMemory();

	// Decide what sound it is appropriate to play
	idStr soundName("");

	if (!owner->m_RelightingLight &&	// grayman #2603 - No rampdown bark if relighting a light.
		!owner->m_ExaminingRope &&		// grayman #2872 - No rampdown bark if examining a rope.
		(owner->m_maxAlertLevel >= owner->thresh_1) &&
		(owner->m_maxAlertLevel < owner->thresh_4))
	{
		if (memory.alertClass == EAlertVisual_2) // grayman #2603
		{
			soundName = "snd_alertdown0sus";
		}
		else if (memory.alertClass == EAlertVisual_1)
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

StatePtr IdleState::CreateInstance()
{
	return StatePtr(new IdleState);
}

void IdleState::OnChangeTarget(idAI* owner)
{
	// re-initialize movement subsystem to catch new path_corners
	InitialiseMovement(owner);
}

// Register this state with the StateLibrary
StateLibrary::Registrar idleStateRegistrar(
	STATE_IDLE, // Task Name
	StateLibrary::CreateInstanceFunc(&IdleState::CreateInstance) // Instance creation callback
);

} // namespace ai
