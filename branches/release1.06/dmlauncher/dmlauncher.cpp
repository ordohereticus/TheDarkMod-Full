/*************************************************************************
 *
 * PROJECT: The Dark Mod - Launcher
 * $Source$
 * $Revision: 4853 $
 * $Date: 2011-05-17 02:06:44 -0400 (Tue, 17 May 2011) $
 * $Author: greebo $
 *
 *************************************************************************/

/*
Darkmod Launcher. Launches doom3. Builds command-line args
from currentfm.txt and dmargs.txt.
*/
#include "Launcher.h"

int main(int argc, char* argv[])
{
	// Instantiate a new Launcher class
	Launcher launcher(argc, argv);

	return launcher.Launch() ? EXIT_SUCCESS : EXIT_FAILURE;
}
