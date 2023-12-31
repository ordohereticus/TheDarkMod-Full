/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2643 $
 * $Date: 2008-07-13 02:24:10 -0400 (Sun, 13 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: SingleBarkTask.cpp 2643 2008-07-13 06:24:10Z greebo $", init_version);

#include "SingleBarkTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

SingleBarkTask::SingleBarkTask() :
	_soundName("")
{}

SingleBarkTask::SingleBarkTask(const idStr& soundName, const CommMessagePtr& message) :
	_soundName(soundName),
	_message(message)
{}

// Get the name of this task
const idStr& SingleBarkTask::GetName() const
{
	static idStr _name(TASK_SINGLE_BARK);
	return _name;
}

void SingleBarkTask::Init(idAI* owner, Subsystem& subsystem)
{
	// This task may not be performed with empty entity pointers
	assert(owner != NULL);

	if (!_soundName.IsEmpty())
	{
		// Push the message and play the sound
		if (_message != NULL)
		{
			owner->AddMessage(_message);
		}

		int duration = owner->PlayAndLipSync(_soundName, "talk1");
		_endTime = gameLocal.time + duration;
	}
	else
	{
		_endTime = gameLocal.time;
	}

}

bool SingleBarkTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("SingleBarkTask performing.\r");

	idAI* owner = _owner.GetEntity();

	if (gameLocal.time >= _endTime)
	{
		return true; // finished!
	}
	return false;
}

void SingleBarkTask::SetSound(const idStr& soundName)
{
	_soundName = soundName;
}

// Save/Restore methods
void SingleBarkTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);
	savefile->WriteString(_soundName);
	savefile->WriteInt(_endTime);

	savefile->WriteBool(_message != NULL);
	if (_message != NULL)
	{
		_message->Save(savefile);
	}
}

void SingleBarkTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);
	savefile->ReadString(_soundName);
	savefile->ReadInt(_endTime);

	bool hasMessage;
	savefile->ReadBool(hasMessage);
	if (hasMessage)
	{
		_message = CommMessagePtr(new CommMessage);
		_message->Restore(savefile);
	}
	else
	{
		_message = CommMessagePtr();
	}
}

SingleBarkTaskPtr SingleBarkTask::CreateInstance()
{
	return SingleBarkTaskPtr(new SingleBarkTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar singleBarkTaskRegistrar(
	TASK_SINGLE_BARK, // Task Name
	TaskLibrary::CreateInstanceFunc(&SingleBarkTask::CreateInstance) // Instance creation callback
);

} // namespace ai
