/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3354 $
 * $Date: 2009-04-04 07:41:43 -0400 (Sat, 04 Apr 2009) $
 * $Author: angua $
 *
 ***************************************************************************/

#ifndef __AI_SINGLE_BARK_TASK_H__
#define __AI_SINGLE_BARK_TASK_H__

#include "CommunicationTask.h"
#include "../../AIComm_Message.h"

namespace ai
{

// Define the name of this task
#define TASK_SINGLE_BARK "SingleBark"

class SingleBarkTask;
typedef boost::shared_ptr<SingleBarkTask> SingleBarkTaskPtr;

class SingleBarkTask :
	public CommunicationTask
{
	int _endTime;

	// The message which should be delivered when barking
	CommMessagePtr _message;

	// Default constructor
	SingleBarkTask();

public:
	// Constructor taking a sound name as argument
	SingleBarkTask(const idStr& soundName, const CommMessagePtr& message = CommMessagePtr());

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
