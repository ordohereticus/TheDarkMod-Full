/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5185 $ (Revision of last commit) 
 $Date: 2012-01-08 00:59:48 -0500 (Sun, 08 Jan 2012) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//
#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: DebugGraph.cpp 5185 2012-01-08 05:59:48Z greebo $");

#include "../Game_local.h"

/*
================
idDebugGraph::idDebugGraph
================
*/
idDebugGraph::idDebugGraph() {
	index = 0;
}

/*
================
idDebugGraph::SetNumSamples
================
*/
void idDebugGraph::SetNumSamples( int num ) {
	index = 0;
	samples.Clear();
	samples.SetNum( num );
	memset( samples.Ptr(), 0, samples.MemoryUsed() );
}

/*
================
idDebugGraph::AddValue
================
*/
void idDebugGraph::AddValue( float value ) {
	samples[ index ] = value;
	index++;
	if ( index >= samples.Num() ) {
		index = 0;
	}
}

/*
================
idDebugGraph::Draw
================
*/
void idDebugGraph::Draw( const idVec4 &color, float scale ) const {
	int i;
	float value1;
	float value2;
	idVec3 vec1;
	idVec3 vec2;

	const idMat3 &axis = gameLocal.GetLocalPlayer()->viewAxis;
	const idVec3 pos = gameLocal.GetLocalPlayer()->GetPhysics()->GetOrigin() + axis[ 1 ] * samples.Num() * 0.5f;

	value1 = samples[ index ] * scale;
	for( i = 1; i < samples.Num(); i++ ) {
		value2 = samples[ ( i + index ) % samples.Num() ] * scale;

		vec1 = pos + axis[ 2 ] * value1 - axis[ 1 ] * ( i - 1 ) + axis[ 0 ] * samples.Num();
		vec2 = pos + axis[ 2 ] * value2 - axis[ 1 ] * i + axis[ 0 ] * samples.Num();

		gameRenderWorld->DebugLine( color, vec1, vec2, gameLocal.msec, false );
		value1 = value2;
	}
}
