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

#ifndef __MATH_SIMD_SSE2_H__
#define __MATH_SIMD_SSE2_H__

/*
===============================================================================

	SSE2 implementation of idSIMDProcessor

===============================================================================
*/

class idSIMD_SSE2 : public idSIMD_SSE {
public:
#if defined(MACOS_X) && defined(__i386__)
	virtual const char * VPCALL GetName( void ) const;
	virtual void VPCALL CmpLT( byte *dst,			const byte bitNum,		const float *src0,		const float constant,	const int count );		

#elif defined(_WIN32)
	virtual const char * VPCALL GetName( void ) const;

	//virtual void VPCALL MatX_LowerTriangularSolve( const idMatX &L, float *x, const float *b, const int n, int skip = 0 );
	//virtual void VPCALL MatX_LowerTriangularSolveTranspose( const idMatX &L, float *x, const float *b, const int n );

	virtual void VPCALL MixedSoundToSamples( short *samples, const float *mixBuffer, const int numSamples );

#endif
};

#endif /* !__MATH_SIMD_SSE2_H__ */
