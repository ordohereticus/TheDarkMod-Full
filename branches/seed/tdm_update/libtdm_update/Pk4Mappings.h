/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include <vector>
#include <list>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/spirit.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>

namespace bs = boost::spirit;

namespace tdm
{

// Example of a PK4 mapping:
// tdm_player01.pk4: ^models/md5/weapons, ^models/md5/chars/thief, ^dds/models/md5/chars/thief

// The list of regex patterns for each PK4
typedef std::vector<boost::regex> Patterns;

// A pair associating a PK4 file name with a pattern list
typedef std::pair<std::string, Patterns> Pk4Mapping;

/**
 * The information loaded from the package PK4 mapping (e.g. "darkmod_pk4s.txt")
 * is stored in this class. The order the mappings are defined in is important,
 * so this class derives from a sequential list of file => patternlist pairs.
 */
class Pk4Mappings :
	public std::list<Pk4Mapping>
{
public:
	void LoadFromFile(const fs::path& mappingFile)
	{
		// Start parsing
		std::ifstream file(mappingFile.file_string().c_str());

		if (!file)
		{
			TraceLog::WriteLine(LOG_VERBOSE, "[Pk4Mappings]: Cannot open file " + mappingFile.file_string());
			return;
		}

		LoadFromStream(file);
	}

	void LoadFromStream(std::istream& stream)
	{
		// Read the whole stream into a string
		std::string buffer(std::istreambuf_iterator<char>(stream), (std::istreambuf_iterator<char>()));

		LoadFromString(buffer);
	}

	void LoadFromString(const std::string& str)
	{
		clear();

		// Comment starting character: #
		bs::rule<> char_start_comment = bs::ch_p('#');

		// Define blank characters
		bs::rule<> blanks_p = * bs::blank_p;

		// Define comment lines
		bs::rule<> l_comment = blanks_p >> char_start_comment >> *(bs::print_p | bs::blank_p) >> bs::eol_p; 

		// Define empty lines
		bs::rule<> l_empty = blanks_p >> bs::eol_p; 

		// A filename, is stopped by a colon
		bs::rule<> filename_ident = +(bs::print_p - bs::ch_p(':') - bs::ch_p(' '));

		// A pattern is one or more of any character except comma and space
		bs::rule<> pattern = +(bs::print_p - bs::ch_p(' ') - bs::ch_p(','));

		// The mapping
		bs::rule<> l_mapping = 
					blanks_p >> 
					filename_ident[ boost::bind(&Pk4Mappings::AddFile, this, _1, _2) ] >> 
					blanks_p >> 
					bs::ch_p(':') >>
					blanks_p >> 
					+(pattern[ boost::bind(&Pk4Mappings::AddPattern, this, _1, _2) ] >> blanks_p >> (*bs::ch_p(',')) >> blanks_p) >>
					blanks_p >> 
					bs::eol_p
		;

		bs::rule<> lines = l_empty | l_comment | l_mapping;
		bs::rule<> mappingDef =  bs::lexeme_d [ * lines ] ;

		bs::parse_info<> info = bs::parse(str.c_str(), mappingDef);

		if (info.full)
		{
			TraceLog::WriteLine(LOG_VERBOSE, "Successfully parsed the whole mapping file.");
		}
		else
		{
			TraceLog::WriteLine(LOG_VERBOSE, "Could not fully parse the mapping file.");
		}
	}

private:
	void AddFile(char const* beg, char const* end)
	{
		push_back(Pk4Mapping(std::string(beg, end), Patterns()));
	}

	void AddPattern(char const* beg, char const* end)
	{
		if (beg == end) return;

		// Set the destination on the last element
		assert(!empty());

		back().second.push_back(boost::regex(beg, end, boost::regex::perl));
	}
};

} // namespace
