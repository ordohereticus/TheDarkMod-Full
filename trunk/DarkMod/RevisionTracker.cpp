/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3152 $
 * $Date: 2009-01-17 00:39:29 -0500 (Sat, 17 Jan 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

#pragma warning(disable : 4533 4800)

static bool init_version = FileVersionList("$Id: RevisionTracker.cpp 3152 2009-01-17 05:39:29Z greebo $", init_version);

#include "RevisionTracker.h"

#include <climits>
#include <vector>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

RevisionTracker::RevisionTracker() :
	_highestRevision(0),
	_lowestRevision(INT_MAX),
	_numFiles(0)
{}

int RevisionTracker::GetHighestRevision() const
{
	return _highestRevision;
}

int RevisionTracker::GetLowestRevision() const
{
	return _lowestRevision;
}

int RevisionTracker::GetNumFiles() const
{
	return _numFiles;
}

void RevisionTracker::AddRevision(int revision) 
{
	_numFiles++;

	if (_highestRevision < revision)
	{
		_highestRevision = revision;
	}

	if (_lowestRevision > revision)
	{
		_lowestRevision = revision;
	}
}

void RevisionTracker::ParseSVNIdString(const char* input)
{
	std::string revStr(input);
	std::vector<std::string> parts;

	// Split the incoming string into parts
	boost::algorithm::split(parts, revStr, boost::algorithm::is_any_of(" "));

	if (parts.size() > 1)
	{
		// The third token is the SVN revision, convert it to integer and pass it along
		Instance().AddRevision(atoi(parts[2].c_str()));
	}
}

// Accessor to the singleton
RevisionTracker& RevisionTracker::Instance()
{
	static RevisionTracker _instance;
	return _instance;
}
