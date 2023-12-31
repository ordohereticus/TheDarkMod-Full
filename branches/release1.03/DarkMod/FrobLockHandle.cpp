/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3306 $
 * $Date: 2009-03-25 12:59:25 -0400 (Wed, 25 Mar 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Gerhard W. Gruber <sparhawk@gmx.at>
//

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: FrobLockHandle.cpp 3306 2009-03-25 16:59:25Z greebo $", init_version);

#include "../game/game_local.h"
#include "DarkModGlobals.h"
#include "FrobLock.h"
#include "FrobLockHandle.h"

//===============================================================================
// CFrobLockHandle
//===============================================================================
const idEventDef EV_TDM_Handle_GetLock( "GetLock", NULL, 'e' );

CLASS_DECLARATION( CFrobHandle, CFrobLockHandle )
	EVENT( EV_TDM_Handle_GetLock,		CFrobLockHandle::Event_GetLock )
END_CLASS

CFrobLockHandle::CFrobLockHandle() :
	m_FrobLock(NULL)
{}

void CFrobLockHandle::Save(idSaveGame *savefile) const
{
	savefile->WriteObject(m_FrobLock);
}

void CFrobLockHandle::Restore( idRestoreGame *savefile )
{
	savefile->ReadObject(reinterpret_cast<idClass*&>(m_FrobLock));
}

void CFrobLockHandle::Spawn()
{}

CFrobLock* CFrobLockHandle::GetFrobLock()
{
	return m_FrobLock;
}

void CFrobLockHandle::SetFrobLock(CFrobLock* lock)
{
	m_FrobLock = lock;

	// Set the frob master accordingly
	SetFrobMaster(m_FrobLock);
}

void CFrobLockHandle::Event_GetLock()
{
	return idThread::ReturnEntity(m_FrobLock);
}

void CFrobLockHandle::OnOpenPositionReached()
{
	// The handle is "opened", trigger the lock, but only if this is the master handle
	if (IsMasterHandle() && m_FrobLock != NULL)
	{
		m_FrobLock->OpenTargets();
	}

	// Let the handle return to its initial position
	Close(true);
}

void CFrobLockHandle::Tap()
{
	// Invoke the base class first
	CFrobHandle::Tap();
	
	// Only the master handle is allowed to trigger sounds
	if (IsMasterHandle() && m_FrobLock != NULL)
	{
		// Start the appropriate sound
		FrobMoverStartSound(m_FrobLock->IsLocked() ? "snd_tap_locked" : "snd_tap_default");
	}
}

bool CFrobLockHandle::LockIsLocked()
{
	return m_FrobLock ? m_FrobLock->IsLocked() : IsLocked();
}
