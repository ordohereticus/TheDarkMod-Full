/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3767 $
 * $Date: 2009-12-01 00:17:07 -0500 (Tue, 01 Dec 2009) $
 * $Author: angua $
 *
 ***************************************************************************/

#ifndef __AI_PATH_TASK_H__
#define __AI_PATH_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_PATH "Path"

class PathTask;
typedef boost::shared_ptr<PathTask> PathTaskPtr;

class PathTask :
	public Task
{
protected:
	idEntityPtr<idPathCorner> _path;
	float _accuracy;

	PathTask();

public:
	PathTask(idPathCorner* path);

	// Get the name of this task
	virtual const idStr& GetName() const = 0;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem) = 0;

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);


	// Class-specific methods
	virtual void SetTargetEntity(idPathCorner* path);
};

} // namespace ai

#endif /* __AI_PATH_TASK_H__ */
