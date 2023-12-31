/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5292 $ (Revision of last commit) 
 $Date: 2012-02-23 11:17:34 -0500 (Thu, 23 Feb 2012) $ (Date of last commit)
 $Author: grayman $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: RouteInfo.cpp 5292 2012-02-23 16:17:34Z grayman $");

#include "RouteInfo.h"

namespace eas {

RouteInfo::RouteInfo() :
	routeType(ROUTE_TO_CLUSTER),
	target(-1),
	routeTravelTime(0) // grayman #3029
{}

RouteInfo::RouteInfo(RouteType type, int targetNum) :
	routeType(type),
	target(targetNum),
	routeTravelTime(0) // grayman #3029
{}

// Copy constructor
RouteInfo::RouteInfo(const RouteInfo& other) :
	routeType(other.routeType),
	target(other.target),
	routeTravelTime(other.routeTravelTime) // grayman #3029
{
	// Copy the RouteNodes of the other list, one by one
	for (RouteNodeList::const_iterator otherNode = other.routeNodes.begin();
		otherNode != other.routeNodes.end(); ++otherNode)
	{
		RouteNodePtr newNode(new RouteNode(**otherNode));
		routeNodes.push_back(newNode);
	}
}

bool RouteInfo::operator==(const RouteInfo& other) const
{
	if ( (routeType == other.routeType ) &&
		 ( target == other.target ) &&
		 ( routeNodes.size() == other.routeNodes.size() ) &&
		 ( routeTravelTime == other.routeTravelTime ) ) // grayman #3029
	{
		for (RouteNodeList::const_iterator i = routeNodes.begin(), j = other.routeNodes.begin(); i != routeNodes.end(); ++i, ++j)
		{
			if (*i != *j)
			{
				return false; // RouteNode mismatch
			}
		}

		return true; // everything matched
	}

	return false; // routeType, routeNodes.size(), target, or routeTravelTime mismatched
}

bool RouteInfo::operator!=(const RouteInfo& other) const
{
	return !operator==(other);
}

void RouteInfo::Save(idSaveGame* savefile) const
{
	savefile->WriteInt(static_cast<int>(routeType));
	savefile->WriteInt(target);

	savefile->WriteInt(static_cast<int>(routeNodes.size()));
	for (RouteNodeList::const_iterator i = routeNodes.begin(); i != routeNodes.end(); ++i)
	{
		(*i)->Save(savefile);
	}

	savefile->WriteInt(routeTravelTime); // grayman #3029
}

void RouteInfo::Restore(idRestoreGame* savefile)
{
	int temp;
	savefile->ReadInt(temp);
	routeType = static_cast<RouteType>(temp);
	savefile->ReadInt(target);

	int num;
	savefile->ReadInt(num);
	routeNodes.clear();
	for (int i = 0; i < num; i++)
	{
		RouteNodePtr node(new RouteNode);
		node->Restore(savefile);
		routeNodes.push_back(node);
	}

	savefile->ReadInt(routeTravelTime); // grayman #3029
}

} // namespace eas
