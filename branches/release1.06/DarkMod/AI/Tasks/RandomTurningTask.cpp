/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4862 $
 * $Date: 2011-05-22 08:41:11 -0400 (Sun, 22 May 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: RandomTurningTask.cpp 4862 2011-05-22 12:41:11Z greebo $", init_version);

#include "../Memory.h"
#include "RandomTurningTask.h"
#include "../Library.h"

namespace ai
{

// Get the name of this task
const idStr& RandomTurningTask::GetName() const
{
	static idStr _name(TASK_RANDOM_TURNING);
	return _name;
}

void RandomTurningTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	_nextYaw = owner->GetCurrentYaw() + (gameLocal.random.RandomFloat() - 0.5) * 360;
	owner->TurnToward(_nextYaw);
	_turning = true;
	_nextTurningTime = gameLocal.time;
}

bool RandomTurningTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("Random Turning Task performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointers
	assert(owner != NULL);

	if (_turning && owner->FacingIdeal())
	{		
		// TODO: un-hardcode
		int turnDelay = static_cast<int>(1000 + gameLocal.random.RandomFloat() * 400);
		
		// Wait a bit before turning again
		_nextTurningTime = gameLocal.time + turnDelay;
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("Turn is done.\r");
		_turning = false;
		_nextYaw = owner->GetCurrentYaw() + (gameLocal.random.RandomFloat() - 0.5) * 180;
		
	}

	if (!_turning && gameLocal.time >= _nextTurningTime)
	{
		owner->TurnToward(_nextYaw);
		_turning = true;
	}

	return false;
}

// Save/Restore methods
void RandomTurningTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	savefile->WriteFloat(_nextYaw);
	savefile->WriteBool(_turning);
	savefile->WriteInt(_nextTurningTime);
}

void RandomTurningTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	savefile->ReadFloat(_nextYaw);
	savefile->ReadBool(_turning);
	savefile->ReadInt(_nextTurningTime);
}

RandomTurningTaskPtr RandomTurningTask::CreateInstance()
{
	return RandomTurningTaskPtr(new RandomTurningTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar randomTurningTaskRegistrar(
	TASK_RANDOM_TURNING, // Task Name
	TaskLibrary::CreateInstanceFunc(&RandomTurningTask::CreateInstance) // Instance creation callback
);

} // namespace ai
