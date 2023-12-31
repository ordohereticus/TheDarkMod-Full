/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5612 $ (Revision of last commit) 
 $Date: 2012-10-28 01:12:59 -0400 (Sun, 28 Oct 2012) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: BloodMarker.cpp 5612 2012-10-28 05:12:59Z greebo $");

#include "Game_local.h"
#include "BloodMarker.h"
#include "StimResponse/Stim.h"

const idEventDef EV_GenerateBloodSplat("_TDM_GenerateBloodSplat", EventArgs(), EV_RETURNS_VOID, "internal");

CLASS_DECLARATION( idEntity, CBloodMarker )
	EVENT( EV_GenerateBloodSplat, CBloodMarker::Event_GenerateBloodSplat )
END_CLASS

void CBloodMarker::Event_GenerateBloodSplat()
{
	idVec3 dir = gameLocal.GetGravity();
	dir.Normalize();

	if (!_isFading)
	{
		// Read the stay duration from the material info
		const idMaterial* material = declManager->FindMaterial(_bloodSplat);

		if (material != NULL)
		{
			gameLocal.ProjectDecal(GetPhysics()->GetOrigin(), dir, 3, false, _size, _bloodSplat, _angle);

			PostEventMS(&EV_GenerateBloodSplat, material->GetDecalInfo().stayTime);
		}
		else 
		{
			gameLocal.Warning("Cannot find blood splat decal %s", _bloodSplat.c_str());
		}
	}
	else
	{
		// We're fading, just spawn one last decal and schedule our removal
		gameLocal.ProjectDecal(GetPhysics()->GetOrigin(), dir, 3, false, _size, _bloodSplatFading, _angle);

		// grayman #3075 - notify the AI who spilled the blood that
		// we're going away.

		if ( _spilledBy != NULL )
		{
			_spilledBy->SetBlood(NULL);
			_spilledBy = NULL;
		}

		PostEventMS(&EV_Remove, 1000);
	}
}

void CBloodMarker::Init(const idStr& splat, const idStr& splatFading, float size, idAI* bleeder) // grayman #3075 - add who bled
{
	_bloodSplat = splat;
	_bloodSplatFading = splatFading;

	// randomly rotate the decal winding
	_angle = gameLocal.random.RandomFloat() * idMath::TWO_PI;
	_size = size;
	_isFading = false;

	// grayman #3075 - note who spilled this blood
	_spilledBy = bleeder;
	bleeder->SetBlood(this);

	AddResponse(ST_WATER);
	EnableResponse(ST_WATER);
}

void CBloodMarker::OnStim(const CStimPtr& stim, idEntity* stimSource)
{
	// Call the base class in any case
	idEntity::OnStim(stim, stimSource);

	if (stim->m_StimTypeId == ST_WATER)
	{
		_isFading = true;
	}
}

// grayman #3075

idAI* CBloodMarker::GetSpilledBy(void)
{
	return _spilledBy;
}

//-----------------------------------------------------------------------------------

void CBloodMarker::Save( idSaveGame *savefile ) const
{
	savefile->WriteString(_bloodSplat);
	savefile->WriteString(_bloodSplatFading);
	savefile->WriteFloat(_angle);
	savefile->WriteFloat(_size);
	savefile->WriteBool(_isFading);
	savefile->WriteObject(_spilledBy); // grayman #3075
}

//-----------------------------------------------------------------------------------

void CBloodMarker::Restore( idRestoreGame *savefile )
{
	savefile->ReadString(_bloodSplat);
	savefile->ReadString(_bloodSplatFading);
	savefile->ReadFloat(_angle);
	savefile->ReadFloat(_size);
	savefile->ReadBool(_isFading);
	savefile->ReadObject(reinterpret_cast<idClass *&>(_spilledBy)); // grayman #3075
}
