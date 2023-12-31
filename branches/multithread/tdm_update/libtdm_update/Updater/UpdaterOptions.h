/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include <boost/regex.hpp>
#include "../ProgramOptions.h"

namespace tdm
{

namespace updater
{

class UpdaterOptions :
	public ProgramOptions
{
public:
	UpdaterOptions()
	{
		SetupDescription();
	}

	// Construct options from command line arguments
	UpdaterOptions(int argc, char* argv[])
	{
		SetupDescription();
		ParseFromCommandLine(argc, argv);

		for (int i = 1; i < argc; ++i)
		{
			_cmdLineArgs.push_back(argv[i]);
		}
	}

	void CheckProxy(const HttpConnectionPtr& conn) const
	{
		std::string proxyStr = Get("proxy");

		if (proxyStr.empty()) 
		{
			TraceLog::WriteLine(LOG_VERBOSE, "No proxy configured.");
			return; // nothing to do
		}

		if (boost::algorithm::ends_with(proxyStr, "/"))
		{
			proxyStr = proxyStr.substr(proxyStr.length() - 1);
		}

		// Get the proxy info out of this string
		boost::regex expr("^(https?)://(([^@]+)|([^@]+)@([^@]+))$",
						  boost::regex::perl|boost::regex::icase);

		boost::smatch matches;
		
		if (boost::regex_match(proxyStr, matches, expr))
		{
			if (matches[3].matched)
			{
				// Non-authenticated proxy
				TraceLog::WriteLine(LOG_VERBOSE, "Using proxy: " + matches[3]);
				conn->SetProxyHost(matches[3]);
			}
			else if (matches[5].matched)
			{
				// Proxy with authentication
				TraceLog::WriteLine(LOG_VERBOSE, "Using proxy with authentication: " + matches[5]);
				conn->SetProxyHost(matches[5]);
				
				// Split the username and password
				std::vector<std::string> parts;
				std::string userPassStr = matches[4].str();
				boost::algorithm::split(parts, userPassStr, boost::algorithm::is_any_of(":"));

				if (parts.size() == 2)
				{
					conn->SetProxyUsername(parts[0]);
					conn->SetProxyPassword(parts[1]);
				}
				else
				{
					TraceLog::WriteLine(LOG_VERBOSE, "Discarding proxy authentication information, wrong format.");
				}
			}
			else
			{
				TraceLog::WriteLine(LOG_VERBOSE, "No proxy host found, ignoring proxy setting.");
			}
		}
		else
		{
			TraceLog::WriteLine(LOG_VERBOSE, "Proxy string doesn't match the required pattern.");
		}
	}

private:
	// Implement base class method
	void SetupDescription()
	{
		// Get options from command line
		_desc.add_options()
			("proxy", bpo::value<std::string>(), "Use a proxy to connect to the internet, example \n--proxy=http://user:pass@proxy:port\n")
			("targetdir", bpo::value<std::string>(), "The folder which should be updated.\n--targetdir=c:\\games\\doom3\\darkmod\n")
			("help", "Display this help page")
			("keep-mirrors", "Don't download updated mirrors list from the server, use local one.")
			("keep-update-packages", "Don't delete downloaded update packages after applying them.")
			("noselfupdate", "Don't perform any special 'update the updater' routines.")
			("dry-run", "Don't do any updates, just perform checks.")
			;
	}
};

}

}
