/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4344 $
 * $Date: 2010-11-28 00:02:54 -0500 (Sun, 28 Nov 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include <cstdlib>
#include <iostream>

#include "TraceLog.h"

#include "Updater/UpdaterOptions.h"
#include "Updater/Updater.h"

#include "LogWriters.h"
#include "Util.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "ConsoleUpdater.h"

using namespace tdm;
using namespace updater;

int main(int argc, char* argv[])
{
	// Start logging
	RegisterLogWriters();

	TraceLog::WriteLine(LOG_STANDARD, 
		(boost::format("TDM Updater v%s (c) 2009-2010 by tels & greebo. Part of The Dark Mod (http://www.thedarkmod.com).") % LIBTDM_UPDATE_VERSION).str());
	TraceLog::WriteLine(LOG_STANDARD, "");

	ConsoleUpdater updater(argc, argv);

	updater.Run();

	int exitCode = EXIT_FAILURE;

	switch (updater.GetOutcome())
	{
		case ConsoleUpdater::None:
			// should not happen?
			break;
		case ConsoleUpdater::Failed:
			exitCode = EXIT_FAILURE;
			break;
		case ConsoleUpdater::Ok:
			exitCode = EXIT_SUCCESS;
			break;
		case ConsoleUpdater::OkNeedRestart:
			exitCode = EXIT_SUCCESS;
			break;
	};

	return exitCode;
}
