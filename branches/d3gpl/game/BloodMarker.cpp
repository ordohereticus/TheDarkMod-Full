/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5121 $ (Revision of last commit) 
 $Date: 2011-12-11 14:12:26 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "BloodMarker.h"
#include "StimResponse/Stim.h"

const idEventDef EV_GenerateBloodSplat("TDM_GenerateBloodSplat", NULL);

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

		PostEventMS(&EV_Remove, 1000);
	}
}

void CBloodMarker::Init(const idStr& splat, const idStr& splatFading, float size)
{
	_bloodSplat = splat;
	_bloodSplatFading = splatFading;

	// randomly rotate the decal winding
	_angle = gameLocal.random.RandomFloat() * idMath::TWO_PI;
	_size = size;
	_isFading = false;

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

//-----------------------------------------------------------------------------------

void CBloodMarker::Save( idSaveGame *savefile ) const
{
	savefile->WriteString(_bloodSplat);
	savefile->WriteString(_bloodSplatFading);
	savefile->WriteFloat(_angle);
	savefile->WriteFloat(_size);
	savefile->WriteBool(_isFading);
}

//-----------------------------------------------------------------------------------

void CBloodMarker::Restore( idRestoreGame *savefile )
{
	savefile->ReadString(_bloodSplat);
	savefile->ReadString(_bloodSplatFading);
	savefile->ReadFloat(_angle);
	savefile->ReadFloat(_size);
	savefile->ReadBool(_isFading);
}
