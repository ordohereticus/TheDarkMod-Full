// vim:ts=4:sw=4:cindent
/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4071 $
 * $Date: 2010-07-18 15:57:08 +0200 (Sun, 18 Jul 2010) $
 * $Author: tels $
 *
 ***************************************************************************/


/*
	StaticMulti - 	A variant of func_static that can use a idPhys_StaticMulti
					for the clipmodel, e.g. has more than one clipmodel. Used
					for entities with many models combined into one rendermodel.

	Copyright (C) 2010-2011 Tels (Donated to The Dark Mod Team)

TODO: track skin changes on the different LOD stages

*/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: StaticMulti.cpp 4071 2010-07-18 13:57:08Z tels $", init_version);

#include "StaticMulti.h"

// if defined, debug output
//#define M_DEBUG 1

// uncomment to measure time to update rendermodels
//#define M_TIMINGS

#ifdef M_TIMINGS
static idTimer timer_updatemodel, timer_total;
int updates = 0;
#endif



//===============================================================
CLASS_DECLARATION( idStaticEntity, CStaticMulti )
	EVENT( EV_Activate,				CStaticMulti::Event_Activate )
END_CLASS

/*
===============
CStaticMulti::CStaticMulti
===============
*/
CStaticMulti::CStaticMulti( void )
{
	active = false;
	m_bNeedModelUpdates = true;
	m_LOD = NULL;

	m_Changes.Clear();
	m_Offsets = NULL;
	m_hModel = NULL;
	m_modelName = "";

	// do not free the hModel of the intial entity (we only do so once we
	// have created our own
	m_bFree_hModel = false;

	m_iVisibleModels = 0;
	m_iMaxChanges = 1;
	
	m_DistCheckTimeStamp = 0;
	m_DistCheckInterval = 0;
	m_fHideDistance = 0.0f;
	m_bDistCheckXYOnly = false;
}

CStaticMulti::~CStaticMulti()
{
	// no need to free these as they are just ptr to a copy
	m_LOD = NULL;

	// make sure the render entity is freed before the model is freed
	FreeModelDef();

	if (m_bFree_hModel)
	{
		renderModelManager->FreeModel( renderEntity.hModel );
		// need this to avoid crashes due to double-free
		renderEntity.hModel = NULL;
	}
}

/*
===============
CStaticMulti::Spawn
===============
*/
void CStaticMulti::Spawn( void )
{
	bool solid = spawnArgs.GetBool( "solid" );

	// ishtvan fix : Let clearing contents happen naturally on Hide instead of
	// checking hidden here and clearing contents prematurely
	if ( solid )
	{
		GetPhysics()->SetContents( CONTENTS_SOLID | CONTENTS_OPAQUE );
	}

	// SR CONTENTS_RESONSE FIX
	if( m_StimResponseColl->HasResponse() )
		GetPhysics()->SetContents( GetPhysics()->GetContents() | CONTENTS_RESPONSE );

	m_bNeedModelUpdates = true;

	if ( spawnArgs.GetBool( "no_model_updates", "0" ) )
	{
		gameLocal.Printf ("%s: Disabling render model updates.\n", GetName() );
		m_bNeedModelUpdates = false;
	}

	int d = (int) (1000.0f * spawnArgs.GetFloat( "dist_check_period", "0" ));
	if (d <= 0)
	{
		d = 0;
	}
	else
	{
		m_bDistCheckXYOnly = spawnArgs.GetBool( "dist_check_xy", "0" );
		m_DistCheckInterval = d;
		m_DistCheckTimeStamp = gameLocal.time - (int) (m_DistCheckInterval * (1.0f + gameLocal.random.RandomFloat()) );
		m_fHideDistance = spawnArgs.GetFloat( "hide_distance", "0.0" );
	}
}

