/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 774 $
 * $Date: 2007-01-27 11:15:01 -0500 (Sat, 27 Jan 2007) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:36  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __MATH_SIMD_3DNOW_H__
#define __MATH_SIMD_3DNOW_H__

/*
===============================================================================

	3DNow! implementation of idSIMDProcessor

===============================================================================
*/

class idSIMD_3DNow : public idSIMD_MMX {
#ifdef _WIN32
public:
	virtual const char * VPCALL GetName( void ) const;

	virtual void VPCALL Memcpy( void *dst,			const void *src,		const int count );

#endif
};

#endif /* !__MATH_SIMD_3DNOW_H__ */
