/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5383 $ (Revision of last commit) 
 $Date: 2012-04-11 05:46:32 -0400 (Wed, 11 Apr 2012) $ (Date of last commit)
 $Author: serpentine $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_engine.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: tr_lightrun.cpp 5383 2012-04-11 09:46:32Z serpentine $");

#include "tr_local.h"

/*


Prelight models

"_prelight_<lightname>", ie "_prelight_light1"

Static surfaces available to dmap will be processed to optimized
shadow and lit surface geometry

Entity models are never prelighted.

Light entity can have a "noPrelight 1" key set to avoid the preprocessing
and carving of the world.  A light that will move should usually have this
set.

Prelight models will usually have multiple surfaces

Shadow volume surfaces will have the material "_shadowVolume"

The exact same vertexes as the ambient surfaces will be used for the
non-shadow surfaces, so there is opportunity to share


Reference their parent surfaces?
Reference their parent area?


If we don't track parts that are in different areas, there will be huge
losses when an areaportal closed door has a light poking slightly
through it.

There is potential benefit to splitting even the shadow volumes
at area boundaries, but it would involve the possibility of an
extra plane of shadow drawing at the area boundary.


interaction	lightName	numIndexes

Shadow volume surface

Surfaces in the world cannot have "no self shadow" properties, because all
the surfaces are considered together for the optimized shadow volume.  If
you want no self shadow on a static surface, you must still make it into an
entity so it isn't considered in the prelight.


r_hidePrelights
r_hideNonPrelights



each surface could include prelight indexes

generation procedure in dmap:

carve original surfaces into areas

for each light
	build shadow volume and beam tree
	cut all potentially lit surfaces into the beam tree
		move lit fragments into a new optimize group

optimize groups

build light models




*/

/*
=================================================================================

LIGHT TESTING

=================================================================================
*/


/*
====================
R_ModulateLights_f

Modifies the shaderParms on all the lights so the level
designers can easily test different color schemes
====================
*/
void R_ModulateLights_f( const idCmdArgs &args ) {

	if ( !tr.primaryWorld ) {
		return;
	}
	else if ( args.Argc() != 4 ) {
		common->Printf( "usage: modulateLights <redFloat> <greenFloat> <blueFloat>\n" );
		return;
	}

	int count = 0;
	float modulate[3] = {atof(args.Argv(1)), atof(args.Argv(2)), atof(args.Argv(3))};
	idRenderLightLocal *light;

	for (int i = 0 ; i < tr.primaryWorld->lightDefs.Num() ; i++ ) {
		light = tr.primaryWorld->lightDefs[i];
		if ( light ) {
			light->parms.shaderParms[0] *= modulate[0];
			light->parms.shaderParms[1] *= modulate[1];
			light->parms.shaderParms[2] *= modulate[2];
			count++;
		}
	}
	common->Printf( "modulated %i lights\n", count );
}



//======================================================================================


/*
===============
R_CreateEntityRefs

Creates all needed model references in portal areas,
chaining them to both the area and the entityDef.

Bumps tr.viewCount.
===============
*/
void R_CreateEntityRefs( idRenderEntityLocal *def ) {
	idVec3		transformed[8], v;

	if ( !def->parms.hModel ) {
		def->parms.hModel = renderModelManager->DefaultModel();
	}

	// if the entity hasn't been fully specified due to expensive animation calcs
	// for md5 and particles, use the provided conservative bounds.
	if ( def->parms.callback ) {
		def->referenceBounds = def->parms.bounds;
	} else {
		def->referenceBounds = def->parms.hModel->Bounds( &def->parms );
	}

	// some models, like empty particles, may not need to be added at all
	if ( def->referenceBounds.IsCleared() ) {
		return;
	}

	if ( r_showUpdates.GetBool() && ( 
		(def->referenceBounds[1][0] - def->referenceBounds[0][0]) > 1024 ||
		(def->referenceBounds[1][1] - def->referenceBounds[0][1]) > 1024 ) ) {
		common->Printf( "big entityRef: %f,%f\n", def->referenceBounds[1][0] - def->referenceBounds[0][0], def->referenceBounds[1][1] - def->referenceBounds[0][1] );
	}

	for (int i = 0 ; i < 8 ; i++) {
		v[0] = def->referenceBounds[i&1][0];
		v[1] = def->referenceBounds[(i>>1)&1][1];
		v[2] = def->referenceBounds[(i>>2)&1][2];

		R_LocalPointToGlobal( def->modelMatrix, v, transformed[i] ); 
	}

	// bump the view count so we can tell if an area already has a reference
	tr.viewCount++;

	// push these points down the BSP tree into areas
	def->world->PushVolumeIntoTree( def, NULL, 8, transformed );
}


