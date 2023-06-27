/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4797 $
 * $Date: 2011-04-17 02:34:47 -0400 (Sun, 17 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef EMISSIONRESULT_H_
#define EMISSIONRESULT_H_

enum EMissionResult {
	MISSION_NOTEVENSTARTED = 0,	// before any map is loaded (at game startup, for instance)
	MISSION_INPROGRESS = 1,		// mission not yet accomplished (in-game)
	MISSION_FAILED = 2,			// mission failed
	MISSION_COMPLETE = 3,		// mission completed
};

#endif /*EMISSIONRESULT_H_*/
