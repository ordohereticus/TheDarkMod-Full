/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod Updater (http://www.thedarkmod.com/)
 
 $Revision: 5534 $ (Revision of last commit) 
 $Date: 2012-08-24 11:29:25 -0400 (Fri, 24 Aug 2012) $ (Date of last commit)
 $Author: grayman $ (Author of last commit)
 
******************************************************************************/

#pragma once

#include "../IniFile.h"
#include "../TraceLog.h"

#include <map>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/erase.hpp> // grayman #3208

namespace tdm
{

// Information about a TDM mirror
struct Mirror
{
	// Display name, e.g. "KeepOfMetalAndGold"
	std::string displayName;

	// URL prefix, e.g. "http://www.keepofmetalandgold.com/files/tdm/"
	std::string	url;

	// Probability [0..1]
	float weight;

	Mirror(const std::string& displayName_,
		   const std::string& url_,
		   float weight_) :
		displayName(displayName_),
		url(url_),
		weight(weight_)
	{
		// grayman #3208 - remove all spaces in the url
		boost::algorithm::ierase_all(url, " ");

		// url should terminate with "/"
		if (!boost::algorithm::ends_with(url, "/"))
		{
			url += "/";
		}
	}
};

/**
 * The list of mirrors available for download.
 * Each mirror has a certain "weight" to accommodate
 * the various download limits imposed on the people
 * who are generously offering to host our files.
 */
class MirrorList :
	public std::vector<Mirror>,
	public IniFile::SectionVisitor
{
public:
	MirrorList()
	{}

	// Construct this mirror list from the given ini file
	MirrorList(const IniFile& iniFile)
	{
		iniFile.ForeachSection(*this);

		NormaliseWeights();
	}

	void VisitSection(const IniFile& iniFile, const std::string& sectionName)
	{
		if (!boost::algorithm::istarts_with(sectionName, "Mirror "))
		{
			return; // ignore non-Mirror sections
		}

		try
		{
			push_back(Mirror(
				sectionName.substr(7), // displayname
				iniFile.GetValue(sectionName, "url"),
				boost::lexical_cast<float>(iniFile.GetValue(sectionName, "weight"))
			));
		}
		catch (boost::bad_lexical_cast&)
		{
			TraceLog::WriteLine(LOG_VERBOSE, "Invalid weight on mirror " + sectionName);
		}
	}

private:
	void NormaliseWeights()
	{
		// Calculate weight sum
		float sum = 0;

		for (const_iterator i = begin(); i != end(); ++i)
		{
			sum += i->weight;
		}

		// Normalise weights
		if (sum > 0)
		{
			for (iterator i = begin(); i != end(); ++i)
			{
				i->weight /= sum;
			}
		}
		else
		{
			TraceLog::WriteLine(LOG_VERBOSE, "Invalid weights, total sum <= 0");
		}
	}
};

} // namespace
