/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2339 $
 * $Date: 2008-05-15 13:09:01 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MultiStateMoverButton.cpp 2339 2008-05-15 17:09:01Z greebo $", init_version);

#include "DarkModGlobals.h"
#include "MultiStateMoverButton.h"
#include "MultiStateMover.h"

//===============================================================================
// CMultiStateMoverButton
//===============================================================================

CLASS_DECLARATION( CFrobButton, CMultiStateMoverButton )
	EVENT( EV_PostSpawn,				CMultiStateMoverButton::Event_PostSpawn)
END_CLASS

void CMultiStateMoverButton::Spawn()
{
	PostEventMS(&EV_PostSpawn, 10);
}

void CMultiStateMoverButton::Event_PostSpawn()
{
	// TODO: Send the button information to the targetted multistatemover
}
