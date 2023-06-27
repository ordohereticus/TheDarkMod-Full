/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5066 $
 * $Date: 2011-12-03 14:49:11 -0500 (Sat, 03 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __DOOR_INFO_H__
#define __DOOR_INFO_H__

#include <boost/shared_ptr.hpp>

namespace ai
{

struct DoorInfo
{
	// The AAS area number
	int areaNum;

	// When this door was seen the last time (-1 == never)
	int lastTimeSeen;

	// When this door was used the last time (-1 == never)
	int lastTimeUsed; // grayman #2345

	// The last time this door was attempted to be opened (-1 == never)
	int lastTimeTriedToOpen;

	// Whether this door was open the last time it was seen
	bool wasOpen;

	// Whether this door was locked at the last open attempt
	bool wasLocked;

	// Whether this door was blocked at the last open attempt
	bool wasBlocked;

	DoorInfo();

	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);
};
typedef boost::shared_ptr<DoorInfo> DoorInfoPtr;

} // namespace ai

#endif /* __DOOR_INFO_H__ */
