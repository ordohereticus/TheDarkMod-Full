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

static bool init_version = FileVersionList("$Id: RepeatedBarkTask.cpp 2643 2008-07-13 06:24:10Z greebo $", init_version);

#include "RepeatedBarkTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

// Default constructor
RepeatedBarkTask::RepeatedBarkTask() :
	_soundName(""), 
	_barkRepeatIntervalMin(0), 
	_barkRepeatIntervalMax(0)
{}

RepeatedBarkTask::RepeatedBarkTask(const idStr& soundName, 
		int barkRepeatIntervalMin, int barkRepeatIntervalMax, 
		const CommMessagePtr& message) : 
	_soundName(soundName), 
	_barkRepeatIntervalMin(barkRepeatIntervalMin), 
	_barkRepeatIntervalMax(barkRepeatIntervalMax),
	_message(message)
{}

// Get the name of this task
const idStr& RepeatedBarkTask::GetName() const
{
	static idStr _name(TASK_REPEATED_BARK);
	return _name;
}

void RepeatedBarkTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	// Initialise it to play the sound now
	_nextBarkTime = gameLocal.time;
	// greebo: Add some random offset of up to <intervalMax> seconds before barking the first time
	// This prevents guards barking in choirs.
	_nextBarkTime += static_cast<int>(gameLocal.random.RandomFloat()*_barkRepeatIntervalMax);
}

bool RepeatedBarkTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("RepeatedBarkTask performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointers
	assert(owner != NULL);

	if (gameLocal.time >= _nextBarkTime)
	{
		// The time has come, bark now

		// Setup the message to be propagated, if we have one
		if (_message != NULL)
		{
			owner->AddMessage(_message);
		}

		int duration = owner->PlayAndLipSync(_soundName, "talk1");

		// Reset the timer
		if (_barkRepeatIntervalMax > 0)
		{
			_nextBarkTime = static_cast<int>(gameLocal.time + duration + _barkRepeatIntervalMin + 
				gameLocal.random.RandomFloat() * (_barkRepeatIntervalMax - _barkRepeatIntervalMin));
		}
		else
		{
			_nextBarkTime = gameLocal.time + duration + _barkRepeatIntervalMin;
		}
	}

	return false; // not finished yet
}

// Save/Restore methods
void RepeatedBarkTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);
	savefile->WriteString(_soundName);
	savefile->WriteInt(_barkRepeatIntervalMin);
	savefile->WriteInt(_barkRepeatIntervalMax);
	savefile->WriteInt(_nextBarkTime);

	savefile->WriteBool(_message != NULL);
	if (_message != NULL)
	{
		_message->Save(savefile);
	}
}

void RepeatedBarkTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);
	savefile->ReadString(_soundName);
	savefile->ReadInt(_barkRepeatIntervalMin);
	savefile->ReadInt(_barkRepeatIntervalMax);
	savefile->ReadInt(_nextBarkTime);

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

RepeatedBarkTaskPtr RepeatedBarkTask::CreateInstance()
{
	return RepeatedBarkTaskPtr(new RepeatedBarkTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar repeatedBarkTaskRegistrar(
	TASK_REPEATED_BARK, // Task Name
	TaskLibrary::CreateInstanceFunc(&RepeatedBarkTask::CreateInstance) // Instance creation callback
);

} // namespace ai
