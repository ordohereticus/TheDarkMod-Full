/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MoveToCoverTask.cpp 2338 2008-05-15 16:23:41Z greebo $", init_version);

#include "../Memory.h"
#include "MoveToCoverTask.h"
#include "../Library.h"

namespace ai
{

// Get the name of this task
const idStr& MoveToCoverTask::GetName() const
{
	static idStr _name(TASK_MOVE_TO_COVER);
	return _name;
}

void MoveToCoverTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Init the base class
	Task::Init(owner, subsystem);
	idActor* enemy = owner->GetEnemy();

	//Move to cover position
	owner->AI_RUN = true;
	owner->AI_FORWARD = true;

	owner->MoveToCover(enemy, owner->lastVisibleEnemyPos);
}

bool MoveToCoverTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO).LogString("Move to Cover Task performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointer
	assert(owner != NULL);

	if (owner->AI_DEST_UNREACHABLE)
	{
		//TODO
		DM_LOG(LC_AI, LT_INFO).LogString("Destination unreachable.\r");
		return true;
	}

	if (owner->AI_MOVE_DONE)
	{
		// Move is done, 
		DM_LOG(LC_AI, LT_INFO).LogString("Move is done.\r");
		owner->TurnToward(owner->lastVisibleEnemyPos);

		// finish this task

		return true;
	}

	return false; // not finished yet
}

MoveToCoverTaskPtr MoveToCoverTask::CreateInstance()
{
	return MoveToCoverTaskPtr(new MoveToCoverTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar moveToCoverTaskRegistrar(
	TASK_MOVE_TO_COVER, // Task Name
	TaskLibrary::CreateInstanceFunc(&MoveToCoverTask::CreateInstance) // Instance creation callback
);

} // namespace ai
