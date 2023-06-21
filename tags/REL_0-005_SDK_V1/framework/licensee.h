/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 2 $
 * $Date: 2004-10-30 11:52:07 -0400 (Sat, 30 Oct 2004) $
 * $Author: sparhawk $
 *
 * $Log$
 * Revision 1.1  2004/10/30 15:52:34  sparhawk
 * Initial revision
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

/*
===============================================================================

	Definitions for information that is related to a licensee's game name and location.

===============================================================================
*/

#define GAME_NAME						"DOOM 3"		// appears on window titles and errors

#define	ENGINE_VERSION					"DOOM 1.0"		// printed in console

// paths
#define	CD_BASEDIR						"Doom"
#define	BASE_GAMEDIR					"base"
#define	DEMO_GAMEDIR					"demo"

// filenames
#define	CD_EXE							"doom.exe"
#define CONFIG_FILE						"DoomConfig.cfg"

// base folder where the source code lives
#define SOURCE_CODE_BASE_FOLDER			"neo"


// default idnet host address
#ifndef IDNET_HOST
	#define IDNET_HOST					"idnet.ua-corp.com"
#endif

// default idnet master port
#ifndef IDNET_MASTER_PORT
	#define IDNET_MASTER_PORT			"27650"
#endif

// default network server port
#ifndef PORT_SERVER
	#define	PORT_SERVER					27666
#endif

// broadcast scan this many ports after PORT_SERVER so a single machine can run multiple servers
#define	NUM_SERVER_PORTS				4

// see ASYNC_PROTOCOL_VERSION
// use a different major for each game
#define ASYNC_PROTOCOL_MAJOR			1

// Savegame Version
// Update when you can no longer maintain compatibility with previous savegames.
#define SAVEGAME_VERSION				16

// editor info
#define EDITOR_REGISTRY_INFO			"Software\\id\\QuakeEd4"		// FIXME: change to DoomEd
#define EDITOR_DEFAULT_PROJECT			"doom.qe4"
#define EDITOR_REGISTRY_KEY				"DOOMRadiant"
#define EDITOR_WINDOWTEXT				"DOOMEdit"

// win32 info
#define WIN32_CONSOLE_CLASS				"DOOM 3 WinConsole"
#define	WIN32_WINDOW_CLASS_NAME			"DOOM3"
#define	WIN32_FAKE_WINDOW_CLASS_NAME	"DOOM3_WGL_FAKE"

// Linux info
#define LINUX_DEFAULT_PATH				"/usr/local/games/doom3"

// CD Key file info
#define CDKEY_FILE						"doomkey"
#define CDKEY_TEXT						"\n// Do not give this file to ANYONE.\n" \
										"// id Software and Activision will NOT ask you to send this file to them.\n"
#define CONFIG_SPEC						"config.spec"
