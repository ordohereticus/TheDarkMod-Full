/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3918 $
 * $Date: 2010-06-07 23:04:15 -0400 (Mon, 07 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _MISSION_INFO_DECL_H_
#define _MISSION_INFO_DECL_H_

#include "../../idlib/precompiled.h"

class CMissionInfoDecl : 
	public idDecl
{
public:
	~CMissionInfoDecl();

	virtual size_t			Size( void ) const;
	virtual const char *	DefaultDefinition( void ) const;
	virtual void			FreeData( void );
	virtual bool			Parse( const char *text, const int textLength );

	/// Key/value data parsed from the xdata decl.
	idDict					data;
};

#endif /* _MISSION_INFO_DECL_H_ */
