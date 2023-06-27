/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5087 $
 * $Date: 2011-12-05 00:34:06 -0500 (Mon, 05 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: FrobLever.cpp 5087 2011-12-05 05:34:06Z greebo $", init_version);

#include "Game_local.h"
#include "DarkModGlobals.h"
#include "FrobLever.h"

const idEventDef EV_TDM_Lever_Operate( "Operate", NULL );
const idEventDef EV_TDM_Lever_Switch( "Switch", "d" );

CLASS_DECLARATION( CBinaryFrobMover, CFrobLever )
	EVENT( EV_TDM_Lever_Operate,		CFrobLever::Event_Operate)
	EVENT( EV_TDM_Lever_Switch,			CFrobLever::Event_Switch)
END_CLASS

void CFrobLever::Save(idSaveGame *savefile) const
{
	savefile->WriteBool(m_Latch);
}

void CFrobLever::Restore( idRestoreGame *savefile )
{
	savefile->ReadBool(m_Latch);
}

void CFrobLever::Spawn()
{}

void CFrobLever::PostSpawn()
{
	// Call the base class first
	CBinaryFrobMover::PostSpawn();

	// Set the latch to TRUE if the mover starts out closed
	m_Latch = IsAtClosedPosition();
}

void CFrobLever::SwitchState(bool newState)
{
	if (newState)
	{
		Open();
	}
	else
	{
		Close();
	}
}

void CFrobLever::Operate()
{
	// Just call the BinaryFrobMover method
	ToggleOpen();
}

void CFrobLever::OnOpenPositionReached()
{
	// Only allow event firing when the latch is TRUE
	if (m_Latch)
	{
		// Set the latch to false, we've reached the position
		m_Latch = false;

		CBinaryFrobMover::OnOpenPositionReached();
	}
}

void CFrobLever::OnClosedPositionReached()
{
	// Only allow event firing when the latch is FALSE
	if (!m_Latch)
	{
		// Set the latch to false, we've reached the position
		m_Latch = true;

		CBinaryFrobMover::OnClosedPositionReached();
	}
}

void CFrobLever::Event_Operate()
{
	Operate();
}

void CFrobLever::Event_Switch(int newState)
{
	SwitchState(newState == 0 ? false : true);
}
