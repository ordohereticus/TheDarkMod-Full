/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4723 $
 * $Date: 2011-03-25 00:49:57 -0400 (Fri, 25 Mar 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MovementSubsystem.cpp 4723 2011-03-25 04:49:57Z greebo $", init_version);

#include "MovementSubsystem.h"
#include "Library.h"
#include "States/State.h"
#include "Tasks/ResolveMovementBlockTask.h"
#include "Tasks/AnimalPatrolTask.h"
#include "Tasks/PathCornerTask.h"
#include "Tasks/PathAnimTask.h"
#include "Tasks/PathTurnTask.h"
#include "Tasks/PathCycleAnimTask.h"
#include "Tasks/PathSitTask.h"
#include "Tasks/PathSleepTask.h"
#include "Tasks/PathWaitTask.h"
#include "Tasks/PathWaitForTriggerTask.h"
#include "Tasks/PathHideTask.h"
#include "Tasks/PathShowTask.h"
#include "Tasks/PathLookatTask.h"
#include "Tasks/PathInteractTask.h"
#include "Tasks/MoveToPositionTask.h"


namespace ai
{

#define HISTORY_SIZE 32
#define HISTORY_BOUNDS_THRESHOLD 40	// units (grayman #2345 changed to 40 for quicker blocking recognition; was 10)
#define BLOCK_TIME_OUT 400			// milliseconds (grayman #2345 changed to 400 for quicker blocking recognition; was 800)
#define BLOCKED_TOO_LONG 600		// milliseconds (grayman #2345 - how long to stay blocked w/o moving, if you're not waiting for someone to pass by)
#define MAX_PATH_CORNER_SEARCH_ITERATIONS 100
#define PAUSE_TIME 3000				// milliseconds (grayman #2345 - how long to stay paused after treadmilling)

MovementSubsystem::MovementSubsystem(SubsystemId subsystemId, idAI* owner) :
	Subsystem(subsystemId, owner),
	_curHistoryIndex(0),
	_historyBoundsThreshold(HISTORY_BOUNDS_THRESHOLD),
	_state(ENotBlocked),
	_lastTimeNotBlocked(-1),
	_blockTimeOut(BLOCK_TIME_OUT),
	_timeBlockStarted(-1), // grayman #2345
	_blockTimeShouldEnd(BLOCKED_TOO_LONG), // grayman #2345
	_timePauseStarted(-1),		// grayman #2345
	_pauseTimeOut(PAUSE_TIME)	// grayman #2345

{
	_patrolling = false;

	_historyBounds.Clear();

	_originHistory.SetNum(HISTORY_SIZE);
	_frameHistory.SetNum(HISTORY_SIZE); // grayman #2345
}

// Called regularly by the Mind to run the currently assigned routine.
bool MovementSubsystem::PerformTask()
{
	idAI* owner = _owner.GetEntity();

	// Watchdog to keep AI from running into things forever
	CheckBlocked(owner);

	Patrol();
	
	return Subsystem::PerformTask();
}


void MovementSubsystem::StartPatrol()
{
	if (!_patrolling)
	{
		idAI* owner = _owner.GetEntity();
		Memory& memory = owner->GetMemory();

		bool animalPatrol = owner->spawnArgs.GetBool("animal_patrol", "0");

		// Check if the owner has patrol routes set
		idPathCorner* path = memory.currentPath.GetEntity();
		idPathCorner* lastPath = memory.lastPath.GetEntity();
		
		if (path == NULL && lastPath == NULL)
		{
			// Get a new random path off the owner's targets, this is the current one
			path = idPathCorner::RandomPath(owner, NULL, owner);
			memory.currentPath = path;

			// Also, pre-select a next path to allow path predictions
			if (path != NULL)
			{
				memory.nextPath = idPathCorner::RandomPath(path, NULL, owner);
			}
		}
		else if (path != NULL)
		{
			// we already have a stored path, patrolling was already started and is resumed now
			// if we are currently sleeping/sitting, just continue where we are (probably a path_wait)
			if (owner->GetMoveType() != MOVETYPE_SLEEP && owner->GetMoveType() != MOVETYPE_SIT)
			{
				idPathCorner* candidate = GetNextPathCorner(path, owner);

				if (candidate != NULL)
				{
					// advance to next path corner, don't resume other path tasks at the current (presumably wrong) position 
					memory.currentPath = candidate;
					memory.nextPath = idPathCorner::RandomPath(candidate, NULL, owner);
				}
				else
				{
					// We don't have a valid path_corner in our current path branch,
					// or we ended in a "dead end" or in a loop, restart the system
					RestartPatrol();
				}
			}
		}
		else // path == NULL && last path != NULL
		{
			// patrol routine had ended before
			// restart patrolling
			RestartPatrol();
		}

		if (memory.currentPath.GetEntity() != NULL || animalPatrol)
		{
			if (animalPatrol)
			{
				// For animals, push the AnimalPatrol task anyway, they don't need paths
				PushTask(AnimalPatrolTask::CreateInstance());
			}
			else
			{
				StartPathTask();
			}
		
			_patrolling = true;
		}
	}
}

idPathCorner* MovementSubsystem::GetNextPathCorner(idPathCorner* curPath, idAI* owner)
{
	if (curPath == NULL) return NULL; // safety check

	idPathCorner* currentTestPath = curPath;
	
	for (int i = 0; i < MAX_PATH_CORNER_SEARCH_ITERATIONS; i++)
	{
		if (idStr::Cmp(currentTestPath->spawnArgs.GetString("classname"), "path_corner") == 0)
		{
			// found a path_corner
			return currentTestPath;
		}

		// get next path
		currentTestPath = idPathCorner::RandomPath(currentTestPath, NULL, owner);

		if (currentTestPath == NULL)
		{
			// dead end, return NULL
			return NULL;
		}
		else if (currentTestPath == curPath)
		{
			// loop detected
			return NULL;
		}
	}

	return NULL;
}

void MovementSubsystem::RestartPatrol()
{
	idAI* owner = _owner.GetEntity();
	Memory& memory = owner->GetMemory();

	idPathCorner* newPath = idPathCorner::RandomPath(owner, NULL, owner);
	memory.currentPath = newPath;
	memory.nextPath = idPathCorner::RandomPath(newPath, NULL, owner);

	// if the first path is a path corner, just start with that
	// otherwise, move to idle position before restarting patrol
	if (idStr::Cmp(newPath->spawnArgs.GetString("classname"), "path_corner") != 0)
	{
		float startPosTolerance = owner->spawnArgs.GetFloat("startpos_tolerance", "-1");
		owner->movementSubsystem->PushTask(
			TaskPtr(new MoveToPositionTask(memory.idlePosition, memory.idleYaw, startPosTolerance))
		);
	}
}

void MovementSubsystem::Patrol()
{
	idAI* owner = _owner.GetEntity();
	Memory& memory = owner->GetMemory();

	if (_patrolling == false)
	{
		return;
	}

	if (_taskQueue.empty())
	{
		NextPath();
		if (memory.currentPath.GetEntity() == NULL)
		{
			DM_LOG(LC_AI, LT_INFO)LOGSTRING("No more targets.\r");
			_patrolling = false;
			return;
		}

		StartPathTask();
	}
}

void MovementSubsystem::NextPath()
{
	idAI* owner = _owner.GetEntity();
	Memory& memory = owner->GetMemory();

	idPathCorner* path = memory.currentPath.GetEntity();

	// The current path gets stored in lastPath (grayman #2345 - but only if it's a path_corner)
	if ((path == NULL) || (idStr::Cmp(path->spawnArgs.GetString("classname"), "path_corner") == 0)) // grayman #2683 - check for null
	{
		memory.lastPath = path;
	}

    // The pre-selected "next path" is now our current one
    idPathCorner* currentPath = memory.nextPath.GetEntity();

    memory.currentPath = currentPath;

    // Now pre-select a new (random) path entity for the next round
    // this information is important for the PathCornerTask to decide which action to take on exit
	idPathCorner* next(NULL);
	if (currentPath != NULL)
	{
		next = idPathCorner::RandomPath(currentPath, NULL, owner);
	}
	
    memory.nextPath = next;
}

void MovementSubsystem::StartPathTask()
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("Starting next path task.\r");

	idPathCorner* path = _owner.GetEntity()->GetMind()->GetMemory().currentPath.GetEntity();

	// This may not be performed with an empty path corner entity,
	// that case should have been caught by the Patrol() routine
	assert(path);

	std::list<TaskPtr> tasks;
	TaskPtr task;

	// Get the classname, this determines the child routine we're spawning.
	idStr classname = path->spawnArgs.GetString("classname");

	// Depending on the classname we spawn one of the various Path*Tasks
	if (classname == "path_corner")
	{
		tasks.push_back(TaskPtr(new PathCornerTask(path)));
	}
	else if (classname == "path_anim")
	{
		if (path->spawnArgs.FindKey("angle") != NULL)
		{
			// We have an angle key set, push a PathTurnTask on top of the anim task
			tasks.push_back(TaskPtr(new PathAnimTask(path)));
			// The "task" variable will be pushed later on in this code
			tasks.push_back(TaskPtr(new PathTurnTask(path)));
		}
		else 
		{
			// No "angle" key set, just schedule the animation task
			tasks.push_back(TaskPtr(new PathAnimTask(path)));
		}
	}
	else if (classname == "path_cycleanim")
	{
		if (path->spawnArgs.FindKey("angle") != NULL)
		{
			// We have an angle key set, push a PathTurnTask on top of the anim task
			tasks.push_back(TaskPtr(new PathCycleAnimTask(path)));
			// The "task" variable will be pushed later on in this code
			tasks.push_back(PathTurnTaskPtr(new PathTurnTask(path)));
		}
		else 
		{
			// No "angle" key set, just schedule the animation task
			tasks.push_back(PathCycleAnimTaskPtr(new PathCycleAnimTask(path)));
		}
	}
	else if (classname == "path_sit")
	{
		if (path->spawnArgs.FindKey("angle") != NULL)
		{
			// We have an angle key set, push a PathTurnTask on top of the anim task
			tasks.push_back(TaskPtr(new PathSitTask(path)));
			// The "task" variable will be pushed later on in this code
			tasks.push_back(PathTurnTaskPtr(new PathTurnTask(path)));
		}
		else 
		{
			// No "angle" key set, just schedule the animation task
			tasks.push_back(PathSitTaskPtr(new PathSitTask(path)));
		}
	}

	else if (classname == "path_sleep")
	{
		if (path->spawnArgs.FindKey("angle") != NULL)
		{
			// We have an angle key set, push a PathTurnTask on top of the sleep task
			tasks.push_back(TaskPtr(new PathSleepTask(path)));
			// The "task" variable will be pushed later on in this code
			tasks.push_back(PathTurnTaskPtr(new PathTurnTask(path)));
		}
		else 
		{
			// No "angle" key set, just schedule the sleep task
			tasks.push_back(PathSleepTaskPtr(new PathSleepTask(path)));
		}
	}

	else if (classname == "path_turn")
	{
		tasks.push_back(PathTurnTaskPtr(new PathTurnTask(path)));
	}
	else if (classname == "path_wait")
	{
		if (path->spawnArgs.FindKey("angle") != NULL)
		{
			// We have an angle key set, push a PathTurnTask on top of the anim task
			tasks.push_back(TaskPtr(new PathWaitTask(path)));
			// The "task" variable will be pushed later on in this code
			tasks.push_back(PathTurnTaskPtr(new PathTurnTask(path)));
		}
		else 
		{
			// No "angle" key set, just schedule the wait task
			tasks.push_back(PathWaitTaskPtr(new PathWaitTask(path)));
		}
	}
	else if (classname == "path_waitfortrigger")
	{
		tasks.push_back(PathWaitForTriggerTaskPtr(new PathWaitForTriggerTask(path)));
	}
	else if (classname == "path_hide")
	{
		tasks.push_back(PathHideTaskPtr(new PathHideTask(path)));
	}
	else if (classname == "path_show")
	{
		tasks.push_back(PathShowTaskPtr(new PathShowTask(path)));
	}
	else if (classname == "path_lookat")
	{
		tasks.push_back(PathLookatTaskPtr(new PathLookatTask(path)));
	}
	else if (classname == "path_interact")
	{
		tasks.push_back(PathInteractTaskPtr(new PathInteractTask(path)));
	}
	else
	{
		// Finish this task
		gameLocal.Warning("Unknown path corner classname '%s'\n", classname.c_str());
		return;
	}
	
	// Push the (rest of the) tasks to the subsystem
	for (std::list<TaskPtr>::iterator i = tasks.begin(); i != tasks.end(); ++i)
	{
		PushTask(*i);
	}
}

void MovementSubsystem::ClearTasks()
{
	Subsystem::ClearTasks();
	_patrolling = false;
}

// grayman #2345 - extricate yourself somehow if allowed. One situtation where you
// shouldn't extricate is if an AI blocking you is trying to resolve the block. His
// attempt won't work if you head off in another direction.

bool MovementSubsystem::AttemptToExtricate()
{
	idAI* owner = _owner.GetEntity();

	// Look around to see if there's somewhere you can go.

	const idVec3& ownerOrigin = owner->GetPhysics()->GetOrigin();

	// Set all attachments to nonsolid, temporarily
	owner->SaveAttachmentContents();
	owner->SetAttachmentContents(0);

	// Look around in 90 degree increments. Move toward
	// whichever destination lets you travel the farthest.

	trace_t result;
	idBounds bnds = owner->GetPhysics()->GetBounds();
	float extricateDistance = 1.5*(bnds[1][0] + bnds[1][1]); // 1.5 x (x_size/2 + y_size/2)
	idAngles angles = owner->viewAxis.ToAngles();
	float forward = angles.yaw;
	float moveFraction = 0;		 // fractional distance from your origin to moveTo

	for (int i = 0 ; i < 3 ; i++)
	{
		float tryAngle = forward + 90.0 + 180.0*gameLocal.random.RandomFloat();
		idAngles tryAngles = idAngles(0,tryAngle,0);
		tryAngles.Normalize180();

		idVec3 moveTo = ownerOrigin + extricateDistance*tryAngles.ToForward();
		gameLocal.clip.TraceBounds(result, ownerOrigin, moveTo, bnds, CONTENTS_SOLID|CONTENTS_CORPSE, owner);

		if (result.fraction >= 1.0f) // didn't hit anything, so quit looking
		{
			owner->MoveToPosition(moveTo);
			owner->RestoreAttachmentContents(); // Put back attachments
			return true;
		}
	}
	owner->RestoreAttachmentContents(); // Put back attachments
	return false;
}

float MovementSubsystem::GetPrevTraveled()
{
	int i = _curHistoryIndex - 2;
	if (i < 0)
	{
		i = _originHistory.Num() + i;
	}
	int j = _curHistoryIndex - 1;
	if (j < 0)
	{
		j = _originHistory.Num() + j;
	}
	const idVec3& originI = _originHistory[i];
	const idVec3& originJ = _originHistory[j];
	idVec3 vecTraveled = originJ - originI;
	vecTraveled.z = 0.0; // ignore vertical component
	float traveledPrev = vecTraveled.LengthFast();
	traveledPrev /= (_frameHistory[j] - _frameHistory[i]); // normalize to a per-frame value
	return traveledPrev;
}

void MovementSubsystem::CheckBlocked(idAI* owner)
{
	// grayman #2345 - this section was moved up from below, so that it happens
	// even when the AI is standing still. A history of origins should reflect
	// standing still.

	const idVec3& ownerOrigin = owner->GetPhysics()->GetOrigin();
	int prevIndex = _curHistoryIndex-1;
	if (prevIndex < 0)
	{
		prevIndex = _originHistory.Num() - 1;
	}
	_originHistory[_curHistoryIndex] = ownerOrigin;
	_frameHistory[_curHistoryIndex++] = gameLocal.framenum;

	// Wrap the index around if needed
	_curHistoryIndex %= _originHistory.Num();

	// Calculate the new bounds
	_historyBounds.FromPoints(_originHistory.Ptr(), _originHistory.Num());

	// grayman #2345 - end of moved section

	// Check the owner's move type to decide whether 
	// we should watch out for possible blocking or not

	if ((owner->GetMoveType() == MOVETYPE_ANIM) && owner->AI_FORWARD)
	{
		// Owner is supposed to be moving

		// grayman #2345 - Rather than checking _historyBoundsThreshold, check the
		// distance traveled since the previous think. This is a more accurate
		// way to recognize being blocked, since it pays more attention to recent
		// history and less to the past. You might be traveling a reasonable distance,
		// but are you getting closer to your goal? Check current yaw against your ideal yaw
		// to see how far off you are.

		idVec3 prevOrigin = _originHistory[prevIndex];
		int prevFrame = _frameHistory[prevIndex];
		idVec3 currentOrigin = ownerOrigin;
		currentOrigin.z = prevOrigin.z = 0.0; // ignore vertical components
		idVec3 vecTraveled = currentOrigin - prevOrigin;
		float traveledPrev = vecTraveled.LengthFast();
		traveledPrev /= (gameLocal.framenum - prevFrame); // normalize to a per-frame value

		float yawDiff = idMath::AngleNormalize180(owner->GetCurrentYaw() - owner->GetIdealYaw()); // how close are you to your ideal yaw?
		idEntity *tactileEntity = owner->GetTactileEntity(); // grayman #2345

		bool belowThreshold = false;
		bool torsoCustomIdleAnim = false;
		bool legsCustomIdleAnim = false;
		if (traveledPrev < 0.1) // movement is very close to zero
		{
			// grayman #2345 - If you're barely moving or stopped, and there's an idle animation playing,
			// don't consider yourself blocked. You're supposed to be moving, but you probably
			// haven't started yet because of the animation. This happens often around doors.

			idStr torsoString = "Torso_CustomIdleAnim";
			idStr legsString = "Legs_CustomIdleAnim";
			torsoCustomIdleAnim = (torsoString.Cmp(owner->GetAnimState(ANIMCHANNEL_TORSO)) == 0);
			legsCustomIdleAnim = (legsString.Cmp(owner->GetAnimState(ANIMCHANNEL_LEGS)) == 0);

			if (!torsoCustomIdleAnim && !legsCustomIdleAnim)
			{
				belowThreshold = true;
			}
		}
		else if (((traveledPrev < 0.3) || (idMath::Fabs(yawDiff) > 45)) && tactileEntity)  // movement is low or you're not heading toward your goal, and you bumped into something
		{
			belowThreshold = true;
		}

		switch (_state)
		{
		case ENotBlocked:
			if (belowThreshold)
			{
				// Yellow alarm, we might be blocked, or we might as well
				// just have been starting to move
				_state = EPossiblyBlocked;
				// Changed state to possibly blocked, record time
				_lastTimeNotBlocked =  gameLocal.time - gameLocal.msec;
			}
			break;
		case EPossiblyBlocked:
			if (belowThreshold)
			{
				if (gameLocal.time >= _lastTimeNotBlocked + _blockTimeOut)
				{
					// Possibly blocked for too long, raise status
					_state = EBlocked;

					// Send a signal to the current State
					owner->GetMind()->GetState()->OnMovementBlocked(owner);
					_timeBlockStarted =  gameLocal.time - gameLocal.msec;
				}
			}
			else if (!torsoCustomIdleAnim && !legsCustomIdleAnim) // Bounds might not be safe yet if you're doing an idle animation
			{
				// grayman #2669 - go backwards after a small wait
				if (gameLocal.time >= _lastTimeNotBlocked + _blockTimeOut*2)
				{
					_state = ENotBlocked; // Bounds are safe, back to green state
				}
			}
			break;
		case EBlocked:
			if (belowThreshold)
			{
				// grayman #2345 - blocked too long w/o moving?
				if (gameLocal.time >= _timeBlockStarted + _blockTimeShouldEnd)
				{
					// Do something to extricate yourself.
					AttemptToExtricate();
				}
			}
			else if (!torsoCustomIdleAnim && !legsCustomIdleAnim) // Bounds might not be safe yet if you're doing an idle animation
			{
				// grayman #2669 - go backwards after a small wait
				if (gameLocal.time >= _timeBlockStarted + _blockTimeShouldEnd*2)
				{
					_state = EPossiblyBlocked;
					_lastTimeNotBlocked =  gameLocal.time - gameLocal.msec;
				}
			}
			break;
		case EResolvingBlock:
			// nothing so far
			break;
		case EWaitingSolid:		// grayman #2345 - Waiting for passing AI and remaining solid
			break;
		case EWaitingNonSolid:	// grayman #2345 - Waiting for passing AI while non-solid
			break;
		};
	}
	else
	{
		// Not moving, or sleeping, or something else
		if (IsWaiting())
		{
			// do nothing
		}
		else
		{
			_state = ENotBlocked; // grayman #2345
//		_historyBounds.Clear(); // grayman #2345 - don't clear this; let it reflect true recent movement, even if standing still
		}
	}

	DebugDraw(owner);
}

void MovementSubsystem::SetBlockedState(const BlockedState newState)
{
	_state = newState;

	if (_state == ENotBlocked)
	{
		_lastTimeNotBlocked = gameLocal.time;
//		_historyBounds.Clear(); // grayman #2345 - never clear this
	}
}

void MovementSubsystem::SetWaiting(bool solid) // grayman #2345
{
	if (solid)
	{
		_state = EWaitingSolid;
	}
	else
	{
		_state = EWaitingNonSolid;
	}
	
	// If you are handling a door, you have to be taken off
	// that door's queue, so that others can handle the door

	idAI* owner = _owner.GetEntity();
	if (owner->m_HandlingDoor)
	{
		CFrobDoor* frobDoor = owner->GetMemory().doorRelated.currentDoor.GetEntity();
		if (frobDoor != NULL)
		{
			frobDoor->GetUserManager().RemoveUser(owner);
			frobDoor->GetUserManager().ResetMaster(frobDoor); // grayman #2345/#2706 - redefine which AI is the master
		}
	}
}

bool MovementSubsystem::IsWaiting(void) // grayman #2345
{
	return ((_state == EWaitingSolid) || (_state == EWaitingNonSolid));
}

bool MovementSubsystem::IsWaitingSolid(void) // grayman #2345
{
	return (_state == EWaitingSolid);
}

bool MovementSubsystem::IsWaitingNonSolid(void) // grayman #2345
{
	return (_state == EWaitingNonSolid);
}

bool MovementSubsystem::IsNotBlocked(void) // grayman #2345
{
	return (_state == ENotBlocked);
}

void MovementSubsystem::ResolveBlock(idEntity* blockingEnt)
{
	idAI* owner = _owner.GetEntity();
	DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Asking %s to resolve a block by %s\r", owner->name.c_str(),blockingEnt->name.c_str());
	
	if (owner->GetMemory().resolvingMovementBlock || !owner->m_canResolveBlock) // grayman #2345
	{
		return; // Already resolving
	}

	// grayman #2706 - if handling a door, the door handling task will disappear, so clean up first

	if (owner->m_HandlingDoor)
	{
		CFrobDoor* frobDoor = owner->GetMemory().doorRelated.currentDoor.GetEntity();
		if (frobDoor)
		{
			frobDoor->GetUserManager().RemoveUser(owner);
			frobDoor->GetUserManager().ResetMaster(frobDoor); // redefine which AI is the master door user
		}
		owner->m_HandlingDoor = false;
		if (owner->m_RestoreMove) // AI run toward where they saw you last. Don't save that location when handling doors.
		{
			SetBlockedState(EResolvingBlock); // preset this so PopMove() calling RestoreMove() doesn't start handling another door 
			owner->PopMove(); // flush the movement state saved when door handling began
		}
	}

	// Push a resolution task
	PushTask(TaskPtr(new ResolveMovementBlockTask(blockingEnt)));

	// Remember this state
	SetBlockedState(EResolvingBlock);
}

bool MovementSubsystem::IsResolvingBlock()
{
	return _state == EResolvingBlock;
}

idVec3 MovementSubsystem::GetLastMove(void)	// grayman #2356 - used to help determine true falling near func_statics
{
	int prevIndex = _curHistoryIndex-1;
	if (prevIndex < 0)
	{
		prevIndex = _originHistory.Num() - 1;
	}
	const idVec3& prevOrigin = _originHistory[prevIndex];
	const idVec3& ownerOrigin = _owner.GetEntity()->GetPhysics()->GetOrigin();
	return (ownerOrigin - prevOrigin);
}

// Save/Restore methods
void MovementSubsystem::Save(idSaveGame* savefile) const
{
	Subsystem::Save(savefile);

	savefile->WriteBool(_patrolling);

	savefile->WriteInt(_originHistory.Num());

	for (int i = 0; i < _originHistory.Num(); ++i)
	{
		savefile->WriteVec3(_originHistory[i]);
	}

	// grayman #2345 - save frame history

	savefile->WriteInt(_frameHistory.Num());

	for (int i = 0; i < _frameHistory.Num(); i++)
	{
		savefile->WriteInt(_frameHistory[i]);
	}

	savefile->WriteInt(_curHistoryIndex);
	savefile->WriteBounds(_historyBounds);
	savefile->WriteFloat(_historyBoundsThreshold);
	savefile->WriteInt(static_cast<int>(_state));
	savefile->WriteInt(_lastTimeNotBlocked);
	savefile->WriteInt(_blockTimeOut);
	savefile->WriteInt(_timeBlockStarted);		// grayman #2345
	savefile->WriteInt(_blockTimeShouldEnd);	// grayman #2345
	savefile->WriteInt(_pauseTimeOut);			// grayman #2345
}

void MovementSubsystem::Restore(idRestoreGame* savefile)
{
	Subsystem::Restore(savefile);

	savefile->ReadBool(_patrolling);

	int num;
	savefile->ReadInt(num);

	_originHistory.SetNum(num);

	for (int i = 0; i < num; ++i)
	{
		savefile->ReadVec3(_originHistory[i]);
	}

	// grayman #2345 - restore frame history

	savefile->ReadInt(num);
	_frameHistory.SetNum(num);

	for (int i = 0; i < num; i++)
	{
		savefile->ReadInt(_frameHistory[i]);
	}

	savefile->ReadInt(_curHistoryIndex);
	savefile->ReadBounds(_historyBounds);
	savefile->ReadFloat(_historyBoundsThreshold);

	int temp;
	savefile->ReadInt(temp);
	assert(temp >= ENotBlocked && temp <= ENumBlockedStates); // grayman #2345 - was EBlocked, which isn't the last value in the enum
	_state = static_cast<BlockedState>(temp);

	savefile->ReadInt(_lastTimeNotBlocked);
	savefile->ReadInt(_blockTimeOut);
	savefile->ReadInt(_timeBlockStarted);		// grayman #2345
	savefile->ReadInt(_blockTimeShouldEnd);		// grayman #2345
	savefile->ReadInt(_pauseTimeOut);			// grayman #2345
}

void MovementSubsystem::DebugDraw(idAI* owner)
{
	if (!cv_ai_debug_blocked.GetBool())
	{
		return;
	}

	if (!_historyBounds.IsCleared())
	{
		gameRenderWorld->DebugBox(colorWhite, idBox(_historyBounds), 3* gameLocal.msec);
	}

	idStr str;
	idVec4 colour;
	switch (_state) 
	{
		case ENotBlocked:
			str = "ENotBlocked";
			colour = colorGreen;
			break;
		case EPossiblyBlocked:
			str = "EPossiblyBlocked";
			colour = colorYellow;
			break;
		case EBlocked:
			str = "EBlocked";
			colour = colorRed;
			break;
		case EResolvingBlock:
			str = "EResolvingBlock";
			colour = colorMagenta;
			break;
		case EWaitingSolid: // grayman #2345
			str = "EWaitingSolid";
			colour = colorWhite;
			break;
		case EWaitingNonSolid: // grayman #2345
			str = "EWaitingNonSolid";
			colour = colorBlue;
			break;
	}

	gameRenderWorld->DrawText(str.c_str(), 
		(owner->GetEyePosition() - owner->GetPhysics()->GetGravityNormal()*60.0f), 
		0.25f, colour, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, 3 * gameLocal.msec);
	}

} // namespace ai