/*
================
CStaticMulti::SetLODData

Store the data like our megamodel (the visible combined rendermodel including data how to
assemble it), and the LOD stages (contain the distance for each LOD stage)
================
*/
void CStaticMulti::SetLODData( const idVec3 &origin, lod_data_t *LOD, idStr modelName, idList<model_ofs_t>* offsets, idStr materialName, const idRenderModel* hModel, const idClipModel* clipModel )
{
	idClipModel *clip;
	bool clipLoaded = false;
	active = true;

	/* ptr to the shared data structures */
	m_LOD = LOD;
	m_Offsets = offsets;

	m_MaterialName = materialName;

#ifdef M_DEBUG
	gameLocal.Printf("%s SetLODData: LOD %p, hModel %p, model %s, offsets %p (%i).\n",
			GetName(), m_LOD, hModel, modelName.c_str(), m_Offsets, m_Offsets ? m_Offsets->Num() : -1 );
#endif

	m_iVisibleModels = m_Offsets->Num();

	// if we need to combine from a func_static, store a ptr to it's renderModel
	m_hModel = hModel;

	// in case it doesn't have LOD
	m_modelName = modelName;

	// set this to false if we don't have LOD
	if (!LOD)
	{
		m_bNeedModelUpdates = false;
	}

	m_Changes.Clear();
	// avoid frequent resizes
	m_Changes.SetGranularity(32);

	// generate the first render model
	UpdateRenderModel(true);

	// Generate our physics model
	physics.SetSelf( this );
	physics.SetOrigin( origin );
	physics.SetAxis( mat3_identity );

	bool solid = spawnArgs.GetBool( "solid" );

	physics.SetContents( CONTENTS_RENDERMODEL );
	if (solid)
	{
		clipLoaded = true;
		// load the clipmodel for the lowest LOD stage for collision detection
		if (clipModel)
		{
			// will make a copy below
			clip = new idClipModel( clipModel );
#ifdef M_DEBUG
			gameLocal.Printf("Reusing clipmodel from renderModel 0x%p, bounds %s.\n", clipModel, clip->GetBounds().ToString() );
#endif
		}
		else
		{
#ifdef M_DEBUG
			gameLocal.Printf("Loading clip for %s.\n", modelName.c_str());
#endif
			clip = new idClipModel;
			clipLoaded = clip->LoadModel( modelName );
		}
	}

	// will be false for non-solid
	if (clipLoaded)
	{
		model_ofs_t* o = offsets->Ptr();
		for (int idx = 0; idx < m_iVisibleModels; idx++)
		{
			int i = idx + 1;
			// add a new copy of the clipmodel for each position
			idClipModel *c = new idClipModel( clip );
			physics.SetClipModel( c, 1.0f, i, true);
			physics.SetOrigin( origin + o[idx].offset, i);
			physics.SetAxis( o[idx].angles.ToMat3(), i);
			// Scale the clipmodel
			physics.Scale( o[idx].scale, i );
			// Make it solid
			physics.SetContents( MASK_SOLID | CONTENTS_MOVEABLECLIP | CONTENTS_RENDERMODEL, i );
		}
		// TODO: nec.?
		physics.SetClipMask( MASK_SOLID | CONTENTS_MOVEABLECLIP | CONTENTS_RENDERMODEL);
	}
#ifdef M_DEBUG
			gameLocal.Printf("%s: SetLODData done.\n", GetName() );
#endif
}

