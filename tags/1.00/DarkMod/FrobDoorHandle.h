/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3302 $
 * $Date: 2009-03-25 11:22:51 -0400 (Wed, 25 Mar 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Gerhard W. Gruber <sparhawk@gmx.at>
//

#ifndef FROBDOORHANDLE_H
#define FROBDOORHANDLE_H

#include "FrobHandle.h"

class CFrobDoor;

/**
 * CFrobDoorHandle is the complement for CFrobDoors, it specialises
 * the generic base class CFrobHandle.
 *
 * Basically, if a handle is frobbed instead of the actual door,
 * all calls are forwarded to its door, so that the player doesn't 
 * notice the difference. From the player's perspective frobbing
 * the handle feels the same as frobbing the door.
 *
 * When frobbing a door with a handle attached, the event chain is like this:
 * Frob > Door::Open > Handle::Tap > Handle moves to open pos > Door::OpenDoor
 */
class CFrobDoorHandle : 
	public CFrobHandle
{
public:
	CLASS_PROTOTYPE( CFrobDoorHandle );

							CFrobDoorHandle();

	void					Spawn();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	/**
	 * greebo: This method is invoked directly or it gets called by the attached master.
	 * For instance, a call to CFrobDoor::Open() gets re-routed here first to let 
	 * the handle animation play before actually trying to open the door.
	 *
	 * The Tap() algorithm attempts to rotate the door handle down until and 
	 * calls OpenDoor() when the handle reaches its end rotation/position.
	 *
	 * Overrides CFrobHandle::Tap()
	 */
	virtual void			Tap();

	/**
	 * Get/set the door associated with this handle.
	 */
	CFrobDoor*				GetDoor();
	void					SetDoor(CFrobDoor* door);

	// greebo: Returns TRUE if the associated door is locked (not to confuse with CBinaryFrobMover::IsLocked())
	bool					DoorIsLocked();

protected:
	// Specialise the OpenPositionReached method of BinaryFrobMover to trigger the door's Open() routine
	virtual void			OnOpenPositionReached();

	// Script event interface
	void					Event_GetDoor();

protected:
	/**
	* Pointer to the door that is associated with this handle
	**/
	CFrobDoor*				m_Door;
};

#endif /* FROBDOORHANDLE_H */
