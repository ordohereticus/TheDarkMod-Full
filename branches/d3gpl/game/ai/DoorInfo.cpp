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

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: DoorInfo.cpp 5121 2011-12-11 19:12:26Z greebo $", init_version);

#include "DoorInfo.h"

namespace ai
{

DoorInfo::DoorInfo() :
	areaNum(-1),
	lastTimeSeen(-1),
	lastTimeUsed(-1), // grayman #2345
	lastTimeTriedToOpen(-1),
	wasOpen(false),
	wasLocked(false),
	wasBlocked(false)
{}

void DoorInfo::Save(idSaveGame* savefile) const
{
	savefile->WriteInt(areaNum);
	savefile->WriteInt(lastTimeSeen);
	savefile->WriteInt(lastTimeTriedToOpen);
	savefile->WriteBool(wasOpen);
	savefile->WriteBool(wasLocked);
	savefile->WriteBool(wasBlocked);
	savefile->WriteInt(lastTimeUsed); // grayman #2345
}

void DoorInfo::Restore(idRestoreGame* savefile)
{
	savefile->ReadInt(areaNum);
	savefile->ReadInt(lastTimeSeen);
	savefile->ReadInt(lastTimeTriedToOpen);
	savefile->ReadBool(wasOpen);
	savefile->ReadBool(wasLocked);
	savefile->ReadBool(wasBlocked);
	savefile->ReadInt(lastTimeUsed); // grayman #2345
}

} // namespace ai