/*
=================================================================================

CREATE LIGHT REFS

=================================================================================
*/

/*
=====================
R_SetLightProject

All values are reletive to the origin
Assumes that right and up are not normalized
This is also called by dmap during map processing.
=====================
*/
void R_SetLightProject( idPlane lightProject[4], const idVec3 origin, const idVec3 target,
					   const idVec3 rightVector, const idVec3 upVector, const idVec3 start, const idVec3 stop ) {
	float		dist, scale, ofs;
	idVec3		normal, startGlobal;
	idVec4		targetGlobal;

	idVec3 right = rightVector;
	const float	rLen = right.Normalize();

	idVec3 up = upVector;
	const float	uLen = up.Normalize();

	normal = up.Cross( right );
	//normal = right.Cross( up );
	normal.Normalize();

	dist = target * normal; //  - ( origin * normal );
	if ( dist < 0 ) {
		dist = -dist;
		normal = -normal;
	}

	scale = ( 0.5f * dist ) / rLen;
	right *= scale;
	scale = -( 0.5f * dist ) / uLen;
	up *= scale;

	lightProject[2] = normal;
	lightProject[2][3] = -( origin * lightProject[2].Normal() );

	lightProject[0] = right;
	lightProject[0][3] = -( origin * lightProject[0].Normal() );

	lightProject[1] = up;
	lightProject[1][3] = -( origin * lightProject[1].Normal() );

	// now offset to center
	targetGlobal.ToVec3() = target + origin;
	targetGlobal[3] = 1;
	ofs = 0.5f - ( targetGlobal * lightProject[0].ToVec4() ) / ( targetGlobal * lightProject[2].ToVec4() );
	lightProject[0].ToVec4() += ofs * lightProject[2].ToVec4();
	ofs = 0.5f - ( targetGlobal * lightProject[1].ToVec4() ) / ( targetGlobal * lightProject[2].ToVec4() );
	lightProject[1].ToVec4() += ofs * lightProject[2].ToVec4();

	// set the falloff vector
	normal = stop - start;

	dist = normal.Normalize();
	if ( dist <= 0 ) {
		dist = 1;
	}

	lightProject[3] = normal * ( 1.0f / dist );
	startGlobal = start + origin;
	lightProject[3][3] = -( startGlobal * lightProject[3].Normal() );
}

/*
===================
R_SetLightFrustum

Creates plane equations from the light projection, positive sides
face out of the light
===================
*/
void R_SetLightFrustum( const idPlane lightProject[4], idPlane frustum[6] ) {

	// we want the planes of s=0, s=q, t=0, and t=q
	frustum[0] = -lightProject[0];
	frustum[1] = -lightProject[1];
	frustum[2] = -(lightProject[2] - lightProject[0]);
	frustum[3] = -(lightProject[2] - lightProject[1]);

	// we want the planes of s=0 and s=1 for front and rear clipping planes
	frustum[4] = -lightProject[3];
	frustum[5] = lightProject[3];
	frustum[5][3] -= 1.0f;

	frustum[5][3] /= frustum[5].Normalize();
	frustum[4][3] /= frustum[4].Normalize();
	frustum[3][3] /= frustum[3].Normalize();
	frustum[2][3] /= frustum[2].Normalize();
	frustum[1][3] /= frustum[1].Normalize();
	frustum[0][3] /= frustum[0].Normalize();
}

