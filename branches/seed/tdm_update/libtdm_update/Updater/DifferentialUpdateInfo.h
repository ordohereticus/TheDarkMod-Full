/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4636 $
 * $Date: 2011-02-26 13:03:16 -0500 (Sat, 26 Feb 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

namespace tdm
{

namespace updater
{

struct DifferentialUpdateInfo
{
	// The version we're upgrading from
	std::string fromVersion;

	// The version we're upgrading to
	std::string toVersion;

	// The package size
	std::size_t filesize;
};

} 

}
