/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 874 $
 * $Date: 2007-03-27 16:59:38 -0400 (Tue, 27 Mar 2007) $
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
public:
#if defined(MACOS_X) && defined(__i386__)
	virtual const char * VPCALL GetName( void ) const;

#elif defined(_WIN32)
	virtual const char * VPCALL GetName( void ) const;

	virtual void VPCALL Memcpy( void *dst,			const void *src,		const int count );
	virtual void VPCALL Memset( void *dst,			const int val,			const int count );

#endif
};

#endif /* !__MATH_SIMD_MMX_H__ */
