/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5341 $ (Revision of last commit) 
 $Date: 2012-03-16 11:06:06 -0400 (Fri, 16 Mar 2012) $ (Date of last commit)
 $Author: grayman $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: ObservantState.cpp 5341 2012-03-16 15:06:06Z grayman $");

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
	DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("%d: ObservantState::CheckAlertLevel - %s - checking alert level; are we in the correct state?\r",gameLocal.time,owner->name.c_str()); // grayman debug
	if (owner->AI_AlertIndex < 1)
	{
		DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("     %s - alert level too low, dropping back to Relaxed\r",owner->name.c_str()); // grayman debug
		// Alert index is too low for this state, fall back

		// grayman #1327 - if you were searching a suspicious
		// door, let it go, in case you haven't already

		Memory& memory = owner->GetMemory();
		CFrobDoor* door = memory.closeMe.GetEntity();
		if ( door )
		{
			memory.closeMe = NULL;
			memory.closeSuspiciousDoor = false;
			door->SetSearching(NULL);
		}

		owner->GetMind()->EndState();
		return false;
	}
	
	if (owner->AI_AlertIndex > 1)
	{
		DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("     %s - alert level too high, bumping up to Suspicious\r",owner->name.c_str()); // grayman debug
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

	DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("%d: ObservantState::Init - %s\r",gameLocal.time,owner->name.c_str()); // grayman debug
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("ObservantState initialised.\r");
	assert(owner);

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner))
	{
		return;
	}

	DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("     %s is in the correct state: Observant\r",owner->name.c_str()); // grayman debug
	// Shortcut reference
	Memory& memory = owner->GetMemory();

	float alertTime = owner->atime1 + owner->atime1_fuzzyness * (gameLocal.random.RandomFloat() - 0.5);
	_alertLevelDecreaseRate = (owner->thresh_2 - owner->thresh_1) / alertTime;

	// grayman #2866 - zero alert decrease rate if handling a door or elevator

	memory.savedAlertLevelDecreaseRate = _alertLevelDecreaseRate; // save for restoration later

	if ((owner->m_HandlingDoor) || (owner->m_HandlingElevator))
	{
		_alertLevelDecreaseRate = 0;
	}

	// Stop playing idle animation
	owner->actionSubsystem->ClearTasks();

	if (owner->GetMoveType() != MOVETYPE_SLEEP)
	{
		// barking
		idStr soundName("");

		if (owner->AlertIndexIncreased() && (memory.alertType != EAlertTypeMissingItem))
		{
			DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("     %s playing an ascending bark\r",owner->name.c_str()); // grayman debug
			if ((memory.alertClass == EAlertVisual_1) || (memory.alertClass == EAlertVisual_2)) // grayman #2603
			{
				soundName = "snd_alert1s"; // did I just see something?
			}
			else if (memory.alertClass == EAlertAudio)
			{
				soundName = "snd_alert1h"; // what was that sound?
			}
			else
			{
				soundName = "snd_alert1"; // what was that?
			}
		}
		else if (owner->HasSeenEvidence())
		{
			if (owner->m_lastAlertLevel >= owner->thresh_3)
			{
				DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("     %s playing a descending bark\r",owner->name.c_str()); // grayman debug
				soundName = "snd_alertdown0SeenEvidence"; // you win this round
			}
		}
		else if (owner->m_lastAlertLevel >= owner->thresh_4)
		{
			DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("     %s playing a descending bark\r",owner->name.c_str()); // grayman debug
			soundName = "snd_alertdown0SeenNoEvidence"; // that put me in a foul mood
		}

		if (memory.alertType != EAlertTypeMissingItem)
		{
			CommunicationTaskPtr barkTask(new SingleBarkTask(soundName));

			owner->commSubsystem->AddCommTask(barkTask);

			DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("     %s wait for 1s, then say the Observant 'coming down' sound %s\r",owner->name.c_str(),soundName.c_str()); // grayman debug
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
	DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("%d: ObservantState::Think - %s at alert level %f\r",gameLocal.time,owner->name.c_str(),owner->AI_AlertLevel); // grayman debug
	UpdateAlertLevel();
	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner))
	{
		return;
	}
	
	// grayman #2866 - zero alert decrease rate if handling a door or elevator

	if ((owner->m_HandlingDoor) || (owner->m_HandlingElevator))
	{
		_alertLevelDecreaseRate = 0;
	}
	else
	{
		_alertLevelDecreaseRate = owner->GetMemory().savedAlertLevelDecreaseRate;
	}

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
