/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4801 $
 * $Date: 2011-04-17 05:33:09 -0400 (Sun, 17 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: CampaignStatistics.cpp 4801 2011-04-17 09:33:09Z greebo $", init_version);

#include "CampaignStatistics.h"

void CampaignStats::Save(idSaveGame* savefile) const
{
	savefile->WriteInt(_stats.Num());

	for (int i = 0; i < _stats.Num(); ++i)
	{
		_stats[i].Save(savefile);
	}
}

void CampaignStats::Restore(idRestoreGame* savefile)
{
	int num;
	savefile->ReadInt(num);
	_stats.SetNum(num);

	for (int i = 0; i < num; ++i)
	{
		_stats[i].Restore(savefile);
	}
}

void CampaignStats::EnsureSize(int size)
{
	if (_stats.Num() < size)
	{
		_stats.SetNum(size);
	}
}
