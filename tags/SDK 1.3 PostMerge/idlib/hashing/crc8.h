/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 17:25:02 -0400 (Fri, 23 Mar 2007) $
 * $Author: sparhawk $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __CRC8_H__
#define __CRC8_H__

/*
===============================================================================

	Calculates a checksum for a block of data
	using the CRC-8.

===============================================================================
*/

void CRC8_InitChecksum( unsigned char &crcvalue );
void CRC8_UpdateChecksum( unsigned char &crcvalue, const void *data, int length );
void CRC8_FinishChecksum( unsigned char &crcvalue );
unsigned char CRC8_BlockChecksum( const void *data, int length );

#endif /* !__CRC8_H__ */
