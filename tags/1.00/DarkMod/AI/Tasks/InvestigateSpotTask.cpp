/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3426 $
 * $Date: 2009-05-07 12:44:17 -0400 (Thu, 07 May 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: InvestigateSpotTask.cpp 3426 2009-05-07 16:44:17Z greebo $", init_version);

#include "InvestigateSpotTask.h"
#include "WaitTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

const int INVESTIGATE_SPOT_TIME_REMOTE = 600; // ms
const int INVESTIGATE_SPOT_TIME_STANDARD = 300; // ms
const int INVESTIGATE_SPOT_TIME_CLOSELY = 2500; // ms

const float MAX_TRAVEL_DISTANCE_WALKING = 300; // units?

InvestigateSpotTask::InvestigateSpotTask() :
	_investigateClosely(false),
	_moveInitiated(false)
{}

InvestigateSpotTask::InvestigateSpotTask(bool investigateClosely) :
	_investigateClosely(investigateClosely),
	_moveInitiated(false)
{}

// Get the name of this task
const idStr& InvestigateSpotTask::GetName() const
{
	static idStr _name(TASK_INVESTIGATE_SPOT);
	return _name;
}

void InvestigateSpotTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Just init the base class
	Task::Init(owner, subsystem);

	// Get a shortcut reference
	Memory& memory = owner->GetMemory();

	// Stop previous moves
	//owner->StopMove(MOVE_STATUS_DONE);

	if (memory.currentSearchSpot != idVec3(idMath::INFINITY, idMath::INFINITY, idMath::INFINITY))
	{
		// Set the goal position
		SetNewGoal(memory.currentSearchSpot);
	}
	else
	{
		// Invalid hiding spot, terminate task
		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("memory.currentSearchSpot not set to something valid, terminating task.\r");
		subsystem.FinishTask();
	}
}

bool InvestigateSpotTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("InvestigateSpotTask performing.\r");

	idAI* owner = _owner.GetEntity();
	assert(owner != NULL);

	if (_exitTime > 0)
	{
		// Return TRUE if the time is over, else FALSE (continue)
		return (gameLocal.time > _exitTime);
	}
	
	// No exit time set, continue with ordinary process

	if (owner->m_HandlingDoor || owner->m_HandlingElevator)
	{
		// Wait, we're busy with a door or elevator
		return false;
	}

	if (!_moveInitiated)
	{
		idVec3 destPos = _searchSpot;

		// greebo: For close investigation, don't step up to the very spot, to prevent the AI
		// from kneeling into bloodspots or corpses
		if (_investigateClosely)
		{
			idVec3 direction = owner->GetPhysics()->GetOrigin() - _searchSpot;
			direction.NormalizeFast();

			// 20 units before the actual spot
			destPos += direction * 20;
		}

		// Let's move
		owner->MoveToPosition(destPos);
		_moveInitiated = true;

		if (owner->GetMoveStatus() == MOVE_STATUS_DEST_UNREACHABLE)
		{
			// Hiding spot not reachable, terminate task in the next round
			DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("_searchSpot not reachable, terminating task.\r");
			_exitTime = gameLocal.time;
		}
		else
		{
			// Run if the point is more than 500 
			// greebo: This is taxing and can be replaced by a simpler distance check 
			// TravelDistance takes about ~0.1 msec on my 2.2 GHz system.
			float travelDist = owner->TravelDistance(owner->GetPhysics()->GetOrigin(), _searchSpot);

			DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("TravelDistance is %f.\r", travelDist);
			owner->AI_RUN = (travelDist > MAX_TRAVEL_DISTANCE_WALKING);
		}

		return false;
	}

	if (owner->GetMoveStatus() == MOVE_STATUS_DEST_UNREACHABLE)
	{
		DM_LOG(LC_AI, LT_INFO)LOGVECTOR("Hiding spot unreachable.\r", _searchSpot);
		return true;
	}
	else if (owner->GetMoveStatus() == MOVE_STATUS_DONE)
	{
		DM_LOG(LC_AI, LT_INFO)LOGVECTOR("Hiding spot investigated: \r", _searchSpot);

		if (_investigateClosely)
		{
			// Stop previous moves
			owner->StopMove(MOVE_STATUS_WAITING);

			// Check the position of the stim, is it closer to the eyes than to the feet?
			// If it's lower than the eye position, kneel down and investigate
			const idVec3& origin = owner->GetPhysics()->GetOrigin();
			idVec3 eyePos = owner->GetEyePosition();
			if ((_searchSpot - origin).LengthSqr() < (_searchSpot - eyePos).LengthSqr())
			{
				// Close to the feet, kneel down and investigate closely
				owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_KneelDown", 6);
				owner->SetAnimState(ANIMCHANNEL_LEGS, "Legs_KneelDown", 6);
			}

			// Wait a bit, setting _exitTime sets the lifetime of this task
			_exitTime = static_cast<int>(
				gameLocal.time + INVESTIGATE_SPOT_TIME_CLOSELY*(1 + gameLocal.random.RandomFloat()*0.2f)
			);
		}
		else
		{
			// Wait a bit, setting _exitTime sets the lifetime of this task
			_exitTime = static_cast<int>(
				gameLocal.time + INVESTIGATE_SPOT_TIME_STANDARD*(1 + gameLocal.random.RandomFloat()*0.2f)
			);
		}
	}
	else
	{
		// Can we already see the point? Only stop moving when the spot shouldn't be investigated closely
		// angua: added distance check to avoid running in circles if the point is too close to a wall.
		if (!_investigateClosely && 
			( owner->CanSeePositionExt(_searchSpot, true, true) 
			|| (_searchSpot - owner->GetPhysics()->GetOrigin()).LengthFast() < 20 ))
		{
			DM_LOG(LC_AI, LT_INFO)LOGVECTOR("Stop, I can see the point now...\r", _searchSpot);

			// Stop moving, we can see the point
			owner->StopMove(MOVE_STATUS_DONE);

			//Look at the point to investigate
			owner->Event_LookAtPosition(_searchSpot, 2.0f);

			// Wait about half a sec., this sets the lifetime of this task
			_exitTime = static_cast<int>(
				gameLocal.time + 600*(1 + gameLocal.random.RandomFloat()*0.2f)
			);
		}
	}

	return false; // not finished yet
}

