/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4578 $
 * $Date: 2011-02-11 03:38:53 -0500 (Fri, 11 Feb 2011) $
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
