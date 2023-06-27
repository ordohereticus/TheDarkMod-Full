/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3428 $
 * $Date: 2009-05-07 13:34:08 -0400 (Thu, 07 May 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_COMM_WAIT_TASK_H__
#define __AI_COMM_WAIT_TASK_H__

#include "CommunicationTask.h"

namespace ai
{

// Define the name of this task
#define TASK_COMM_WAIT "CommWaitTask"

class CommWaitTask;
typedef boost::shared_ptr<CommWaitTask> CommWaitTaskPtr;

// A simple silent task, causes the AI to shut up for a while
class CommWaitTask :
	public CommunicationTask
{
	int _duration;
	int _endTime;

	// Default constructor
	CommWaitTask();

public:
	// Constructor: pass the duration of the silence
	CommWaitTask(int duration, int priority = 0);

	// Get the name of this task
	virtual const idStr& GetName() const;

	virtual void Init(idAI* owner, Subsystem& subsystem);
	virtual bool Perform(Subsystem& subsystem);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static CommWaitTaskPtr CreateInstance();
};

} // namespace ai

#endif /* __AI_COMM_WAIT_TASK_H__ */
