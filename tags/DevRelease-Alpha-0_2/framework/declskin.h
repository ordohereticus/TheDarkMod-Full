/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 256 $
 * $Date: 2005-11-26 17:50:07 -0500 (Sat, 26 Nov 2005) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:34  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __DECLSKIN_H__
#define __DECLSKIN_H__

/*
===============================================================================

	idDeclSkin

===============================================================================
*/

typedef struct {
	const idMaterial *		from;			// 0 == any unmatched shader
	const idMaterial *		to;
} skinMapping_t;

class idDeclSkin : public idDecl {
public:
	virtual size_t			Size( void ) const;
	virtual bool			SetDefaultText( void );
	virtual const char *	DefaultDefinition( void ) const;
	virtual bool			Parse( const char *text, const int textLength );
	virtual void			FreeData( void );

	const idMaterial *		RemapShaderBySkin( const idMaterial *shader ) const;

							// model associations are just for the preview dialog in the editor
	const int				GetNumModelAssociations() const;
	const char *			GetAssociatedModel( int index ) const;

private:
	idList<skinMapping_t>	mappings;
	idStrList				associatedModels;
};

#endif /* !__DECLSKIN_H__ */
