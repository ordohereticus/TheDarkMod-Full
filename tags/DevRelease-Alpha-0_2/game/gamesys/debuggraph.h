/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 256 $
 * $Date: 2005-11-26 17:50:07 -0500 (Sat, 26 Nov 2005) $
 * $Author: $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:33  sparhawk
 * Initial revision
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
