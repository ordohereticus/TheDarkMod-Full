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

#ifndef __AI_HANDLE_ELEVATOR_TASK_H__
#define __AI_HANDLE_ELEVATOR_TASK_H__

#include "Task.h"
#include "../EAS/RouteInfo.h"
#include "../../BinaryFrobMover.h"

namespace ai
{

// Define the name of this task
#define TASK_HANDLE_ELEVATOR "HandleElevator"

class HandleElevatorTask;
typedef boost::shared_ptr<HandleElevatorTask> HandleElevatorTaskPtr;

class HandleElevatorTask :
	public Task
{
private:
	enum State
	{
		EMovingTowardsStation,
		EInitiateMoveToFetchButton,
		EMovingToFetchButton,
		EPressFetchButton,
		EWaitForElevator,
		EMoveOntoElevator,
		EInitiateMoveToRideButton,
		EMovingToRideButton,
		EPressRideButton,
		ERideOnElevator,
		EGetOffElevator,
		ENumStates,
	} _state;

	int _waitEndTime;

	// The actual route info structure
	eas::RouteInfo _routeInfo;

	// Is TRUE if this task has finished successfully
	bool _success;
	
	// Private constructor
	HandleElevatorTask();
public:

	HandleElevatorTask(const eas::RouteInfoPtr& routeInfo);
	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	virtual void OnFinish(idAI* owner);

	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static HandleElevatorTaskPtr CreateInstance();

private:
	// Checks if the elevator station is reachable, returns TRUE if this is the case
	bool IsElevatorStationReachable(CMultiStateMoverPosition* pos);

	void DebugDraw(idAI* owner);

	// Lets the AI move towards the position entity (is slightly more complicated than just idAI::MoveToPos)
	bool MoveToPositionEntity(idAI* owner, CMultiStateMoverPosition* pos);

	// Lets the Ai move to the button
	bool MoveToButton(idAI* owner, CMultiStateMoverButton* button);

};

} // namespace ai

#endif /* __AI_HANDLE_ELEVATOR_TASK_H__ */
