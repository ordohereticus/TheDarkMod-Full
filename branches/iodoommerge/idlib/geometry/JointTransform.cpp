/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5161 $ (Revision of last commit) 
 $Date: 2012-01-06 06:18:21 -0500 (Fri, 06 Jan 2012) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "precompiled.h"
#pragma hdrstop


/*
=============
idJointMat::ToJointQuat
=============
*/
idJointQuat idJointMat::ToJointQuat( void ) const {
	idJointQuat	jq;
	float		trace;
	float		s;
	float		t;
	int     	i;
	int			j;
	int			k;

	static int 	next[3] = { 1, 2, 0 };

	trace = mat[0 * 4 + 0] + mat[1 * 4 + 1] + mat[2 * 4 + 2];

	if ( trace > 0.0f ) {

		t = trace + 1.0f;
		s = idMath::InvSqrt( t ) * 0.5f;

		jq.q[3] = s * t;
		jq.q[0] = ( mat[1 * 4 + 2] - mat[2 * 4 + 1] ) * s;
		jq.q[1] = ( mat[2 * 4 + 0] - mat[0 * 4 + 2] ) * s;
		jq.q[2] = ( mat[0 * 4 + 1] - mat[1 * 4 + 0] ) * s;

	} else {

		i = 0;
		if ( mat[1 * 4 + 1] > mat[0 * 4 + 0] ) {
			i = 1;
		}
		if ( mat[2 * 4 + 2] > mat[i * 4 + i] ) {
			i = 2;
		}
		j = next[i];
		k = next[j];

		t = ( mat[i * 4 + i] - ( mat[j * 4 + j] + mat[k * 4 + k] ) ) + 1.0f;
		s = idMath::InvSqrt( t ) * 0.5f;

		jq.q[i] = s * t;
		jq.q[3] = ( mat[j * 4 + k] - mat[k * 4 + j] ) * s;
		jq.q[j] = ( mat[i * 4 + j] + mat[j * 4 + i] ) * s;
		jq.q[k] = ( mat[i * 4 + k] + mat[k * 4 + i] ) * s;
	}

	jq.t[0] = mat[0 * 4 + 3];
	jq.t[1] = mat[1 * 4 + 3];
	jq.t[2] = mat[2 * 4 + 3];

	return jq;
}
