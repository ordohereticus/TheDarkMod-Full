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

static bool init_version = FileVersionList("$Id: CommWaitTask.cpp 5093 2011-12-06 17:34:56Z greebo $", init_version);

#include "CommWaitTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

CommWaitTask::CommWaitTask() :
	CommunicationTask(""),
	_duration(0),
	_endTime(-1)
{}

CommWaitTask::CommWaitTask(int duration, int priority) :
	CommunicationTask(""),
	_duration(duration),
	_endTime(-1)
{
	_priority = priority;
}

// Get the name of this task
const idStr& CommWaitTask::GetName() const
{
	static idStr _name(TASK_COMM_WAIT);
	return _name;
}

void CommWaitTask::Init(idAI* owner, Subsystem& subsystem)
{
	CommunicationTask::Init(owner, subsystem);

	_endTime = gameLocal.time + _duration;
}

bool CommWaitTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("CommWaitTask performing.\r");

	return (gameLocal.time >= _endTime);
}

// Save/Restore methods
void CommWaitTask::Save(idSaveGame* savefile) const
{
	CommunicationTask::Save(savefile);

	savefile->WriteInt(_duration);
	savefile->WriteInt(_endTime);
}

void CommWaitTask::Restore(idRestoreGame* savefile)
{
	CommunicationTask::Restore(savefile);

	savefile->ReadInt(_duration);
	savefile->ReadInt(_endTime);
}

CommWaitTaskPtr CommWaitTask::CreateInstance()
{
	return CommWaitTaskPtr(new CommWaitTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar commWaitTaskRegistrar(
	TASK_COMM_WAIT, // Task Name
	TaskLibrary::CreateInstanceFunc(&CommWaitTask::CreateInstance) // Instance creation callback
);

} // namespace ai
