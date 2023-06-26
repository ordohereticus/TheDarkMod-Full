/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4650 $
 * $Date: 2011-03-04 13:18:20 -0500 (Fri, 04 Mar 2011) $
 * $Author: stgatilov $
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
