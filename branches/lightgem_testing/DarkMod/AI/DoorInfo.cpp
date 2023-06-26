/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4650 $
 * $Date: 2011-03-04 13:18:20 -0500 (Fri, 04 Mar 2011) $
 * $Author: stgatilov $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: DoorInfo.cpp 4650 2011-03-04 18:18:20Z stgatilov $", init_version);

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
