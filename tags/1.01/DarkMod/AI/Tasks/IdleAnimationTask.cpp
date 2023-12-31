/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3693 $
 * $Date: 2009-09-05 10:43:56 -0400 (Sat, 05 Sep 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: IdleAnimationTask.cpp 3693 2009-09-05 14:43:56Z greebo $", init_version);

#include "IdleAnimationTask.h"
#include "../Memory.h"
#include "../Library.h"

#include <vector>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace ai
{

IdleAnimationTask::IdleAnimationTask() :
	_nextAnimationTime(-1),
	_idleAnimationInterval(-1),
	_lastIdleAnim(-1)
{}

// Get the name of this task
const idStr& IdleAnimationTask::GetName() const
{
	static idStr _name(TASK_IDLE_ANIMATION);
	return _name;
}

void IdleAnimationTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	Memory& memory = owner->GetMemory();

	// Read the animation set and interval from the owner's spawnarg
	_idleAnimationInterval = SEC2MS(owner->spawnArgs.GetInt("idle_animations_interval", "-1"));

	// Read the general-purpose animations first
	std::string animStringList(owner->spawnArgs.GetString("idle_animations", ""));

	std::vector<std::string> anims; // will hold the separated strings
	boost::algorithm::split(anims, animStringList, boost::algorithm::is_any_of(" ,"));

	// Copy the strings into the idList<idStr>
	for (std::size_t i = 0; i < anims.size(); i++)
	{
		if (!anims[i].empty())
		{
			_idleAnimations.Append(idStr(anims[i].c_str()));
		}
	}

	// Now read the anims for the torso only
	animStringList = owner->spawnArgs.GetString("idle_animations_torso", "");
	boost::algorithm::split(anims, animStringList, boost::algorithm::is_any_of(" ,"));

	// Copy the strings into the idList<idStr>
	for (std::size_t i = 0; i < anims.size(); i++)
	{
		if (!anims[i].empty())
		{
			_idleAnimationsTorso.Append(idStr(anims[i].c_str()));
		}
	}

	// Now read the anims for sitting AI
	animStringList = owner->spawnArgs.GetString("idle_animations_sitting", "");
	boost::algorithm::split(anims, animStringList, boost::algorithm::is_any_of(" ,"));

	// Copy the strings into the idList<idStr>
	for (std::size_t i = 0; i < anims.size(); i++)
	{
		if (!anims[i].empty())
		{
			_idleAnimationsSitting.Append(idStr(anims[i].c_str()));
		}
	}

	if (_idleAnimationInterval > 0 && (_idleAnimations.Num() > 0 || _idleAnimationsTorso.Num() || _idleAnimationsSitting.Num()))
	{
		_nextAnimationTime = static_cast<int>(gameLocal.time + gameLocal.random.RandomFloat()*_idleAnimationInterval);
	}
	else
	{
		// No idle animation interval set or no animations, finish this task
		subsystem.FinishTask();
	}
}

bool IdleAnimationTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("IdleAnimationTask performing.\r");

	idAI* owner = _owner.GetEntity();

	// This task may not be performed with empty entity pointers
	assert(owner != NULL);

	Memory& memory = owner->GetMemory();

	if (gameLocal.time > _nextAnimationTime)
	{
		// angua: don't play idle animations while sitting / lying down or getting up
		// TODO: Disable the playIdleAnimation flag rather than catch all those cases
		if (memory.playIdleAnimations && 
			!owner->AI_RUN &&
			owner->GetMoveType() != MOVETYPE_SIT_DOWN &&
			owner->GetMoveType() != MOVETYPE_LAY_DOWN &&
			owner->GetMoveType() != MOVETYPE_SLEEP &&
			owner->GetMoveType() != MOVETYPE_GET_UP &&
			owner->GetMoveType() != MOVETYPE_GET_UP_FROM_LYING)
		{
			// Check if the AI is moving or sitting, this determines which channel we can play on
			if (!owner->AI_FORWARD && owner->GetMoveType() != MOVETYPE_SIT)
			{
				// AI is not walking or sitting, play animations affecting all channels
				int animIdx = gameLocal.random.RandomInt(_idleAnimations.Num());

				// If we have more than one anim, don't play the same anim twice
				while (animIdx == _lastIdleAnim && _idleAnimations.Num() > 1)
				{
					animIdx = gameLocal.random.RandomInt(_idleAnimations.Num());
				}

				_lastIdleAnim = animIdx;

				const idStr& animName = _idleAnimations[animIdx];

				// Check if the animation exists
				if (owner->GetAnim(ANIMCHANNEL_TORSO, animName) == 0 || 
					owner->GetAnim(ANIMCHANNEL_LEGS, animName) == 0)
				{
					gameLocal.Warning("Could not find anim %s on entity %s", animName.c_str(), owner->name.c_str());
					DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Could not find anim %s on entity %s\r", animName.c_str(), owner->name.c_str());
					return true; // done with errors
				}

				// Issue the playanim call
				owner->SetNextIdleAnim(animName);

				owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_CustomIdleAnim", 4);
				owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_CustomIdleAnim", 4);
			}
			else if (owner->GetMoveType() == MOVETYPE_SIT)
			{
				// AI is sitting, only use sitting animations on torso channel
				int animIdx = gameLocal.random.RandomInt(_idleAnimationsSitting.Num());

				// If we have more than one anim, don't play the same anim twice
				while (animIdx == _lastIdleAnim && _idleAnimationsSitting.Num() > 1)
				{
					animIdx = gameLocal.random.RandomInt(_idleAnimationsSitting.Num());
				}

				_lastIdleAnim = animIdx;

				const idStr& animName = _idleAnimationsSitting[animIdx];

				// Check if the animation exists
				if (owner->GetAnim(ANIMCHANNEL_TORSO, animName) == 0)			
				{
					gameLocal.Warning("Could not find anim %s on entity %s for channel TORSO", animName.c_str(), owner->name.c_str());
					DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Could not find anim %s on entity %s for channel TORSO\r", animName.c_str(), owner->name.c_str());
					return true; // done with errors
				}

				owner->SetNextIdleAnim(animName);
				owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_CustomIdleAnim", 4);

			}
			else
			{
				// AI is walking, only use animations for the Torso channel
				int animIdx = gameLocal.random.RandomInt(_idleAnimationsTorso.Num());

				// If we have more than one anim, don't play the same anim twice
				while (animIdx == _lastIdleAnim && _idleAnimationsTorso.Num() > 1)
				{
					animIdx = gameLocal.random.RandomInt(_idleAnimationsTorso.Num());
				}

				_lastIdleAnim = animIdx;

				const idStr& animName = _idleAnimationsTorso[animIdx];

				// Check if the animation exists
				if (owner->GetAnim(ANIMCHANNEL_TORSO, animName) == 0)			
				{
					gameLocal.Warning("Could not find anim %s on entity %s for channel TORSO", animName.c_str(), owner->name.c_str());
					DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Could not find anim %s on entity %s for channel TORSO\r", animName.c_str(), owner->name.c_str());
					return true; // done with errors
				}

				owner->SetNextIdleAnim(animName);
				owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_CustomIdleAnim", 4);
			}
		}
		
		// Reset the timer
		_nextAnimationTime = static_cast<int>(
			gameLocal.time + _idleAnimationInterval*(0.8f + gameLocal.random.RandomFloat()*0.4f)
		);
	}

	return false; // not finished yet
}

