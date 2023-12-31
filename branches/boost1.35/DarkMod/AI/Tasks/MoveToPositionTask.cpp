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

static bool init_version = FileVersionList("$Id: MoveToPositionTask.cpp 2443 2008-06-07 13:48:49Z angua $", init_version);

#include "../Memory.h"
#include "MoveToPositionTask.h"
#include "../Library.h"

namespace ai
{

// This should be unreachable if no target position is specified.
MoveToPositionTask::MoveToPositionTask() :
	_targetPosition(idMath::INFINITY, idMath::INFINITY, idMath::INFINITY),
	_targetYaw(idMath::INFINITY)
{}

MoveToPositionTask::MoveToPositionTask(const idVec3 targetPosition, float targetYaw) :
	_targetPosition(targetPosition),
	_targetYaw(targetYaw)
{}


// Get the name of this task
const idStr& MoveToPositionTask::GetName() const
{
	static idStr _name(TASK_MOVE_TO_POSITION);
	return _name;
}

void MoveToPositionTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);
}

bool MoveToPositionTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("run to position Task performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointer
	assert(owner != NULL);

	if (!owner->MoveToPosition(_targetPosition))
	{
		// Destination unreachable, end task
		return true;
	}
		
	if (owner->AI_MOVE_DONE)
	{
		// Position reached, turn to the given yaw, if valid
		if (_targetYaw != idMath::INFINITY)
		{
			owner->TurnToward(_targetYaw);
		}
		return true;
	}
		 
	return false; // not finished yet
}

void MoveToPositionTask::SetPosition(idVec3 position)
{
	_targetPosition = position;
}

// Save/Restore methods
void MoveToPositionTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	savefile->WriteVec3(_targetPosition);
	savefile->WriteFloat(_targetYaw);
}

void MoveToPositionTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	savefile->ReadVec3(_targetPosition);
	savefile->ReadFloat(_targetYaw);
}

MoveToPositionTaskPtr MoveToPositionTask::CreateInstance()
{
	return MoveToPositionTaskPtr(new MoveToPositionTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar moveToPositionTaskRegistrar(
	TASK_MOVE_TO_POSITION, // Task Name
	TaskLibrary::CreateInstanceFunc(&MoveToPositionTask::CreateInstance) // Instance creation callback
);

} // namespace ai
