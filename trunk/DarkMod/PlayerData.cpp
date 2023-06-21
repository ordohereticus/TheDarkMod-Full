/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 918 $
 * $Date: 2007-04-21 04:42:18 -0400 (Sat, 21 Apr 2007) $
 * $Author: orbweaver $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: PlayerData.cpp 918 2007-04-21 08:42:18Z orbweaver $", init_version);

#include "../DarkMod/DarkModGlobals.h"
#include "../DarkMod/PlayerData.h"

CDarkModPlayer::CDarkModPlayer(void)
{
	m_FrobEntity = NULL;
	m_FrobJoint = INVALID_JOINT;
	m_FrobID = 0;
	m_FrobEntityPrevious = NULL;
	m_LightgemValue = 0;

	// TODO: Spawn grabber from a .def file (maybe?)
	this->grabber = new CGrabber();
}

CDarkModPlayer::~CDarkModPlayer(void)
{
	// remove grabber object	
	this->grabber->PostEventSec( &EV_Remove, 0 );
}

unsigned long CDarkModPlayer::AddLight(idLight *light)
{
	if(light)
	{
		m_LightList.Append(light);
		DM_LOG(LC_FUNCTION, LT_DEBUG)LOGSTRING("%08lX [%s] %lu added to LightList\r", light, light->name.c_str(), m_LightList.Num());
	}

	return m_LightList.Num();
}

unsigned long CDarkModPlayer::RemoveLight(idLight *light)
{
	int n;

	if(light)
	{
		if((n = m_LightList.FindIndex(light)) != -1)
		{
			m_LightList.RemoveIndex(n);
			DM_LOG(LC_FUNCTION, LT_DEBUG)LOGSTRING("%08lX [%s] %lu removed from LightList\r", light, light->name.c_str(), m_LightList.Num());
		}
	}

	return m_LightList.Num();
}
