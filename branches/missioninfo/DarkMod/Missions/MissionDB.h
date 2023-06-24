/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3918 $
 * $Date: 2010-06-07 23:04:15 -0400 (Mon, 07 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef _MISSION_DB_H_
#define _MISSION_DB_H_

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include "MissionInfo.h"

namespace fs = boost::filesystem;

/**
 * greebo: The mission database class holds the list of available TDM missions
 * and the corresponding data like play statistics, size info, etc.
 */
class CMissionDB
{
private:
	// Named mission info structures (fs_game => info)
	typedef std::map<std::string, CMissionInfoPtr> MissionInfoMap;
	MissionInfoMap _missionInfo;

public:
	CMissionDB();

	// Loads all stored data from disk
	void Init();

	// Returns the mission info structure for this name
	// Always returns non-NULL, if the name is not existing, 
	// a new structure will be created
	const CMissionInfoPtr& GetMissionInfo(const idStr& name);

	void LoadFromFile(const fs::path& file);
};
typedef boost::shared_ptr<CMissionDB> CMissionDBPtr;

#endif /* _MISSION_DB_H_ */
