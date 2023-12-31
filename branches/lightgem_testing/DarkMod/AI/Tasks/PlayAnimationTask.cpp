/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2693 $
 * $Date: 2008-07-18 12:28:56 -0400 (Fri, 18 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: PlayAnimationTask.cpp 2693 2008-07-18 16:28:56Z greebo $", init_version);

#include "../Memory.h"
#include "PlayAnimationTask.h"
#include "../Library.h"

#define DEFAULT_BLEND_FRAMES 4

namespace ai
{

PlayAnimationTask::PlayAnimationTask() :
	_blendFrames(DEFAULT_BLEND_FRAMES),
	_playCycle(false)
{}

PlayAnimationTask::PlayAnimationTask(const idStr& animName, int blendFrames, bool playCycle) :
	_animName(animName),
	_blendFrames(blendFrames),
	_playCycle(playCycle)
{}

// Get the name of this task
const idStr& PlayAnimationTask::GetName() const
{
	static idStr _name(TASK_PLAY_ANIMATION);
	return _name;
}

void PlayAnimationTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	// Parse animation spawnargs here
	if (_animName.IsEmpty())
	{
		gameLocal.Warning("Cannot start PlayAnimationTask with empty animation name.\n");
		subsystem.FinishTask();
	}

	StartAnim(owner);
}

void PlayAnimationTask::OnFinish(idAI* owner)
{
	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle", _blendFrames);
	owner->SetWaitState("");
}

bool PlayAnimationTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("PlayAnimationTask performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with an empty owner pointer
	assert(owner != NULL);

	// Exit when the waitstate is not "customAnim" anymore
	idStr waitState(owner->WaitState());

	if (waitState != "customAnim")
	{
		// We're finished, what now?
		if (_playCycle)
		{
			// Starte the anim cycle again
			StartAnim(owner);
		}
		else
		{
			return true;
		}
	}

	return false;
}

void PlayAnimationTask::StartAnim(idAI* owner)
{
	// Synchronise the leg channel
	owner->Event_OverrideAnim(ANIMCHANNEL_LEGS);

	// Play the anim on the TORSO channel (will override the LEGS channel)
	owner->Event_PlayAnim(ANIMCHANNEL_TORSO, _animName);
		
	// Set the name of the state script
	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_CustomAnim", _blendFrames);
	owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_CustomAnim", _blendFrames);
	
	// greebo: Set the waitstate, this gets cleared by 
	// the script function when the animation is done.
	owner->SetWaitState("customAnim");
}

// Save/Restore methods
void PlayAnimationTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	savefile->WriteString(_animName);
	savefile->WriteInt(_blendFrames);
	savefile->WriteBool(_playCycle);
}

void PlayAnimationTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	savefile->ReadString(_animName);
	savefile->ReadInt(_blendFrames);
	savefile->ReadBool(_playCycle);
}

PlayAnimationTaskPtr PlayAnimationTask::CreateInstance()
{
	return PlayAnimationTaskPtr(new PlayAnimationTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar playAnimationTaskRegistrar(
	TASK_PLAY_ANIMATION, // Task Name
	TaskLibrary::CreateInstanceFunc(&PlayAnimationTask::CreateInstance) // Instance creation callback
);

} // namespace ai
