/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_SINGLE_BARK_TASK_H__
#define __AI_SINGLE_BARK_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_SINGLE_BARK "SingleBark"

class SingleBarkTask;
typedef boost::shared_ptr<SingleBarkTask> SingleBarkTaskPtr;

class SingleBarkTask :
	public Task
{
	// The name of the sound to be played
	idStr _soundName;
	int _endTime;

	// Default constructor
	SingleBarkTask();

public:
	// Constructor taking a sound name as argument
	SingleBarkTask(const idStr& soundName);

	// Get the name of this task
	virtual const idStr& GetName() const;

	virtual void Init(idAI* owner, Subsystem& subsystem);
	virtual bool Perform(Subsystem& subsystem);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static SingleBarkTaskPtr CreateInstance();

	// Class-specific methods
	virtual void SetSound(const idStr& soundName);
};

} // namespace ai

#endif /* __AI_SINGLE_BARK_TASK_H__ */
