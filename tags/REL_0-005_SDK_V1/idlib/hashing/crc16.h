/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 2 $
 * $Date: 2004-10-30 11:52:07 -0400 (Sat, 30 Oct 2004) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:37  sparhawk
 * Initial revision
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
