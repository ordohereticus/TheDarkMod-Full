/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3292 $
 * $Date: 2009-03-24 11:19:37 -0400 (Tue, 24 Mar 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _FROB_LOCK_H_
#define _FROB_LOCK_H_

/** 
 * greebo: This class represents a pickable lock. It supports
 * attachment of BinaryFrobMovers which are used as levers.
 */
class CFrobLock :
	public idStaticEntity
{
public:
	CLASS_PROTOTYPE( CFrobLock );

	void	Spawn();

	void	Save(idSaveGame *savefile) const;
	void	Restore(idRestoreGame *savefile);

protected:
	void	PostSpawn();

};

#endif /* _FROB_LOCK_H_ */
