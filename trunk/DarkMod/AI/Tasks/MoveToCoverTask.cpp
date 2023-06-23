/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2443 $
 * $Date: 2008-06-07 09:48:49 -0400 (Sat, 07 Jun 2008) $
 * $Author: angua $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MoveToCoverTask.cpp 2443 2008-06-07 13:48:49Z angua $", init_version);

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
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("Move to Cover Task performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointer
	assert(owner != NULL);

	if (owner->AI_DEST_UNREACHABLE)
	{
		//TODO
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("Destination unreachable.\r");
		return true;
	}

	if (owner->AI_MOVE_DONE)
	{
		// Move is done, 
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("Move is done.\r");
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