/*
===============
CStaticMulti::UpdateRenderModel

Updates the rendermodel if nec. and returns true if it was updated.
===============
*/
bool CStaticMulti::UpdateRenderModel( const bool force )
{
	if ( (!force) && 
		 (active == false ||
		  // During Restore() the offsets might not yet be there, so just return
		 (!m_Offsets) ||
		 m_Changes.Num() < m_iMaxChanges ) )
	{
		// no update nec. yet:
		return false;
	}

#ifdef M_TIMINGS
	updates ++;
	timer_total.Start();
#endif

	idVec3 origin = GetPhysics()->GetOrigin();
	int n = m_Changes.Num();

#ifdef M_DEBUG
	idAngles angles = GetPhysics()->GetAxis().ToAngles();
	gameLocal.Printf("%s updating renderModel at %s (angles %s) with %i changes (%i models).\n", GetName(), origin.ToString(), angles.ToString(), n, m_Offsets->Num());
#endif

	// apply all our changes to the offsets list
	for (int i = 0; i < n; i++)
	{
#ifdef M_DEBUG
		gameLocal.Printf("%s updating offset %i from LOD %i to %i\n", GetName(), i, m_Offsets->Ptr()[ m_Changes[i].entity ].lod, m_Changes[i].newLOD);
#endif
		m_Offsets->Ptr()[ m_Changes[i].entity ].lod = m_Changes[i].newLOD;
	}
	// now clear the changes
	m_Changes.Clear();

	// count visible models
	m_iVisibleModels = 0;
	model_ofs_t* p = m_Offsets->Ptr();
	n = m_Offsets->Num();
	for (int i = 0; i < n; i++)
	{
		if (p[ i ].lod >= 0)
		{
			m_iVisibleModels ++;
		}
	}

#ifdef M_DEBUG
		gameLocal.Printf("Has %i visible models.\n", m_iVisibleModels);
#endif
	
	if (m_iVisibleModels == 0)
	{
		// no visible models, do not update rendermodel (this would crash),
		// instead just hide ourselves
#ifdef M_DEBUG
		gameLocal.Printf ("%s: All models invisible, hiding myself.\n", GetName() );
#endif
		if (!fl.hidden) { Hide(); }
		return false;
	}

	// compute a list of rendermodels
	idList< const idRenderModel*> LODs;
	// default model
	if (m_hModel)
	{
		LODs.Append(m_hModel);
	}
	else
	{
		idStr m = m_modelName;
		if (m_LOD)
		{
			m = m_LOD->ModelLOD[0];
		}
		const idRenderModel* hModel = NULL;
		if (!m.IsEmpty())
		{
			hModel = renderModelManager->FindModel( m );
			if (!hModel)
			{
				gameLocal.Warning("Could not load model %s.\n", m.c_str() );
			}
		}
		LODs.Append(hModel);
	}

	for (int i = 0; i < LOD_LEVELS; i ++)
	{
		// func_static as source?
		if (m_hModel)
		{
			LODs.Append(m_hModel);
			continue;
		}
		idStr m = m_modelName;
		if (m_LOD)
		{
			m = m_LOD->ModelLOD[i];
		}
		const idRenderModel* hModel = NULL;
		if (!m.IsEmpty())
		{
			hModel = renderModelManager->FindModel( m );
			if (!hModel)
			{
				gameLocal.Warning("Could not load model %s.\n", m.c_str() );
			}
		}
		LODs.Append(hModel);
	}
	const idList< const idRenderModel*> *l = &LODs;

	const idMaterial* m = NULL;
	if (!m_MaterialName.IsEmpty() )
	{
		declManager->FindMaterial( m_MaterialName, false );
	}

#ifdef M_TIMINGS
	timer_updatemodel.Start();
#endif

	if (force)
	{
		if (renderEntity.hModel)
		{
#ifdef M_DEBUG
			gameLocal.Printf("StaticMulti %s: Allocating new render model.\n", GetName());
#endif
			FreeModelDef();
			// do not free the rendermodel, somebody else (since the dummy model is used) has a ptr to it
			// renderModelManager->FreeModel( renderEntity.hModel );
		}
		// signal Restore() that it must free the hModel from now on
		m_bFree_hModel = true;
		renderEntity.hModel = gameLocal.m_ModelGenerator->DuplicateLODModels( l, "megamodel", m_Offsets, &origin, m );
	}
	else
	{
#ifdef M_DEBUG
			gameLocal.Printf("StaticMulti %s: Updating existing render model.\n", GetName());
#endif
		if (!renderEntity.hModel)
		{
			renderEntity.hModel = renderModelManager->AllocModel();
		}
		// and now re-use the already existing object
		gameLocal.m_ModelGenerator->DuplicateLODModels( l, "megamodel", m_Offsets, &origin, m, renderEntity.hModel);
	}

//	for (int ii =0; ii < 100; ii++)
//	{
//		gameLocal.m_ModelGenerator->DuplicateLODModels( l, "megamodel", m_Offsets, &origin, m, renderEntity.hModel);
//	}
#ifdef M_TIMINGS
	timer_updatemodel.Stop();
#endif

	// force an update because the bounds/origin/axis may stay the same while the model changes
	renderEntity.forceUpdate = true;

	// add to refresh list
	if ( modelDefHandle == -1 ) {
		modelDefHandle = gameRenderWorld->AddEntityDef( &renderEntity );
	} else {
		gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );
	}

	// now show ourselves
