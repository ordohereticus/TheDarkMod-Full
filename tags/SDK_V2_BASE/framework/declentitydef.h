/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 2 $
 * $Date: 2004-10-30 11:52:07 -0400 (Sat, 30 Oct 2004) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:34  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __DECLENTITYDEF_H__
#define __DECLENTITYDEF_H__

/*
===============================================================================

	idDeclEntityDef

===============================================================================
*/

class idDeclEntityDef : public idDecl {
public:
	idDict					dict;

	virtual size_t			Size( void ) const;
	virtual const char *	DefaultDefinition() const;
	virtual bool			Parse( const char *text, const int textLength );
	virtual void			FreeData( void );
	virtual void			Print( void );
};

#endif /* !__DECLENTITYDEF_H__ */
