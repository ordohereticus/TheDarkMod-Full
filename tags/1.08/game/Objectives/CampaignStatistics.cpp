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

static bool versioned = RegisterVersionedFile("$Id: CampaignStatistics.cpp 5185 2012-01-08 05:59:48Z greebo $");

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
