/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "IniFile.h"
#include "CRC.h"
#include "File.h"

namespace tdm
{

struct ReleaseFile
{
	// True if this file is a PK4 / ZIP package
	bool isArchive;

	// Filename including path
	boost::filesystem::path file;

	// CRC32 checksum
	boost::uint32_t	crc;

	// The file size in bytes
	std::size_t	filesize;

	// Members of this archive
	std::set<ReleaseFile> members;

	// This is TRUE for DoomConfig.cfg, for example. 
	// Is only used when comparing release versions in the updater code
	bool localChangesAllowed;

	// The download ID of this file (-1 == no download ID)
	int downloadId;

	ReleaseFile() :
		isArchive(false),
		localChangesAllowed(false),
		downloadId(-1)
	{}

	ReleaseFile(const boost::filesystem::path& pathToFile) :
		isArchive(false),
		file(pathToFile),
		localChangesAllowed(false),
		downloadId(-1)
	{}

	ReleaseFile(const boost::filesystem::path& pathToFile, boost::uint32_t crc_) :
		isArchive(false),
		file(pathToFile),
		crc(crc_),
		localChangesAllowed(false),
		downloadId(-1)
	{}

	// Implement less operator for use in std::set or std::map
	bool operator<(const ReleaseFile& other) const
	{
		return file < other.file;
	}

	// A ReleaseFile is equal if filename, size, archive flag, CRC and all members are equal
	bool operator==(const ReleaseFile& other) const
	{
		if (file != other.file || crc != other.crc || filesize != other.filesize || isArchive != isArchive)
		{
			return false;
		}

		if (members.size() != other.members.size()) 
		{
			return false; // member size mismatch
		}

		// Equal member size, compare all 
		for (std::set<ReleaseFile>::const_iterator i = members.begin(), j = other.members.begin();
			 i != members.end(); ++i, ++j)
		{
			if (*i != *j) return false;
		}

		return true; // all checks passed
	}

	bool operator!=(const ReleaseFile& other) const
	{
		return !(this->operator==(other));
	}

	bool ContainsUpdater(const std::string& executable) const
	{
		for (std::set<ReleaseFile>::const_iterator m = members.begin(); m != members.end(); ++m)
		{
			if (boost::algorithm::to_lower_copy(m->file.string()) == boost::algorithm::to_lower_copy(executable))
			{
				return true;
			}
		}

		return false;
	}
};

/** 
 * A FileSet represents a full release package, including
 * PK4 files and extracted files like INI, tdmlauncher.exe, etc.
 * Each file contains a checksum so that it can be compared to 
 * an available package on the update servers.
 */
class ReleaseFileSet :
	public std::map<std::string, ReleaseFile>
{
public:
	ReleaseFileSet()
	{}

	// Construct a set by specifying the INI file it is described in
	// The INI file is usually the crc_info.txt file on the servers
	static ReleaseFileSet LoadFromIniFile(const IniFile& iniFile)
	{
		ReleaseFileSet set;

		class Visitor :
			public IniFile::SectionVisitor
		{
		private:
			ReleaseFileSet& _set;

		public:
			Visitor(ReleaseFileSet& set) :
				_set(set)
			{}
				
			void VisitSection(const IniFile& iniFile, const std::string& section)
			{
				if (boost::algorithm::istarts_with(section, "File"))
				{
					std::string filename = section.substr(5);

					std::pair<ReleaseFileSet::iterator, bool> result = _set.insert(
						ReleaseFileSet::value_type(filename, ReleaseFile(filename)));
					
					result.first->second.crc = CRC::ParseFromString(iniFile.GetValue(section, "crc"));
					result.first->second.filesize = boost::lexical_cast<std::size_t>(iniFile.GetValue(section, "size"));

					if (boost::algorithm::iends_with(filename, "pk4") || 
						boost::algorithm::iends_with(filename, "zip"))
					{
						result.first->second.isArchive = true;
					}
					else
					{
						result.first->second.isArchive = false;
					}
				}
				else if (boost::algorithm::istarts_with(section, "Member"))
				{
					std::string combo = section.substr(7);

					// bar.zip:foo.pk4
					std::string archiveName = combo.substr(0, combo.rfind(':'));
					std::string memberName = combo.substr(combo.rfind(':') + 1);

					// Find or insert the archive
					std::pair<ReleaseFileSet::iterator, bool> result = _set.insert(
						ReleaseFileSet::value_type(archiveName, ReleaseFile(archiveName)));

					// Find or insert the member
					ReleaseFile member(memberName);

					member.isArchive = false;
					member.crc = CRC::ParseFromString(iniFile.GetValue(section, "crc"));
					member.filesize = boost::lexical_cast<std::size_t>(iniFile.GetValue(section, "size"));

					result.first->second.members.insert(member);
				}
			}
		} _visitor(set);

		// Traverse the settings using the ReleaseFileSet as visitor
		iniFile.ForeachSection(_visitor);

		return set;
	}

	// Construct a release file set from the given folder
	static ReleaseFileSet LoadFromFolder(const fs::path& folder, 
										 const std::set<std::string> ignoreList = std::set<std::string>())
	{
		ReleaseFileSet set;

		for (fs::recursive_directory_iterator i = fs::recursive_directory_iterator(folder); 
			i != fs::recursive_directory_iterator(); ++i)
		{
			fs::path file = *i;

			if (fs::is_directory(file))
			{
				continue; // skip directories
			}

			std::string filename = file.leaf();

			fs::path relativePath = File::GetRelativePath(file, folder);

			if (ignoreList.find(boost::to_lower_copy(filename)) != ignoreList.end())
			{
				TraceLog::WriteLine(LOG_STANDARD, "Ignoring file: " + relativePath.string());
				continue;
			}
		
			TraceLog::WriteLine(LOG_STANDARD, "Found file: " + relativePath.string());

			if (File::IsArchive(file))
			{
				ReleaseFile archive(relativePath);

				archive.isArchive = true;
				archive.crc = CRC::GetCrcForFile(file);
				archive.filesize = static_cast<std::size_t>(fs::file_size(file));

				// Add all members of this archive to the ReleaseFile
				class ZipFileVisitor : 
					public ZipFileRead::Visitor
				{
				private:
					ReleaseFile& _archive;

				public:
					ZipFileVisitor(ReleaseFile& archive) :
						_archive(archive)
					{}

					void VisitFile(const ZipFileRead::MemberInfo& info)
					{
						ReleaseFile file(info.filename);

						file.crc = info.crc;
						file.filesize = info.uncompressedSize;

						TraceLog::WriteLine(LOG_VERBOSE, "  Adding archive member " + info.filename + 
							" with CRC " + (boost::format("%x") % file.crc).str());

						_archive.members.insert(file);
					}

				} _visitor(archive);

				TraceLog::WriteLine(LOG_STANDARD, "  This is an archive, checksum is: " + (boost::format("%x") % archive.crc).str());

				// Open this archive
				ZipFileReadPtr zipFile = Zip::OpenFileRead(file);

				if (zipFile == NULL)
				{
					TraceLog::WriteLine(LOG_STANDARD, "  Failed to open archive: " + file.file_string());
					continue;
				}

				zipFile->ForeachFile(_visitor);

				TraceLog::WriteLine(LOG_STANDARD, (boost::format("  Archive has %d members") % archive.members.size()).str());
				
				set[relativePath.string()] = archive;
			}
		}

		return set;
	}
};

} // namespace
