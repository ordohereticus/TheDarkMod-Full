/***************************************************************************
 *
 * PROJECT: The Dark Mod - Packager
 * $Revision: 4887 $
 * $Date: 2011-06-14 04:06:50 -0400 (Tue, 14 Jun 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include <cstdlib>
#include <iostream>

#include "TraceLog.h"
#include "ExceptionSafeThread.h"

#include "LogWriters.h"
#include "Packager/Packager.h"
#include "Packager/PackagerOptions.h"
#include "Util.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include <map>

using namespace tdm;
using namespace packager;

int main(int argc, char* argv[])
{
	// Start logging
	RegisterLogWriters();

	TraceLog::WriteLine(LOG_STANDARD, "TDM Packager v0.02 (c) 2010 by greebo & Tels. Part of The Dark Mod (http://www.thedarkmod.com).");
	TraceLog::WriteLine(LOG_STANDARD, "");

	// Parse the command line
	PackagerOptions options(argc, argv);

	if (options.Empty() || options.IsSet("help"))
	{
		options.PrintHelp();
		return EXIT_SUCCESS;
	}

	try
	{
		if (options.IsSet("create-manifest"))
		{
			if (options.Get("darkmoddir").empty())
			{
				options.PrintHelp();
				return EXIT_SUCCESS;
			}

			Packager packager(options);

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			// Load old manifest if existing
			packager.LoadManifest();

			// Store it for later reference
			packager.SaveManifestAsOldManifest();

			// Read release base manifest (files to be released in any case)
			packager.LoadBaseManifest();

			// TODO Analyse script files
			// TODO Parse declarations
			// TODO Add hardcoded entityDefs

			// Load darkmod_maps.txt (INCLUDE/EXCLUDE/FM)
			packager.LoadInstructionFile();

			// TODO ? Add stuff from maps/prefabs (this is redundant I think)
			// TODO Parse GUIs
			// TODO Parse MD5 Meshes

			// Add versioned files as specified by INCLUDE 
			packager.CollectFilesForManifest();

			// Sort manifest and remove duplicates
			packager.CleanupAndSortManifest();

			// Print the manifest differences
			packager.ShowManifestComparison();

			// Write manifest to disk
			packager.SaveManifest();
		}
		else if (options.IsSet("create-update-package"))
		{
			if (options.Get("basedir").empty() || options.Get("headdir").empty() ||
				options.Get("baseversion").empty() || options.Get("headversion").empty() ||
				options.Get("outputdir").empty())
			{
				options.PrintHelp();
				return EXIT_SUCCESS;
			}

			Packager packager(options);

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			// Analyse base
			packager.GatherBaseSet();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			// Analyse head
			packager.GatherHeadSet();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			// Calculate the set difference of the two packages
			packager.CalculateSetDifference();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			// Pack the changed files into an update PK4
			packager.CreateUpdatePackage();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			TraceLog::WriteLine(LOG_STANDARD, "Done.");
		}
		else if (options.IsSet("update-version-info-file"))
		{
			if (options.Get("basedir").empty() || options.Get("baseversion").empty() ||
				options.Get("outputdir").empty())
			{
				options.PrintHelp();
				return EXIT_SUCCESS;
			}

			Packager packager(options);

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			// Analyse base
			packager.GatherBaseSet();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			// Pack the changed files into an update PK4
			packager.CreateVersionInformation();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			TraceLog::WriteLine(LOG_STANDARD, "Done.");
		}
		else if (options.IsSet("register-update-package"))
		{
			if (options.Get("package").empty() || options.Get("outputdir").empty())
			{
				options.PrintHelp();
				return EXIT_SUCCESS;
			}

			Packager packager(options);

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			packager.RegisterUpdatePackage(options.Get("package"));

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			TraceLog::WriteLine(LOG_STANDARD, "Done.");
		}
		else if (options.IsSet("create-package"))
		{
			if (options.Get("darkmoddir").empty() || options.Get("outputdir").empty())
			{
				options.PrintHelp();
				return EXIT_SUCCESS;
			}

			Packager packager(options);

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			packager.LoadManifest();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			packager.CheckRepository();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			packager.LoadPk4Mapping();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			packager.SortFilesIntoPk4s();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			packager.CreatePackage();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			TraceLog::WriteLine(LOG_STANDARD, "Done.");
		}
		else if (options.IsSet("check-repository"))
		{
			if (options.Get("darkmoddir").empty())
			{
				options.PrintHelp();
				return EXIT_SUCCESS;
			}

			Packager packager(options);

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			packager.LoadManifest();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			packager.CheckRepository();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			TraceLog::WriteLine(LOG_STANDARD, "Done.");
		}
		else if (options.IsSet("create-crc-info"))
		{
			if (options.Get("basedir").empty())
			{
				options.PrintHelp();
				return EXIT_SUCCESS;
			}

			Packager packager(options);

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");
	
			// Analyse base
			packager.GatherBaseSet();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			// Write the base CRC info to the basedir.
			packager.CreateCrcInfoFile();

			TraceLog::WriteLine(LOG_STANDARD, "---------------------------------------------------------");

			TraceLog::WriteLine(LOG_STANDARD, "Done.");
		}
	}
	catch (std::runtime_error& ex)
	{
		TraceLog::Error(ex.what());
		return EXIT_FAILURE;
	}

	
	return EXIT_SUCCESS;
}
