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

static bool init_version = FileVersionList("$Id: PathTask.cpp 4862 2011-05-22 12:41:11Z greebo $", init_version);

#include "../Memory.h"
#include "PathTask.h"
#include "../Library.h"

namespace ai
{

PathTask::PathTask()
{}

PathTask::PathTask(idPathCorner* path)
{
	_path = path;
}

void PathTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	if (_path.GetEntity() == NULL)
	{
		gameLocal.Error("Path Entity not set before Init()");
	}

	idPathCorner* path = _path.GetEntity();

	_accuracy = path->spawnArgs.GetFloat("move_to_position_tolerance", "-1");
}


void PathTask::SetTargetEntity(idPathCorner* path) 
{
	assert(path);
	_path = path;
}

// Save/Restore methods
void PathTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	_path.Save(savefile);
	savefile->WriteFloat(_accuracy);
}

void PathTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	_path.Restore(savefile);
	savefile->ReadFloat(_accuracy);
}


} // namespace ai
