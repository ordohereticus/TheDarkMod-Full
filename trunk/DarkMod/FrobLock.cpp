/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3295 $
 * $Date: 2009-03-25 04:09:01 -0400 (Wed, 25 Mar 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: FrobLock.cpp 3295 2009-03-25 08:09:01Z greebo $", init_version);

#include "../game/game_local.h"
#include "DarkModGlobals.h"
#include "FrobLock.h"

CLASS_DECLARATION( idStaticEntity, CFrobLock )
	EVENT( EV_PostSpawn,	CFrobLock::PostSpawn )
END_CLASS

void CFrobLock::Save(idSaveGame *savefile) const
{
	m_Lock.Save(savefile);
}

void CFrobLock::Restore( idRestoreGame *savefile )
{
	m_Lock.Restore(savefile);
}

void CFrobLock::Spawn()
{
	// Load the lock spawnargs
	m_Lock.InitFromSpawnargs(spawnArgs);

	PostEventMS(&EV_PostSpawn, 0);
}

void CFrobLock::PostSpawn()
{
	// TODO: Find lever entities
}

bool CFrobLock::IsLocked()
{
	return m_Lock.IsLocked();
}

bool CFrobLock::IsPickable()
{
	return m_Lock.IsPickable();
}
