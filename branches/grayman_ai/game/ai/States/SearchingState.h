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

#ifndef __AI_SEARCHING_STATE_H__
#define __AI_SEARCHING_STATE_H__

#include "State.h"

/**
* greebo: A SearchingState is handling the AI's search routines.
* 
* The routine needs "memory.alertPos" to be set as prerequisite.
*
* The boolean variable "memory.stimulusLocationItselfShouldBeSearched" can be used
* to let the AI the "memory.alertPos" position as first hiding spot.
* If the boolean is not set to TRUE, the hiding spot search is based around memory.alertPos.
*
* The actual hiding spot search algorithm is called over multiple
* frames. Once finished, the AI can use its results (unless the 
* stimulusLocationItselfShouldBeSearched bool is set to TRUE, then alertPos is used as
* first hiding spot right away.
*
* For each hiding spot, an InvestigateSpotTask is invoked which takes care of the details.
*/

namespace ai
{

#define STATE_SEARCHING "Searching"

class SearchingState :
	public State
{
public:
	// Get the name of this state
	virtual const idStr& GetName() const;

	// This is called when the state is first attached to the AI's Mind.
	virtual void Init(idAI* owner);

	// Gets called each time the mind is thinking
	virtual void Think(idAI* owner);

	// Incoming events issued by the Subsystems
	virtual void OnSubsystemTaskFinished(idAI* owner, SubsystemId subSystem);

	// greebo: Gets called when the AI is alerted by a suspicious sound (override)
	virtual void OnAudioAlert();

	static StatePtr CreateInstance();

protected:
	// Override base class method
	virtual bool CheckAlertLevel(idAI* owner);

	/**
	 * This method is used to start a new hiding spot search. Any existing search in progress is replaced.
	 */
	virtual void StartNewHidingSpotSearch(idAI* owner);

	// This is called each frame to complete a multiframe hiding spot search
	virtual void PerformHidingSpotSearch(idAI* owner);

	// Gets called when a new hiding spot should be acquired for searching.
	// Stores the result in the AI's Memory (hiding spot indices)
	// return TRUE when a hiding spot is available, FALSE if not.
	virtual bool ChooseNextHidingSpotToSearch(idAI* owner);
};

} // namespace ai

#endif /* __AI_SEARCHING_STATE_H__ */
