/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_PATH_TURN_TASK_H__
#define __AI_PATH_TURN_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_PATH_TURN "PathTurn"

class PathTurnTask;
typedef boost::shared_ptr<PathTurnTask> PathTurnTaskPtr;

class PathTurnTask :
	public Task
{
	idEntityPtr<idPathCorner> _path;

	PathTurnTask();

public:
	PathTurnTask(idPathCorner* path);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static PathTurnTaskPtr CreateInstance();

	// Class-specific methods
	virtual void SetTargetEntity(idPathCorner* path);
};

} // namespace ai

#endif /* __AI_PATH_TURN_TASK_H__ */
