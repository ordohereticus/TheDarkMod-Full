/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 465 $
 * $Date: 2006-06-21 09:08:20 -0400 (Wed, 21 Jun 2006) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.2  2006/06/21 13:05:52  sparhawk
 * Added version tracking per cpp module
 *
 * Revision 1.1.1.1  2004/10/30 15:52:35  sparhawk
 * Initial release
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Source$  $Revision: 465 $   $Date: 2006-06-21 09:08:20 -0400 (Wed, 21 Jun 2006) $", init_version);

idPluecker pluecker_origin( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );

/*
================
idPluecker::FromPlanes

  pluecker coordinate for the intersection of two planes
================
*/
bool idPluecker::FromPlanes( const idPlane &p1, const idPlane &p2 ) {

	p[0] = -( p1[2] * -p2[3] - p2[2] * -p1[3] );
	p[1] = -( p2[1] * -p1[3] - p1[1] * -p2[3] );
	p[2] = p1[1] * p2[2] - p2[1] * p1[2];

	p[3] = -( p1[0] * -p2[3] - p2[0] * -p1[3] );
	p[4] = p1[0] * p2[1] - p2[0] * p1[1];
	p[5] = p1[0] * p2[2] - p2[0] * p1[2];

	return ( p[2] != 0.0f || p[5] != 0.0f || p[4] != 0.0f );
}

/*
================
idPluecker::Distance3DSqr

  calculates square of shortest distance between the two
  3D lines represented by their pluecker coordinates
================
*/
float idPluecker::Distance3DSqr( const idPluecker &a ) const {
	float d, s;
	idVec3 dir;

	dir[0] = -a.p[5] *  p[4] -  a.p[4] * -p[5];
	dir[1] =  a.p[4] *  p[2] -  a.p[2] *  p[4];
	dir[2] =  a.p[2] * -p[5] - -a.p[5] *  p[2];
	if ( dir[0] == 0.0f && dir[1] == 0.0f && dir[2] == 0.0f ) {
		return -1.0f;	// FIXME: implement for parallel lines
	}
	d = a.p[4] * ( p[2]*dir[1] - -p[5]*dir[0]) +
		a.p[5] * ( p[2]*dir[2] -  p[4]*dir[0]) +
		a.p[2] * (-p[5]*dir[2] -  p[4]*dir[1]);
	s = PermutedInnerProduct( a ) / d;
	return ( dir * dir ) * ( s * s );
}

/*
=============
idPluecker::ToString
=============
*/
const char *idPluecker::ToString( int precision ) const {
	return idStr::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}
