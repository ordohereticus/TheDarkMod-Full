/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 602 $
 * $Date: 2006-11-01 11:12:48 -0500 (Wed, 01 Nov 2006) $
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
