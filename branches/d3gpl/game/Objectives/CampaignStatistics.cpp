/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5092 $
 * $Date: 2011-12-06 12:30:27 -0500 (Tue, 06 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: CampaignStatistics.cpp 5092 2011-12-06 17:30:27Z greebo $", init_version);

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
