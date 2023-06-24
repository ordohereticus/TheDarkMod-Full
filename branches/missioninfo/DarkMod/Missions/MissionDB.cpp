/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3920 $
 * $Date: 2010-06-08 04:45:18 -0400 (Tue, 08 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MissionDB.cpp 3920 2010-06-08 08:45:18Z greebo $", init_version);

#include "MissionDB.h"
#include "MissionInfoDecl.h"

CMissionDB::CMissionDB()
{}

void CMissionDB::Init()
{
	// Nothing for now
}

void CMissionDB::Shutdown()
{
	// Save changed declarations
	for (MissionInfoMap::iterator i = _missionInfo.begin(); i != _missionInfo.end(); ++i)
	{
		i->second->Save();
	}
}

const CMissionInfoPtr& CMissionDB::GetMissionInfo(const idStr& name)
{
	MissionInfoMap::iterator i = _missionInfo.find(name.c_str());
	
	if (i != _missionInfo.end())
	{
		return i->second;
	}

	// Get the mission info declaration (or create it if not found so far)
	CMissionInfoDecl* decl = CMissionInfoDecl::FindOrCreate(name);

	CMissionInfoPtr missionInfo(new CMissionInfo(name, decl));

	std::pair<MissionInfoMap::iterator, bool> result = _missionInfo.insert(
		MissionInfoMap::value_type(name.c_str(), missionInfo));

	// Load the data from the text files, if possible
	missionInfo->LoadMetaData();

	return result.first->second;
}
