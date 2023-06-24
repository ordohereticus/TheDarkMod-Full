/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3740 $
 * $Date: 2009-11-03 04:37:10 -0500 (Tue, 03 Nov 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: PlayerData.cpp 3740 2009-11-03 09:37:10Z greebo $", init_version);

#include "../DarkMod/DarkModGlobals.h"
#include "../DarkMod/PlayerData.h"

CDarkModPlayer::CDarkModPlayer()
{
	m_LightgemValue = 0;
}

void CDarkModPlayer::Save( idSaveGame *savefile ) const
{
	savefile->WriteInt(m_LightgemValue);
	savefile->WriteFloat(m_fColVal);
}

void CDarkModPlayer::Restore( idRestoreGame *savefile )
{
	savefile->ReadInt(m_LightgemValue);
	savefile->ReadFloat(m_fColVal);
}

