/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4540 $
 * $Date: 2011-02-03 13:59:49 -0500 (Thu, 03 Feb 2011) $
 * $Author: grayman $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: DoorInfo.cpp 4540 2011-02-03 18:59:49Z grayman $", init_version);

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