#ifdef M_DEBUG
	gameLocal.Printf ("%s: Some models visible, showing myself.\n", GetName() );
#endif
	if (fl.hidden)
	{
		Show();
#ifdef M_DEBUG
		gameLocal.Printf ("%s: Was hidden, showing myself.\n", GetName() );
#endif
	}

#ifdef M_TIMINGS
	timer_total.Stop();
#endif

	return true;
}

/*
================
CStaticMulti::Think
================
*/
void CStaticMulti::Think( void ) 
{
	lod_data_t* LOD;

	// Distance dependence checks
	if ( active && m_bNeedModelUpdates && (gameLocal.time - m_DistCheckTimeStamp) >= m_DistCheckInterval ) 
	{
#ifdef M_TIMINGS
		if (updates > 0)
		{
			gameLocal.Printf( "%s: updates %i, total time %0.2f ms (per update %0.2f ms), rendermodel %0.2f ms (per update %0.2f ms)\n",
					GetName(),
					updates,
					timer_total.Milliseconds(),
					timer_total.Milliseconds() / updates,
					timer_updatemodel.Milliseconds(),
					timer_updatemodel.Milliseconds() / updates );
		}
#endif

		m_DistCheckTimeStamp = gameLocal.time;

		idVec3 origin = GetPhysics()->GetOrigin();
		idVec3 vGravNorm = GetPhysics()->GetGravityNormal();

//		gameLocal.Printf("%s thinking at %s.\n", GetName(), origin.ToString());

		// TODO: skip this if the distance to the player has not changed

		// Calculate the offset for each model position alone, so precompute certain constants:
		float lod_bias = cv_lod_bias.GetFloat(); lod_bias *= lod_bias;
		idVec3 playerOrigin = gameLocal.GetLocalPlayer()->GetPhysics()->GetOrigin();

		bool bDistCheckXYOnly = false;

		if (!m_LOD)
		{
			// TODO: fill this structure
			LOD = NULL;
		}
		else
		{
			LOD = m_LOD;
			bDistCheckXYOnly = LOD->bDistCheckXYOnly ? true : false;
		}

#ifdef M_DEBUG_1
		if (LOD)
		{
			gameLocal.Printf("%s LOD data %p.\n", GetName(), LOD );
				gameLocal.Printf(" checkXY %i\n", LOD->bDistCheckXYOnly );
				gameLocal.Printf(" interval %i\n", LOD->DistCheckInterval );
			for (int i = 0; i < LOD_LEVELS; i ++)
			{
				gameLocal.Printf(" LOD %i dist %0.2f.\n", i, LOD->DistLODSq[i] );
				gameLocal.Printf(" LOD %i model %s.\n",   i, LOD->ModelLOD[i].c_str() );
				gameLocal.Printf(" LOD %i skin %s.\n",    i, LOD->SkinLOD[i].c_str() );
				gameLocal.Printf(" LOD %i offset %s.\n",  i, LOD->OffsetLOD[i].ToString() );
			}
		}
#endif

		// TODO: go through all offsets and calculate the new LOD stage
		int num = m_Offsets->Num();
		for (int i = 0; i < num; i++)
		{
			model_ofs_t ofs = m_Offsets->Ptr()[i];
			// 0 => default model, 1 => first stage etc
			int orgLOD = ofs.lod - 1;
			m_LODLevel = orgLOD;

			idVec3 delta = origin + ofs.offset - playerOrigin;
			if (bDistCheckXYOnly)
			{
				delta -= (vGravNorm * delta) * vGravNorm;
			}
			// divide by the user LOD bias setting (squared)
			float dist = delta.LengthSqr() / lod_bias;

			float fAlpha  = ThinkAboutLOD( LOD, dist );

			if (fAlpha == 0)
			{
				// the entity should be invisible
				m_LODLevel = -2;
			}
			// if differs, add a changeset
			// TODO: compare flags, if they differ, also add a change
			if (orgLOD != m_LODLevel)
			{
#ifdef M_DEBUG
				gameLocal.Printf("%s: changing from LOD %i to %i\n", GetName(), orgLOD, m_LODLevel);
#endif
				// 0 => default model, 1 => first stage etc
				// TODO: compute flags for noclip
				int flags = 0;
				m_LODLevel ++;
				if ( LOD && LOD->noshadowsLOD & (1 << m_LODLevel) )
				{
					flags += SEED_MODEL_NOSHADOW;
				}
				AddChange(i, m_LODLevel, flags);
			}
		}

		// restore our value (it is not used, anyway)
		m_LODLevel = 0;

		// update the render model if nec.
		UpdateRenderModel();
	}

#ifdef M_DEBUG
	idPhysics *p = GetPhysics();
   	idVec4 markerColor (0.3, 0.8, 1.0, 1.0);
   	idVec4 centerColor (1, 0.8, 0.2, 1.0);
   	idVec3 arrowLength (0.0, 0.0, 50.0);

	// our center
	idVec3 org = renderEntity.origin;
    gameRenderWorld->DebugArrow
			(
			centerColor,
			org + arrowLength * 2,
			org,
			3,
	    	1 );
	int num = p->GetNumClipModels();

	// DEBUG draw arrows for each part of the physics object
	for (int i = 0; i < num; i++)
	{
		idVec3 org = p->GetOrigin( i );
	    gameRenderWorld->DebugArrow
			(
			markerColor,
			org + arrowLength,
			org,
			3,
	    	1 );
	}
#endif
}

