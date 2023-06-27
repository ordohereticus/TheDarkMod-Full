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

#ifndef __CRC16_H__
#define __CRC16_H__

/*
===============================================================================

	Calculates a checksum for a block of data
	using the CCITT standard CRC-16.

===============================================================================
*/

void CRC16_InitChecksum( unsigned short &crcvalue );
void CRC16_UpdateChecksum( unsigned short &crcvalue, const void *data, int length );
void CRC16_FinishChecksum( unsigned short &crcvalue );
unsigned short CRC16_BlockChecksum( const void *data, int length );

#endif /* !__CRC16_H__ */
