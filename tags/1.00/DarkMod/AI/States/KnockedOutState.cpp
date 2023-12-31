/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3698 $
 * $Date: 2009-09-07 07:01:56 -0400 (Mon, 07 Sep 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: KnockedOutState.cpp 3698 2009-09-07 11:01:56Z greebo $", init_version);

#include "KnockedOutState.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

// Get the name of this state
const idStr& KnockedOutState::GetName() const
{
	static idStr _name(STATE_KNOCKED_OUT);
	return _name;
}

void KnockedOutState::Init(idAI* owner)
{
	// Init base class first
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("KnockedOutState initialised.\r");
	assert(owner);
	
	// Clear all the subsystems
	owner->movementSubsystem->ClearTasks();
	owner->senseSubsystem->ClearTasks();
	owner->actionSubsystem->ClearTasks();
	owner->commSubsystem->ClearTasks();

	_waitingForKnockout = true;

	// Stop move!
	owner->StopMove(MOVE_STATUS_DONE);

	//owner->StopAnim(ANIMCHANNEL_TORSO, 0);
	//owner->StopAnim(ANIMCHANNEL_LEGS, 0);
	//owner->StopAnim(ANIMCHANNEL_HEAD, 0);

	// angua: disabled for Thief's Den release
	// anims didn't look good and produced problems
/*
	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_KO", 0);
	owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_KO", 0);

	// greebo: Set the waitstate, this gets cleared by 
	// the script function when the animation is done.
	owner->SetWaitState(ANIMCHANNEL_TORSO, "knock_out");
	owner->SetWaitState(ANIMCHANNEL_LEGS, "knock_out");
	// Don't do anything else, the KO animation will finish in a few frames
	// and the AI is done afterwards.
*/
	owner->SetAnimState(ANIMCHANNEL_HEAD, "Head_KO", 0);
	owner->SetWaitState(ANIMCHANNEL_HEAD, "knock_out");
}

// Gets called each time the mind is thinking
void KnockedOutState::Think(idAI* owner)
{
	if (_waitingForKnockout 
		&&	idStr(owner->WaitState(ANIMCHANNEL_TORSO)) != "knock_out"
		&&	idStr(owner->WaitState(ANIMCHANNEL_LEGS)) != "knock_out"
		&&	idStr(owner->WaitState(ANIMCHANNEL_HEAD)) != "knock_out") 
	{
		owner->PostKnockOut();
		_waitingForKnockout = false;
	}
}

// Save/Restore methods
void KnockedOutState::Save(idSaveGame* savefile) const
{
	State::Save(savefile);
	savefile->WriteBool(_waitingForKnockout);
}

void KnockedOutState::Restore(idRestoreGame* savefile) 
{
	State::Restore(savefile);
	savefile->ReadBool(_waitingForKnockout);
}

StatePtr KnockedOutState::CreateInstance()
{
	return StatePtr(new KnockedOutState);
}

// Register this state with the StateLibrary
StateLibrary::Registrar knockedOutStateRegistrar(
	STATE_KNOCKED_OUT, // Task Name
	StateLibrary::CreateInstanceFunc(&KnockedOutState::CreateInstance) // Instance creation callback
);

} // namespace ai
