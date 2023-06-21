/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 473 $
 * $Date: 2006-06-27 04:48:45 -0400 (Tue, 27 Jun 2006) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:37  sparhawk
 * Initial revision
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
