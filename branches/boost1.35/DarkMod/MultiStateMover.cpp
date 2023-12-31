/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2443 $
 * $Date: 2008-06-07 09:48:49 -0400 (Sat, 07 Jun 2008) $
 * $Author: angua $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MultiStateMover.cpp 2443 2008-06-07 13:48:49Z angua $", init_version);

#include "MultiStateMover.h"

CLASS_DECLARATION( idMover, CMultiStateMover )
	EVENT( EV_Activate,		CMultiStateMover::Event_Activate )
	EVENT( EV_PostSpawn,	CMultiStateMover::Event_PostSpawn )
END_CLASS

CMultiStateMover::CMultiStateMover() :
	forwardDirection(0,0,0)
{}

void CMultiStateMover::Spawn() 
{
	forwardDirection = spawnArgs.GetVector("forward_direction", "0 0 1");
	forwardDirection.Normalize();

	// Schedule a post-spawn event to analyse the targets
	PostEventMS(&EV_PostSpawn, 1);
}

void CMultiStateMover::FindPositionEntities()
{
	// Go through all the targets and find the PositionEntities
	for (int i = 0; i < targets.Num(); i++) 
	{
		idEntity* target = targets[i].GetEntity();

		if (!target->IsType(CMultiStateMoverPosition::Type)) 
		{
			continue;
		}

		CMultiStateMoverPosition* moverPos = static_cast<CMultiStateMoverPosition*>(target);

		DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("Parsing multistate position entity %s.\r", moverPos->name.c_str());
		
		idStr positionName;
		if (!moverPos->spawnArgs.GetString("position", "", positionName) || positionName.IsEmpty())
		{
			gameLocal.Warning("'position' spawnarg on %s is missing.\n", moverPos->name.c_str());
			continue;
		}

		if (GetPositionInfoIndex(positionName) != -1) 
		{
			gameLocal.Warning("Multiple positions with name %s defined for %s.\n", positionName.c_str(), name.c_str());
			continue;
		}

		// greebo: Seems like the position entity is valid, let's build an info structure
		MoverPositionInfo info;

		info.positionEnt = moverPos;
		info.name = positionName;

		positionInfo.Append(info);		

		// Associate the mover position with this entity
		moverPos->SetMover(this);
	}

	// Now remove all the MultiStatePositionInfo entities from the elevator targets 
	// to avoid triggering of positionInfo entities when the elevator is reaching different floors.
	for (int i = 0; i < positionInfo.Num(); i++)
	{
		RemoveTarget(positionInfo[i].positionEnt.GetEntity());
	}
}

void CMultiStateMover::Event_PostSpawn() 
{
	FindPositionEntities();
	DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("Found %d multistate position entities.\r", positionInfo.Num());
}

bool CMultiStateMover::IsAtPosition(CMultiStateMoverPosition* position)
{
	if (position == NULL) return false;

	const idVec3& pos = position->GetPhysics()->GetOrigin();
	const idVec3& curPos = GetPhysics()->GetOrigin();
	
	// Compare our current position with the given one, using a small tolerance
	return (pos.Compare(curPos, VECTOR_EPSILON));
}

void CMultiStateMover::RegisterButton(CMultiStateMoverButton* button, EMMButtonType type)
{
	if (button == NULL) return;

	switch (type)
	{
	case BUTTON_TYPE_RIDE:
		rideButtons.Alloc() = button;
		break;
	case BUTTON_TYPE_FETCH:
		fetchButtons.Alloc() = button;
		break;
	default:
		gameLocal.Warning("Unknown button state type registered: %s", button->name.c_str());
		break;
	};
}

CMultiStateMoverButton* CMultiStateMover::GetButton(
	CMultiStateMoverPosition* toPosition, CMultiStateMoverPosition* fromPosition, EMMButtonType type)
{
	// Sanity checks
	if (toPosition == NULL) return NULL;
	if (type == BUTTON_TYPE_RIDE && fromPosition == NULL) return NULL;

	switch (type)
	{
	case BUTTON_TYPE_RIDE:
		{
			const idVec3& fromOrigin = fromPosition->GetPhysics()->GetOrigin();
			for (int i = 0; i < rideButtons.Num(); i++)
			{
				CMultiStateMoverButton* rideButton = rideButtons[i].GetEntity();
				if (rideButton == NULL) continue;

				if (rideButton->spawnArgs.GetString("position") != toPosition->spawnArgs.GetString("position")) 
				{
					// Wrong position
					continue;
				}

				// Check if the position of the buttons is appropriate for the given fromPosition
				if (idMath::Fabs(fromOrigin.z - rideButton->GetPhysics()->GetOrigin().z) < 100)
				{
					return rideButton;
				}
			}
		}
		break;
	case BUTTON_TYPE_FETCH:
		{
			const idVec3& toOrigin = toPosition->GetPhysics()->GetOrigin();
			for (int i = 0; i < fetchButtons.Num(); i++)
			{
				CMultiStateMoverButton* fetchButton = fetchButtons[i].GetEntity();
				if (fetchButton == NULL) continue;

				if (fetchButton->spawnArgs.GetString("position") != toPosition->spawnArgs.GetString("position")) 
				{
					// Wrong position
					continue;
				}

				// Check if the position of the buttons is appropriate for the given toPosition
				if (idMath::Fabs(toOrigin.z - fetchButton->GetPhysics()->GetOrigin().z) < 100)
				{
					return fetchButton;
				}
			}
		}
		break;
	default:
		gameLocal.Warning("Unknown button state type passed to MultiStateMover::GetButton(): %s", name.c_str());
		break;
	};

	return NULL;
}