void CStaticMulti::Save( idSaveGame *savefile ) const {

	savefile->WriteBool( active );
	savefile->WriteBool( m_bNeedModelUpdates );
	savefile->WriteStaticObject( physics );
	savefile->WriteInt( m_DistCheckInterval );
	savefile->WriteInt( m_DistCheckTimeStamp );
	savefile->WriteBool( m_bDistCheckXYOnly );
	savefile->WriteString( m_MaterialName );
	savefile->WriteString( m_modelName );
	savefile->WriteInt( m_iVisibleModels );
	savefile->WriteBool( m_bFree_hModel );

	savefile->WriteInt( m_Changes.Num() );
	for (int i = 0; i < m_Changes.Num(); i++ )
	{
		savefile->WriteInt( m_Changes[i].entity );
		savefile->WriteInt( m_Changes[i].oldLOD );
		savefile->WriteInt( m_Changes[i].newLOD );
		savefile->WriteInt( m_Changes[i].oldFlags );
		savefile->WriteInt( m_Changes[i].newFlags );
	}
}

void CStaticMulti::Restore( idRestoreGame *savefile )
{
	int n;

	//gameLocal.Printf("Restoring %s\n", GetName());

	savefile->ReadBool( active );
	savefile->ReadBool( m_bNeedModelUpdates );
	savefile->ReadStaticObject( physics );
	RestorePhysics( &physics );
	savefile->ReadInt( m_DistCheckInterval );
	savefile->ReadInt( m_DistCheckTimeStamp );
	savefile->ReadBool( m_bDistCheckXYOnly );
	savefile->ReadString( m_MaterialName );
	savefile->ReadString( m_modelName );
	savefile->ReadInt( m_iVisibleModels );
	savefile->ReadBool( m_bFree_hModel );

	// These will be set by the SEED entity managing us
	// no update until the data is there!
	m_Offsets = NULL;
	m_LOD = NULL;
	m_hModel = NULL;

	m_Changes.Clear();
	savefile->ReadInt( n );
	m_Changes.SetGranularity(64);
	m_Changes.SetNum(n);
	for (int i = 0; i < n; i ++)
	{
		savefile->ReadInt( m_Changes[i].entity );
		savefile->ReadInt( m_Changes[i].oldLOD );
		savefile->ReadInt( m_Changes[i].newLOD );
		savefile->ReadInt( m_Changes[i].oldFlags );
		savefile->ReadInt( m_Changes[i].newFlags );
	}

	// stop us freeing it later, but not now as it is a shared ptr
	renderEntity.hModel = NULL;

	// hide until the LOD data arrives
	//if (!fl.hidden) { Hide(); }

	BecomeInactive( TH_THINK | TH_PHYSICS );
}

