/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3572 $
 * $Date: 2009-07-24 01:52:30 -0400 (Fri, 24 Jul 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: GreetingBarkTask.cpp 3572 2009-07-24 05:52:30Z greebo $", init_version);

#include "GreetingBarkTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

GreetingBarkTask::GreetingBarkTask() :
	SingleBarkTask()
{}

GreetingBarkTask::GreetingBarkTask(const idStr& soundName, idActor* greetingTarget) :
	SingleBarkTask(soundName),
	_greetingTarget(greetingTarget)
{}

// Get the name of this task
const idStr& GreetingBarkTask::GetName() const
{
	static idStr _name(TASK_GREETING_BARK);
	return _name;
}

void GreetingBarkTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Set up the message for the other AI before propagating the Init() call

	// TODO

	_message = CommMessagePtr(new CommMessage(
		CommMessage::Greeting_CommType, 
		owner, _greetingTarget, // from this AI to the other
		NULL,
		owner->GetPhysics()->GetOrigin()
	));

	// Init the base class
	SingleBarkTask::Init(owner, subsystem);

	// This task may not be performed with empty entity pointers
	assert(owner != NULL);

}

bool GreetingBarkTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("GreetingBarkTask performing.\r");

	return false;
}

// Save/Restore methods
void GreetingBarkTask::Save(idSaveGame* savefile) const
{
	SingleBarkTask::Save(savefile);

	savefile->WriteObject(_greetingTarget);
}

void GreetingBarkTask::Restore(idRestoreGame* savefile)
{
	SingleBarkTask::Restore(savefile);

	savefile->ReadObject(reinterpret_cast<idClass*&>(_greetingTarget));
}

GreetingBarkTaskPtr GreetingBarkTask::CreateInstance()
{
	return GreetingBarkTaskPtr(new GreetingBarkTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar greetingBarkTaskRegistrar(
	TASK_GREETING_BARK, // Task Name
	TaskLibrary::CreateInstanceFunc(&GreetingBarkTask::CreateInstance) // Instance creation callback
);

} // namespace ai
