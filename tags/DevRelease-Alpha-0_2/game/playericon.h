/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 256 $
 * $Date: 2005-11-26 17:50:07 -0500 (Sat, 26 Nov 2005) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:31  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef	__PLAYERICON_H__
#define	__PLAYERICON_H__

typedef enum {
	ICON_LAG,
	ICON_CHAT,
	ICON_NONE
} playerIconType_t;

class idPlayerIcon {
public:
	
public:
	idPlayerIcon();
	~idPlayerIcon();

	void	Draw( idPlayer *player, jointHandle_t joint );
	void	Draw( idPlayer *player, const idVec3 &origin );

public:
	playerIconType_t	iconType;
	renderEntity_t		renderEnt;
	qhandle_t			iconHandle;

public:
	void	FreeIcon( void );
	bool	CreateIcon( idPlayer* player, playerIconType_t type, const char *mtr, const idVec3 &origin, const idMat3 &axis );
	bool	CreateIcon( idPlayer* player, playerIconType_t type, const idVec3 &origin, const idMat3 &axis );
	void	UpdateIcon( idPlayer* player, const idVec3 &origin, const idMat3 &axis );

};


#endif	/* !_PLAYERICON_H_ */

