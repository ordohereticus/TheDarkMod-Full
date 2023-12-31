/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3092 $
 * $Date: 2008-12-28 16:26:07 -0500 (Sun, 28 Dec 2008) $
 * $Author: angua $
 *
 ***************************************************************************/

#ifndef __AI_PATH_LOOKAT_TASK_H__
#define __AI_PATH_LOOKAT_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_PATH_LOOKAT "PathLookat"

class PathLookatTask;
typedef boost::shared_ptr<PathLookatTask> PathLookatTaskPtr;

class PathLookatTask :
	public Task
{
	idEntityPtr<idPathCorner> _path;

	PathLookatTask();

	idEntity* _focusEnt;
	float _duration;

public:
	PathLookatTask(idPathCorner* path);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static PathLookatTaskPtr CreateInstance();

	// Class-specific methods
	virtual void SetTargetEntity(idPathCorner* path);
};

} // namespace ai

#endif /* __AI_PATH_LOOKAT_TASK_H__ */
