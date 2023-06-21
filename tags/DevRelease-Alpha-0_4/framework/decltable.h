/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 774 $
 * $Date: 2007-01-27 11:15:01 -0500 (Sat, 27 Jan 2007) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:34  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __DECLTABLE_H__
#define __DECLTABLE_H__

/*
===============================================================================

	tables are used to map a floating point input value to a floating point
	output value, with optional wrap / clamp and interpolation

===============================================================================
*/

class idDeclTable : public idDecl {
public:
	virtual size_t			Size( void ) const;
	virtual const char *	DefaultDefinition( void ) const;
	virtual bool			Parse( const char *text, const int textLength );
	virtual void			FreeData( void );

	float					TableLookup( float index ) const;

private:
	bool					clamp;
	bool					snap;
	idList<float>			values;
};

#endif /* !__DECLTABLE_H__ */
