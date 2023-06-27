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

static bool init_version = FileVersionList("$Id: PathSleepTask.cpp 5093 2011-12-06 17:34:56Z greebo $", init_version);

#include "../Memory.h"
#include "PathSleepTask.h"
#include "PathTurnTask.h"
#include "WaitTask.h"
#include "RepeatedBarkTask.h"
#include "../States/IdleSleepState.h"
#include "../Library.h"

namespace ai
{

PathSleepTask::PathSleepTask() :
	PathTask()
{}

PathSleepTask::PathSleepTask(idPathCorner* path) :
	PathTask(path)
{
	_path = path;
}

// Get the name of this task
const idStr& PathSleepTask::GetName() const
{
	static idStr _name(TASK_PATH_SLEEP);
	return _name;
}

void PathSleepTask::Init(idAI* owner, Subsystem& subsystem)
{
	PathTask::Init(owner, subsystem);

	if(_path.GetEntity()->spawnArgs.GetBool("lay_down_left", "1"))
	{
		owner->AI_LAY_DOWN_LEFT = true;
	}
	else
	{
		owner->AI_LAY_DOWN_LEFT = false;
	}

	if (owner->GetMoveType() == MOVETYPE_ANIM)
	{
		owner->LayDown();
	}
}

bool PathSleepTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("PathSleepTask performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with an empty owner pointer
	assert(owner != NULL);

	if (owner->GetMoveType() == MOVETYPE_SLEEP)
	{
		return true;
	}
	return false;
}

PathSleepTaskPtr PathSleepTask::CreateInstance()
{
	return PathSleepTaskPtr(new PathSleepTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar pathSleepTaskRegistrar(
	TASK_PATH_SLEEP, // Task Name
	TaskLibrary::CreateInstanceFunc(&PathSleepTask::CreateInstance) // Instance creation callback
);

} // namespace ai
