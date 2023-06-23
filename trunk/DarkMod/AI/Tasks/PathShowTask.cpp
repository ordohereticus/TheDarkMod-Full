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

static bool init_version = FileVersionList("$Id: PathShowTask.cpp 2338 2008-05-15 16:23:41Z greebo $", init_version);

#include "../Memory.h"
#include "PatrolTask.h"
#include "PathShowTask.h"
#include "../Library.h"

namespace ai
{

PathShowTask::PathShowTask() 
{}

PathShowTask::PathShowTask(idPathCorner* path) 
{
	_path = path;
}

// Get the name of this task
const idStr& PathShowTask::GetName() const
{
	static idStr _name(TASK_PATH_SHOW);
	return _name;
}

void PathShowTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	idPathCorner* path = _path.GetEntity();

	if (path == NULL) {
		gameLocal.Error("PathShowTask: Path Entity not set before Init()");
	}
}

bool PathShowTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO).LogString("Path Show task performing.\r");

	idPathCorner* path = _path.GetEntity();
	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointers
	assert(path != NULL && owner != NULL);

	if (owner->CanBecomeSolid())
	{
		owner->Show();
	}

	if (!owner->IsHidden())
	{
		// Trigger path targets, now that we've reached the corner
		owner->ActivateTargets(owner);

		// Move is done, fall back to PatrolTask
		DM_LOG(LC_AI, LT_INFO).LogString("entity is visible.\r");

		return true; // finish this task
	}
	return false;
}

void PathShowTask::SetTargetEntity(idPathCorner* path) 
{
	assert(path);
	_path = path;
}

// Save/Restore methods
void PathShowTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	_path.Save(savefile);
}

void PathShowTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	_path.Restore(savefile);
}

PathShowTaskPtr PathShowTask::CreateInstance()
{
	return PathShowTaskPtr(new PathShowTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar pathShowTaskRegistrar(
	TASK_PATH_SHOW, // Task Name
	TaskLibrary::CreateInstanceFunc(&PathShowTask::CreateInstance) // Instance creation callback
);

} // namespace ai
