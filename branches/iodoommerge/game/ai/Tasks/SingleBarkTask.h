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