/*
====================
R_FreeLightDefFrustum
====================
*/
void R_FreeLightDefFrustum( idRenderLightLocal *ldef ) {

	// free the frustum tris
	if ( ldef->frustumTris ) {
		R_FreeStaticTriSurf( ldef->frustumTris );
		ldef->frustumTris = NULL;
	}
	// free frustum windings
	for ( int i = 0; i < 6; i++ ) {
		if ( ldef->frustumWindings[i] ) {
			delete ldef->frustumWindings[i];
			ldef->frustumWindings[i] = NULL;
		}
	}
}

/*
=================
R_DeriveLightData

Fills everything in based on light->parms
=================
*/
void R_DeriveLightData( idRenderLightLocal *light ) {

	if ( light->parms.shader ) {
		light->lightShader = light->parms.shader;
	}

	// If shader/images are missing, use defaults.
	else if ( !light->lightShader ) {
		light->lightShader = ( light->parms.pointLight ) ? tr.defaultShaderPoint : tr.defaultShaderProj;
	}

	light->falloffImage = light->lightShader->LightFalloffImage();
	if ( !light->falloffImage ) {
		// use the falloff from the default shader of the correct type
		light->falloffImage = ( light->parms.pointLight ) 
			? tr.defaultShaderPoint->LightFalloffImage()
			: tr.defaultShaderProj->LightFalloffImage(); // projected lights by default don't diminish with distance
	}

	// set the projection
	if ( !light->parms.pointLight ) {
		// projected light
		R_SetLightProject( light->lightProject, vec3_origin /* light->parms.origin */, light->parms.target, 
			light->parms.right, light->parms.up, light->parms.start, light->parms.end);
	} else {
		// point light
		memset( light->lightProject, 0, sizeof( light->lightProject ) );
		light->lightProject[0][0] = 0.5f / light->parms.lightRadius[0];
		light->lightProject[1][1] = 0.5f / light->parms.lightRadius[1];
		light->lightProject[3][2] = 0.5f / light->parms.lightRadius[2];
		light->lightProject[0][3] = 0.5f;
		light->lightProject[1][3] = 0.5f;
		light->lightProject[2][3] = 1.0f;
		light->lightProject[3][3] = 0.5f;
	}

	// set the frustum planes
	R_SetLightFrustum( light->lightProject, light->frustum );

	// rotate the light planes and projections by the axis
	R_AxisToModelMatrix( light->parms.axis, light->parms.origin, light->modelMatrix );

	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->frustum[0], light->frustum[0] );
	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->frustum[1], light->frustum[1] );
	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->frustum[2], light->frustum[2] );
	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->frustum[3], light->frustum[3] );
	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->frustum[4], light->frustum[4] );
	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->frustum[5], light->frustum[5] );
	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->lightProject[0], light->lightProject[0] );
	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->lightProject[1], light->lightProject[1] );
	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->lightProject[2], light->lightProject[2] );
	R_LocalPlaneToGlobal( light->modelMatrix, (const idPlane)light->lightProject[3], light->lightProject[3] );

	// adjust global light origin for off center projections and parallel projections
	// we are just faking parallel by making it a very far off center for now
	if ( light->parms.parallel ) {
		idVec3	 dir = light->parms.lightCenter;

		if ( !dir.Normalize() ) {
			// make point straight up if not specified
			dir[2] = 1;
		}
		light->globalLightOrigin = light->parms.origin + dir * 100000;
	} 
	else {
		light->globalLightOrigin = light->parms.origin + light->parms.axis * light->parms.lightCenter;
	}

	R_FreeLightDefFrustum( light );

	light->frustumTris = R_PolytopeSurface( 6, light->frustum, light->frustumWindings );

	// a projected light will have one shadowFrustum, a point light will have
	// six unless the light center is outside the box
	R_MakeShadowFrustums( light );
}

