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

static bool init_version = FileVersionList("$Id: HandleDoorTask.cpp 2443 2008-06-07 13:48:49Z angua $", init_version);

#include "../Memory.h"
#include "HandleDoorTask.h"
#include "../AreaManager.h"

namespace ai
{

// Get the name of this task
const idStr& HandleDoorTask::GetName() const
{
	static idStr _name(TASK_HANDLE_DOOR);
	return _name;
}

void HandleDoorTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Init the base class
	Task::Init(owner, subsystem);

	Memory& memory = owner->GetMemory();

	CFrobDoor* frobDoor = memory.doorRelated.currentDoor.GetEntity();
	if (frobDoor == NULL)
	{
		return;
	}

	if (!owner->m_bCanOperateDoors)
	{
		if (!frobDoor->IsOpen() || !FitsThrough())
		{
			owner->StopMove(MOVE_STATUS_DEST_UNREACHABLE);
			// add AAS area number of the door to forbidden areas
			idAAS*	aas = owner->GetAAS();
			if (aas != NULL)
			{
				int areaNum = frobDoor->GetFrobMoverAasArea(aas);
				gameLocal.m_AreaManager.AddForbiddenArea(areaNum, owner);
			}
			subsystem.FinishTask();
			return;
		}
	}

	// Let the owner save its move
	owner->PushMove();
	owner->m_HandlingDoor = true;


	CFrobDoor* doubleDoor = frobDoor->GetDoubleDoor();

	_wasLocked = false;

	if (frobDoor->IsLocked())
	{
		_wasLocked = true;
	}

	const idVec3& frobDoorOrg = frobDoor->GetPhysics()->GetOrigin();
	const idVec3& openDir = frobDoor->GetOpenDir();

	//calculate where to stand when the door swings away from us
	idVec3 awayPos = GetAwayPos(owner, frobDoor);
	// calculate where to stand when the door swings towards us
	idVec3 towardPos =  GetTowardPos(owner, frobDoor);
	

	// check if the door swings towards or away from us
	if (openDir * (owner->GetPhysics()->GetOrigin() - frobDoorOrg) > 0)
	{
		// Door opens towards us
		_frontPos = towardPos;
		_backPos = awayPos;
	}
	else
	{
		// Door opens away from us
		_frontPos = awayPos;
		_backPos = towardPos;
	}

	_doorHandlingState = EStateNone;
}

