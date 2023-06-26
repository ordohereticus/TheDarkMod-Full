/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3278 $
 * $Date: 2009-03-20 15:53:12 -0400 (Fri, 20 Mar 2009) $
 * $Author: angua $
 *
 ***************************************************************************/

#ifndef __AI_PATH_SHOW_TASK_H__
#define __AI_PATH_SHOW_TASK_H__

#include "PathTask.h"

namespace ai
{

// Define the name of this task
#define TASK_PATH_SHOW "PathShow"

class PathShowTask;
typedef boost::shared_ptr<PathShowTask> PathShowTaskPtr;

class PathShowTask :
	public PathTask
{
private:
	PathShowTask();
public:
	PathShowTask(idPathCorner* path);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	// Creates a new Instance of this task
	static PathShowTaskPtr CreateInstance();
};

} // namespace ai

#endif /* __AI_PATH_SHOW_TASK_H__ */