/*
=================
R_CreateLightRefs
=================
*/
#define	MAX_LIGHT_VERTS	40
void R_CreateLightRefs( idRenderLightLocal *light ) {
	idVec3	points[MAX_LIGHT_VERTS];

	const srfTriangles_t *tri = light->frustumTris;

	// because a light frustum is made of only six intersecting planes,
	// we should never be able to get a stupid number of points...
	if ( tri->numVerts > MAX_LIGHT_VERTS ) {
		common->Error( "R_CreateLightRefs: %i points in frustumTris!", tri->numVerts );
	}

	for (int i = 0 ; i < tri->numVerts ; i++ ) {
		points[i] = tri->verts[i].xyz;
	}

	if ( r_showUpdates.GetBool() && (tri->bounds[1][0] - tri->bounds[0][0] > 1024 || tri->bounds[1][1] - tri->bounds[0][1] > 1024) ) {
		common->Printf( "big lightRef: %f,%f\n", tri->bounds[1][0] - tri->bounds[0][0], tri->bounds[1][1] - tri->bounds[0][1] );
	}

	// determine the areaNum for the light origin, which may let us
	// cull the light if it is behind a closed door
	// it is debatable if we want to use the entity origin or the center offset origin,
	// but we definitely don't want to use a parallel offset origin
	light->areaNum = light->world->PointInArea( light->globalLightOrigin );
	if ( light->areaNum == -1 ) {
		light->areaNum = light->world->PointInArea( light->parms.origin );
	}

	// bump the view count so we can tell if an
	// area already has a reference
	tr.viewCount++;

	// if we have a prelight model that includes all the shadows for the major world occluders,
	// we can limit the area references to those visible through the portals from the light center.
	// We can't do this in the normal case, because shadows are cast from back facing triangles, which
	// may be in areas not directly visible to the light projection center.
	if ( light->parms.prelightModel && r_useLightPortalFlow.GetBool() && light->lightShader->LightCastsShadows() ) {
		light->world->FlowLightThroughPortals( light );
	} else {
		// push these points down the BSP tree into areas
		light->world->PushVolumeIntoTree( NULL, light, tri->numVerts, points );
	}
}

/*
===============
R_RenderLightFrustum

Called by the editor and dmap to operate on light volumes
===============
*/
void R_RenderLightFrustum( const renderLight_t &renderLight, idPlane lightFrustum[6] ) {
	idRenderLightLocal	fakeLight;

	//memset( &fakeLight, 0, sizeof( fakeLight ) );
	fakeLight.parms = renderLight;

	R_DeriveLightData( &fakeLight );
	
	R_FreeStaticTriSurf( fakeLight.frustumTris );

	lightFrustum[0] = fakeLight.frustum[0];
	lightFrustum[1] = fakeLight.frustum[1];
	lightFrustum[2] = fakeLight.frustum[2];
	lightFrustum[3] = fakeLight.frustum[3];
	lightFrustum[4] = fakeLight.frustum[4];
	lightFrustum[5] = fakeLight.frustum[5];

}


//=================================================================================

/*
===============
WindingCompletelyInsideLight
===============
*/
bool WindingCompletelyInsideLight( const idWinding *w, const idRenderLightLocal *ldef ) {
	float d = 0.0f;

	for (int i = 0 ; i < w->GetNumPoints() ; i++ ) {
		for (int j = 0 ; j < 6 ; j++ ) {
			
			d = (*w)[i].ToVec3() * ldef->frustum[j].Normal() + ldef->frustum[j][3];
			if ( d > 0.0f ) {
				return false;
			}
		}
	}
	return true;
}

/*
======================
R_CreateLightDefFogPortals

When a fog light is created or moved, see if it completely
encloses any portals, which may allow them to be fogged closed.
======================
*/
void R_CreateLightDefFogPortals( idRenderLightLocal *ldef ) {
	areaReference_t		*lref;

	ldef->foggedPortals = NULL;
	if ( !ldef->lightShader->IsFogLight() ||
		// some fog lights will explicitly disallow portal fogging
		ldef->lightShader->TestMaterialFlag( MF_NOPORTALFOG)) {
		return;
	}

	const portal_t	*prt;
	portalArea_t	*area;
	doublePortal_t	*dp;

	for ( lref = ldef->references ; lref ; lref = lref->ownerNext ) {
		// check all the models in this area
		area = lref->area;

		for ( prt = area->portals ; prt ; prt = prt->next ) {
			dp = prt->doublePortal;

			// we only handle a single fog volume covering a portal
			// this will never cause incorrect drawing, but it may
			// fail to cull a portal 
			if ( dp->fogLight ) {
				continue;
			}
			
			if ( WindingCompletelyInsideLight( prt->w, ldef ) ) {
				dp->fogLight = ldef;
				dp->nextFoggedPortal = ldef->foggedPortals;
				ldef->foggedPortals = dp;
			}
		}
	}
}

