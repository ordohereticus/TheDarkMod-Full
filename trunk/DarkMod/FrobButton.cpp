/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2557 $
 * $Date: 2008-06-21 15:42:20 -0400 (Sat, 21 Jun 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: FrobButton.cpp 2557 2008-06-21 19:42:20Z greebo $", init_version);

#include "../game/game_local.h"
#include "DarkModGlobals.h"
#include "FrobButton.h"
#include "sndProp.h"

//===============================================================================
// CFrobButton
//===============================================================================

const idEventDef EV_TDM_Button_Operate("Operate", NULL);

CLASS_DECLARATION( CBinaryFrobMover, CFrobButton )
	EVENT( EV_TDM_Button_Operate,	CFrobButton::Operate)
END_CLASS

void CFrobButton::Save(idSaveGame *savefile) const
{
	// nothing to save (yet)
}

void CFrobButton::Restore( idRestoreGame *savefile )
{
	// nothing to restore (yet)
}

void CFrobButton::Spawn()
{
}

void CFrobButton::Operate()
{
	ToggleOpen();
}

void CFrobButton::ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse )
{
	// Check if the impulse is applied in the right direction
	if (impulse * m_Translation >= 0)
	{
		Operate();
	}
}
