/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4687 $
 * $Date: 2011-03-10 11:29:30 -0500 (Thu, 10 Mar 2011) $
 * $Author: grayman $
 *
 ***************************************************************************/

#ifndef __AI_MOVEMENTSUBSYSTEM_H__
#define __AI_MOVEMENTSUBSYSTEM_H__

#include <boost/shared_ptr.hpp>
#include <list>

#include "Tasks/Task.h"
#include "Subsystem.h"
#include "Queue.h"

namespace ai
{

class MovementSubsystem :
	public Subsystem 
{
public:
	enum BlockedState
	{
		ENotBlocked,		// moving along
		EPossiblyBlocked,	// might be blocked, watching...
		EBlocked,			// not been making progress for too long
		EResolvingBlock,	// resolving block
		EWaitingSolid,		// grayman #2345 - waiting while non-solid for an AI to pass by
		EWaitingNonSolid,	// grayman #2345 - waiting while solid for an AI to pass by
		ENumBlockedStates,	// grayman #2345 - invalid index - this always has to be the last in the list
	};

protected:
	bool _patrolling;

	// The origin history, contains the origin position of the last few frames
	idList<idVec3> _originHistory;

	// grayman #2345 - The frame history, contains the frames where origins were recorded
	idList<int> _frameHistory;

	// Currently active list index
	int _curHistoryIndex;

	// These bounds contain all origin locations of the last few frames
	idBounds _historyBounds;

	// Mininum radius the bounds need to have, otherwise state is raised to EPossiblyBlocked
	float _historyBoundsThreshold;

	BlockedState _state;

	int _lastTimeNotBlocked;

	// The amount of time allowed to pass during EPossiblyBlocked before state is set to EBlocked 
	int _blockTimeOut;
	
	int _timeBlockStarted;		// grayman #2345 - When a block started 
	int _blockTimeShouldEnd;	// grayman #2345 - The amount of time allowed to pass during EBlocked before trying to extricate yourself
	int _timePauseStarted;		// grayman #2345 - when a treadmill pause started
	int _pauseTimeOut;			// grayman #2345 - amount of time to pause after treadmilling

public:
	MovementSubsystem(SubsystemId subsystemId, idAI* owner);

	// Called regularly by the Mind to run the currently assigned routine.
	// @returns: TRUE if the subsystem is enabled and the task was performed, 
	// @returns: FALSE if the subsystem is disabled and nothing happened.
	virtual bool PerformTask();

	void StartPatrol();
	void RestartPatrol();

	void Patrol();

	virtual void StartPathTask();

	virtual void NextPath();

	virtual void ClearTasks();


	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Returns the current "blocked" state
	BlockedState GetBlockedState() const
	{
		return _state;
	}

	void SetBlockedState(const BlockedState newState);

	void ResolveBlock(idEntity* blockingEnt);

	bool IsResolvingBlock();

	void SetWaiting(bool solid);	// grayman #2345

	bool IsWaiting();		// grayman #2345

	bool IsWaitingSolid();	// grayman #2345

	bool IsWaitingNonSolid(); // grayman #2345

	bool IsNotBlocked();	// grayman #2345

	idVec3 GetLastMove();	// grayman #2356

	float GetPrevTraveled(); // grayman #2345

	/**
	 * grayman #2345 - Called when the AI tries to extricate itself from a stuck position
	 */
	bool AttemptToExtricate();

protected:
	virtual void CheckBlocked(idAI* owner);

	// Returns the next actual path_corner entity, or NULL if nothing found or stuck in loops/dead ends
	idPathCorner* GetNextPathCorner(idPathCorner* curPath, idAI* owner);

private:
	void DebugDraw(idAI* owner);
};
typedef boost::shared_ptr<MovementSubsystem> MovementSubsystemPtr;

} // namespace ai

#endif /* __AI_MOVEMENTSUBSYSTEM_H__ */
