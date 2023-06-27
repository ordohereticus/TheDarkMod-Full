/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5093 $
 * $Date: 2011-12-06 12:34:56 -0500 (Tue, 06 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MoveToCoverTask.cpp 5093 2011-12-06 17:34:56Z greebo $", init_version);

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
	owner->m_pathRank = owner->rank; // grayman #2345

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
