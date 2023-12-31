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

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: TimerManager.cpp 5185 2012-01-08 05:59:48Z greebo $");

#ifdef TIMING_BUILD

#include "TimerManager.h"

namespace debugtools {

void TimerManager::Save(idSaveGame* savefile) const
{
	int num = _timers.size();
	savefile->WriteInt(num);
	for (TimerMap::const_iterator iterator = _timers.begin(); iterator != _timers.end(); ++iterator)
	{
		savefile->WriteInt(iterator->first);

		savefile->WriteString(iterator->second.entityName);
		savefile->WriteString(iterator->second.name);
		savefile->WriteInt(iterator->second.runCount);
		savefile->WriteFloat(static_cast<float>(iterator->second.runTime));
		savefile->WriteFloat(static_cast<float>(iterator->second.maxTime));
		savefile->WriteInt(iterator->second.maxTimeCall);
	}
}

void TimerManager::Restore(idRestoreGame* savefile)
{
	int num;
	savefile->ReadInt(num);
	for (int i = 1; i <= num; i++)
	{
		int timerId;
		savefile->ReadInt(timerId);
		TimerInfo info;
		savefile->ReadString(info.entityName);
		savefile->ReadString(info.name);
		savefile->ReadInt(info.runCount);
		float temp;
		savefile->ReadFloat(temp);
		info.runTime = static_cast<double>(temp);
		savefile->ReadFloat(temp);
		info.maxTime = static_cast<double>(temp);
		savefile->ReadInt(info.maxTimeCall);

		_timers.insert(TimerMap::value_type(timerId, info));
	}
}


int	TimerManager::CreateTimer(const idStr& entityname, const idStr& name)
{
	int timerId = _timers.size() + 1;

	TimerInfo info;
	info.entityName = entityname;
	info.name = name;
	info.runCount = 0;
	info.runTime = 0;
	info.maxTime = 0;
	info.maxTimeCall = 0;

	_timers.insert(TimerMap::value_type(timerId, info));

	return timerId;
}

void TimerManager::StartTimer(int timerId)
{
	TimerMap::iterator found = _timers.find(timerId);
	assert(found != _timers.end());

	TimerInfo& info = found->second;
	if (!info.timer.Running())
	{
		info.timer.Clear();
		info.timer.Start();
	}
}

void TimerManager::StopTimer(int timerId)
{
	TimerMap::iterator found = _timers.find(timerId);
	assert(found != _timers.end());

	TimerInfo& info = found->second;
	if (info.timer.Running())
	{
		info.timer.Stop();
		info.runCount++;
		info.runTime += info.timer.Milliseconds();
		if (info.timer.Milliseconds() > info.maxTime)
		{
			info.maxTime = info.timer.Milliseconds();
			info.maxTimeCall = info.runCount;
		}
		info.timer.Clear();
	}
}

void TimerManager::ResetTimers()
{
	for (TimerMap::iterator i = _timers.begin(); i != _timers.end(); ++i)
	{
		TimerInfo& info = i->second;

		info.runCount = 0;
		info.runTime = 0;
		info.maxTime = 0;
		info.maxTimeCall = 0;
	}
}

void TimerManager::PrintTimerResults()
{
	gameLocal.Printf("Timer Info at frame: %d\n", gameLocal.framenum);

	for (TimerMap::iterator iterator = _timers.begin(); iterator != _timers.end(); ++iterator)
	{
		TimerInfo& info = iterator->second;
		float meanRunTime = info.runCount > 0 ? (info.runTime / info.runCount) : 0;
		
		gameLocal.Printf("%s %s \n", info.entityName.c_str(), info.name.c_str());
		gameLocal.Printf("Number of calls: %d, ", info.runCount);
		gameLocal.Printf("Total run time: %lf ms\n", info.runTime);
		gameLocal.Printf("Mean run time per call: %lf ms\n", meanRunTime);
		gameLocal.Printf("Max runtime: %lf ms at call number %d\n", info.maxTime, info.maxTimeCall);
		gameLocal.Printf("---------------------------------\n");
	}
}

void TimerManager::DumpTimerResults(const char* const separator, const char* const comma)
{
	idStr buffer = va("Entity%sTimer%sNumCalls%sTotalRunTime / ms%sMeanRunTime / ms%sMaxRunTime / ms%sAt Call\n", 
		separator,separator,separator,separator,separator,separator);

	for (TimerMap::iterator iterator = _timers.begin(); iterator != _timers.end(); ++iterator)
	{
		TimerInfo& info = iterator->second;
		float meanRunTime = info.runCount > 0 ? (info.runTime / info.runCount) : 0;
		buffer += va("%s%s%s%s%d%s%lf%s%lf%s%lf%s%d\n", 
			info.entityName.c_str(), separator,
			info.name.c_str(), separator,
			info.runCount, separator,
			info.runTime, separator,
			meanRunTime, separator,
			info.maxTime, separator,
			info.maxTimeCall);
	}

	buffer.Replace(".", comma);

	fileSystem->WriteFile(va("/timerResult_%d.csv", gameLocal.framenum), buffer.c_str(), buffer.Length());
	gameLocal.Printf("Wrote file: timerResult_%d.csv\n", gameLocal.framenum);
}

void TimerManager::Clear()
{
	_timers.clear();
}

TimerManager& TimerManager::Instance() 
{
	static TimerManager _manager;
	return _manager;
}

} // namespace debugtools

#endif /* ifdef TIMING_BUILD */
