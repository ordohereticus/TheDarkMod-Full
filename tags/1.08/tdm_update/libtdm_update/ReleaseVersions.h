/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod Updater (http://www.thedarkmod.com/)
 
 $Revision: 5122 $ (Revision of last commit) 
 $Date: 2011-12-11 14:47:31 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#pragma once

#include <string>
#include <boost/regex.hpp>
#include "ReleaseFileset.h"
#include "IniFile.h"

namespace tdm
{

class ReleaseVersions :
	public std::map<std::string, ReleaseFileSet>,
	public IniFile::SectionVisitor
{
public:
	void LoadFromIniFile(const IniFile& iniFile)
	{
		iniFile.ForeachSection(*this);
	}

	void VisitSection(const IniFile& iniFile, const std::string& sectionName)
	{
		// Get the info from the section header
		boost::regex expr("^Version([\\.0-9]+) File (.*)$",
						  boost::regex::perl|boost::regex::icase);

		boost::smatch matches;
		
		if (boost::regex_match(sectionName, matches, expr))
		{
			std::string version = matches[1].str();
			std::string filename = matches[2].str();

			ReleaseFileSet& set = FindOrInsertVersion(version);

			ReleaseFile file(filename);

			file.crc = CRC::ParseFromString(iniFile.GetValue(sectionName, "crc"));
			file.filesize = boost::lexical_cast<std::size_t>(iniFile.GetValue(sectionName, "filesize"));
			file.localChangesAllowed = iniFile.GetValue(sectionName, "allow_local_modifications") == "1";

			TraceLog::WriteLine(LOG_VERBOSE, (boost::format("Found version %s file: %s with checksum %x") %
				version % filename % file.crc).str());

			set.insert(ReleaseFileSet::value_type(filename, file));
		}
	}

private:
	ReleaseFileSet& FindOrInsertVersion(const std::string& version)
	{
		iterator found = find(version);

		if (found != end())
		{
			return found->second;
		}

		return insert(value_type(version, ReleaseFileSet())).first->second;
	}
};

} // namespace
