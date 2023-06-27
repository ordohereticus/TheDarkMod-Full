/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5082 $
 * $Date: 2011-12-04 23:40:41 -0500 (Sun, 04 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_WORLDSPAWN_H__
#define __GAME_WORLDSPAWN_H__

/*
===============================================================================

  World entity.

===============================================================================
*/

class idWorldspawn : public idEntity {
public:
	CLASS_PROTOTYPE( idWorldspawn );

	virtual ~idWorldspawn();

	void			Spawn( void );

	void			Save( idRestoreGame *savefile );
	void			Restore( idRestoreGame *savefile );

private:
	void			Event_Remove( void );
};

#endif /* !__GAME_WORLDSPAWN_H__ */
