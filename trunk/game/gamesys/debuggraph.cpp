/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 915 $
 * $Date: 2007-04-19 16:10:27 -0400 (Thu, 19 Apr 2007) $
 * $Author: orbweaver $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//
#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: debuggraph.cpp 915 2007-04-19 20:10:27Z orbweaver $", init_version);

#include "../game_local.h"

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
