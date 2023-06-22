/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 12:53:28 -0400 (Tue, 16 Oct 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __MD4_H__
#define __MD4_H__

/*
===============================================================================

	Calculates a checksum for a block of data
	using the MD4 message-digest algorithm.

===============================================================================
*/

unsigned long MD4_BlockChecksum( const void *data, int length );

#endif /* !__MD4_H__ */
