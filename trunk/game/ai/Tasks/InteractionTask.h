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

#ifndef __AI_INTERACTION_TASK_H__
#define __AI_INTERACTION_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_INTERACTION "Interaction"

class InteractionTask;
typedef boost::shared_ptr<InteractionTask> InteractionTaskPtr;

class InteractionTask :
	public Task
{
	idEntity* _interactEnt;

	int _waitEndTime;

	InteractionTask();

public:
	InteractionTask(idEntity* interactEnt);

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
	static InteractionTaskPtr CreateInstance();
};

} // namespace ai

#endif /* __AI_INTERACTION_TASK_H__ */
