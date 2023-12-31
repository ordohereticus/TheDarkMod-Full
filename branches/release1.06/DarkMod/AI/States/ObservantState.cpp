/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4870 $
 * $Date: 2011-05-31 13:59:19 -0400 (Tue, 31 May 2011) $
 * $Author: grayman $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ObservantState.cpp 4870 2011-05-31 17:59:19Z grayman $", init_version);

#include "ObservantState.h"
#include "../Memory.h"
#include "../../AIComm_Message.h"
#include "../Tasks/RandomHeadturnTask.h"
#include "../Tasks/SingleBarkTask.h"
#include "../Tasks/CommWaitTask.h"
#include "../Tasks/WaitTask.h"
#include "SuspiciousState.h"
#include "../Library.h"

namespace ai
{

// Get the name of this state
const idStr& ObservantState::GetName() const
{
	static idStr _name(STATE_OBSERVANT);
	return _name;
}

bool ObservantState::CheckAlertLevel(idAI* owner)
{
	if (owner->AI_AlertIndex < 1)
	{
		// Alert index is too low for this state, fall back
		owner->GetMind()->EndState();
		return false;
	}
	else if (owner->AI_AlertIndex > 1)
	{
		// Alert index is too high, switch to the higher State
		owner->GetMind()->PushState(owner->backboneStates[ESuspicious]);
		return false;
	}

	// Alert Index is matching, return OK
	return true;
}

void ObservantState::Init(idAI* owner)
{
	// Init base class first
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("ObservantState initialised.\r");
	assert(owner);

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;

	float alertTime = owner->atime1 + owner->atime1_fuzzyness * (gameLocal.random.RandomFloat() - 0.5);
	_alertLevelDecreaseRate = (owner->thresh_2 - owner->thresh_1) / alertTime;

	// Shortcut reference
	Memory& memory = owner->GetMemory();

	// Stop playing idle animation
	owner->actionSubsystem->ClearTasks();

	if (owner->GetMoveType() != MOVETYPE_SLEEP)
	{
		// barking
		idStr soundName("");

		if (owner->AlertIndexIncreased() && (memory.alertType != EAlertTypeMissingItem))
		{
			if ((memory.alertClass == EAlertVisual_1) || (memory.alertClass == EAlertVisual_2)) // grayman #2603
			{
				soundName = "snd_alert1s";
			}
			else if (memory.alertClass == EAlertAudio)
			{
				soundName = "snd_alert1h";
			}
			else
			{
				soundName = "snd_alert1";
			}
		}
		else if (owner->HasSeenEvidence())
		{
			if (owner->m_lastAlertLevel >= owner->thresh_3)
			{
				soundName = "snd_alertdown0SeenEvidence";
			}
		}
		else if (owner->m_lastAlertLevel >= owner->thresh_4)
		{
			soundName = "snd_alertdown0SeenNoEvidence";
		}

		if (memory.alertType != EAlertTypeMissingItem)
		{
			CommunicationTaskPtr barkTask(new SingleBarkTask(soundName));

			owner->commSubsystem->AddCommTask(barkTask);

			// Push a wait task (1 sec) with the bark priority -1, to have it queued
			owner->commSubsystem->AddCommTask(
				CommunicationTaskPtr(new CommWaitTask(1000, barkTask->GetPriority() - 1))
			);
		}
	}

	// Let the AI update their weapons (make them nonsolid)
	owner->UpdateAttachmentContents(false);
}

// Gets called each time the mind is thinking
void ObservantState::Think(idAI* owner)
{
	UpdateAlertLevel();
	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;
	
	if (owner->GetMoveType() != MOVETYPE_SLEEP)
	{
		// Let the AI check its senses
		owner->PerformVisualScan();
	}
}

StatePtr ObservantState::CreateInstance()
{
	return StatePtr(new ObservantState);
}

// Register this state with the StateLibrary
StateLibrary::Registrar observantStateRegistrar(
	STATE_OBSERVANT, // Task Name
	StateLibrary::CreateInstanceFunc(&ObservantState::CreateInstance) // Instance creation callback
);

} // namespace ai
