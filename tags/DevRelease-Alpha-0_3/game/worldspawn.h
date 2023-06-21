/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 473 $
 * $Date: 2006-06-27 04:48:45 -0400 (Tue, 27 Jun 2006) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:30  sparhawk
 * Initial revision
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

					~idWorldspawn();

	void			Spawn( void );

	void			Save( idRestoreGame *savefile );
	void			Restore( idRestoreGame *savefile );

private:
	void			Event_Remove( void );
};

#endif /* !__GAME_WORLDSPAWN_H__ */
