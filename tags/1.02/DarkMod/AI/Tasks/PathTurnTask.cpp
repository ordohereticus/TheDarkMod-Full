/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3767 $
 * $Date: 2009-12-01 00:17:07 -0500 (Tue, 01 Dec 2009) $
 * $Author: angua $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: PathTurnTask.cpp 3767 2009-12-01 05:17:07Z angua $", init_version);

#include "../Memory.h"
#include "PatrolTask.h"
#include "PathTurnTask.h"
#include "../Library.h"

namespace ai
{

PathTurnTask::PathTurnTask() :
	PathTask()
{}

PathTurnTask::PathTurnTask(idPathCorner* path) :
	PathTask(path)
{
	_path = path;
}

// Get the name of this task
const idStr& PathTurnTask::GetName() const
{
	static idStr _name(TASK_PATH_TURN);
	return _name;
}

void PathTurnTask::Init(idAI* owner, Subsystem& subsystem)
{
	PathTask::Init(owner, subsystem);

	idPathCorner* path = _path.GetEntity();

	//Start turning
	float angle = path->spawnArgs.GetFloat("angle","0");
	owner->TurnToward(angle);
}

bool PathTurnTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("Path Turn Task performing.\r");

	idPathCorner* path = _path.GetEntity();
	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointers
	assert(path != NULL && owner != NULL);

	// Move on to next target when turning is done
	if (owner->FacingIdeal())
	{
		// Trigger path targets, now that we've reached the corner
		owner->ActivateTargets(owner);
		
		// NextPath();

		// Move is done, fall back to PatrolTask
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("Turn is done.\r");

		return true; // finish this task
	}
	return false;
}

PathTurnTaskPtr PathTurnTask::CreateInstance()
{
	return PathTurnTaskPtr(new PathTurnTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar pathTurnTaskRegistrar(
	TASK_PATH_TURN, // Task Name
	TaskLibrary::CreateInstanceFunc(&PathTurnTask::CreateInstance) // Instance creation callback
);

} // namespace ai
