/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5121 $ (Revision of last commit) 
 $Date: 2011-12-11 14:12:26 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "../../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: PathTask.cpp 5121 2011-12-11 19:12:26Z greebo $", init_version);

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
