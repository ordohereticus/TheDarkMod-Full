/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5185 $ (Revision of last commit) 
 $Date: 2012-01-08 00:59:48 -0500 (Sun, 08 Jan 2012) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: PathTurnTask.cpp 5185 2012-01-08 05:59:48Z greebo $");

#include "../Memory.h"
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