/*
====================
R_FreeLightDefDerivedData

Frees all references and lit surfaces from the light
====================
*/
void R_FreeLightDefDerivedData( idRenderLightLocal *ldef ) {
	areaReference_t	*lref;

	// remove any portal fog references
	doublePortal_t *dp = ldef->foggedPortals;
	while ( dp ) {
		dp->fogLight = NULL;
		dp = dp->nextFoggedPortal;
	}

	// free all the interactions
	while ( ldef->firstInteraction ) {
		ldef->firstInteraction->UnlinkAndFree();
	}

	// free all the references to the light
	lref = ldef->references;
	while ( lref ) {
		// unlink from the area
		lref->areaNext->areaPrev = lref->areaPrev;
		lref->areaPrev->areaNext = lref->areaNext;

		// put it back on the free list for reuse
		ldef->world->areaReferenceAllocator.Free( lref );

		lref = lref->ownerNext;
	}

	ldef->references = NULL;
	R_FreeLightDefFrustum( ldef );
}

/*
===================
R_FreeEntityDefDerivedData

Used by both RE_FreeEntityDef and RE_UpdateEntityDef
Does not actually free the entityDef.
===================
*/
void R_FreeEntityDefDerivedData( idRenderEntityLocal *def, bool keepDecals, bool keepCachedDynamicModel ) {
	areaReference_t	*ref;

	// demo playback needs to free the joints, while normal play
	// leaves them in the control of the game
	if ( session->readDemo ) {
		if ( def->parms.joints ) {
			Mem_Free16( def->parms.joints );
			def->parms.joints = NULL;
		}
		if ( def->parms.callbackData ) {
			Mem_Free( def->parms.callbackData );
			def->parms.callbackData = NULL;
		}
		for ( int i = 0; i < MAX_RENDERENTITY_GUI; i++ ) {
			if ( def->parms.gui[ i ] ) {
				delete def->parms.gui[ i ];
				def->parms.gui[ i ] = NULL;
			}
		}
	}

	// free all the interactions
	while ( def->firstInteraction != NULL ) {
		def->firstInteraction->UnlinkAndFree();
	}

	// clear the dynamic model if present
	if ( def->dynamicModel ) {
		def->dynamicModel = NULL;

		if ( !keepCachedDynamicModel ) {
			delete def->cachedDynamicModel;
			def->cachedDynamicModel = NULL;
		}
	}

	if ( !keepDecals ) {
		R_FreeEntityDefDecals( def );
		R_FreeEntityDefOverlay( def );
	}

	// free the entityRefs from the areas
	ref = def->entityRefs;
	while (ref) {
		// unlink from the area
		ref->areaNext->areaPrev = ref->areaPrev;
		ref->areaPrev->areaNext = ref->areaNext;

		// put it back on the free list for reuse
		def->world->areaReferenceAllocator.Free( ref );

		ref = ref->ownerNext;
	}

	def->entityRefs = NULL;
}

/*
==================
R_ClearEntityDefDynamicModel

If we know the reference bounds stays the same, we
only need to do this on entity update, not the full
R_FreeEntityDefDerivedData
==================
*/
void R_ClearEntityDefDynamicModel( idRenderEntityLocal *def ) {
	idInteraction *inter = def->firstInteraction;

	// free all the interaction surfaces
	while ( inter != NULL && !inter->IsEmpty() ) {
		inter->FreeSurfaces();
		inter = inter->entityNext;
	}

	// clear the dynamic model if present
	if ( def->dynamicModel ) {
		def->dynamicModel = NULL;
	}
}

