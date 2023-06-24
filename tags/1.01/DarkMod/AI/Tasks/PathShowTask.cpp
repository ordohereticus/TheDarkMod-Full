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

static bool init_version = FileVersionList("$Id: PathShowTask.cpp 3767 2009-12-01 05:17:07Z angua $", init_version);

#include "../Memory.h"
#include "PatrolTask.h"
#include "PathShowTask.h"
#include "../Library.h"

namespace ai
{

PathShowTask::PathShowTask() :
	PathTask()
{}

PathShowTask::PathShowTask(idPathCorner* path) :
	PathTask(path)
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
	PathTask::Init(owner, subsystem);
}

bool PathShowTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("Path Show task performing.\r");

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

		// NextPath();

		// Move is done, fall back to PatrolTask
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("entity is visible.\r");

		return true; // finish this task
	}
	return false;
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
