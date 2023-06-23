/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_IDLE_ANIMATION_TASK_H__
#define __AI_IDLE_ANIMATION_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_IDLE_ANIMATION "IdleAnimation"

class IdleAnimationTask;
typedef boost::shared_ptr<IdleAnimationTask> IdleAnimationTaskPtr;

class IdleAnimationTask :
	public Task
{
	int _nextAnimationTime;

	idList<idStr> _idleAnimations;
	idList<idStr> _idleAnimationsTorso;
	int _idleAnimationInterval;

	// Default constructor is private
	IdleAnimationTask();
public:
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
	static IdleAnimationTaskPtr CreateInstance();
};

} // namespace ai

#endif /* __AI_IDLE_ANIMATION_TASK_H__ */
