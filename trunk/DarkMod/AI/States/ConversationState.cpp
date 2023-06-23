/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2674 $
 * $Date: 2008-07-16 14:01:10 -0400 (Wed, 16 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ConversationState.cpp 2674 2008-07-16 18:01:10Z greebo $", init_version);

#include "ConversationState.h"
#include "../Memory.h"
#include "../Tasks/IdleAnimationTask.h"
#include "ObservantState.h"
#include "../Library.h"

namespace ai
{

// Get the name of this state
const idStr& ConversationState::GetName() const
{
	static idStr _name(STATE_CONVERSATION);
	return _name;
}

bool ConversationState::CheckAlertLevel(idAI* owner)
{
	if (owner->AI_AlertIndex > 0)
	{
		// Alert index is too high, switch to the higher State
		owner->GetMind()->PushState(STATE_OBSERVANT);
		return false;
	}

	// Alert Index is matching, return OK
	return true;
}

void ConversationState::SetConversation(int index)
{
	// TODO: Sanity-Check

	_conversation = index;
}

void ConversationState::Init(idAI* owner)
{
	// Init base class first
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("ConversationState initialised.\r");
	assert(owner);

	// Memory shortcut
	Memory& memory = owner->GetMemory();
	memory.alertClass = EAlertNone;
	memory.alertType = EAlertTypeNone;

	_alertLevelDecreaseRate = 0.01f;

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;

	// Check dialogue prerequisites
	if (!CheckConversationPrerequisites())
	{
		owner->mind->EndState();
		return;
	}

	owner->GetSubsystem(SubsysAction)->ClearTasks();
	owner->GetSubsystem(SubsysSenses)->ClearTasks();
	owner->GetSubsystem(SubsysCommunication)->ClearTasks();
}

// Gets called each time the mind is thinking
void ConversationState::Think(idAI* owner)
{
	Memory& memory = owner->GetMemory();

	UpdateAlertLevel();

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;

	// Let the AI check its senses
	owner->PerformVisualScan();
}

bool ConversationState::CheckConversationPrerequisites()
{
	// TODO
	return true;
}

void ConversationState::Execute(const ConversationCommandPtr& command)
{

}

ConversationState::Status ConversationState::GetStatus()
{
	return _status;
}

void ConversationState::Save(idSaveGame* savefile) const
{
	State::Save(savefile);

	savefile->WriteInt(_conversation);
	savefile->WriteInt(static_cast<int>(_status));
}

void ConversationState::Restore(idRestoreGame* savefile)
{
	State::Restore(savefile);

	savefile->ReadInt(_conversation);
	int temp;
	savefile->ReadInt(temp);
	assert(temp >= EDoingNothing && temp < ENumConversationStati);
	_status = static_cast<Status>(temp);
}

StatePtr ConversationState::CreateInstance()
{
	return StatePtr(new ConversationState);
}

// Register this state with the StateLibrary
StateLibrary::Registrar conversationStateRegistrar(
	STATE_CONVERSATION, // Task Name
	StateLibrary::CreateInstanceFunc(&ConversationState::CreateInstance) // Instance creation callback
);

} // namespace ai
