/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5121 $ (Revision of last commit) 
 $Date: 2011-12-11 14:12:26 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#ifndef __GAMEPLAY_TIMER_H__
#define __GAMEPLAY_TIMER_H__

#include "../idlib/precompiled.h"
#include <ctime>

/**
 * greebo: This class keeps track of the total gameplay time. Just call Update()
 * in regular intervals and the class checks the time difference since the last update call.
 *
 * For saving and loading, this class provides separate routines.
 */
class GamePlayTimer
{
	std::time_t _lastTime;
	std::time_t _curTime;

	// The passed time in seconds
	unsigned int _timePassed;

	// TRUE if the timer updates the passed time
	bool _enabled;
public:
	GamePlayTimer() :
		_timePassed(0)
	{}

	// Defines the starting point
	void Start()
	{
		// Remember this time as starting point
		std::time(&_lastTime);
		SetEnabled(true);
	}

	void Stop() 
	{
		SetEnabled(false);
	}

	bool IsEnabled() const
	{
		return _enabled;
	}

	void SetEnabled(bool enabled)
	{
		_enabled = enabled;
	}

	void Clear()
	{
		_timePassed = 0;
		std::time(&_lastTime);
	}

	void Update()
	{
		if (!_enabled) 
		{
			return;
		}

		// Get the current time and calculate the difference
		std::time(&_curTime);

		// Increase the time that has passed
		_timePassed += _curTime - _lastTime;

		// Remember this last check time
		_lastTime = _curTime;
	}

	idStr GetTime() const {
		return TimeToStr(_timePassed);
	}

	// Returns the gameplay time in seconds
	unsigned int GetTimeInSeconds() const
	{
		return _timePassed;
	}

	void Save(idSaveGame *savefile) const
	{
		savefile->WriteUnsignedInt(_timePassed);
		savefile->WriteBool(_enabled);
	}

	void Restore(idRestoreGame *savefile)
	{
		savefile->ReadUnsignedInt(_timePassed);
		savefile->ReadBool(_enabled);
	}

	// Formats the given gameplay time
	static idStr TimeToStr(unsigned int time) {
		unsigned int hours = static_cast<unsigned int>(idMath::Floor(time / 3600.0f));
		unsigned int minutes = time % 3600;
		minutes = static_cast<unsigned int>(idMath::Floor(minutes / 60.0f));
		unsigned int seconds = time % 60;

		return va("%02d:%02d:%02d", hours, minutes, seconds);
	}
};

#endif /* __GAMEPLAY_TIMER_H__ */
