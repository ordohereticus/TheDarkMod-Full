/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 602 $
 * $Date: 2006-11-01 11:12:48 -0500 (Wed, 01 Nov 2006) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:03  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __LISTGUI_H__
#define __LISTGUI_H__

/*
===============================================================================

	feed data to a listDef
	each item has an id and a display string

===============================================================================
*/

class idListGUI {
public:
	virtual				~idListGUI() { }

	virtual void		Config( idUserInterface *pGUI, const char *name ) = 0;
	virtual void		Add( int id, const idStr& s ) = 0;
						// use the element count as index for the ids
	virtual void		Push( const idStr& s ) = 0;
	virtual bool		Del( int id ) = 0;
	virtual void		Clear( void ) = 0;
	virtual int			Num( void ) = 0;
	virtual int			GetSelection( char *s, int size, int sel = 0 ) const = 0; // returns the id, not the list index (or -1)
	virtual void		SetSelection( int sel ) = 0;
	virtual int			GetNumSelections() = 0;
	virtual bool		IsConfigured( void ) const = 0;
						// by default, any modification to the list will trigger a full GUI refresh immediately
	virtual void		SetStateChanges( bool enable ) = 0;
	virtual void		Shutdown( void ) = 0;
};

#endif /* !__LISTGUI_H__ */