void IdleAnimationTask::OnFinish(idAI* owner)
{
	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle", 5);
	owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle", 5);
	owner->SetWaitState("");
}

// Save/Restore methods
void IdleAnimationTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);
	savefile->WriteInt(_nextAnimationTime);
	savefile->WriteInt(_idleAnimationInterval);

	savefile->WriteInt(_idleAnimations.Num());
	for (int i = 0; i < _idleAnimations.Num(); i++)
	{
		savefile->WriteString(_idleAnimations[i].c_str());
	}

	savefile->WriteInt(_idleAnimationsTorso.Num());
	for (int i = 0; i < _idleAnimationsTorso.Num(); i++)
	{
		savefile->WriteString(_idleAnimationsTorso[i].c_str());
	}

	savefile->WriteInt(_idleAnimationsSitting.Num());
	for (int i = 0; i < _idleAnimationsSitting.Num(); i++)
	{
		savefile->WriteString(_idleAnimationsSitting[i].c_str());
	}


	savefile->WriteInt(_lastIdleAnim);
}

void IdleAnimationTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);
	savefile->ReadInt(_nextAnimationTime);
	savefile->ReadInt(_idleAnimationInterval);

	int num;
	savefile->ReadInt(num);
	_idleAnimations.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		savefile->ReadString(_idleAnimations[i]);
	}

	savefile->ReadInt(num);
	_idleAnimationsTorso.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		savefile->ReadString(_idleAnimationsTorso[i]);
	}

	savefile->ReadInt(num);
	_idleAnimationsSitting.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		savefile->ReadString(_idleAnimationsSitting[i]);
	}


	savefile->ReadInt(_lastIdleAnim);
}

IdleAnimationTaskPtr IdleAnimationTask::CreateInstance()
{
	return IdleAnimationTaskPtr(new IdleAnimationTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar idleAnimationTaskRegistrar(
	TASK_IDLE_ANIMATION, // Task Name
	TaskLibrary::CreateInstanceFunc(&IdleAnimationTask::CreateInstance) // Instance creation callback
);

} // namespace ai
