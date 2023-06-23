/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2691 $
 * $Date: 2008-07-18 10:35:43 -0400 (Fri, 18 Jul 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_PLAY_ANIMATION_TASK_H__
#define __AI_PLAY_ANIMATION_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_PLAY_ANIMATION "PlayAnimation"

class PlayAnimationTask;
typedef boost::shared_ptr<PlayAnimationTask> PlayAnimationTaskPtr;

class PlayAnimationTask :
	public Task
{
	idStr _animName;

	// Private constructor
	PlayAnimationTask();

public:
	// Pass the animation name directly here (like "idle_armwipe")
	PlayAnimationTask(const idStr& animName);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	virtual void OnFinish(idAI* owner);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static PlayAnimationTaskPtr CreateInstance();
};

} // namespace ai

#endif /* __AI_PLAY_ANIMATION_TASK_H__ */
