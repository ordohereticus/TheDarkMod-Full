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

static bool versioned = RegisterVersionedFile("$Id: WaitTask.cpp 5185 2012-01-08 05:59:48Z greebo $");

#include "../Memory.h"
#include "WaitTask.h"
#include "../Library.h"

namespace ai
{

WaitTask::WaitTask() :
	_waitTime(0)
{}

WaitTask::WaitTask(const int waitTime) : 
	_waitTime(waitTime)
{}

// Get the name of this task
const idStr& WaitTask::GetName() const
{
	static idStr _name(TASK_WAIT);
	return _name;
}

void WaitTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	_waitEndTime = gameLocal.time + _waitTime;
}

bool WaitTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("WaitTask performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointer
	assert(owner != NULL);

	// This task does nothing but wait until the time is over.
	 if (_waitEndTime <= gameLocal.time)
	{
		return true;
	}
		
	return false; // not finished yet
}

void WaitTask::SetTime(int waitTime)
{
	_waitTime = waitTime;
}

// Save/Restore methods
void WaitTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	savefile->WriteInt(_waitTime);
	savefile->WriteInt(_waitEndTime);
}

void WaitTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	savefile->ReadInt(_waitTime);
	savefile->ReadInt(_waitEndTime);
}

WaitTaskPtr WaitTask::CreateInstance()
{
	return WaitTaskPtr(new WaitTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar waitTaskRegistrar(
	TASK_WAIT, // Task Name
	TaskLibrary::CreateInstanceFunc(&WaitTask::CreateInstance) // Instance creation callback
);

} // namespace ai
