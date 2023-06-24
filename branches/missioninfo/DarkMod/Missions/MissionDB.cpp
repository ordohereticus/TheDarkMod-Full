/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3919 $
 * $Date: 2010-06-08 03:06:50 -0400 (Tue, 08 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MissionDB.cpp 3919 2010-06-08 07:06:50Z greebo $", init_version);

#include "MissionDB.h"
#include "MissionInfoDecl.h"

namespace fs = boost::filesystem;

CMissionDB::CMissionDB()
{}

void CMissionDB::Init()
{
	// Traverse all declarations?
}

const CMissionInfoPtr& CMissionDB::GetMissionInfo(const idStr& name)
{
	MissionInfoMap::iterator i = _missionInfo.find(name.c_str());
	
	if (i == _missionInfo.end())
	{
		// Get the mission info declaration (or create it if not found so far)
		CMissionInfoDecl* decl = CMissionInfoDecl::FindOrCreate(name);

		std::pair<MissionInfoMap::iterator, bool> result = _missionInfo.insert(
			MissionInfoMap::value_type(name.c_str(), CMissionInfoPtr(new CMissionInfo(decl)))
		);

		i = result.first;
	}

	return i->second;
}

void CMissionDB::LoadFromFile(const fs::path& file)
{
	// TODO
}
