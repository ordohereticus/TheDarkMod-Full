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

static bool init_version = FileVersionList("$Id: DoorInfo.cpp 5092 2011-12-06 17:30:27Z greebo $", init_version);

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