void CMultiStateMover::Save(idSaveGame *savefile) const
{
	savefile->WriteInt(positionInfo.Num());
	for (int i = 0; i < positionInfo.Num(); i++)
	{
		positionInfo[i].positionEnt.Save(savefile);
		savefile->WriteString(positionInfo[i].name);
	}

	savefile->WriteVec3(forwardDirection);

	savefile->WriteInt(fetchButtons.Num());
	for (int i = 0; i < fetchButtons.Num(); i++)
	{
		fetchButtons[i].Save(savefile);
	}

	savefile->WriteInt(rideButtons.Num());
	for (int i = 0; i < rideButtons.Num(); i++)
	{
		rideButtons[i].Save(savefile);
	}
}

void CMultiStateMover::Restore(idRestoreGame *savefile)
{
	int num;
	savefile->ReadInt(num);
	positionInfo.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		positionInfo[i].positionEnt.Restore(savefile);
		savefile->ReadString(positionInfo[i].name);
	}

	savefile->ReadVec3(forwardDirection);

	savefile->ReadInt(num);
	fetchButtons.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		fetchButtons[i].Restore(savefile);
	}

	savefile->ReadInt(num);
	rideButtons.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		rideButtons[i].Restore(savefile);
	}
}

void CMultiStateMover::Activate(idEntity* activator)
{
	if (activator == NULL) return;

	// Get the "position" spawnarg from the activator
	idStr targetPosition;
	if (!activator->spawnArgs.GetString("position", "", targetPosition))
	{
		return; // no "position" key found => exit
	}

	int targetPositionIndex = GetPositionInfoIndex(targetPosition);

	if (targetPositionIndex == -1) 
	{
		gameLocal.Warning("Multistate mover is targetted by an entity with unknown 'position': %s", targetPosition.c_str());
		return;
	}

	// We appear to have a valid position index, start moving
	CMultiStateMoverPosition* positionEnt = positionInfo[targetPositionIndex].positionEnt.GetEntity();

	if (IsAtPosition(positionEnt))
	{
		// nothing to do, we're already at the target position
		return;
	}

	const idVec3& targetPos = positionEnt->GetPhysics()->GetOrigin();
	const idVec3& curPos = GetPhysics()->GetOrigin();
	
	// Check if we are at a defined position
	CMultiStateMoverPosition* curPositionEnt = GetPositionEntity(curPos);

	// We're done moving if the velocity is very close to zero
	bool isAtRest = GetPhysics()->GetLinearVelocity().Length() <= VECTOR_EPSILON;

	if (isAtRest)
	{
		if (spawnArgs.GetBool("trigger_on_leave", "0"))
		{
			// We're leaving our position, trigger targets
			ActivateTargets(this);
		}

		if (curPositionEnt != NULL) 
		{
			// Fire the event on the position entity
			curPositionEnt->OnMultistateMoverLeave(this);
		}
	}

	// Set the rotation direction of any targetted func_rotaters
	SetGearDirection(targetPos);

	// Finally start moving (this will update the "stage" members of the mover)
	MoveToPos(targetPos);
}

const idList<MoverPositionInfo>& CMultiStateMover::GetPositionInfoList()
{
	if (positionInfo.Num() == 0)
	{
		// No position entities? Find them now.
		FindPositionEntities();
	}

	return positionInfo;
}

void CMultiStateMover::DoneMoving()
{
	idMover::DoneMoving();

	if (spawnArgs.GetBool("trigger_on_reached", "0")) 
	{
		// Trigger targets now that we've reached our goal position
		ActivateTargets(this);
	}

	// Try to locate the position entity
	CMultiStateMoverPosition* positionEnt = GetPositionEntity(GetPhysics()->GetOrigin());
	if (positionEnt != NULL)
	{
		// Fire the event on the position entity
		positionEnt->OnMultistateMoverArrive(this);
	}
}

void CMultiStateMover::SetGearDirection(const idVec3& targetPos)
{
	// greebo: Look if we need to control the rotation of the targetted rotators
	if (!spawnArgs.GetBool("control_gear_direction", "0")) 
	{
		return;
	}

	// Check if we're moving forward or backward
	idVec3 moveDir = targetPos - GetPhysics()->GetOrigin();
	moveDir.NormalizeFast();

	// The dot product (== angle) shows whether we're moving forward or backwards
	bool movingForward = (moveDir * forwardDirection >= 0);

	for (int i = 0; i < targets.Num(); i++)
	{
		idEntity* target = targets[i].GetEntity();
		if (target->IsType(idRotater::Type))
		{
			idRotater* rotater = static_cast<idRotater*>(target);
			rotater->SetDirection(movingForward);
		}
	}
}

int CMultiStateMover::GetPositionInfoIndex(const idStr& name) const
{
	for (int i = 0; i < positionInfo.Num(); i++) 
	{
		if (positionInfo[i].name == name) 
		{
			return i;
		}
	}

	return -1; // not found
}

CMultiStateMoverPosition* CMultiStateMover::GetPositionEntity(const idVec3& pos) const
{
	int positionIndex = GetPositionInfoIndex(pos);
	return (positionIndex != -1) ? positionInfo[positionIndex].positionEnt.GetEntity() : NULL;
}

int CMultiStateMover::GetPositionInfoIndex(const idVec3& pos) const
{
	for (int i = 0; i < positionInfo.Num(); i++) 
	{
		idEntity* positionEnt = positionInfo[i].positionEnt.GetEntity();
		assert(positionEnt != NULL);

		if (positionEnt->GetPhysics()->GetOrigin().Compare(pos, VECTOR_EPSILON))
		{
			return i;
		}
	}

	return -1; // not found
}

void CMultiStateMover::Event_Activate(idEntity* activator)
{
	Activate(activator);
}
