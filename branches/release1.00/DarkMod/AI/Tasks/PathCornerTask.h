/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3689 $
 * $Date: 2009-09-03 08:40:05 -0400 (Thu, 03 Sep 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_PATH_CORNER_TASK_H__
#define __AI_PATH_CORNER_TASK_H__

#include "PathTask.h"

namespace ai
{

// Define the name of this task
#define TASK_PATH_CORNER "PathCorner"

class PathCornerTask;
typedef boost::shared_ptr<PathCornerTask> PathCornerTaskPtr;

class PathCornerTask :
	public PathTask
{
private:
	bool _moveInitiated;

	// Position last time this task was executed, used for path prediction
	idVec3 _lastPosition;

	// Frame this task was last executed
	int _lastFrameNum;

	// Whether to anticipate the AI reaching path corners
	bool _usePathPrediction;

	PathCornerTask();

public:
	PathCornerTask(idPathCorner* path);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static PathCornerTaskPtr CreateInstance();

};

} // namespace ai

#endif /* __AI_PATH_CORNER_TASK_H__ */
