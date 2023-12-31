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

static bool versioned = RegisterVersionedFile("$Id: Mind.cpp 5185 2012-01-08 05:59:48Z greebo $");

#include "Mind.h"
#include "States/IdleState.h"
#include "Tasks/SingleBarkTask.h"
#include "States/CombatState.h"
#include "Library.h"
#include "../AbsenceMarker.h"
#include "../AIComm_Message.h"

namespace ai
{

// This is the default state
#define STATE_DEFAULT STATE_IDLE

Mind::Mind(idAI* owner) :
	_memory(owner),
	_subsystemIterator(SubsystemCount),
	_switchState(false)
{
	// Set the idEntityPtr
	_owner = owner;
}

void Mind::Think()
{
	// Clear the recyclebin, it might hold a finished state from the last frame
	_recycleBin = StatePtr();

	// greebo: We do not check for NULL pointers in the owner at this point, 
	// as this method is called by the owner itself, it _has_ to exist.
	idAI* owner = _owner.GetEntity();
	assert(owner != NULL);

	if (_stateQueue.empty())
	{
		// We start with the idle state
		PushState(owner->backboneStates[ai::ERelaxed]);
	}

	// At this point, we MUST have a State
	assert(_stateQueue.size() > 0);

	const StatePtr& state = _stateQueue.front();

	// Thinking
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("Mind is thinking... %s\r", owner->name.c_str());


	// Should we switch states (i.e. initialise a new one)?
	if (_switchState)
	{
		// Clear the flag
		_switchState = false;

		// Initialise the state, this will put the Subsystem Tasks in place
		state->Init(owner);
	}

	if (!_switchState)
	{
		// Let the State do its monitoring task
		state->Think(owner);
	}

	// Try to perform the subsystem tasks, skipping inactive subsystems
	// Maximum number of tries is SubsystemCount.
	for (int i = 0; i < static_cast<int>(SubsystemCount); i++)
	{
		// Increase the iterator and wrap around, if necessary
		_subsystemIterator = static_cast<SubsystemId>(
			(static_cast<int>(_subsystemIterator) + 1) % static_cast<int>(SubsystemCount)
		);

		// Subsystems return TRUE when their task was executed
		if (owner->GetSubsystem(_subsystemIterator)->PerformTask())
		{
			// Task performed, break, iterator will be increased next round
			break;
		}
	}
}

void Mind::PushState(const idStr& stateName)
{
	// Get a new state with the given name
	StatePtr newState = StateLibrary::Instance().CreateInstance(stateName.c_str());

	if (newState != NULL)
	{
		PushState(newState);
	}
	else
	{
		gameLocal.Error("Mind: Could not push state %s", stateName.c_str());
	}
}

void Mind::PushState(const StatePtr& state)
{
	assert(state != NULL);

	// Push the state to the front of the queue
	_stateQueue.push_front(state);
	state->SetOwner(_owner.GetEntity());

	// Trigger a stateswitch next round
	_switchState = true;
}

bool Mind::EndState()
{
	if (!_stateQueue.empty())
	{
		// Don't destroy the State object this round
		_recycleBin = _stateQueue.front();

		DM_LOG(LC_AI, LT_INFO)LOGSTRING("Ending State %s (%s)\r", _recycleBin->GetName().c_str(), _owner.GetEntity()->name.c_str());

		// Remove the current state from the queue
		_stateQueue.pop_front();

		// Trigger a stateswitch next round in any case
		_switchState = true;
	}

	if (_stateQueue.empty())
	{
		// No states left, add the default state at least
		PushState(STATE_DEFAULT);
	}
	
	// Return TRUE if there are additional states left
	return true;
}

void Mind::SwitchState(const idStr& stateName)
{
	if (_stateQueue.size() > 0)
	{
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("Switching to %s to %s (%s)\r", stateName.c_str(), _stateQueue.front()->GetName().c_str(), _owner.GetEntity()->name.c_str());
	}
	else
	{
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("Switching to %s (%s)\r", stateName.c_str(), _owner.GetEntity()->name.c_str());
	}

	// greebo: Switch the state without destroying the current State object immediately
	if (_stateQueue.size() > 0)
	{
		// Store the shared_ptr in the temporary container
		_recycleBin = _stateQueue.front();
		// Remove the first element from the queue
		_stateQueue.pop_front();
	}

	// Add the new task
	PushState(stateName);
}

void Mind::SwitchState(const StatePtr& state)
{
	// greebo: Switch the state without destroying the current State object immediately
	if (_stateQueue.size() > 0)
	{
		// Store the shared_ptr in the temporary container
		_recycleBin = _stateQueue.front();
		// Remove the first element from the queue
		_stateQueue.pop_front();
	}

	// Add the new task
	PushState(state);
}


void Mind::ClearStates()
{
	// grayman #2603 - before clearing the states, check if the AI was relighting
	// a light. That light has to be marked as no longer being relit.

	idAI* owner = _owner.GetEntity();
	assert(owner);
	if (owner->m_RelightingLight)
	{
		Memory& memory = owner->GetMemory();
		idLight* light = memory.relightLight.GetEntity();
		if (light)
		{
			light->SetBeingRelit(false); // this light is no longer being relit
		}
	}

	_switchState = true;
	_stateQueue.clear();
}

bool Mind::SetTarget()
{
	// greebo: Ported from ai_darkmod_base::setTarget() written by SZ
	idAI* owner = _owner.GetEntity();
	assert(owner);

	idActor* target = NULL;

	// NOTE: To work properly, the priority here must be: check tactile first, then sight.
	
	// Done if we already have a target
	if (owner->GetEnemy() != NULL)
	{
		// gameLocal.Printf ("%s: Target already assigned, using that one.", owner->GetName() );
		return true;
	}

	// If the AI touched you, you're a target
	if (owner->AI_TACTALERT)
	{
		idEntity* tactEnt = owner->GetTactEnt();

		if (tactEnt == NULL || !tactEnt->IsType(idActor::Type)) 
		{
			// Invalid enemy type, todo?
			//DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Tactile entity is of wrong type: %s\r", tactEnt->name.c_str());
			return false;
		}

		target = static_cast<idActor*>(tactEnt);
		
		/** 
		* If the entity that bumped the AI is an inanimate object, isEnemy will return 0,
		* so the AI will not try to attack an inanimate object.
		**/
		if (owner->IsEnemy(target))
		{
			DM_LOG(LC_AI, LT_INFO)LOGSTRING("Set tactile alert enemy to entity %s\r", target->name.c_str());

			// set the bool back
			owner->AI_TACTALERT = false;
			owner->GetMemory().lastTimeEnemySeen = gameLocal.time;

			// Return TRUE if the enemy is valid
			return owner->SetEnemy(target);
		}
		else
		{
			// They bumped into a non entity, so they should ignore it and not set an
			// alert from it.
			// set the bool back (man, this is annoying)
			owner->AI_TACTALERT = false;
			return false;
		}
	}
	// If the AI saw you, you're a target
	else if (owner->AI_VISALERT)
	{	
		target = owner->FindEnemy(false);

		if (target != NULL)
		{
			// Try to set the enemy, returns TRUE if valid
			return owner->SetEnemy(target);
		}
		else
		{
			target = owner->FindEnemyAI(false);

			if (target != NULL)
			{
				// Try to set the enemy, returns TRUE if valid
				return owner->SetEnemy(target);
			}
		}
		
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("No target\r");
		
		return false;
	}
	/*
	* Sound is the only thing that does not guarantee combat
	* The AI will just stay in the highest alert state and
	* run at the sound location until it bumps into something
	* (No cheating here!)
	*/
	else if (owner->AI_HEARDSOUND)
	{
		// do not set HEARDSOUND to false here because we still want to use it after exit
		return false;
	}

	// something weird must have happened
	return false;
}

bool Mind::PerformCombatCheck()
{
	idAI* owner = _owner.GetEntity();
	assert(owner);

	Memory& memory = owner->GetMemory();

	// Check for an enemy, if this returns TRUE, we have an enemy
	bool targetFound = SetTarget();
	
	if (targetFound)
	{
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("COMBAT NOW!\r");
		
		// Spotted an enemy
		memory.enemiesHaveBeenSeen = true;

		memory.alertType = EAlertTypeEnemy;
		
		idActor* enemy = owner->GetEnemy();

		memory.lastEnemyPos = enemy->GetPhysics()->GetOrigin();
		memory.posEnemySeen = owner->GetPhysics()->GetOrigin();	// grayman #2903
		memory.timeEnemySeen = gameLocal.time;	// grayman #2903
		
		return true; // entered combat mode
	}

	// If we got here there is no target

	return false; // combat mode not justified
}

void Mind::Save(idSaveGame* savefile) const 
{
	_owner.Save(savefile);
	_stateQueue.Save(savefile);
	_memory.Save(savefile);
}

void Mind::Restore(idRestoreGame* savefile) 
{
	_owner.Restore(savefile);
	_stateQueue.Restore(savefile);
	_memory.Restore(savefile);
}

} // namespace ai
