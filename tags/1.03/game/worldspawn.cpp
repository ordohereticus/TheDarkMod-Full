/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2589 $
 * $Date: 2008-06-30 01:09:36 -0400 (Mon, 30 Jun 2008) $
 * $Author: dram $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//
/*
game_worldspawn.cpp

Worldspawn class.  Each map has one worldspawn which handles global spawnargs.

*/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: worldspawn.cpp 2589 2008-06-30 05:09:36Z dram $", init_version);

#include "game_local.h"

/*
================
idWorldspawn

Every map should have exactly one worldspawn.
================
*/
CLASS_DECLARATION( idEntity, idWorldspawn )
	EVENT( EV_Remove,				idWorldspawn::Event_Remove )
	EVENT( EV_SafeRemove,			idWorldspawn::Event_Remove )
END_CLASS

/*
================
idWorldspawn::Spawn
================
*/
void idWorldspawn::Spawn( void ) {
	idStr				scriptname;
	idThread			*thread;
	const function_t	*func;
	const idKeyValue	*kv;

	assert( gameLocal.world == NULL );
	gameLocal.world = this;

	g_gravity.SetFloat( spawnArgs.GetFloat( "gravity", va( "%f", DEFAULT_GRAVITY ) ) );

	// Commented out by Dram. TDM does not use stamina
	/*
	// disable stamina on hell levels
	if ( spawnArgs.GetBool( "no_stamina" ) ) {
		pm_stamina.SetFloat( 0.0f );
	}*/

	// load script
	scriptname = gameLocal.GetMapName();
	scriptname.SetFileExtension( ".script" );
	if ( fileSystem->ReadFile( scriptname, NULL, NULL ) > 0 ) {
		gameLocal.program.CompileFile( scriptname );

		// call the main function by default
		func = gameLocal.program.FindFunction( "main" );
		if ( func != NULL ) {
			thread = new idThread( func );
			thread->DelayedStart( 0 );
		}
	}

	// call any functions specified in worldspawn
	kv = spawnArgs.MatchPrefix( "call" );
	while( kv != NULL ) {
		func = gameLocal.program.FindFunction( kv->GetValue() );
		if ( func == NULL ) {
			gameLocal.Error( "Function '%s' not found in script for '%s' key on worldspawn", kv->GetValue().c_str(), kv->GetKey().c_str() );
		}

		thread = new idThread( func );
		thread->DelayedStart( 0 );
		kv = spawnArgs.MatchPrefix( "call", kv );
	}

	// activate worldspawn's targets when it spawns
	PostEventMS( &EV_ActivateTargets, 0, this );
}

/*
=================
idWorldspawn::Save
=================
*/
void idWorldspawn::Save( idRestoreGame *savefile ) {
}

/*
=================
idWorldspawn::Restore
=================
*/
void idWorldspawn::Restore( idRestoreGame *savefile ) {
	assert( gameLocal.world == this );

	g_gravity.SetFloat( spawnArgs.GetFloat( "gravity", va( "%f", DEFAULT_GRAVITY ) ) );

	// Commented out by Dram. TDM does not use stamina
	/*// disable stamina on hell levels
	if ( spawnArgs.GetBool( "no_stamina" ) ) {
		pm_stamina.SetFloat( 0.0f );
	}*/
}

/*
================
idWorldspawn::~idWorldspawn
================
*/
idWorldspawn::~idWorldspawn() {
	if ( gameLocal.world == this ) {
		gameLocal.world = NULL;
	}
}

/*
================
idWorldspawn::Event_Remove
================
*/
void idWorldspawn::Event_Remove( void ) {
	gameLocal.Error( "Tried to remove world" );
}