bool HandleDoorTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("HandleDoorTask performing.\r");

	idAI* owner = _owner.GetEntity();
	Memory& memory = owner->GetMemory();

	CFrobDoor* frobDoor = memory.doorRelated.currentDoor.GetEntity();
	if (frobDoor == NULL)
	{
		return true;
	}

	const idVec3& frobDoorOrg = frobDoor->GetPhysics()->GetOrigin();
	const idVec3& openPos = frobDoorOrg + frobDoor->GetOpenPos();
	const idVec3& closedPos = frobDoorOrg + frobDoor->GetClosedPos();

	// if our current door is part of a double door, this is the other part.
	CFrobDoor* doubleDoor = frobDoor->GetDoubleDoor();

	const idBounds& bounds = owner->GetPhysics()->GetBounds();
	float size = bounds[1][0];

	if (cv_ai_door_show.GetBool()) 
	{
		gameRenderWorld->DebugArrow(colorYellow, _frontPos, _frontPos + idVec3(0, 0, 20), 2, 1000);
		gameRenderWorld->DebugArrow(colorGreen, _backPos, _backPos + idVec3(0, 0, 20), 2, 1000);
		idStr str;
		switch (_doorHandlingState)
		{
			case EStateNone:
				str = "EStateNone";
				break;
			case EStateMovingToFrontPos:
				str = "EStateMovingToFrontPos";
				break;
			case EStateWaitBeforeOpen:
				str = "EStateWaitBeforeOpen";
				break;
			case EStateStartOpen:
				str = "EStateStartOpen";
				break;
			case EStateOpeningDoor:
				str = "EStateOpeningDoor";
				break;
			case EStateMovingToBackPos:
				str = "EStateMovingToBackPos";
				break;
			case EStateWaitBeforeClose:
				str = "EStateWaitBeforeClose";
				break;
			case EStateStartClose:
				str = "EStateStartClose";
				break;
			case EStateClosingDoor:
				str = "EStateClosingDoor";
				break;
		}
		gameRenderWorld->DrawText(str.c_str(), 
			(owner->GetEyePosition() - owner->GetPhysics()->GetGravityNormal()*60.0f), 
			0.25f, colorYellow, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, 4 * gameLocal.msec);
	}

	// Door is closed
	if (!frobDoor->IsOpen())
	{
		switch (_doorHandlingState)
		{
			case EStateNone:
				if (doubleDoor != NULL && doubleDoor->IsOpen())
				{
					// the other part of the double door is already open
					// no need to open this one
					ResetDoor(owner, doubleDoor);
					if (!owner->MoveToPosition(_frontPos))
					{
						// TODO: position not reachable, need a better one
					}
					_doorHandlingState = EStateMovingToFrontPos;
					break;
				}

				if (!owner->MoveToPosition(_frontPos))
				{
					// TODO: position not reachable, need a better one
				}
				_doorHandlingState = EStateMovingToFrontPos;
				break;

			case EStateMovingToFrontPos:
				if (doubleDoor != NULL && doubleDoor->IsOpen())
				{
					// the other part of the double door is already open
					// no need to open this one
					ResetDoor(owner, doubleDoor);
					break;
				}
				if (owner->AI_MOVE_DONE)
				{
					// reached position
					owner->StopMove(MOVE_STATUS_DONE);
					owner->TurnToward(closedPos);
					_waitEndTime = gameLocal.time + 750;
					_doorHandlingState = EStateWaitBeforeOpen;
				}
				break;

			case EStateWaitBeforeOpen:
				if (doubleDoor != NULL && doubleDoor->IsOpen())
				{
					// the other part of the double door is already open
					// no need to open this one
					ResetDoor(owner, doubleDoor);
					break;
				}
				if (gameLocal.time >= _waitEndTime)
				{
					owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Use_righthand", 4);

					_doorHandlingState = EStateStartOpen;
					_waitEndTime = gameLocal.time + 500;
				}
				break;

			case EStateStartOpen:
				if (doubleDoor != NULL && doubleDoor->IsOpen())
				{
					// the other part of the double door is already open
					// no need to open this one
					ResetDoor(owner, doubleDoor);
					break;
				}
				if (gameLocal.time >= _waitEndTime)
				{
					if (!OpenDoor())
					{
						return true;
					}
				}
				break;

			case EStateOpeningDoor:
				// we have already started opening the door, but it is closed,
				if (doubleDoor != NULL && doubleDoor->IsOpen())
				{
					// the other part of the double door is already open
					// no need to open this one
					ResetDoor(owner, doubleDoor);
					break;
				}
				// try again
				owner->StopMove(MOVE_STATUS_DONE);
				owner->TurnToward(closedPos);
				if (!OpenDoor())
				{
					return true;
				}
				break;

			case EStateMovingToBackPos:
				// door has closed while we were attempting to walk through it.
				// end this task (it will be initiated again if we are still in front of the door).
				return true;
				break;
				
			case EStateWaitBeforeClose:
				// door has already closed before we were attempting to do it
				// no need for more waiting
				return true;
				break;


			case EStateStartClose:
				// door has already closed before we were attempting to do it
				// no need for more waiting
				return true;
				break;


			case EStateClosingDoor:
				// we have moved through the door and closed it
				if (_wasLocked && owner->CanUnlock(frobDoor))
				{
					// if the door was locked before, lock it again
					frobDoor->Lock(false);
				}
				if (doubleDoor != NULL && doubleDoor->IsOpen())
				{
					// the other part of the double door is still open
					// we want to close this one too
					ResetDoor(owner, doubleDoor);
					owner->MoveToPosition(_backPos);
					_doorHandlingState = EStateMovingToBackPos;
					break;
				}
				// continue what we were doing before.
				return true;
				break;

			default:
				break;
		}
	}
	// Door is open
	else
	{
		switch (_doorHandlingState)
		{
			case EStateNone:
				// check if we are blocking the door
				if (frobDoor->IsBlocked() || 
					(frobDoor->WasInterrupted() && 
					frobDoor->WasStoppedDueToBlock()))
				{
					if (frobDoor->GetLastBlockingEnt() == owner)
					{
						// we are blocking the door, open it
						owner->StopMove(MOVE_STATUS_DONE);
						owner->TurnToward(closedPos);
						if (!OpenDoor())
						{
							return true;
						}
					}
				}
				else if (frobDoor->WasInterrupted())
				{
					if (FitsThrough())
					{
						owner->MoveToPosition(_backPos);
						_doorHandlingState = EStateMovingToBackPos;
					}
					else
					{
						owner->MoveToPosition(_frontPos);
						_doorHandlingState = EStateMovingToFrontPos;
					}
				}

				// door is open and possibly in the way, may need to close it
				// check if there is a way around
				else 
				{
					idTraceModel trm(bounds);
					idClipModel clip(trm);
	
					idVec3 testVector = openPos - frobDoorOrg;
					testVector.z = 0;
					float length = testVector.LengthFast();
					length += size * SQUARE_ROOT_OF_2;
					testVector.NormalizeFast();
					idVec3 testPoint = frobDoorOrg + testVector * length;

					int contents = gameLocal.clip.Contents(testPoint, &clip, mat3_identity, CONTENTS_SOLID, owner);

					if (contents)
					{
						// check if the door swings towards or away from us
						const idVec3& openDir = frobDoor->GetOpenDir();
						if (openDir * (owner->GetPhysics()->GetOrigin() - frobDoorOrg) > 0)
						{
							// Door opens towards us
							idVec3 closedDir = closedPos - frobDoorOrg;
							closedDir.z = 0;
							idVec3 org = owner->GetPhysics()->GetOrigin();
							idVec3 ownerDir = org - frobDoorOrg;
							ownerDir.z = 0;
							idVec3 frontPosDir = _frontPos - frobDoorOrg;
							frontPosDir.z = 0;

							float l1 = closedDir * ownerDir;
							float l2 = closedDir * frontPosDir;

							if (l1 * l2 < 0)
							{	
								// can't reach standard position
								idVec3 parallelOffset = openPos - frobDoorOrg;
								parallelOffset.z = 0;
								float len = parallelOffset.LengthFast();
								parallelOffset.NormalizeFast();
								parallelOffset *= len - 1.2f * size;

								idVec3 normalOffset = closedPos - frobDoorOrg;
								normalOffset.z = 0;
								normalOffset.NormalizeFast();
								normalOffset *= 1.5f * size;

								_frontPos = frobDoorOrg + parallelOffset - normalOffset;
							}
							
							owner->MoveToPosition(_frontPos);
							_doorHandlingState = EStateMovingToBackPos;
						}
						else
						{
							//Door opens away from us
							owner->MoveToPosition(_backPos);
							_doorHandlingState = EStateMovingToBackPos;

						}
					}
					else
					{
						// door is not in the way and open, just continue walking
						return true;
					}
				}
				break;
			

			case EStateMovingToFrontPos:
				// check if the door was blocked or interrupted
				if (frobDoor->IsBlocked() || 
					frobDoor->WasInterrupted() || 
					frobDoor->WasStoppedDueToBlock())
				{
					if (FitsThrough())
					{
						// gap is large enough, move to back position
						owner->MoveToPosition(_backPos);
						_doorHandlingState = EStateMovingToBackPos;
					}
					else
					{
						// need to open the door further when we reach the position for opening
						if (owner->AI_MOVE_DONE)
						{
							// reached front position
							owner->StopMove(MOVE_STATUS_DONE);
							owner->TurnToward(closedPos);
							_waitEndTime = gameLocal.time + 650;
							_doorHandlingState = EStateWaitBeforeOpen;
						}
					}
				}
				// door is already open, move to back position
				else
				{
					owner->MoveToPosition(_backPos);
					_doorHandlingState = EStateMovingToBackPos;
				}
				break;


			case EStateWaitBeforeOpen:
				// check blocked or interrupted
				if (frobDoor->IsBlocked() || 
					frobDoor->WasInterrupted() || 
					frobDoor->WasStoppedDueToBlock())
				{
					if (!FitsThrough())
					{
						if (gameLocal.time >= _waitEndTime)
						{
							if (!OpenDoor())
							{
								return true;
							}
						}
					}
				}
				else
				{
					// no need for waiting, door is already open, let's move
					owner->MoveToPosition(_backPos);
					_doorHandlingState = EStateMovingToBackPos;
				}
				break;

			case EStateStartOpen:
				if (frobDoor->IsBlocked() || 
					(frobDoor->WasInterrupted() || 
					frobDoor->WasStoppedDueToBlock()))
				{
					if (!FitsThrough())
					{
						if (gameLocal.time >= _waitEndTime)
						{
							if (!OpenDoor())
							{
								return true;
							}
						}
					}
				}
				// no need for waiting, door is already open, let's move
				owner->MoveToPosition(_backPos);
				_doorHandlingState = EStateMovingToBackPos;
				break;


			case EStateOpeningDoor:
				// check blocked
				if (frobDoor->IsBlocked() || 
					(frobDoor->WasInterrupted() && 
					frobDoor->WasStoppedDueToBlock()))
				{
					if (FitsThrough())
					{
						// gap is large enough, move to back position
						owner->MoveToPosition(_backPos);
						_doorHandlingState = EStateMovingToBackPos;
					}
					else
					{
						if (frobDoor->GetLastBlockingEnt() == owner)
						{
							// we are blocking the door
							owner->StopMove(MOVE_STATUS_DONE);
							owner->TurnToward(closedPos);
							if (!OpenDoor())
							{
								return true;
							}
						}
						else
						{
							// something else is blocking the door
							// possibly the player, another AI or an object
							// try closing the door and opening it again
							frobDoor->Close(false);
							_waitEndTime = gameLocal.time + 300;
							_doorHandlingState = EStateWaitBeforeOpen;
							// TODO: need to stop after a few tries
						}
					}
				}
				//check interrupted
				else if (frobDoor->WasInterrupted())
				{
					if (FitsThrough())
					{
						// gap is large enough, move to back position
						owner->MoveToPosition(_backPos);
						_doorHandlingState = EStateMovingToBackPos;
					}
					else
					{
						// can't move through already, need to open further
						if (!OpenDoor())
						{
							return true;
						}
					}
				}
				// door is fully open, let's get moving
				else if	(!frobDoor->IsChangingState())
				{
					owner->MoveToPosition(_backPos);
					_doorHandlingState = EStateMovingToBackPos;
				}
				break;


			case EStateMovingToBackPos:
				// check blocked
				if (frobDoor->IsBlocked() || 
					(frobDoor->WasInterrupted() && 
					frobDoor->WasStoppedDueToBlock()))
				{
					if (frobDoor->GetLastBlockingEnt() == owner)
					{
						// we are blocking the door
						owner->StopMove(MOVE_STATUS_DONE);
						owner->TurnToward(closedPos);
						_doorHandlingState = EStateOpeningDoor;
						if (!OpenDoor())
						{
							return true;
						}
					}
				}
				else if (frobDoor->WasInterrupted() && !FitsThrough())
				{
					// end this, task, it will be reinitialized wehn needed
					return true;
				}
				
				// reached back position
				else if (owner->AI_MOVE_DONE)
				{
					if (owner->ShouldCloseDoor(frobDoor))
					{
						// close the door
						owner->StopMove(MOVE_STATUS_DONE);
						owner->TurnToward(openPos);
						_waitEndTime = gameLocal.time + 650;
						_doorHandlingState = EStateWaitBeforeClose;
					}
					else
					{
						return true;
					}
				}
				break;

				
			case EStateWaitBeforeClose:
				if (gameLocal.time >= _waitEndTime)
				{
					owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Use_righthand", 4);
					_doorHandlingState = EStateStartClose;
					_waitEndTime = gameLocal.time + 500;

				}
				break;

			case EStateStartClose:
				if (gameLocal.time >= _waitEndTime)
				{
					frobDoor->Close(false);
					_doorHandlingState = EStateClosingDoor;
				}
				break;


			case EStateClosingDoor:
				// check blocked or interrupted
				if (frobDoor->IsBlocked() || 
					frobDoor->WasInterrupted() || 
					frobDoor->WasStoppedDueToBlock())
				{
					return true;
				}
				break;

			default:
				break;
		}
	}

	return false; // not finished yet
}

