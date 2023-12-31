/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5525 $ (Revision of last commit) 
 $Date: 2012-08-15 12:27:31 -0400 (Wed, 15 Aug 2012) $ (Date of last commit)
 $Author: angua $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: LightGem.cpp 5525 2012-08-15 16:27:31Z angua $");

#include "LightGem.h"

// Temporary profiling related macros

//#define ENABLE_PROFILING

#ifdef ENABLE_PROFILING
#define PROFILE_BLOCK( block_tag )																				\
class __profile_block {																							\
	idTimer m_timeStamp;																						\
public:																											\
	__profile_block()  {																						\
	m_timeStamp.Start();																						\
	}																											\
	~__profile_block()  {																						\
	m_timeStamp.Stop();																							\
	gameLocal.Printf( #block_tag" : %lf \n\n", m_timeStamp.Milliseconds() );									\
	}																											\
}	_profile_blockInstance_##block_tag;																			\


#define PROFILE_BLOCK_START( block_tag )																		\
	idTimer timer##block_tag;																					\
	timer##block_tag.Start()																					\

// PROFILE_BLOCK_END requires PROFILE_BLOCK_START to be placed before it, to work.
#define PROFILE_BLOCK_END( block_tag )																			\
	timer##block_tag.Stop();																					\
	gameLocal.Printf( #block_tag": %lf \n", timer##block_tag.Milliseconds() )									\

#else

#define PROFILE_BLOCK( block_tag )
#define PROFILE_BLOCK_START( block_tag )
#define PROFILE_BLOCK_END( block_tag )	

#endif
//------------------------
// Construction/Destruction
//----------------------------------------------------
LightGem::LightGem()
{
	// our image buffer will be X*Y*Number of channels (RGB)*Size of internal storage type
	// this allocation will be destroyed in the destructor
	m_LightgemImgBuffer = (unsigned char*)malloc( (DARKMOD_LG_RENDER_WIDTH * DARKMOD_LG_RENDER_WIDTH * DARKMOD_LG_BPP) * sizeof(ILuint) );
}

LightGem::~LightGem()
{
	Deinitialize();
	free(m_LightgemImgBuffer);
}


//----------------------------------------------------
// Initialization
//----------------------------------------------------
void LightGem::Initialize()
{
}

void LightGem::Deinitialize()
{
}

void LightGem::Clear()
{
	m_LightgemSurface = NULL;
	m_LightgemShotSpot = 0;

	memset(m_LightgemShotValue, 0, sizeof(m_LightgemShotValue));
}

void LightGem::SpawnLightGemEntity( idMapFile *	a_mapFile )
{
	static const char *LightgemName = DARKMOD_LG_ENTITY_NAME;
	idMapEntity *mapEnt = a_mapFile->FindEntity(LightgemName);

	if ( mapEnt == NULL ) {
		mapEnt = new idMapEntity();
		a_mapFile->AddEntity(mapEnt);
		mapEnt->epairs.Set("classname", "func_static");
		mapEnt->epairs.Set("name", LightgemName);
		if ( strlen(cv_lg_model.GetString()) == 0 ) {
			mapEnt->epairs.Set("model", DARKMOD_LG_RENDER_MODEL);
		} else {
			mapEnt->epairs.Set("model", cv_lg_model.GetString());
		}
		mapEnt->epairs.Set("origin", "0 0 0");
		mapEnt->epairs.Set("noclipmodel", "1");
	}

	//memset(&m_Lightgem_rv, 0, sizeof(m_Lightgem_rv)); // should not be required

	//m_Lightgem_rv.globalMaterial = (idMaterial*)declManager->FindMaterial( "models/props/misc/lightgem_surface" );
	//m_Lightgem_rv.globalMaterial = gameLocal.GetGlobalMaterial();

	m_Lightgem_rv.viewID = DARKMOD_LG_VIEWID;
	
	m_Lightgem_rv.width = SCREEN_WIDTH;			// cv_lg_screen_width.GetInteger();
	m_Lightgem_rv.height = SCREEN_HEIGHT;		// cv_lg_screen_height.GetInteger();
	m_Lightgem_rv.fov_x = 
		m_Lightgem_rv.fov_y = DARKMOD_LG_RENDER_FOV;	// square, TODO: investigate lowering the value to increase performance on tall maps
	m_Lightgem_rv.x = 
		m_Lightgem_rv.y = 0;

	m_Lightgem_rv.viewaxis = idMat3(	
		 0.0f, 0.0f, 1.0f,
		 0.0f, 1.0f, 0.0f,
		-1.0f, 0.0f, 0.0f
		);

	//m_Lightgem_rv.forceUpdate = false;
	//m_Lightgem_rv.cramZNear = false;			// Needs testing
}

void LightGem::InitializeLightGemEntity( void )
{
	m_LightgemSurface = gameLocal.FindEntity(DARKMOD_LG_ENTITY_NAME);
	m_LightgemSurface.GetEntity()->GetRenderEntity()->allowSurfaceInViewID = DARKMOD_LG_VIEWID;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->suppressShadowInViewID = 0;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noDynamicInteractions = false;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noShadow = true;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noSelfShadow = true;

	DM_LOG(LC_LIGHT, LT_INFO)LOGSTRING("LightgemSurface: [%08lX]\r", m_LightgemSurface.GetEntity());
}

//----------------------------------------------------
// State Persistence 
//----------------------------------------------------

void LightGem::Save( idSaveGame & a_saveGame )
{
	m_LightgemSurface.Save( &a_saveGame );
	a_saveGame.WriteInt(m_LightgemShotSpot);
	for (int i = 0; i < DARKMOD_LG_MAX_RENDERPASSES; i++) {
		a_saveGame.WriteFloat(m_LightgemShotValue[i]);
	}
}

void LightGem::Restore( idRestoreGame & a_savedGame )
{
	m_LightgemSurface.Restore( &a_savedGame );
	a_savedGame.ReadInt(m_LightgemShotSpot);
	for (int i = 0; i < DARKMOD_LG_MAX_RENDERPASSES; i++) {
		a_savedGame.ReadFloat(m_LightgemShotValue[i]);
	}

	m_LightgemSurface.GetEntity()->GetRenderEntity()->allowSurfaceInViewID = DARKMOD_LG_VIEWID;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->suppressShadowInViewID = 0;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noDynamicInteractions = false;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noShadow = true;
	m_LightgemSurface.GetEntity()->GetRenderEntity()->noSelfShadow = true;

	DM_LOG(LC_LIGHT, LT_INFO)LOGSTRING("LightgemSurface: [%08lX]\r", m_LightgemSurface.GetEntity());
}

//----------------------------------------------------
// Calculation
//----------------------------------------------------

float LightGem::Calculate(idPlayer *player)
{
	PROFILE_BLOCK( LightGem_Calculate );
	PROFILE_BLOCK_START( LightGem_Calculate_Setup);

	// If player is hidden (i.e the whole player entity is actually hidden)
	if ( player->GetModelDefHandle() == -1 ) {
		return 0.0f;
	}

	{ // Get position for lg
		idEntity* lg = m_LightgemSurface.GetEntity();
		renderEntity_t* prent = lg->GetRenderEntity();

		const idVec3& Cam = player->GetEyePosition();
		idVec3 LGPos = player->GetPhysics()->GetOrigin();// Set the lightgem position to that of the player

		LGPos.x += (Cam.x - LGPos.x) * 0.3f + cv_lg_oxoffs.GetFloat(); // Move the lightgem out a fraction along the leaning x vector
		LGPos.y += (Cam.y - LGPos.y) * 0.3f + cv_lg_oyoffs.GetFloat(); // Move the lightgem out a fraction along the leaning y vector
	
		// Prevent lightgem from clipping into the floor while crouching
		if ( static_cast<idPhysics_Player*>(player->GetPlayerPhysics())->IsCrouching() ) {
			LGPos.z += 50.0f + cv_lg_ozoffs.GetFloat() ;
		} else {
			LGPos.z = Cam.z + cv_lg_ozoffs.GetFloat(); // Set the lightgem's Z-axis position to that of the player's eyes
		}

		m_Lightgem_rv.vieworg = LGPos;
		lg->SetOrigin(LGPos); // Move the lightgem testmodel to the players feet based on the eye position

		gameRenderWorld->UpdateEntityDef(lg->GetModelDefHandle(), prent); // Make sure the lg is in the updated position
	}

	// Give the rv the current ambient light values - Not all of the other values, avoiding fancy effects.
	m_Lightgem_rv.shaderParms[2] = gameLocal.globalShaderParms[2]; // Ambient R
	m_Lightgem_rv.shaderParms[3] = gameLocal.globalShaderParms[3]; // Ambient G
	m_Lightgem_rv.shaderParms[4] = gameLocal.globalShaderParms[4]; // Ambient B

	// angua: render view needs current time, otherwise it will be unable to see time-dependent changes in light shaders such as flickering torches
	m_Lightgem_rv.time = gameLocal.GetTime();

	// Make sure the player model is hidden in the lightgem renders
	renderEntity_t* prent = player->GetRenderEntity();
	const int pdef = player->GetModelDefHandle();
	const int playerid = prent->suppressSurfaceInViewID;
	const int psid = prent->suppressShadowInViewID;
	prent->suppressShadowInViewID = DARKMOD_LG_VIEWID;
	prent->suppressSurfaceInViewID = DARKMOD_LG_VIEWID;

	// And the player's head 
	renderEntity_t* hrent = player->GetHeadEntity()->GetRenderEntity();
	const int hdef = player->GetHeadEntity()->GetModelDefHandle();
	const int headid = hrent->suppressSurfaceInViewID;
	const int hsid = hrent->suppressShadowInViewID;
	hrent->suppressShadowInViewID = DARKMOD_LG_VIEWID;
	hrent->suppressSurfaceInViewID = DARKMOD_LG_VIEWID;

	// Let the game know about the changes
	gameRenderWorld->UpdateEntityDef(pdef, prent); 
	gameRenderWorld->UpdateEntityDef(hdef, hrent);

	DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("RenderTurn %u", m_LightgemShotSpot);
	PROFILE_BLOCK_END( LightGem_Calculate_Setup);

	//
	// Render passes of the lightgem, screenshots + comp done in loop
	//

	float fRetVal = 0.0f;
	const int k = cv_lg_hud.GetInteger() - 1;
	static const int nRenderPasses = cv_lg_renderpasses.GetInteger();

	renderSystem->CropRenderSize(DARKMOD_LG_RENDER_WIDTH, DARKMOD_LG_RENDER_WIDTH, true, true);

	for (int i = 0; i < nRenderPasses; i++)	{
		PROFILE_BLOCK( LightGem_Calculate_ForLoop);

		// If splitting is enabled and it is not the turn for the shot, we skip it.
		if ( cv_lg_split.GetBool() && m_LightgemShotSpot != i ) {
			continue;
		}

		m_LightgemShotValue[i] = 0.0f;

		// Render up and down alternately 
		m_Lightgem_rv.viewaxis.TransposeSelf();

		// If the hud is enabled we either process all of them in case it is set to 0,
		// then we don't care which one is actually displayed (most likely the last or
		// the first one), or we only show the one that should be shown.
		if ( k == -1 || k == i ) {
			// We always use a square image, because we render now an overhead shot which
			// covers all four side of the player at once, using a diamond or pyramid shape.
			// The result is an image that is split in four triangles with an angle of 
			// 45 degree, thus the square shape.
			PROFILE_BLOCK_START	( LightGem_Calculate_ForLoop_RenderScene );

			gameRenderWorld->SetRenderView(&m_Lightgem_rv); // most likely not needed
			gameRenderWorld->RenderScene(&m_Lightgem_rv);

			PROFILE_BLOCK_END	( LightGem_Calculate_ForLoop_RenderScene );

			PROFILE_BLOCK_START	( LightGem_Calculate_ForLoop_CaptureRenderToBuffer );
			DM_LOG(LC_LIGHT, LT_DEBUG)LOGSTRING("Rendering to lightgem render buffer\n");

			renderSystem->CaptureRenderToBuffer(m_LightgemImgBuffer);
			PROFILE_BLOCK_END	( LightGem_Calculate_ForLoop_CaptureRenderToBuffer );

#if 0
			{ // Save render if we have a path specified (for debugging)
				const char* dp = cv_lg_path.GetString();
				if ( dp[0] ) {
					idStr dps;
					sprintf(dps, "screenshots/%s_%u", dp, i);

					g_Global.m_RenderImage.SaveImageToVfs((dps + idStr("_new.tga")).c_str());
					renderSystem->CaptureRenderToFile((dps + idStr("_old.tga")).c_str());
				}
			}
#endif

			PROFILE_BLOCK_START	( LightGem_Calculate_ForLoop_AnalyzeRenderImage );
			AnalyzeRenderImage();
			PROFILE_BLOCK_END	( LightGem_Calculate_ForLoop_AnalyzeRenderImage );

			PROFILE_BLOCK_START	( LightGem_Calculate_ForLoop_Cleanup );

			// Check which of the images has the brightest value, and this is what we will use.
			for (int l = 0; l < DARKMOD_LG_MAX_IMAGESPLIT; l++) {
				if (m_fColVal[l] > m_LightgemShotValue[i]) {
					m_LightgemShotValue[i] = m_fColVal[l];
				}
			}

			PROFILE_BLOCK_END	( LightGem_Calculate_ForLoop_Cleanup );
		}
	}

	renderSystem->UnCrop();

	PROFILE_BLOCK_START	( LightGem_Calculate_UnSetup );

	// and switch back our normal render definition - player model and head are returned
	prent->suppressSurfaceInViewID = playerid;
	prent->suppressShadowInViewID = psid;
	hrent->suppressSurfaceInViewID = headid;
	hrent->suppressShadowInViewID = hsid;
	gameRenderWorld->UpdateEntityDef(pdef, prent);
	gameRenderWorld->UpdateEntityDef(hdef, hrent);

	m_LightgemShotSpot++;
	if( m_LightgemShotSpot >= nRenderPasses ) {
		m_LightgemShotSpot = 0;
	}

	// we want to return the highest shot value
	for (int i = 0; i < nRenderPasses; i++) {
		if ( m_LightgemShotValue[i] > fRetVal ) {
			fRetVal = m_LightgemShotValue[i];
		}
	}

	PROFILE_BLOCK_END	( LightGem_Calculate_UnSetup );

	return fRetVal;
}

void LightGem::AnalyzeRenderImage()
{
	const unsigned char *buffer = m_LightgemImgBuffer;
	
	// The lightgem will simply blink if the renderbuffer doesn't work.
	if ( buffer == NULL ) {
		DM_LOG(LC_SYSTEM, LT_ERROR)LOGSTRING("Unable to read image from lightgem render-buffer\r");

		for ( int i = 0; i < DARKMOD_LG_MAX_IMAGESPLIT; i++ ) {
			m_fColVal[i] = (gameLocal.time % 1024 ) > 512;
		}

		return;
	}
	
	/* 	Split up the image into the 4 triangles

		 \11/	0 - east of lightgem render
		3 \/ 0	1 - north of lg
		3 /\ 0	2 - south of lg
		 /22\	3 - west of lg
	
		Note : Serp - This is a simplification of the early version which used two nested loops
	*/

	int in = 0;

	for ( int x = 0; x < DARKMOD_LG_RENDER_WIDTH; x++ ) {
		for ( int y = 0; y < DARKMOD_LG_RENDER_WIDTH; y++, buffer += DARKMOD_LG_BPP ) { // increment the buffer pos
			if ( y <= x && x + y >= (DARKMOD_LG_RENDER_WIDTH -1) ) {
				in = 0;
			} else if ( y < x ) {
				in = 1;
			} else if ( y > (DARKMOD_LG_RENDER_WIDTH -1) - x ) {
				in = 2;
			} else {
				in = 3;
			}

			// The order is RGB. NOTE: Serp - I have moved the scale into the rgb multiplier
			// this is to move the calculation into the precompiler (could move to .h directly)
			m_fColVal[in] += (buffer[0] * (DARKMOD_LG_RED  * DARKMOD_LG_SCALE) +
							  buffer[1] * (DARKMOD_LG_GREEN* DARKMOD_LG_SCALE) +
							  buffer[2] * (DARKMOD_LG_BLUE * DARKMOD_LG_SCALE));
		}
	}

	// Calculate the average for each value
	// Could be moved to the return
	m_fColVal[0] *= DARKMOD_LG_TRIRATIO;
	m_fColVal[1] *= DARKMOD_LG_TRIRATIO;
	m_fColVal[2] *= DARKMOD_LG_TRIRATIO;
	m_fColVal[3] *= DARKMOD_LG_TRIRATIO;
}
