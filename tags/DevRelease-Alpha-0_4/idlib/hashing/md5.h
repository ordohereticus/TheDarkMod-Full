/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 774 $
 * $Date: 2007-01-27 11:15:01 -0500 (Sat, 27 Jan 2007) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:37  sparhawk
 * Initial revision
 *
 ***************************************************************************/


#ifndef __MD5_H__
#define __MD5_H__

/*
===============================================================================

	Calculates a checksum for a block of data
	using the MD5 message-digest algorithm.

===============================================================================
*/

unsigned long MD5_BlockChecksum( const void *data, int length );

#endif /* !__MD5_H__ */
