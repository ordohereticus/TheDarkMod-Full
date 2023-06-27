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

static bool init_version = FileVersionList("$Id: RouteInfo.cpp 5092 2011-12-06 17:30:27Z greebo $", init_version);

#include "RouteInfo.h"

namespace eas {

RouteInfo::RouteInfo() :
	routeType(ROUTE_TO_CLUSTER),
	target(-1)
{}

RouteInfo::RouteInfo(RouteType type, int targetNum) :
	routeType(type),
	target(targetNum)
{}

// Copy constructor
RouteInfo::RouteInfo(const RouteInfo& other) :
	routeType(other.routeType),
	target(other.target)
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
	if (routeType == other.routeType && target == other.target && routeNodes.size() == other.routeNodes.size())
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

	return false; // routeType, routeNodes.size() or target mismatched
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
}

} // namespace eas
