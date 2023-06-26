/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4801 $
 * $Date: 2011-04-17 05:33:09 -0400 (Sun, 17 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef TDM_CAMPAIGN_STATISTICS_H
#define TDM_CAMPAIGN_STATISTICS_H

#include "../idlib/precompiled.h"

#include "MissionStatistics.h"

/**
 * Multiple mission stats structs combined => campaign stats.
 * First mission is carrying index 0.
 */
class CampaignStats
{
private:
	// The internal array of statistics
	idList<SMissionStats> _stats;

public:
	// greebo: Use this operator to get access to the stats of the mission with the given index
	// The internal list will automatically be resized to fit.
	SMissionStats& operator[] (int index)
	{
		EnsureSize(index + 1);
		return _stats[index];
	}

	const SMissionStats& operator[] (int index) const
	{
		return _stats[index];
	}

	int Num() const
	{
		return _stats.Num();
	}

	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);

private:
	void EnsureSize(int size);
};

#endif