void InvestigateSpotTask::SetNewGoal(const idVec3& newPos)
{
	idAI* owner = _owner.GetEntity();
	if (owner == NULL)
	{
		return;
	}

	// Copy the value
	_searchSpot = newPos;
	// Reset the "move started" flag
	_moveInitiated = false;

	// Set the exit time back to negative default, so that the AI starts walking again
	_exitTime = -1;

	// Check if we can see the point from where we are (only for remote inspection)
	if (!_investigateClosely && owner->CanSeePositionExt(_searchSpot, false, true))
	{
		DM_LOG(LC_AI, LT_INFO)LOGVECTOR("I can see the point...\r", _searchSpot);

		if (!owner->CheckFOV(_searchSpot))
		{
			// Search spot is not within FOV, turn towards the position
			owner->TurnToward(_searchSpot);
		}

		// In any case, look at the point to investigate
		owner->Event_LookAtPosition(_searchSpot, 2.0f);

		// Wait about half a sec.
		_exitTime = static_cast<int>(
			gameLocal.time + INVESTIGATE_SPOT_TIME_REMOTE*(1 + gameLocal.random.RandomFloat()*0.2f)
		);
	}
}

void InvestigateSpotTask::SetInvestigateClosely(bool closely)
{
	_investigateClosely = closely;
}

void InvestigateSpotTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	savefile->WriteInt(_exitTime);
	savefile->WriteBool(_investigateClosely);
	savefile->WriteVec3(_searchSpot);
}

void InvestigateSpotTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	savefile->ReadInt(_exitTime);
	savefile->ReadBool(_investigateClosely);
	savefile->ReadVec3(_searchSpot);
}

InvestigateSpotTaskPtr InvestigateSpotTask::CreateInstance()
{
	return InvestigateSpotTaskPtr(new InvestigateSpotTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar investigateSpotTaskRegistrar(
	TASK_INVESTIGATE_SPOT, // Task Name
	TaskLibrary::CreateInstanceFunc(&InvestigateSpotTask::CreateInstance) // Instance creation callback
);

} // namespace ai
