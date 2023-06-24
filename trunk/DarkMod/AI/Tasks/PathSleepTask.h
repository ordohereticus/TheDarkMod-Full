/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3731 $
 * $Date: 2009-10-29 03:50:38 -0400 (Thu, 29 Oct 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_PATH_SLEEP_TASK_H__
#define __AI_PATH_SLEEP_TASK_H__

#include "PathTask.h"

namespace ai
{

// Define the name of this task
#define TASK_PATH_SLEEP "PathSleep"

class PathSleepTask;
typedef boost::shared_ptr<PathSleepTask> PathSleepTaskPtr;

class PathSleepTask :
	public PathTask
{
private:
	// Private constructor
	PathSleepTask();

public:
	PathSleepTask(idPathCorner* path);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	virtual void OnFinish(idAI* owner);

	// Creates a new Instance of this task
	static PathSleepTaskPtr CreateInstance();

};

} // namespace ai

#endif /* __AI_PATH_SLEEP_TASK_H__ */
