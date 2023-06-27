/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5086 $
 * $Date: 2011-12-05 00:30:50 -0500 (Mon, 05 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef BLOODMARKER_H
#define BLOODMARKER_H

#pragma hdrstop

#include "Entity.h"

class CBloodMarker : public idEntity
{
public:
	CLASS_PROTOTYPE( CBloodMarker );

protected:
	idStr					_bloodSplat;
	idStr					_bloodSplatFading;
	float					_angle;
	float					_size;

	// True if this bloodsplat is in the process of disappearing
	bool					_isFading;

public:
	void					Init(const idStr& splat, const idStr& splatFading, float size);
	void					Event_GenerateBloodSplat();

	/**
	 * greebo: Overrides the OnStim method of the base class to check
	 * for water stims.
	 */
	void					OnStim(const CStimPtr& stim, idEntity* stimSource);

	// Save and restore
	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );
};

// End of header wrapper
#endif
