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

static bool versioned = RegisterVersionedFile("$Id: MoveToCoverTask.cpp 5185 2012-01-08 05:59:48Z greebo $");

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
