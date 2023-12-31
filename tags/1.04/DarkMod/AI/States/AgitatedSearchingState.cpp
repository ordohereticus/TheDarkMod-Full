/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3858 $
 * $Date: 2010-03-20 23:43:30 -0400 (Sat, 20 Mar 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: AgitatedSearchingState.cpp 3858 2010-03-21 03:43:30Z greebo $", init_version);

#include "AgitatedSearchingState.h"
#include "../Memory.h"
#include "../Tasks/InvestigateSpotTask.h"
#include "../Tasks/SingleBarkTask.h"
#include "../Tasks/RepeatedBarkTask.h"
#include "../Tasks/WaitTask.h"
#include "CombatState.h"
#include "../Library.h"
#include "../../AbsenceMarker.h"
#include "../../AIComm_Message.h"

namespace ai
{

// Get the name of this state
const idStr& AgitatedSearchingState::GetName() const
{
	static idStr _name(STATE_AGITATED_SEARCHING);
	return _name;
}

bool AgitatedSearchingState::CheckAlertLevel(idAI* owner)
{
	if (owner->AI_AlertIndex < 4)
	{
		// Alert index is too low for this state, fall back
		owner->GetMind()->EndState();
		return false;
	}
	else if (owner->AI_AlertIndex > 4)
	{
		// Alert index is too high, switch to the higher State
		owner->Event_CloseHidingSpotSearch();
		owner->GetMind()->PushState(owner->backboneStates[ECombat]);
		return false;
	}

	// Alert Index is matching, return OK
	return true;
}

void AgitatedSearchingState::CalculateAlertDecreaseRate(idAI* owner)
{
	float alertTime = owner->atime4 + owner->atime4_fuzzyness * (gameLocal.random.RandomFloat() - 0.5);
	_alertLevelDecreaseRate = (owner->thresh_5 - owner->thresh_4) / alertTime;
}

void AgitatedSearchingState::Init(idAI* owner)
{
	// Init base class first (note: we're not calling SearchingState::Init() on purpose here)
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("AgitatedSearchingState initialised.\r");
	assert(owner);

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;

	// Shortcut reference
	Memory& memory = owner->GetMemory();

	CalculateAlertDecreaseRate(owner);
	
	if (owner->GetMoveType() == MOVETYPE_SIT || owner->GetMoveType() == MOVETYPE_SLEEP)
	{
		owner->GetUp();
	}

	// Setup a new hiding spot search
	StartNewHidingSpotSearch(owner);

	CommMessagePtr message = CommMessagePtr(new CommMessage(
		CommMessage::DetectedSomethingSuspicious_CommType, 
		owner, NULL, // from this AI to anyone
		NULL,
		memory.alertPos
	));


	if (owner->AlertIndexIncreased())
	{

		if (memory.alertedDueToCommunication == false && (memory.alertType == EAlertTypeSuspicious || memory.alertType == EAlertTypeEnemy))
		{
			owner->commSubsystem->AddCommTask(
				CommunicationTaskPtr(new SingleBarkTask("snd_alert4",message))
			);
		}
	}

	owner->commSubsystem->AddSilence(5000);

	int minTime = SEC2MS(owner->spawnArgs.GetFloat("searchbark_delay_min", "10"));
	int maxTime = SEC2MS(owner->spawnArgs.GetFloat("searchbark_delay_max", "15"));

	if (owner->HasSeenEvidence())
	{
		owner->commSubsystem->AddCommTask(
			CommunicationTaskPtr(new RepeatedBarkTask("snd_state4SeenEvidence", minTime, maxTime, message))
		);
	}
	else
	{
		owner->commSubsystem->AddCommTask(
			CommunicationTaskPtr(new RepeatedBarkTask("snd_state4SeenNoEvidence", minTime, maxTime, message))
		);
	}
	
	owner->DrawWeapon();

	// Let the AI update their weapons (make them solid)
	owner->UpdateAttachmentContents(true);
}

// Gets called each time the mind is thinking
void AgitatedSearchingState::Think(idAI* owner)
{
	SearchingState::Think(owner);
}

StatePtr AgitatedSearchingState::CreateInstance()
{
	return StatePtr(static_cast<State*>(new AgitatedSearchingState));
}

// Register this state with the StateLibrary
StateLibrary::Registrar agitatedSearchingStateRegistrar(
	STATE_AGITATED_SEARCHING, // Task Name
	StateLibrary::CreateInstanceFunc(&AgitatedSearchingState::CreateInstance) // Instance creation callback
);

} // namespace ai
