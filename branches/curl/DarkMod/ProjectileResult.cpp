/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3911 $
 * $Date: 2010-06-06 06:30:18 -0400 (Sun, 06 Jun 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2006 Chris Sarantos <csarantos@gmail.com>

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ProjectileResult.cpp 3911 2010-06-06 10:30:18Z greebo $", init_version);

#include "ProjectileResult.h"
#include "../game/game_local.h"
#include "../game/projectile.h"
#include "StimResponse/Stim.h"
#include "DarkModGlobals.h"

//===============================================================================
//CProjectileResult
//===============================================================================

// TODO: Make sure BINDING works with the new setup (copybind)

/**
* Name of the script to be run when projectile hits activation surface
**/
const char* s_ACTIVE_SCRIPT_NAME = "active";

/**
* Name of script to be run when projectile hits dud surface
**/
const char* s_DUD_SCRIPT_NAME = "dud";

const idEventDef EV_TDM_GetFinalVel( "getFinalVel", NULL, 'v' );
const idEventDef EV_TDM_GetFinalAngVel( "getFinalAngVel", NULL, 'v' );
const idEventDef EV_TDM_GetAxialDir( "getAxialDir", NULL, 'v' );
const idEventDef EV_TDM_GetProjMass( "getProjMass", NULL, 'f' );
const idEventDef EV_TDM_GetSurfType( "getSurfType", NULL, 's' );
const idEventDef EV_TDM_GetSurfNormal( "getSurfNormal", NULL, 'v' );
const idEventDef EV_TDM_GetStruckEnt( "getStruckEnt", NULL, 'e' );
const idEventDef EV_TDM_GetIncidenceAngle( "getIncidenceAngle", NULL, 'f' );

CLASS_DECLARATION( idEntity, CProjectileResult )
	EVENT( EV_TDM_GetFinalVel,				CProjectileResult::Event_GetFinalVel )
	EVENT( EV_TDM_GetFinalAngVel,			CProjectileResult::Event_GetFinalAngVel )
	EVENT( EV_TDM_GetAxialDir,				CProjectileResult::Event_GetAxialDir )
	EVENT( EV_TDM_GetProjMass,				CProjectileResult::Event_GetProjMass )
	EVENT( EV_TDM_GetSurfType,				CProjectileResult::Event_GetSurfType )
	EVENT( EV_TDM_GetSurfNormal,			CProjectileResult::Event_GetSurfNormal )
	EVENT( EV_TDM_GetStruckEnt,				CProjectileResult::Event_GetStruckEnt )
	EVENT( EV_TDM_GetIncidenceAngle,		CProjectileResult::Event_GetIncidenceAngle )
END_CLASS

CProjectileResult::CProjectileResult( void )
{
	// initialize m_ProjData
	m_ProjData.FinalOrigin.Zero(); 
	m_ProjData.FinalAxis.Zero();
	m_ProjData.LinVelocity.Zero();
	m_ProjData.AngVelocity.Zero();
	m_ProjData.AxialDir.Zero();
	m_ProjData.mass = 0;

	m_bActivated = false;

	// initialize the trace data
	m_Collision.endpos.Zero();
	m_Collision.endAxis.Zero();
	m_Collision.c.point.Zero();
	m_Collision.c.normal.Set( 0, 0, 1 );
}

CProjectileResult::~CProjectileResult(void)
{
}


void CProjectileResult::Init
		(
			SFinalProjData *pData, const trace_t &collision,
			idProjectile *pProj, bool bActivate
		)
{
	idVec3 dotprod, LinVelocity;
	float fTemp;
	int StimFalloffExponent = 1;
	float StimRadius = 10.0; // we use a (hopefully) reasonable default radius if none is set.
	float StimRadiusFinal = -1;
	int StimDuration(0), StimEvalInterval(0);
	float StimMagnitude(1.0f);
	bool bStimUseBounds(false);
	bool bCollisionBased(false);
	idVec3 stimBounds[2];
	idVec3 stimVelocity;

	// copy in the data
	m_Collision = collision;
	
	m_ProjData.Owner = pData->Owner.GetEntity();
	m_ProjData.FinalOrigin = pData->FinalOrigin;
	m_ProjData.FinalAxis = pData->FinalAxis;
	m_ProjData.LinVelocity = pData->LinVelocity;
	m_ProjData.AngVelocity = pData->AngVelocity;
	m_ProjData.AxialDir = pData->AxialDir;
	m_ProjData.mass = pData->mass;
	m_ProjData.SurfaceType = pData->SurfaceType;

	m_bActivated = bActivate;

	// calculate and store the (max) angle of incidence
// NOTE: For now, angle of incidence is based on velocity, not axis
// To base it on axis we would need all projectiles to follow some modeleing
// convention (e.g., z is always the axial direction, or something)
	LinVelocity = m_ProjData.LinVelocity;

	LinVelocity.NormalizeFast();
	fTemp = (LinVelocity * collision.c.normal);
	
	m_ProjData.IncidenceAngle = idMath::Fabs( idMath::ACos( fTemp ) );

	// Move to the point of the collision
	GetPhysics()->SetOrigin( pData->FinalOrigin );
	GetPhysics()->SetAxis( pData->FinalAxis );

	// SZ: Dec 19: Had to change it from Hide() to Show() so the AI could see them
	// Show self so AI can see it
	Show();

	// greebo: Loop over the stim indices and add the stims one by one. 
	// The loop is cancelled on the first empty index.
	int stimIdx = 1;
	while (stimIdx > 0)
	{
		// Try to find a string like "sr_type_1"
		idStr key;		
		sprintf(key, "sr_type_%u", stimIdx);

		idStr value = spawnArgs.GetString(key, "");

		if (value.IsEmpty())
		{
			// Set the index to negative values to end the loop
			stimIdx = -1;
		}
		else
		{
			StimType stimType = ST_DEFAULT;

			if (value.IsNumeric())
			{
				stimType = static_cast<StimType>(spawnArgs.GetInt(key));
			}
			else
			{
				stimType = CStimResponse::GetStimType(spawnArgs.GetString(key));
			}

			// The stim type of the projectile result is defined on the projectile itself
			// even though it is not used there. Logically, the stim type is a part of the
			// projectile definition though, since this class is only a helper class.
			
			if (stimType != ST_DEFAULT)
			{
				spawnArgs.GetFloat(va("sr_radius_%u", stimIdx), "10", StimRadius);
				spawnArgs.GetFloat(va("sr_radius_final_%u", stimIdx), "-1", StimRadiusFinal);
				spawnArgs.GetVector(va("sr_bounds_mins_%u", stimIdx), "0 0 0", stimBounds[0]);
				spawnArgs.GetVector(va("sr_bounds_maxs_%u", stimIdx), "0 0 0", stimBounds[1]);
				spawnArgs.GetInt(va("sr_falloffexponent_%u", stimIdx), "1", StimFalloffExponent);
				spawnArgs.GetInt(va("sr_duration_%u", stimIdx), "0", StimDuration );
				spawnArgs.GetVector(va("sr_velocity_%u", stimIdx), "0 0 0", stimVelocity );
				spawnArgs.GetInt(va("sr_time_interval_%u", stimIdx), "0", StimEvalInterval );
				spawnArgs.GetBool(va("sr_use_bounds_%u", stimIdx), "0", bStimUseBounds );
				spawnArgs.GetBool(va("sr_collision_%u", stimIdx), "0", bCollisionBased );
				spawnArgs.GetFloat(va("sr_magnitude_%u", stimIdx), "1.0", StimMagnitude );

				CStimPtr stim = AddStim(stimType, StimRadius);
				
				// TODO: Move these sets to the AddStim arguments once Addstim is rewritten
				stim->m_Duration = StimDuration;
				stim->m_TimeInterleave = StimEvalInterval;
				stim->m_bUseEntBounds = bStimUseBounds;
				stim->m_bCollisionBased = bCollisionBased;
				stim->m_Magnitude = StimMagnitude;
				stim->m_FallOffExponent = StimFalloffExponent;
				stim->m_Velocity = stimVelocity;
				stim->m_RadiusFinal = StimRadiusFinal;

				// Check for valid bounds vectors
				if (stimBounds[0] != idVec3(0,0,0))
				{
					stim->m_Bounds = idBounds(stimBounds[0], stimBounds[1]);
					stim->m_Radius = 0;
					DM_LOG(LC_STIM_RESPONSE, LT_DEBUG)LOGSTRING("Stim with bounds setup\r");
				}

				stim->SetEnabled(spawnArgs.GetBool(va("sr_state_%u", stimIdx), "1"));

				DM_LOG(LC_WEAPON, LT_DEBUG)LOGSTRING("Stim index %u type %u with radius %f added to entity %08lX\r", stimIdx, stimType, StimRadius, this);
			}

			// Set the index to the next number, to keep the loop alive
			stimIdx++;
		}
	}

	// Handle binding
	if( spawnArgs.GetBool( "copy_bind", "0") && pProj 
		&& pProj->spawnArgs.GetBool("bindOnImpact", "0") )
	{
		Event_CopyBind( pProj );
	}

	// Is this projectile originating from a named shooter?
	idStr shooter = pProj->spawnArgs.GetString("shooter", "");
	if (!shooter.IsEmpty()) {
		// Copy the shooter information to this projectile result
		spawnArgs.Set("shooter", shooter.c_str());
	}

	// greebo: Store the name of the owner into the projectile spawnargs
	if (m_ProjData.Owner.GetEntity() != NULL)
	{
		spawnArgs.Set("projectile_owner", m_ProjData.Owner.GetEntity()->name);
	}

	// Run scripts:
	RunResultScript();
}



void CProjectileResult::RunResultScript( void )
{
	DM_LOG(LC_WEAPON, LT_DEBUG)LOGSTRING( "Running projectile result script\r" );

	const char *funName;

	if( m_bActivated )
		funName = s_ACTIVE_SCRIPT_NAME;
	else
		funName = s_DUD_SCRIPT_NAME;

	const function_t *pScript = scriptObject.GetFunction( funName );
	DM_LOG(LC_WEAPON, LT_DEBUG)LOGSTRING( "Attempting to run script %s\r", funName );

	// run the script
	if(pScript)
	{
		DM_LOG(LC_WEAPON, LT_DEBUG)LOGSTRING( "Script found, running...\r" );
		
		idThread *pThread = new idThread;
		pThread->CallFunction(this, pScript, true);
		pThread->DelayedStart(0);
	}
}

void CProjectileResult::Event_GetFinalVel( void ) 
{
	idThread::ReturnVector( m_ProjData.LinVelocity );
}

void CProjectileResult::Event_GetFinalAngVel( void ) 
{
	idThread::ReturnVector( m_ProjData.AngVelocity );
}

void CProjectileResult::Event_GetAxialDir( void ) 
{
	idThread::ReturnVector( m_ProjData.AxialDir );
}

void CProjectileResult::Event_GetProjMass( void ) 
{
	idThread::ReturnFloat( m_ProjData.mass );
}

void CProjectileResult::Event_GetSurfType( void ) 
{
	DM_LOG(LC_WEAPON,LT_DEBUG)LOGSTRING("WEAPON: Surface type returned by GetSurfType was: %s \r", m_ProjData.SurfaceType.c_str() );
	idThread::ReturnString( m_ProjData.SurfaceType.c_str() );
}

void CProjectileResult::Event_GetSurfNormal( void ) 
{
	idThread::ReturnVector( m_Collision.c.normal );
}

void CProjectileResult::Event_GetStruckEnt( void ) 
{
	idThread::ReturnEntity( gameLocal.entities[ m_Collision.c.entityNum ] );
}

void CProjectileResult::Event_GetIncidenceAngle( void )
{
	idThread::ReturnFloat( m_ProjData.IncidenceAngle );
}
