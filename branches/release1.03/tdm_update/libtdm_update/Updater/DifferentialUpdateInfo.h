/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4344 $
 * $Date: 2010-11-28 00:02:54 -0500 (Sun, 28 Nov 2010) $
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
