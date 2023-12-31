/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2829 $
 * $Date: 2008-09-13 14:08:37 -0400 (Sat, 13 Sep 2008) $
 * $Author: angua $
 *
 ***************************************************************************/

#ifndef __AI_PATH_INTERACT_TASK_H__
#define __AI_PATH_INTERACT_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_PATH_INTERACT "PathInteract"

class PathInteractTask;
typedef boost::shared_ptr<PathInteractTask> PathInteractTaskPtr;

class PathInteractTask :
	public Task
{
	idEntityPtr<idPathCorner> _path;

	idEntity* _target;

	int _waitEndTime;

	PathInteractTask();

public:
	PathInteractTask(idPathCorner* path);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	void OnFinish(idAI* owner);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static PathInteractTaskPtr CreateInstance();
};

} // namespace ai

#endif /* __AI_PATH_INTERACT_TASK_H__ */
