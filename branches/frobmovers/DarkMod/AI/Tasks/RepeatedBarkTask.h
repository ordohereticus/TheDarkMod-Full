/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_REPEATED_BARK_TASK_H__
#define __AI_REPEATED_BARK_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_REPEATED_BARK "RepeatedBark"

class RepeatedBarkTask;
typedef boost::shared_ptr<RepeatedBarkTask> RepeatedBarkTaskPtr;

class RepeatedBarkTask :
	public Task
{
private:
	// Corresponds to AI spawnarg "bark_repeat_patrol"
	idStr _soundName;
	int _barkRepeatIntervalMin;
	int _barkRepeatIntervalMax;
	int _nextBarkTime;

	// Default Constructor
	RepeatedBarkTask();

public:
	/**
	 * greebo: Pass the sound shader name plus the interval range in milliseconds
	 */
	RepeatedBarkTask(const idStr& soundName, int barkRepeatIntervalMin, int barkRepeatIntervalMax);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static RepeatedBarkTaskPtr CreateInstance();
};

} // namespace ai

#endif /* __AI_REPEATED_BARK_TASK_H__ */
