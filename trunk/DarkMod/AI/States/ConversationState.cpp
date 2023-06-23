/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2677 $
 * $Date: 2008-07-16 15:21:26 -0400 (Wed, 16 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ConversationState.cpp 2677 2008-07-16 19:21:26Z greebo $", init_version);

#include "ConversationState.h"
#include "../Memory.h"
#include "../Tasks/IdleAnimationTask.h"
#include "ObservantState.h"
#include "../Library.h"

// greebo: This spawnarg holds the currently played conversation sound
#define CONVERSATION_SPAWNARG "snd_TEMP_conv"

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

bool ConversationState::Execute(ConversationCommand& command)
{
	bool success = true;

	idAI* owner = _owner.GetEntity();
	assert(owner != NULL);

	switch (command.GetType())
	{
	case ConversationCommand::ETalk:
		Talk(owner, command.GetArgument(0));
		// TODO: Set end time
		_status = EExecuting;
		break;
	default:
		gameLocal.Warning("Unknown command type found %d", command.GetType());
		DM_LOG(LC_CONVERSATION, LT_ERROR)LOGSTRING("Unknown command type found %d", command.GetType());
		success = false;
	};

	return success;
}

int ConversationState::Talk(idAI* owner, const idStr& soundName)
{
	const idKeyValue* kv = owner->spawnArgs.FindKey(soundName);

	if (kv != NULL && kv->GetValue().Icmpn( "snd_", 4 ) == 0)
	{
		// The conversation argument is pointing to a valid spawnarg on the owner
		owner->spawnArgs.Set(CONVERSATION_SPAWNARG, kv->GetValue());
	}
	else
	{
		// The spawnargs don't define the sound shader, set the shader directly
		owner->spawnArgs.Set(CONVERSATION_SPAWNARG, soundName);
	}

	// Start the sound
	int length = owner->PlayAndLipSync(CONVERSATION_SPAWNARG, "talk1");

	// Clear the spawnarg again
	owner->spawnArgs.Set(CONVERSATION_SPAWNARG, "");

	return length;
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
