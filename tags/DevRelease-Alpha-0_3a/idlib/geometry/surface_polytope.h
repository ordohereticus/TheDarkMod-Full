/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 602 $
 * $Date: 2006-11-01 11:12:48 -0500 (Wed, 01 Nov 2006) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:37  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __SURFACE_POLYTOPE_H__
#define __SURFACE_POLYTOPE_H__

/*
===============================================================================

	Polytope surface.

	NOTE: vertexes are not duplicated for texture coordinates.

===============================================================================
*/

class idSurface_Polytope : public idSurface {
public:
						idSurface_Polytope( void );

	void				FromPlanes( const idPlane *planes, const int numPlanes );

	void				SetupTetrahedron( const idBounds &bounds );
	void				SetupHexahedron( const idBounds &bounds );
	void				SetupOctahedron( const idBounds &bounds );
	void				SetupDodecahedron( const idBounds &bounds );
	void				SetupIcosahedron( const idBounds &bounds );
	void				SetupCylinder( const idBounds &bounds, const int numSides );
	void				SetupCone( const idBounds &bounds, const int numSides );

	int					SplitPolytope( const idPlane &plane, const float epsilon, idSurface_Polytope **front, idSurface_Polytope **back ) const;

protected:

};

/*
====================
idSurface_Polytope::idSurface_Polytope
====================
*/
ID_INLINE idSurface_Polytope::idSurface_Polytope( void ) {
}

#endif /* !__SURFACE_POLYTOPE_H__ */
