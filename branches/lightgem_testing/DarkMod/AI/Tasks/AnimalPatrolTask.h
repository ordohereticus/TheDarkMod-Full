/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4472 $
 * $Date: 2011-01-24 20:49:21 -0500 (Mon, 24 Jan 2011) $
 * $Author: grayman $
 *
 ***************************************************************************/

#ifndef __AI_ANIMAL_PATROL_TASK_H__
#define __AI_ANIMAL_PATROL_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_ANIMAL_PATROL "AnimalPatrol"

class AnimalPatrolTask;
typedef boost::shared_ptr<AnimalPatrolTask> AnimalPatrolTaskPtr;

class AnimalPatrolTask :
	public Task
{
	// greebo: These are the various states the animal is in
	// It's a basic set of actions, chosen randomly, repeating
	enum EState
	{
		stateNone,
		stateMovingToNextSpot,
		stateMovingToNextPathCorner,
		stateDoingSomething,
		stateWaiting,
		statePreMovingToNextSpot, // grayman #2356 - no path corners, go elsewhere
		stateCount,
	} _state;

	// For waiting state
	int _waitEndTime;

	// grayman #2356 - for 'move to position' state
	int _moveEndTime;

	// Private constructor
	AnimalPatrolTask();

public:
	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static AnimalPatrolTaskPtr CreateInstance();

private:
	// Helper methods, corresponding to the EState enum
	void chooseNewState(idAI* owner);

	void movingToNextSpot(idAI* owner);
	void movingToNextPathCorner(idAI* owner);
	void waiting(idAI* owner);

	void switchToState(EState newState, idAI* owner);
};

} // namespace ai

#endif /* __AI_ANIMAL_PATROL_TASK_H__ */
