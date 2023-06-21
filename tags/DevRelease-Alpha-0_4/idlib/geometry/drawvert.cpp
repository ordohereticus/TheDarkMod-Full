/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 465 $
 * $Date: 2006-06-21 09:08:20 -0400 (Wed, 21 Jun 2006) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.2  2006/06/21 13:07:38  sparhawk
 * Added version tracking per cpp module
 *
 * Revision 1.1.1.1  2004/10/30 15:52:37  sparhawk
 * Initial release
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Source$  $Revision: 465 $   $Date: 2006-06-21 09:08:20 -0400 (Wed, 21 Jun 2006) $", init_version);

/*
=============
idDrawVert::Normalize
=============
*/
void idDrawVert::Normalize( void ) {
	normal.Normalize();
	tangents[1].Cross( normal, tangents[0] );
	tangents[1].Normalize();
	tangents[0].Cross( tangents[1], normal );
	tangents[0].Normalize();
}
