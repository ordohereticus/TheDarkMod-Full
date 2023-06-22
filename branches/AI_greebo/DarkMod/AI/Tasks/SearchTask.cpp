/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 18:53:28 +0200 (Di, 16 Okt 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: SearchTask.cpp 1435 2007-10-16 16:53:28Z greebo $", init_version);

#include "SearchTask.h"
#include "../States/IdleState.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

// Get the name of this task
const idStr& SearchTask::GetName() const
{
	static idStr _name(TASK_SEARCH);
	return _name;
}

void SearchTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	// nothing so far, remove me (FIXME)
}

bool SearchTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO).LogString("SearchTask performing.\r");

	idAI* owner = _owner.GetEntity();
	
	// This task may not be performed with empty entity pointers
	assert(owner != NULL);

	// Get a shortcut reference
	Memory& memory = owner->GetMind()->GetMemory();

	// Let the mind check its senses (TRUE = process new stimuli)
	//owner->GetMind()->PerformSensoryScan(true);

	

	return false; // not finished yet
}

SearchTaskPtr SearchTask::CreateInstance()
{
	return SearchTaskPtr(new SearchTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar searchTaskRegistrar(
	TASK_SEARCH, // Task Name
	TaskLibrary::CreateInstanceFunc(&SearchTask::CreateInstance) // Instance creation callback
);

} // namespace ai
