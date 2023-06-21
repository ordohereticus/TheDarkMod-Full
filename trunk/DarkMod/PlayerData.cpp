/******************************************************************************/
/*                                                                            */
/*         DarkModGlobals (C) by Gerhard W. Gruber in Germany 2004            */
/*                          All rights reserved                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************
 *
 * PROJECT: DarkMod
 * $Source$
 * $Revision: 13 $
 * $Date: 2004-10-31 15:03:36 -0500 (Sun, 31 Oct 2004) $
 * $Author: sparhawk $
 * $Name$
 *
 * $Log$
 * Revision 1.1  2004/10/31 20:03:36  sparhawk
 * CDarkMod created to contain relevant player data seperate from id code.
 *
 *
 * DESCRIPTION: This file data which is relevant to the player. Examples
 * are: pointer to the inventory, currently highlighted entity adn others.
 *
 *****************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "playerdata.h"

CDarkModPlayer::CDarkModPlayer(void)
{
	m_FrobEntity = NULL;
}

CDarkModPlayer::~CDarkModPlayer(void)
{
}

