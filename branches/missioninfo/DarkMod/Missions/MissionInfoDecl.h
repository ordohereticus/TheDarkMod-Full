/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3919 $
 * $Date: 2010-06-08 03:06:50 -0400 (Tue, 08 Jun 2010) $
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

	/// Key/value data parsed from the mission info decl.
	idDict					data;

	// Returns the declaration or NULL if no declaration has been registered so far
	static CMissionInfoDecl*	Find(const idStr& name);

	// Creates a new declaration with the given name. When written, the declaration
	// will be saved to the default filename as given in the corresponding CVAR.
	static CMissionInfoDecl*	Create(const idStr& name);

	// Convenience method, combining the above two. The declaration will be created
	// using Create() if not found after calling Find().
	static CMissionInfoDecl*	FindOrCreate(const idStr& name);
};

#endif /* _MISSION_INFO_DECL_H_ */