void HandleDoorTask::ResetDoor(idAI* owner, CFrobDoor* newDoor)
{
	Memory& memory = owner->GetMemory();

	// reset the active door to this door					
	memory.doorRelated.currentDoor = newDoor;
	// recalculate standing positions
	const idVec3& frobDoorOrg = newDoor->GetPhysics()->GetOrigin();
	const idVec3& openDir = newDoor->GetOpenDir();
	idVec3 awayPos = GetAwayPos(owner, newDoor);
	idVec3 towardPos =  GetTowardPos(owner, newDoor);
	
	if (_doorHandlingState == EStateWaitBeforeClose
		|| _doorHandlingState == EStateStartClose
		|| _doorHandlingState == EStateClosingDoor)
	{
		// we have already walked through, so we are standing on the side of the backpos
		if (openDir * (owner->GetPhysics()->GetOrigin() - frobDoorOrg) > 0)
		{
			_frontPos = awayPos;
			_backPos = towardPos;
			
		}
		else
		{
			_frontPos = towardPos;
			_backPos = awayPos;
		}
	}
	else
	{
		if (openDir * (owner->GetPhysics()->GetOrigin() - frobDoorOrg) > 0)
		{
			_frontPos = towardPos;
			_backPos = awayPos;
		}
		else
		{
			_frontPos = awayPos;
			_backPos = towardPos;
		}
	}
}
idVec3 HandleDoorTask::GetAwayPos(idAI* owner, CFrobDoor* frobDoor)
{
	const idVec3& frobDoorOrg = frobDoor->GetPhysics()->GetOrigin();
	const idVec3& openDir = frobDoor->GetOpenDir();
	const idVec3& closedPos = frobDoorOrg + frobDoor->GetClosedPos();

	idBounds frobDoorBounds = frobDoor->GetPhysics()->GetAbsBounds();

	idBounds bounds = owner->GetPhysics()->GetBounds();
	float size = bounds[1][0];

	idVec3 dir = closedPos - frobDoorOrg;
	dir.z = 0;
	idVec3 dirNorm = dir;
	dirNorm.NormalizeFast();
	float dist = dir.LengthFast();

	idVec3 openDirNorm = openDir;
	openDirNorm.z = 0;
	openDirNorm.NormalizeFast();

	idVec3 parallelAwayOffset = dirNorm;
	parallelAwayOffset *= size * 1.4f;

	idVec3 normalAwayOffset = openDirNorm;
	normalAwayOffset *= size * 2.5;

	idVec3 awayPos = closedPos - parallelAwayOffset - normalAwayOffset;
	awayPos.z = frobDoorBounds[0].z + 5;

	return awayPos;
}

