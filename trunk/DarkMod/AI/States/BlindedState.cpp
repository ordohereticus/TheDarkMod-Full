/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3354 $
 * $Date: 2009-04-04 07:41:43 -0400 (Sat, 04 Apr 2009) $
 * $Author: angua $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: BlindedState.cpp 3354 2009-04-04 11:41:43Z angua $", init_version);

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
//	owner->GetSubsystem(SubsysCommunication)->ClearTasks();// TODO_AI
	owner->actionSubsystem->ClearTasks();

	owner->StopMove(MOVE_STATUS_DONE);

	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Blinded", 4);
	owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Blinded", 4);

	owner->SetWaitState(ANIMCHANNEL_TORSO, "blinded");
	owner->SetWaitState(ANIMCHANNEL_LEGS, "blinded");

	// The communication system should bark
/*	owner->GetSubsystem(SubsysCommunication)->PushTask(
		TaskPtr(new SingleBarkTask("snd_blinded"))
	);*/

	_endTime = gameLocal.time + 4000 + static_cast<int>(gameLocal.random.RandomFloat() * 2000);

	// Set alert level a little bit below combat
	if(owner->AI_AlertLevel < owner->thresh_5 - 1)
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
