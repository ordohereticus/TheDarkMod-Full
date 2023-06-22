/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 18:53:28 +0200 (Di, 16 Okt 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: PatrolBarkTask.cpp 1435 2007-10-16 16:53:28Z greebo $", init_version);

#include "PatrolBarkTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

// Get the name of this task
const idStr& PatrolBarkTask::GetName() const
{
	static idStr _name(TASK_PATROL_BARK);
	return _name;
}

void PatrolBarkTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	// Get the repeat interval in seconds, convert to ms
	_barkRepeatInterval = SEC2MS(owner->spawnArgs.GetFloat("bark_repeat_patrol", "45"));

	// If the last chat time is not yet set, initialise it to play the sound now
	if (owner->GetMind()->GetMemory().lastPatrolChatTime == -1)
	{
		owner->GetMind()->GetMemory().lastPatrolChatTime = gameLocal.time - _barkRepeatInterval;
	}
}

void PatrolBarkTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO).LogString("Patrol Bark Task performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointers
	assert(owner != NULL);

	if (gameLocal.time - owner->GetMind()->GetMemory().lastPatrolChatTime > _barkRepeatInterval)
	{
		// The time has come, bark now
		owner->PlayAndLipSync("snd_relaxed", "talk1");

		// Reset the timer
		owner->GetMind()->GetMemory().lastPatrolChatTime = gameLocal.time;
	}
}

// Save/Restore methods
void PatrolBarkTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);
	savefile->WriteInt(_barkRepeatInterval);
}

void PatrolBarkTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);
	savefile->ReadInt(_barkRepeatInterval);
}

PatrolBarkTaskPtr PatrolBarkTask::CreateInstance()
{
	return PatrolBarkTaskPtr(new PatrolBarkTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar patrolBarkTaskRegistrar(
	TASK_PATROL_BARK, // Task Name
	TaskLibrary::CreateInstanceFunc(&PatrolBarkTask::CreateInstance) // Instance creation callback
);

} // namespace ai
