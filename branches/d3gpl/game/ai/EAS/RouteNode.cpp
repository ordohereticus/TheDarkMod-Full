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

#include "../../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: RouteNode.cpp 5121 2011-12-11 19:12:26Z greebo $", init_version);

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
