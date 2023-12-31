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

#ifndef __AI_EAS_ROUTENODE_H__
#define __AI_EAS_ROUTENODE_H__

#include <list>
#include <boost/shared_ptr.hpp>

namespace eas {

enum ActionType {
	ACTION_WALK = 0,		// AI just needs to walk to the target
	ACTION_USE_ELEVATOR,	// AI needs to use an elevator
	NUM_ACTIONS,
};

struct RouteNode
{
	ActionType type;		// what needs to be done in this route section (walk?, use elevator?)
	int toArea;				// the target AAS area number
	int toCluster;			// the target AAS cluster number
	int elevator;			// the elevator number (is -1 if no elevator to be used in this node)
	int elevatorStation;	// The elevator station number of this position (-1 if unused)

	// Default constructor
	RouteNode();

	// Specialised constructor
	RouteNode(ActionType t, int goalArea, int goalCluster, int elevatorNum = -1, int elevatorStationNum = -1);

	// Copy constructor
	RouteNode(const RouteNode& other);

	bool operator==(const RouteNode& other) const;
	bool operator!=(const RouteNode& other) const;

	void Save(idSaveGame* savefile) const;
	void Restore(idRestoreGame* savefile);
};
typedef boost::shared_ptr<RouteNode> RouteNodePtr;
typedef std::list<RouteNodePtr> RouteNodeList;

} // namespace eas

#endif /* __AI_EAS_ROUTENODE_H__ */
