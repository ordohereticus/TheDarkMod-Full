/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5185 $ (Revision of last commit) 
 $Date: 2012-01-08 00:59:48 -0500 (Sun, 08 Jan 2012) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: FrobLever.cpp 5185 2012-01-08 05:59:48Z greebo $");

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
