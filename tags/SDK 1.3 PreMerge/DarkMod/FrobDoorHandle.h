/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 17:25:02 -0400 (Fri, 23 Mar 2007) $
 * $Author: sparhawk $
 *
 ***************************************************************************/

// Copyright (C) 2004 Gerhard W. Gruber <sparhawk@gmx.at>
//

#ifndef FROBDOORHANDLE_H
#define FROBDOORHANDLE_H

class CFrobDoor;

/**
 * CFrobDoorHandle is the complement for CFrobDoors. This is
 * quite similar to the doors itself, because they are attached
 * to it. If a handle is frobbed, instead of the actual door,
 * all calls are basically forwarded to it's door, so that the 
 * player doesn't see a difference. From the player perspective
 * the handle acts the same as the door.
 */
class CFrobDoorHandle : public CBinaryFrobMover {
public:
	friend CFrobDoor;

	CLASS_PROTOTYPE( CFrobDoorHandle );

							CFrobDoorHandle( void );

	void					Spawn( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );

	/**
	 * Get the door that is currently associated with this handle.
	 */
	CFrobDoor				*GetDoor(void);
	void					Event_GetDoor(void);
	void					Event_Tap(void);

	/**
	 * Find the door, that is associated with this handle.
	 */
	CFrobDoor				*FindDoor(idStr &Doorname);

	/**
	 * Functions that must be forwarded to the door.
	 */
	void					SetFrobbed(bool val);
	bool					IsFrobbed(void);
	bool					UsedBy(idEntity *);
	void					FrobAction(bool bMaster);

	// These functions need to be disabled on the handle. Therefore
	// they are provided but empty.
	void					ClosePortal(void);
	void					DoneStateChange(void);
	void					DoneRotating(void);
	void					DoneMoving(void);

	void					Tap(void);

protected:
	/**
	* Pointer to the door that is associated with this handle
	**/
	CFrobDoor				*m_Door;
	bool					m_FrobLock;
	idStr					m_DoorHandleScript;

private:
};

#endif /* !TDMDOORHANDLE_H */
