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

#ifndef __MATH_SIMD_SSE3_H__
#define __MATH_SIMD_SSE3_H__

/*
===============================================================================

	SSE3 implementation of idSIMDProcessor

===============================================================================
*/

class idSIMD_SSE3 : public idSIMD_SSE2 {
#ifdef _WIN32
public:
	virtual const char * VPCALL GetName( void ) const;

	virtual void VPCALL TransformVerts( idDrawVert *verts, const int numVerts, const idJointMat *joints, const idVec4 *weights, const int *index, const int numWeights );

#endif
};

#endif /* !__MATH_SIMD_SSE3_H__ */
