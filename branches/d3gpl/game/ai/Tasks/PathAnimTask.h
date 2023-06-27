/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3278 $
 * $Date: 2009-03-20 15:53:12 -0400 (Fri, 20 Mar 2009) $
 * $Author: angua $
 *
 ***************************************************************************/

#ifndef __AI_PATH_ANIM_TASK_H__
#define __AI_PATH_ANIM_TASK_H__

#include "PathTask.h"

namespace ai
{

// Define the name of this task
#define TASK_PATH_ANIM "PathAnim"

class PathAnimTask;
typedef boost::shared_ptr<PathAnimTask> PathAnimTaskPtr;

class PathAnimTask :
	public PathTask
{
private:
	// Private constructor
	PathAnimTask();

public:
	PathAnimTask(idPathCorner* path);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	virtual void OnFinish(idAI* owner);

	// Creates a new Instance of this task
	static PathAnimTaskPtr CreateInstance();

};

} // namespace ai

#endif /* __AI_PATH_ANIM_TASK_H__ */
