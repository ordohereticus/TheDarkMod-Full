/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5092 $
 * $Date: 2011-12-06 12:30:27 -0500 (Tue, 06 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: RouteNode.cpp 5092 2011-12-06 17:30:27Z greebo $", init_version);

#include "RouteNode.h"

namespace eas {

RouteNode::RouteNode() :
	type(ACTION_WALK),
	toArea(0),
	toCluster(0),
	elevator(-1),
	elevatorStation(-1)
{}

RouteNode::RouteNode(ActionType t, int goalArea, int goalCluster, int elevatorNum, int elevatorStationNum) :
	type(t),
	toArea(goalArea),
	toCluster(goalCluster),
	elevator(elevatorNum),
	elevatorStation(elevatorStationNum)
{}

// Copy constructor
RouteNode::RouteNode(const RouteNode& other) :
	type(other.type),
	toArea(other.toArea),
	toCluster(other.toCluster),
	elevator(other.elevator),
	elevatorStation(other.elevatorStation)
{}

bool RouteNode::operator==(const RouteNode& other) const
{
	return (type == other.type && toArea == other.toArea && toCluster == other.toCluster && 
		     elevator == other.elevator && elevatorStation == other.elevatorStation);
}

bool RouteNode::operator!=(const RouteNode& other) const
{
	return !operator==(other);
}

void RouteNode::Save(idSaveGame* savefile) const
{
	savefile->WriteInt(static_cast<int>(type));
	savefile->WriteInt(toArea);
	savefile->WriteInt(toCluster);
	savefile->WriteInt(elevator);
	savefile->WriteInt(elevatorStation);
}

void RouteNode::Restore(idRestoreGame* savefile)
{
	int temp;
	savefile->ReadInt(temp);
	type = static_cast<ActionType>(temp);
	savefile->ReadInt(toArea);
	savefile->ReadInt(toCluster);
	savefile->ReadInt(elevator);
	savefile->ReadInt(elevatorStation);
}

} // namespace eas
