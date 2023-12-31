/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3575 $
 * $Date: 2009-07-24 02:57:08 -0400 (Fri, 24 Jul 2009) $
 * $Author: greebo $
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
protected:
	int _startDelay;

	int _endTime;

	// The message which should be delivered when barking
	CommMessagePtr _message;

	// Default constructor
	SingleBarkTask();

public:
	// Constructor taking a sound name as argument
	// Optional arguments are the message to deliver
	// and the time to pass in ms before the bark should be played
	SingleBarkTask(const idStr& soundName, 
				   const CommMessagePtr& message = CommMessagePtr(),
				   int startDelayMS = 0);

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
	virtual void SetMessage(const CommMessagePtr& message);
};

} // namespace ai

#endif /* __AI_SINGLE_BARK_TASK_H__ */
