/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_MOVE_TO_POSITION_H__
#define __AI_MOVE_TO_POSITION_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_MOVE_TO_POSITION "MoveToPosition"

class MoveToPositionTask;
typedef boost::shared_ptr<MoveToPositionTask> MoveToPositionTaskPtr;

class MoveToPositionTask :
	public Task
{
private:

	// The target position
	idVec3 _targetPosition;

	// Target yaw (is not INFINITY if set)
	float _targetYaw;

	// Default constructor
	MoveToPositionTask();

public:
	// Constructor taking the target position (and optional target yaw) as input argument
	MoveToPositionTask(const idVec3 targetPosition, float targetYaw = idMath::INFINITY);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	void SetPosition(idVec3 targetPosition);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static MoveToPositionTaskPtr CreateInstance();
};

} // namespace ai

#endif /* __AI_MOVE_TO_POSITION_H__ */