idVec3 HandleDoorTask::GetTowardPos(idAI* owner, CFrobDoor* frobDoor)
{
	// calculate where to stand when the door swings towards us
	const idVec3& frobDoorOrg = frobDoor->GetPhysics()->GetOrigin();
	const idVec3& openDir = frobDoor->GetOpenDir();
	const idVec3& openPos = frobDoorOrg + frobDoor->GetOpenPos();
	const idVec3& closedPos = frobDoorOrg + frobDoor->GetClosedPos();

	idBounds frobDoorBounds = frobDoor->GetPhysics()->GetAbsBounds();

	idBounds bounds = owner->GetPhysics()->GetBounds();
	float size = bounds[1][0];
	idTraceModel trm(bounds);
	idClipModel clip(trm);

	idVec3 dir = closedPos - frobDoorOrg;
	dir.z = 0;
	idVec3 dirNorm = dir;
	dirNorm.NormalizeFast();
	float dist = dir.LengthFast();
	
	idVec3 openDirNorm = openDir;
	openDirNorm.z = 0;
	openDirNorm.NormalizeFast();

	// next to the door
	idVec3 parallelTowardOffset = dirNorm;
	parallelTowardOffset *= dist + size * 2;

	idVec3 normalTowardOffset = openDirNorm;
	normalTowardOffset *= size * 2;

	idVec3 towardPos = frobDoorOrg + parallelTowardOffset + normalTowardOffset;
	towardPos.z = frobDoorBounds[0].z + 5;

	// check if we can stand at this position
	int contents = gameLocal.clip.Contents(towardPos, &clip, mat3_identity, CONTENTS_SOLID, owner);

	if (contents)
	{
		if (cv_ai_door_show.GetBool())
		{
			gameRenderWorld->DebugBounds(colorRed, bounds, towardPos, 10000);
		}

		// at 45� swinging angle
		parallelTowardOffset = dirNorm;

		normalTowardOffset = openDirNorm;

		towardPos = parallelTowardOffset + normalTowardOffset;
		towardPos.NormalizeFast();
		towardPos *= (dist + size * 2);
		towardPos += frobDoorOrg;
		towardPos.z = frobDoorBounds[0].z + 5;

		int contents = gameLocal.clip.Contents(towardPos, &clip, mat3_identity, CONTENTS_SOLID, owner);

		if (contents)
		{
			if (cv_ai_door_show.GetBool())
			{
				gameRenderWorld->DebugBounds(colorRed, bounds, towardPos, 10000);
			}

			// far away from the door
			parallelTowardOffset = dirNorm * size * 1.2f;

			normalTowardOffset = openDirNorm;
			normalTowardOffset *= dist + 2.5f * size;

			towardPos = frobDoorOrg + parallelTowardOffset + normalTowardOffset;
			towardPos.z = frobDoorBounds[0].z + 5;

			int contents = gameLocal.clip.Contents(towardPos, &clip, mat3_identity, CONTENTS_SOLID, owner);

			if (contents)
			{
				// TODO: no suitable position found
				if (cv_ai_door_show.GetBool())
				{
					gameRenderWorld->DebugBounds(colorGreen, bounds, towardPos, 10000);
				}
			}
			else if (cv_ai_door_show.GetBool())
			{
				gameRenderWorld->DebugBounds(colorGreen, bounds, towardPos, 10000);
			}
		}
		else if (cv_ai_door_show.GetBool())
		{
			gameRenderWorld->DebugBounds(colorGreen, bounds, towardPos, 10000);
		}
	}
	else if (cv_ai_door_show.GetBool())
	{
		 gameRenderWorld->DebugBounds(colorGreen, bounds, towardPos, 10000);
	}
	return towardPos;
}

