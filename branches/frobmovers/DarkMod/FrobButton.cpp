/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2538 $
 * $Date: 2008-06-20 02:25:38 -0400 (Fri, 20 Jun 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: FrobButton.cpp 2538 2008-06-20 06:25:38Z greebo $", init_version);

#include "../game/game_local.h"
#include "DarkModGlobals.h"
#include "FrobButton.h"
#include "sndProp.h"

//===============================================================================
// CFrobButton
//===============================================================================

const idEventDef EV_TDM_Button_Operate("Operate", NULL);

CLASS_DECLARATION( CBinaryFrobMover, CFrobButton )
	EVENT( EV_TDM_Button_Operate,			CFrobButton::Operate)
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
	Open(false);
}

void CFrobButton::Open(bool bMaster)
{
	CBinaryFrobMover::Open(false);
}

void CFrobButton::Close(bool bMaster)
{
	CBinaryFrobMover::Close(false);
}

void CFrobButton::ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse )
{
	// Check if the impulse is applied in the right direction
	if (impulse * m_Translation >= 0)
	{
		Operate();
	}
}


// A button can't close or open a portal, so we block it.
void CFrobButton::ClosePortal()
{
}

void CFrobButton::OpenPortal()
{
}
