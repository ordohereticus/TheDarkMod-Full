/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3292 $
 * $Date: 2009-03-24 11:19:37 -0400 (Tue, 24 Mar 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: FrobLock.cpp 3292 2009-03-24 15:19:37Z greebo $", init_version);

#include "../game/game_local.h"
#include "DarkModGlobals.h"
#include "FrobLock.h"

CLASS_DECLARATION( idStaticEntity, CFrobLock )
	EVENT( EV_PostSpawn,	CFrobLock::PostSpawn )
END_CLASS

void CFrobLock::Save(idSaveGame *savefile) const
{
	// TODO
}

void CFrobLock::Restore( idRestoreGame *savefile )
{
	// TODO
}

void CFrobLock::Spawn()
{
	PostEventMS(&EV_PostSpawn, 0);
}

void CFrobLock::PostSpawn()
{
	// TODO: Find lever entities
}
