/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2388 $
 * $Date: 2008-05-26 15:03:49 -0400 (Mon, 26 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: FrobButton.cpp 2388 2008-05-26 19:03:49Z greebo $", init_version);

#include "../game/game_local.h"
#include "DarkModGlobals.h"
#include "FrobButton.h"
#include "sndProp.h"

//===============================================================================
//CFrobButton
//===============================================================================


const idEventDef EV_TDM_Button_Open( "Open", "f" );
const idEventDef EV_TDM_Button_Close( "Close", "f" );
const idEventDef EV_TDM_Button_Operate("Operate", NULL);

CLASS_DECLARATION( CBinaryFrobMover, CFrobButton )
	EVENT( EV_TDM_Button_Open,				CFrobButton::Open)
	EVENT( EV_TDM_Button_Close,				CFrobButton::Close)
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
