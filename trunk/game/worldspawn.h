/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2198 $
 * $Date: 2008-04-22 12:33:35 -0400 (Tue, 22 Apr 2008) $
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