bool HandleDoorTask::FitsThrough()
{
	// this calculates the gap (depending on the size of the door and the opening angle)
	// and checks if it is large enough for the AI to fit through it.
	idAI* owner = _owner.GetEntity();
	Memory& memory = owner->GetMemory();
	CFrobDoor* frobDoor = memory.doorRelated.currentDoor.GetEntity();

	idAngles tempAngle;
	idPhysics_Parametric* physics = frobDoor->GetMoverPhysics();
	physics->GetLocalAngles( tempAngle );

	const idVec3& closedPos = frobDoor->GetClosedPos();
	idVec3 dir = closedPos;
	dir.z = 0;
	float dist = dir.LengthFast();

	idAngles alpha = frobDoor->GetClosedAngles() - tempAngle;
	float absAlpha = idMath::Fabs(alpha.yaw);
	float sinAlpha = idMath::Sin(DEG2RAD(absAlpha * 0.5f));
	float delta = idMath::Fabs(2 * dist * sinAlpha);

	idBounds bounds = owner->GetPhysics()->GetBounds();
	float size = 2 * SQUARE_ROOT_OF_2 * bounds[1][0] + 10;

	return (delta >= size);
}

bool HandleDoorTask::OpenDoor()
{
	idAI* owner = _owner.GetEntity();
	Memory& memory = owner->GetMemory();
	CFrobDoor* frobDoor = memory.doorRelated.currentDoor.GetEntity();

	// Update our door info structure
	DoorInfo& doorInfo = memory.GetDoorInfo(frobDoor);
	doorInfo.lastTimeSeen = gameLocal.time;
	doorInfo.lastTimeTriedToOpen = gameLocal.time;
	doorInfo.wasLocked = frobDoor->IsLocked();

	if (frobDoor->IsLocked())
	{
		if (!owner->CanUnlock(frobDoor))
		{
			// Door is locked and we cannot unlock it
			owner->StopMove(MOVE_STATUS_DEST_UNREACHABLE);
			// Rattle the door once
			frobDoor->Open(false);
			owner->AI_DEST_UNREACHABLE = true;
				
			// add AAS area number of the door to forbidden areas
			idAAS*	aas = owner->GetAAS();
			if (aas != NULL)
			{
				int areaNum = frobDoor->GetFrobMoverAasArea(aas);
				gameLocal.m_AreaManager.AddForbiddenArea(areaNum, owner);
			}

			return false;
		}
		else
		{
			frobDoor->Unlock(false);
			doorInfo.wasLocked = frobDoor->IsLocked();
		}
	}

	owner->StopMove(MOVE_STATUS_DONE);
	frobDoor->Open(false);
	_doorHandlingState = EStateOpeningDoor;

	return true;
}