/*
===================
R_FreeEntityDefDecals
===================
*/
void R_FreeEntityDefDecals( idRenderEntityLocal *def ) {
	while( def->decals ) {
		idRenderModelDecal *next = def->decals->Next();
		idRenderModelDecal::Free( def->decals );
		def->decals = next;
	}
}

/*
===================
R_FreeEntityDefFadedDecals
===================
*/
void R_FreeEntityDefFadedDecals( idRenderEntityLocal *def, int time ) {
	def->decals = idRenderModelDecal::RemoveFadedDecals( def->decals, time );
}

/*
===================
R_FreeEntityDefOverlay
===================
*/
void R_FreeEntityDefOverlay( idRenderEntityLocal *def ) {
	if ( def->overlay ) {
		idRenderModelOverlay::Free( def->overlay );
		def->overlay = NULL;
	}
}

/*
===================
R_FreeDerivedData

ReloadModels and RegenerateWorld call this
// FIXME: need to do this for all worlds
===================
*/
void R_FreeDerivedData( void ) {
	int i;
	idRenderWorldLocal *rw;
	idRenderEntityLocal *def;
	idRenderLightLocal *light;

	for ( int j = 0; j < tr.worlds.Num(); j++ ) {
		rw = tr.worlds[j];

		for ( i = 0; i < rw->entityDefs.Num(); i++ ) {
			def = rw->entityDefs[i];
			if ( !def ) {
				continue;
			}
			R_FreeEntityDefDerivedData( def, false, false );
		}

		for ( i = 0; i < rw->lightDefs.Num(); i++ ) {
			light = rw->lightDefs[i];
			if ( !light ) {
				continue;
			}
			R_FreeLightDefDerivedData( light );
		}
	}
}

/*
===================
R_CheckForEntityDefsUsingModel
===================
*/
void R_CheckForEntityDefsUsingModel( idRenderModel *model ) {
	idRenderWorldLocal *rw;
	idRenderEntityLocal	*def;

	for ( int j = 0; j < tr.worlds.Num(); j++ ) {
		rw = tr.worlds[j];

		for (int i = 0 ; i < rw->entityDefs.Num(); i++ ) {
			def = rw->entityDefs[i];
			if ( def && def->parms.hModel == model ) {
				//assert( 0 );
				// this should never happen but Radiant messes it up all the time so just free the derived data
				R_FreeEntityDefDerivedData( def, false, false );
			}
		}
	}
}

/*
===================
R_ReCreateWorldReferences

ReloadModels and RegenerateWorld call this
// FIXME: need to do this for all worlds
===================
*/
void R_ReCreateWorldReferences( void ) {
	int i;
	idRenderWorldLocal *rw;
	idRenderEntityLocal *def;
	idRenderLightLocal *light;

	// let the interaction generation code know this shouldn't be optimized for
	// a particular view
	tr.viewDef = NULL;

	for ( int j = 0; j < tr.worlds.Num(); j++ ) {
		rw = tr.worlds[j];

		for ( i = 0 ; i < rw->entityDefs.Num() ; i++ ) {
			def = rw->entityDefs[i];
			if ( !def ) {
				continue;
			}

			// the world model entities are put specifically in a single
			// area, instead of just pushing their bounds into the tree
			if ( i < rw->numPortalAreas ) {
				rw->AddEntityRefToArea( def, &rw->portalAreas[i] );
			} else {
				R_CreateEntityRefs( def );
			}
		}

		for ( i = 0 ; i < rw->lightDefs.Num() ; i++ ) {
			light = rw->lightDefs[i];
			if ( !light ) {
				continue;
			}

			renderLight_t parms = light->parms;
			light->world->FreeLightDef( i );
			rw->UpdateLightDef( i, &parms );
		}
	}
}

/*
===================
R_RegenerateWorld_f

Frees and regenerates all references and interactions, which
must be done when switching between display list mode and immediate mode
===================
*/
void R_RegenerateWorld_f( const idCmdArgs &args ) {
	R_FreeDerivedData();

	// watch how much memory we allocate
	tr.staticAllocCount = 0;

	R_ReCreateWorldReferences();

	common->Printf( "Regenerated world, staticAllocCount = %i.\n", tr.staticAllocCount );
}
