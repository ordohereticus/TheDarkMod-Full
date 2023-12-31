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

#ifndef __AI_INVESTIGATE_SPOT_TASK_H__
#define __AI_INVESTIGATE_SPOT_TASK_H__

#include "Task.h"

namespace ai
{

/**
* greebo: This task requires memory.currentSearchSpot to be valid.
* 
* This task is intended to be pushed into the action Subsystem and
* performs single-handedly how the given hiding spot should be handled.
*
* Note: This Task employs the Movement Subsystem when the algorithm
* judges to walk/run over to the given search spot.
**/

// Define the name of this task
#define TASK_INVESTIGATE_SPOT "InvestigateSpot"

class InvestigateSpotTask;
typedef boost::shared_ptr<InvestigateSpotTask> InvestigateSpotTaskPtr;

class InvestigateSpotTask :
	public Task
{
	// The search spot to investigate
	idVec3 _searchSpot;

	// The time this task may exit
	int _exitTime;

	// Set to TRUE, if the AI should investigate the spot very closely
	// usually by playing the kneel_down animation.
	bool _investigateClosely;

	// Whether this task has told the AI to actually move to the searchspot
	bool _moveInitiated;

	// Private default constructor
	InvestigateSpotTask();
public:
	// @param: see member _investigateClosely
	InvestigateSpotTask(bool investigateClosely);

	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	/** 
	 * greebo: Sets a new goal position for this task.
	 *
	 * @newPos: The new position
	 */
	virtual void SetNewGoal(const idVec3& newPos);

	/** 
	 * greebo: Sets the "should investigate closely" flag.
	 */
	virtual void SetInvestigateClosely(bool closely);

	virtual void OnFinish(idAI* owner); // grayman #2560

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static InvestigateSpotTaskPtr CreateInstance();
};

} // namespace ai

#endif /* __AI_INVESTIGATE_SPOT_TASK_H__ */