void HandleDoorTask::OnFinish(idAI* owner)
{
	Memory& memory = owner->GetMemory();

	if (owner->m_HandlingDoor)
	{
		owner->PopMove();
		owner->m_HandlingDoor = false;
	}

	CFrobDoor* frobDoor = memory.doorRelated.currentDoor.GetEntity();

	if (frobDoor != NULL) 
	{
		// Update our door info structure
		DoorInfo& doorInfo = memory.GetDoorInfo(frobDoor);
		doorInfo.lastTimeSeen = gameLocal.time;
		doorInfo.wasLocked = frobDoor->IsLocked();
		doorInfo.wasOpen = frobDoor->IsOpen();
	}

	memory.doorRelated.currentDoor = NULL;
	_doorHandlingState = EStateNone;
}

void HandleDoorTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	savefile->WriteVec3(_frontPos);
	savefile->WriteVec3(_backPos);
	savefile->WriteInt(static_cast<int>(_doorHandlingState));
	savefile->WriteInt(_waitEndTime);
	savefile->WriteBool(_wasLocked);
}

void HandleDoorTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	savefile->ReadVec3(_frontPos);
	savefile->ReadVec3(_backPos);
	int temp;
	savefile->ReadInt(temp);
	_doorHandlingState = static_cast<EDoorHandlingState>(temp);
	savefile->ReadInt(_waitEndTime);
	savefile->ReadBool(_wasLocked);
}

HandleDoorTaskPtr HandleDoorTask::CreateInstance()
{
	return HandleDoorTaskPtr(new HandleDoorTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar handleDoorTaskRegistrar(
	TASK_HANDLE_DOOR, // Task Name
	TaskLibrary::CreateInstanceFunc(&HandleDoorTask::CreateInstance) // Instance creation callback
);

} // namespace ai
