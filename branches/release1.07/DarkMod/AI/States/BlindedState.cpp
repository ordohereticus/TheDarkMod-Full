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

static bool init_version = FileVersionList("$Id: BlindedState.cpp 4991 2011-10-11 17:49:38Z grayman $", init_version);

#include "BlindedState.h"
#include "../Tasks/SingleBarkTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

// Get the name of this state
const idStr& BlindedState::GetName() const
{
	static idStr _name(STATE_BLINDED);
	return _name;
}


void BlindedState::Init(idAI* owner)
{
	// Init base class first
	State::Init(owner);

	owner->movementSubsystem->ClearTasks();
	owner->senseSubsystem->ClearTasks();
	owner->actionSubsystem->ClearTasks();

	owner->StopMove(MOVE_STATUS_DONE);

	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Blinded", 4);
	owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Blinded", 4);

	owner->SetWaitState(ANIMCHANNEL_TORSO, "blinded");
	owner->SetWaitState(ANIMCHANNEL_LEGS, "blinded");

	Memory& memory = owner->GetMemory();
	memory.stopRelight = true; // grayman #2603 - abort a relight in progress
	memory.stopExaminingRope = true; // grayman #2872 - abort a rope examination

	CommMessagePtr message(new CommMessage(
		CommMessage::RequestForHelp_CommType, 
		owner, NULL, // from this AI to anyone 
		NULL,
		memory.alertPos
	));

	owner->commSubsystem->AddCommTask(
		CommunicationTaskPtr(new SingleBarkTask("snd_blinded", message))
	);

	float duration = SEC2MS(owner->spawnArgs.GetFloat("blind_time", "4")) + 
		(gameLocal.random.RandomFloat() - 0.5f) * 2 * SEC2MS(owner->spawnArgs.GetFloat("blind_time_fuzziness", "2"));

	_endTime = gameLocal.time + static_cast<int>(duration);

	// Set alert level a little bit below combat
	if (owner->AI_AlertLevel < owner->thresh_5 - 1)
	{
		owner->SetAlertLevel(owner->thresh_5 - 1);
	}

	_oldAcuity = owner->GetAcuity("vis");
	owner->SetAcuity("vis", 0);
}

// Gets called each time the mind is thinking
void BlindedState::Think(idAI* owner)
{
	if (gameLocal.time >= _endTime)
	{
		owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle", 4);
		owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle", 4);

		owner->SetWaitState(ANIMCHANNEL_TORSO, "");
		owner->SetWaitState(ANIMCHANNEL_LEGS, "");

		owner->SetAcuity("vis", _oldAcuity);

		owner->GetMind()->EndState();
		return;
	}
}

void BlindedState::Save(idSaveGame* savefile) const
{
	State::Save(savefile);

	savefile->WriteInt(_endTime);
	savefile->WriteFloat(_oldAcuity);
}

void BlindedState::Restore(idRestoreGame* savefile)
{
	State::Restore(savefile);

	savefile->ReadInt(_endTime);
	savefile->ReadFloat(_oldAcuity);
}

StatePtr BlindedState::CreateInstance()
{
	return StatePtr(new BlindedState);
}

// Register this state with the StateLibrary
StateLibrary::Registrar blindedStateRegistrar(
	STATE_BLINDED, // Task Name
	StateLibrary::CreateInstanceFunc(&BlindedState::CreateInstance) // Instance creation callback
);

} // namespace ai
