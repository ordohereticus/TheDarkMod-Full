/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3911 $
 * $Date: 2010-06-06 06:30:18 -0400 (Sun, 06 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/
#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: Stim.cpp 3911 2010-06-06 10:30:18Z greebo $", init_version);

#include "Stim.h"

#include "StimResponseTimer.h"

/********************************************************************/
/*                     CStim                                        */
/********************************************************************/
CStim::CStim(idEntity *e, StimType type, int uniqueId) : 
	CStimResponse(e, type, uniqueId)
{
	m_bUseEntBounds = false;
	m_bCollisionBased = false;
	m_bCollisionFired = false;
	m_CollisionEnts.Clear();
	m_TimeInterleave = 0;
	m_TimeInterleaveStamp = 0;
	m_Radius = 0.0f;
	m_RadiusFinal = -1.0f;
	m_FallOffExponent = 0;
	m_Magnitude = 0.0f;
	m_MaxResponses = 0;
	m_CurResponses = 0;
	m_ApplyTimer = 0;
	m_ApplyTimerVal = 0;
	m_MaxFireCount = -1;
	m_Bounds.Zero();
	m_Velocity = idVec3(0,0,0);
}

CStim::~CStim()
{
	RemoveTimerFromGame();
}

void CStim::Save(idSaveGame *savefile) const
{
	CStimResponse::Save(savefile);

	m_Timer.Save(savefile);

	savefile->WriteInt(m_ResponseIgnore.Num());
	for (int i = 0; i < m_ResponseIgnore.Num(); i++)
	{
		m_ResponseIgnore[i].Save(savefile);
	}

	savefile->WriteBool(m_bUseEntBounds);
	savefile->WriteBool(m_bCollisionBased);
	savefile->WriteBool(m_bCollisionFired);

	// Don't save collision ents (probably not required)
	
	savefile->WriteInt(m_TimeInterleave);
	savefile->WriteInt(m_TimeInterleaveStamp);
	savefile->WriteInt(m_MaxFireCount);
	savefile->WriteFloat(m_Radius);
	savefile->WriteFloat(m_RadiusFinal);
	savefile->WriteBounds(m_Bounds);
	savefile->WriteVec3(m_Velocity);
	savefile->WriteFloat(m_Magnitude);
	savefile->WriteInt(m_FallOffExponent);
	savefile->WriteInt(m_MaxResponses);
	savefile->WriteInt(m_CurResponses);
	savefile->WriteInt(m_ApplyTimer);
	savefile->WriteInt(m_ApplyTimerVal);
}

void CStim::Restore(idRestoreGame *savefile)
{
	CStimResponse::Restore(savefile);

	m_Timer.Restore(savefile);

	int num;
	savefile->ReadInt(num);
	m_ResponseIgnore.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		m_ResponseIgnore[i].Restore(savefile);
	}

	savefile->ReadBool(m_bUseEntBounds);
	savefile->ReadBool(m_bCollisionBased);
	savefile->ReadBool(m_bCollisionFired);

	// Don't restore collision ents (probably not required)
	
	savefile->ReadInt(m_TimeInterleave);
	savefile->ReadInt(m_TimeInterleaveStamp);
	savefile->ReadInt(m_MaxFireCount);
	savefile->ReadFloat(m_Radius);
	savefile->ReadFloat(m_RadiusFinal);
	savefile->ReadBounds(m_Bounds);
	savefile->ReadVec3(m_Velocity);
	savefile->ReadFloat(m_Magnitude);
	savefile->ReadInt(m_FallOffExponent);
	savefile->ReadInt(m_MaxResponses);
	savefile->ReadInt(m_CurResponses);
	savefile->ReadInt(m_ApplyTimer);
	savefile->ReadInt(m_ApplyTimerVal);
}

void CStim::AddResponseIgnore(idEntity *e)
{
	if(CheckResponseIgnore(e) != true)
	{
		idEntityPtr<idEntity> entPtr;
		entPtr = e;
		m_ResponseIgnore.Append(entPtr);
	}
}

void CStim::RemoveResponseIgnore(idEntity *e)
{
	for (int i = 0; i < m_ResponseIgnore.Num(); i++)
	{
		if (m_ResponseIgnore[i].GetEntity() == e)
		{
			m_ResponseIgnore.RemoveIndex(i);
			break;
		}
	}
}

bool CStim::CheckResponseIgnore(idEntity* e)
{
	int n = m_ResponseIgnore.Num();

	for (int i = 0; i < n; i++)
	{
		if (m_ResponseIgnore[i].GetEntity() == e)
		{
			return true;
		}
	}

	return false;
}

void CStim::ClearResponseIgnoreList()
{
	m_ResponseIgnore.Clear();
}

float CStim::GetRadius()
{
	// greebo: Check for a time-dependent radius
	if (m_RadiusFinal > 0 && m_Duration != 0)
	{
		// Calculate how much of the stim duration has passed already
		float timeFraction = static_cast<float>(gameLocal.time - m_EnabledTimeStamp) / m_Duration;
		timeFraction = idMath::ClampFloat(0, 1, timeFraction);

		// Linearly interpolate the radius
		return m_Radius + (m_RadiusFinal - m_Radius) * timeFraction;
	}
	else // constant radius
	{
		return m_Radius;
	}
}

CStimResponseTimer* CStim::AddTimerToGame()
{
	gameLocal.m_StimTimer.AddUnique(this);
	m_Timer.SetTicks(sys->ClockTicksPerSecond()/1000);

	return(&m_Timer);
}

void CStim::RemoveTimerFromGame()
{
	gameLocal.m_StimTimer.Remove(this);
}

void CStim::PostFired (int numResponses)
{
}
