/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5121 $ (Revision of last commit) 
 $Date: 2011-12-11 14:12:26 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#ifndef __AI_TASK_H__
#define __AI_TASK_H__

#include <boost/shared_ptr.hpp>

namespace ai
{

// Forward declaration
class Subsystem;

/**
 * greebo: This is the basic declaration of a Task.
 * 
 * Tasks are attached to a subsystem, which act as "slots". 
 * Multiple tasks can be attached to each subsystem, which manages
 * them in some sort of queue. Only one Task can be executed at once
 * for each subsystem.
 *
 * A task needs to have a unique name and a Perform() method.
 */
class Task
{
protected:
	// Each task has an owning entity
	idEntityPtr<idAI> _owner;

	// true if this task has been initialised
	bool _initialised;

	// Protected constructor, only subclasses may call this
	Task() :
		_initialised(false)
	{}

public:
	
	// Get the name of this task
	virtual const idStr& GetName() const = 0;

	// Performs the task, whatever this may be
	// Returns TRUE if the task is finished, FALSE if it should continue.
	virtual bool Perform(Subsystem& subsystem) = 0;

	// Let the task perform some initialisation. This is called
	// right after the task is installed into the subsystem.
	virtual void Init(idAI* owner, Subsystem& subsystem)
	{
		_owner = owner;
		_initialised = true;
	}

	/**
	 * greebo: OnFinish gets called when the Task is regularly (by returning true)
	 *         or forcedly (by being removed from the queue / cleartasks / etc.)
	 *         This gives the task the opportunity to react/cleanup.
	 *
	 * Note: OnFinish MUST NOT alter the Subsystem, only perform cleanup taks
	 *       affecting the Task itself or the owning AI.
	 */
	virtual void OnFinish(idAI* owner)
	{} // empty default implementation

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const
	{
		_owner.Save(savefile);
		savefile->WriteBool(_initialised);
	}

	virtual void Restore(idRestoreGame* savefile)
	{
		_owner.Restore(savefile);
		savefile->ReadBool(_initialised);
	}

	virtual bool IsInitialised()
	{
		return _initialised;
	}

	// grayman #2706 - can we abort this task? override in task instances
	virtual bool CanAbort()
	{
		return true;
	}
};
typedef boost::shared_ptr<Task> TaskPtr;

} // namespace ai

#endif /* __AI_TASK_H__ */
