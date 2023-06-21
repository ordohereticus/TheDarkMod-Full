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
// DebugGraph.h

class idDebugGraph {
public:
					idDebugGraph();
	void			SetNumSamples( int num );
	void			AddValue( float value );
	void			Draw( const idVec4 &color, float scale ) const;

private:
	idList<float>	samples;
	int				index;
};
