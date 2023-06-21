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
 * $Revision: 17 $
 * $Date: 2004-11-03 16:47:38 -0500 (Wed, 03 Nov 2004) $
 * $Author: sparhawk $
 * $Name$
 *
 * $Log$
 * Revision 1.2  2004/11/03 21:47:17  sparhawk
 * Changed debug LogString for better performance and group settings
 *
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

#include "../DarkMod/DarkModGlobals.h"

CDarkModPlayer::CDarkModPlayer(void)
{
	m_FrobEntity = NULL;
}

CDarkModPlayer::~CDarkModPlayer(void)
{
}
