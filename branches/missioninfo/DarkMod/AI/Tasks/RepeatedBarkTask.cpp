/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3427 $
 * $Date: 2009-05-07 13:02:16 -0400 (Thu, 07 May 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: RepeatedBarkTask.cpp 3427 2009-05-07 17:02:16Z greebo $", init_version);

#include "RepeatedBarkTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

// Default constructor
RepeatedBarkTask::RepeatedBarkTask() :
	CommunicationTask(""),
	_barkRepeatIntervalMin(0), 
	_barkRepeatIntervalMax(0)
{}

RepeatedBarkTask::RepeatedBarkTask(const idStr& soundName, 
		int barkRepeatIntervalMin, int barkRepeatIntervalMax, 
		const CommMessagePtr& message) : 
	CommunicationTask(soundName),
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
	// Init the base class
	CommunicationTask::Init(owner, subsystem);

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

		_barkLength = owner->PlayAndLipSync(_soundName, "talk1");

		_barkStartTime = gameLocal.time;

		// Reset the timer
		if (_barkRepeatIntervalMax > 0)
		{
			_nextBarkTime = static_cast<int>(_barkStartTime + _barkLength + _barkRepeatIntervalMin + 
				gameLocal.random.RandomFloat() * (_barkRepeatIntervalMax - _barkRepeatIntervalMin));
		}
		else
		{
			_nextBarkTime = _barkStartTime + _barkLength + _barkRepeatIntervalMin;
		}
	}

	return false; // not finished yet
}

// Save/Restore methods
void RepeatedBarkTask::Save(idSaveGame* savefile) const
{
	CommunicationTask::Save(savefile);

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
	CommunicationTask::Restore(savefile);

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
