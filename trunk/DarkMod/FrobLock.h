/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3295 $
 * $Date: 2009-03-25 04:09:01 -0400 (Wed, 25 Mar 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _FROB_LOCK_H_
#define _FROB_LOCK_H_

#include "PickableLock.h"

/** 
 * greebo: This class represents a pickable lock. It supports
 * attachment of BinaryFrobMovers which are used as levers.
 */
class CFrobLock :
	public idStaticEntity
{
	// The actual lock implementation
	PickableLock	m_Lock;

public:
	CLASS_PROTOTYPE( CFrobLock );

	void	Spawn();

	bool	IsLocked();
	bool	IsPickable();

	void	Save(idSaveGame *savefile) const;
	void	Restore(idRestoreGame *savefile);

protected:
	void	PostSpawn();

};

#endif /* _FROB_LOCK_H_ */
