/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3741 $
 * $Date: 2009-11-03 05:38:51 -0500 (Tue, 03 Nov 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef PLAYERDATA_H
#define PLAYERDATA_H

#include "Grabber.h"

/**
 * CDarkModPlayer is a class that maintains player data. The purpose of this
 * this class is mainly to be indenependent from idPlayer and seperate the code
 * from id's code.
 * Player data will store all additional data that is required like inventory,
 * special player states, currently highlighted entity and others.
 *
 * greebo: TODO: Move frob stuff to idPlayer, this class is a global and it should not hold
 * this kind of stuff, this basically rules out any future multiplayer efforts.
 */
class CDarkModPlayer {
public:
	CDarkModPlayer();

	void Save( idSaveGame *savefile ) const;
	void Restore( idRestoreGame *savefile );

public:
};

#endif
