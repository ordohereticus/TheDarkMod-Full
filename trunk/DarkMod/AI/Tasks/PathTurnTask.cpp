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

static bool init_version = FileVersionList("$Id: PathTurnTask.cpp 2443 2008-06-07 13:48:49Z angua $", init_version);

#include "../Memory.h"
#include "PatrolTask.h"
#include "PathTurnTask.h"
#include "../Library.h"

namespace ai
{

PathTurnTask::PathTurnTask()
{}

PathTurnTask::PathTurnTask(idPathCorner* path)
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
	// Just init the base class
	Task::Init(owner, subsystem);

	idPathCorner* path = _path.GetEntity();

	if (path == NULL) {
		gameLocal.Error("PathTurnTask: Path Entity not set before Init()");
	}

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

		// Move is done, fall back to PatrolTask
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("Turn is done.\r");

		return true; // finish this task
	}
	return false;
}

void PathTurnTask::SetTargetEntity(idPathCorner* path) 
{
	assert(path);
	_path = path;
}

// Save/Restore methods
void PathTurnTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	_path.Save(savefile);
}

void PathTurnTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	_path.Restore(savefile);
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
