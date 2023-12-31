/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 12:53:28 -0400 (Tue, 16 Oct 2007) $
 * $Author: greebo $
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
