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

#ifndef __HONEYMAN_H__
#define __HONEYMAN_H__

/*
===============================================================================

	Calculates a checksum for a block of data
	using the simplified version of the pathalias hashing
	function by Steve Belovin and Peter Honeyman.

===============================================================================
*/

void Honeyman_InitChecksum( unsigned long &crcvalue );
void Honeyman_UpdateChecksum( unsigned long &crcvalue, const void *data, int length );
void Honeyman_FinishChecksum( unsigned long &crcvalue );
unsigned long Honeyman_BlockChecksum( const void *data, int length );

#endif /* !__HONEYMAN_H__ */
