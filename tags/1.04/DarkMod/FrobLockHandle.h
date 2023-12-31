/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3302 $
 * $Date: 2009-03-25 11:22:51 -0400 (Wed, 25 Mar 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _FROB_LOCK_HANDLE_H
#define _FROB_LOCK_HANDLE_H

#include "FrobHandle.h"

class CFrobLock;

/**
 * CFrobLockHandle is meant to be used as moveable part of a CFrobLock.
 * It behaves similarly to the CFrobDoorHandle class, but is specialised for 
 * use with the static froblock.
 */
class CFrobLockHandle : 
	public CFrobHandle
{
public:
	CLASS_PROTOTYPE( CFrobLockHandle );

							CFrobLockHandle();

	void					Spawn();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	/**
	 * greebo: This method is invoked directly or it gets called by the attached master.
	 *
	 * Overrides CFrobHandle::Tap()
	 */
	virtual void			Tap();

	/**
	 * Get/set the lock associated with this handle.
	 */
	CFrobLock*				GetFrobLock();
	void					SetFrobLock(CFrobLock* lock);

	// greebo: Returns TRUE if the associated lock is locked (not to confuse with CBinaryFrobMover::IsLocked())
	bool					LockIsLocked();

protected:
	// Specialise the OpenPositionReached method of BinaryFrobMover to trigger the lock's open event
	virtual void			OnOpenPositionReached();

	// Script event interface
	void					Event_GetLock();

protected:
	/**
	* Pointer to the lock that is associated with this handle
	**/
	CFrobLock*				m_FrobLock;
};

#endif /* _FROB_LOCK_HANDLE_H */
