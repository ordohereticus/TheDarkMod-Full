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

#ifndef __MATH_SIMD_MMX_H__
#define __MATH_SIMD_MMX_H__

/*
===============================================================================

	MMX implementation of idSIMDProcessor

===============================================================================
*/

class idSIMD_MMX : public idSIMD_Generic {
#ifdef _WIN32
public:
	virtual const char * VPCALL GetName( void ) const;

	virtual void VPCALL Memcpy( void *dst,			const void *src,		const int count );
	virtual void VPCALL Memset( void *dst,			const int val,			const int count );

#endif
};

#endif /* !__MATH_SIMD_MMX_H__ */