/*
================
CStaticMulti::Event_Activate
================
*/
void CStaticMulti::Event_Activate( idEntity *activator ) {

	int spawnTime = gameLocal.time;

	active = !active;

	const idKeyValue *kv = spawnArgs.FindKey( "hide" );
	if ( kv ) {
		if ( IsHidden() ) {
			Show();
		} else {
			Hide();
		}
	}

	renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( spawnTime );
	renderEntity.shaderParms[5] = active;
	// this change should be a good thing, it will automatically turn on 
	// lights etc.. when triggered so that does not have to be specifically done
	// with trigger parms.. it MIGHT break things so need to keep an eye on it
	renderEntity.shaderParms[ SHADERPARM_MODE ] = ( renderEntity.shaderParms[ SHADERPARM_MODE ] ) ?  0.0f : 1.0f;
	BecomeActive( TH_UPDATEVISUALS );
}

/*
===============
CStaticMulti::AddChange
===============
*/
void CStaticMulti::AddChange( const int entity, const int newLOD, const int newFlags ) {

	if (!m_Offsets)
	{
		// During Restore() the offsets might not yet be there, so just return
		return;
	}

	// go through our changes and see if we already have one for this entity
	int n = m_Changes.Num();

	for (int i = 0; i < n; i++)
	{
		if (m_Changes[i].entity == entity)
		{
			// If the new change changes this back to what it already was,
			// remove the change set:
			// TODO: track skin changes
			if ( ( m_Changes[i].oldLOD == newLOD ) &&
				 ( m_Changes[i].oldFlags == newFlags ) )
			{
#ifdef M_DEBUG
				gameLocal.Printf("%s: Removing change for entity %i\n", GetName(), entity);
#endif
				// false => we don't need the list to be kept sorted
				m_Changes.RemoveIndex(i,false);
			}
			else
			{
#ifdef M_DEBUG
				gameLocal.Printf("%s: Modifying change for entity %i from LOD %i to %i (was %i)\n", GetName(), entity, m_Changes[i].oldLOD, newLOD, m_Changes[i].newLOD );
#endif
				// keep the change set with the new values
				m_Changes[i].newLOD = newLOD;
				m_Changes[i].newFlags = newFlags;
			}

			// done
			return;
		}
	}

	// found no change set, add one
	model_changeinfo_t change;

	change.entity = entity;
	change.oldLOD = m_Offsets->Ptr()[ entity ].lod;
	change.oldFlags = m_Offsets->Ptr()[ entity ].flags;
	change.newLOD = newLOD;
	change.newFlags = newFlags;

#ifdef M_DEBUG
	gameLocal.Printf("%s: Adding change for entity %i from LOD %i (flags %i) to %i (flags %i)\n", GetName(), entity, change.oldLOD, change.oldFlags, change.newLOD, change.newFlags );
#endif
	m_Changes.Append( change );

	// done	
}

