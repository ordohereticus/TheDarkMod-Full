/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2897 $
 * $Date: 2008-09-27 04:05:44 -0400 (Sat, 27 Sep 2008) $
 * $Author: ishtvan $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: moveable.cpp 2897 2008-09-27 08:05:44Z ishtvan $", init_version);

#include "game_local.h"
#include "../DarkMod/MissionData.h"
#include "../DarkMod/StimResponse/StimResponseCollection.h"

/*
===============================================================================

  idMoveable
	
===============================================================================
*/

const idEventDef EV_BecomeNonSolid( "becomeNonSolid" );
const idEventDef EV_SetOwnerFromSpawnArgs( "<setOwnerFromSpawnArgs>" );
const idEventDef EV_IsAtRest( "isAtRest", NULL, 'd' );
const idEventDef EV_EnableDamage( "enableDamage", "f" );

CLASS_DECLARATION( idEntity, idMoveable )
	EVENT( EV_Activate,					idMoveable::Event_Activate )
	EVENT( EV_BecomeNonSolid,			idMoveable::Event_BecomeNonSolid )
	EVENT( EV_SetOwnerFromSpawnArgs,	idMoveable::Event_SetOwnerFromSpawnArgs )
	EVENT( EV_IsAtRest,					idMoveable::Event_IsAtRest )
	EVENT( EV_EnableDamage,				idMoveable::Event_EnableDamage )
END_CLASS


static const float BOUNCE_SOUND_MIN_VELOCITY	= 80.0f;
static const float BOUNCE_SOUND_MAX_VELOCITY	= 200.0f;
static const float SLIDING_VELOCITY_THRESHOLD = 5.0f;

/*
================
idMoveable::idMoveable
================
*/
idMoveable::idMoveable( void ) {
	minDamageVelocity		= 100.0f;
	maxDamageVelocity		= 200.0f;
	nextCollideFxTime		= 0;
	nextDamageTime			= 0;
	nextSoundTime			= 0;
	m_nextCollideScriptTime	= 0;
	// 0 => never, -1 => always, positive number X => X times
	m_collideScriptCounter	= 0;
	m_minScriptVelocity		= 0.0f;
	initialSpline			= NULL;
	initialSplineDir		= vec3_zero;
	explode					= false;
	unbindOnDeath			= false;
	allowStep				= false;
	canDamage				= false;

	// greebo: A fraction of -1 is considered to be an invalid trace here
	memset(&lastCollision, 0, sizeof(lastCollision));
	lastCollision.fraction = -1;

	isPushed = false;
	wasPushedLastFrame = false;
	pushDirection = vec3_zero;
	lastPushOrigin = vec3_zero;
}

/*
================
idMoveable::~idMoveable
================
*/
idMoveable::~idMoveable( void ) {
	delete initialSpline;
	initialSpline = NULL;
}

/*
================
idMoveable::Spawn
================
*/
void idMoveable::Spawn( void ) {
	idTraceModel trm;
	float density, friction, bouncyness, mass, air_friction_linear, air_friction_angular;
	int clipShrink;
	idStr clipModelName;
	idVec3 maxForce, maxTorque;

	// check if a clip model is set
	spawnArgs.GetString( "clipmodel", "", clipModelName );
	if ( !clipModelName[0] ) {
		clipModelName = spawnArgs.GetString( "model" );		// use the visual model
	}

	// tels: support "model" "" with "noclipmodel" "0" - do not attempt to load
	// the clipmodel from the non-existing model name in this case:
	if (clipModelName.Length()) {
		if ( !collisionModelManager->TrmFromModel( clipModelName, trm ) ) {
			gameLocal.Error( "idMoveable '%s': cannot load collision model %s", name.c_str(), clipModelName.c_str() );
			return;
		}

		// angua: check if the cm is valid
		if (idMath::Fabs(trm.bounds[0].x) == idMath::INFINITY)
		{
			gameLocal.Error( "idMoveable '%s': invalid collision model %s", name.c_str(), clipModelName.c_str() );
		}

		// if the model should be shrunk
		clipShrink = spawnArgs.GetInt( "clipshrink" );
		if ( clipShrink != 0 ) {
			trm.Shrink( clipShrink * CM_CLIP_EPSILON );
		}
	}

	// get rigid body properties
	spawnArgs.GetFloat( "density", "0.5", density );
	density = idMath::ClampFloat( 0.001f, 1000.0f, density );
	spawnArgs.GetFloat( "bouncyness", "0.6", bouncyness );
	bouncyness = idMath::ClampFloat( 0.0f, 1.0f, bouncyness );
	explode = spawnArgs.GetBool( "explode" );
	unbindOnDeath = spawnArgs.GetBool( "unbindondeath" );

	spawnArgs.GetFloat( "friction", "0.05", friction );
	// reverse compatibility, new contact_friction key replaces friction only if present
	if( spawnArgs.FindKey("contact_friction") )
	{
		spawnArgs.GetFloat( "contact_friction", "0.05", friction );
	}
	spawnArgs.GetFloat( "linear_friction", "0.6", air_friction_linear );
	spawnArgs.GetFloat( "angular_friction", "0.6", air_friction_angular );

	fxCollide = spawnArgs.GetString( "fx_collide" );
	nextCollideFxTime = 0;

	// tels:
	m_scriptCollide = spawnArgs.GetString( "script_collide" );
	m_nextCollideScriptTime = 0;
	m_collideScriptCounter = spawnArgs.GetInt( "collide_script_counter", "1" );
	// override the default of 1 with 0 if no script is defined
	if (m_scriptCollide == "")
	{
		m_collideScriptCounter = 0;
	}
	m_minScriptVelocity = spawnArgs.GetFloat( "min_script_velocity", "5.0" ); 

	damage = spawnArgs.GetString( "def_damage", "" );
	canDamage = spawnArgs.GetBool( "damageWhenActive" ) ? false : true;
	minDamageVelocity = spawnArgs.GetFloat( "minDamageVelocity", "100" );
	maxDamageVelocity = spawnArgs.GetFloat( "maxDamageVelocity", "200" );
	nextDamageTime = 0;
	nextSoundTime = 0;

	health = spawnArgs.GetInt( "health", "0" );

	// tels: load a visual model, as well as an optional brokenModel
	LoadModels();

	// setup the physics
	physicsObj.SetSelf( this );
	physicsObj.SetClipModel( new idClipModel( trm ), density );
	physicsObj.GetClipModel()->SetMaterial( GetRenderModelMaterial() );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() );
	physicsObj.SetAxis( GetPhysics()->GetAxis() );
	physicsObj.SetBouncyness( bouncyness );
	physicsObj.SetFriction( air_friction_linear, air_friction_angular, friction );
	physicsObj.SetGravity( gameLocal.GetGravity() );

	int contents = CONTENTS_SOLID | CONTENTS_OPAQUE;
	// ishtvan: overwrite with custom contents, if present
	if( m_CustomContents != -1 )
		contents = m_CustomContents;

	// greebo: Set the frobable contents flag if the spawnarg says so
	if (spawnArgs.GetBool("frobable", "0"))
	{
		contents |= CONTENTS_FROBABLE;
	}
	
	physicsObj.SetContents( contents );

	physicsObj.SetClipMask( MASK_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_MOVEABLECLIP );
	SetPhysics( &physicsObj );

	if ( spawnArgs.GetFloat( "mass", "10", mass ) ) {
		physicsObj.SetMass( mass );
	}

	// tels
	if ( spawnArgs.GetVector( "max_force", "", maxForce ) ) {
		physicsObj.SetMaxForce( maxForce );
	}
	if ( spawnArgs.GetVector( "max_torque", "", maxTorque ) ) {
		physicsObj.SetMaxTorque( maxTorque );
	}
	
	if ( spawnArgs.GetBool( "nodrop" ) ) {
		physicsObj.PutToRest();
	} else {
		physicsObj.DropToFloor();
	}

	if ( spawnArgs.GetBool( "noimpact" ) || spawnArgs.GetBool( "notPushable" ) ) {
		physicsObj.DisableImpact();
	}

	if ( spawnArgs.GetBool( "nonsolid" ) ) {
		BecomeNonSolid();
	}

	// SR CONTENTS_RESONSE FIX
	if( m_StimResponseColl->HasResponse() )
	{
		physicsObj.SetContents( physicsObj.GetContents() | CONTENTS_RESPONSE );
	}

	m_preHideContents = physicsObj.GetContents();
	m_preHideClipMask = physicsObj.GetClipMask();

	allowStep = spawnArgs.GetBool( "allowStep", "1" );

	PostEventMS( &EV_SetOwnerFromSpawnArgs, 0 );
}

/*
================
idMoveable::Save
================
*/
void idMoveable::Save( idSaveGame *savefile ) const {

	savefile->WriteString( brokenModel );
	savefile->WriteString( damage );
	savefile->WriteString( m_scriptCollide );
	savefile->WriteInt( m_collideScriptCounter );
	savefile->WriteInt( m_nextCollideScriptTime );
	savefile->WriteFloat( m_minScriptVelocity );
	savefile->WriteString( fxCollide );
	savefile->WriteInt( nextCollideFxTime );
	savefile->WriteFloat( minDamageVelocity );
	savefile->WriteFloat( maxDamageVelocity );
	savefile->WriteBool( explode );
	savefile->WriteBool( unbindOnDeath );
	savefile->WriteBool( allowStep );
	savefile->WriteBool( canDamage );
	savefile->WriteInt( nextDamageTime );
	savefile->WriteInt( nextSoundTime );
	savefile->WriteInt( initialSpline != NULL ? (int)initialSpline->GetTime( 0 ) : -1 );
	savefile->WriteVec3( initialSplineDir );

	savefile->WriteStaticObject( physicsObj );

	savefile->WriteTrace(lastCollision);
	savefile->WriteBool(isPushed);
	savefile->WriteBool(wasPushedLastFrame);
	savefile->WriteVec3(pushDirection);
	savefile->WriteVec3(lastPushOrigin);
}

/*
================
idMoveable::Restore
================
*/
void idMoveable::Restore( idRestoreGame *savefile ) {
	int initialSplineTime;

	savefile->ReadString( brokenModel );
	savefile->ReadString( damage );
	savefile->ReadString( m_scriptCollide );
	savefile->ReadInt( m_collideScriptCounter );
	savefile->ReadInt( m_nextCollideScriptTime );
	savefile->ReadFloat( m_minScriptVelocity );
	savefile->ReadString( fxCollide );
	savefile->ReadInt( nextCollideFxTime );
	savefile->ReadFloat( minDamageVelocity );
	savefile->ReadFloat( maxDamageVelocity );
	savefile->ReadBool( explode );
	savefile->ReadBool( unbindOnDeath );
	savefile->ReadBool( allowStep );
	savefile->ReadBool( canDamage );
	savefile->ReadInt( nextDamageTime );
	savefile->ReadInt( nextSoundTime );
	savefile->ReadInt( initialSplineTime );
	savefile->ReadVec3( initialSplineDir );

	if ( initialSplineTime != -1 ) {
		InitInitialSpline( initialSplineTime );
	} else {
		initialSpline = NULL;
	}

	savefile->ReadStaticObject( physicsObj );
	RestorePhysics( &physicsObj );

	savefile->ReadTrace(lastCollision);
	savefile->ReadBool(isPushed);
	savefile->ReadBool(wasPushedLastFrame);
	savefile->ReadVec3(pushDirection);
	savefile->ReadVec3(lastPushOrigin);
}

/*
================
idMoveable::Hide
================
*/
void idMoveable::Hide( void ) {
	idEntity::Hide();
	physicsObj.SetContents( 0 );
}

/*
================
idMoveable::Show
================
*/
void idMoveable::Show( void ) 
{
	idEntity::Show();
	physicsObj.SetContents( m_preHideContents );
}

/*
=================
idMoveable::Collide
=================
*/

bool idMoveable::Collide( const trace_t &collision, const idVec3 &velocity ) {
	float v, f;
	idVec3 dir;
	idEntity *ent;
	const idMaterial *material(NULL);
	idStr SndNameLocal;
	const char *SndName(NULL);

	// greebo: Check whether we are colliding with the nearly exact same point again
	bool sameCollisionAgain = (lastCollision.fraction != -1 && lastCollision.c.point.Compare(collision.c.point, 0.05f));

	// greebo: Save the collision info for the next call
	lastCollision = collision;

	v = -( velocity * collision.c.normal );
	if ( !sameCollisionAgain )
	{
 		if ( v > BOUNCE_SOUND_MIN_VELOCITY && gameLocal.time > nextSoundTime )
		{ 
			material = collision.c.material;
			if( material != NULL)
			{
				g_Global.GetSurfName( material, SndNameLocal );
				SndNameLocal = "snd_bounce_" + SndNameLocal;
				SndName = spawnArgs.GetString( SndNameLocal.c_str() );

				if( *SndName == '\0' )
				{
				SndNameLocal = "snd_bounce";
				}
			}

			f = v > BOUNCE_SOUND_MAX_VELOCITY ? 1.0f : idMath::Sqrt( v - BOUNCE_SOUND_MIN_VELOCITY ) * ( 1.0f / idMath::Sqrt( BOUNCE_SOUND_MAX_VELOCITY - BOUNCE_SOUND_MIN_VELOCITY ) );
			if ( StartSound( SndNameLocal.c_str(), SND_CHANNEL_ANY, 0, false, NULL ) ) {
				// don't set the volume unless there is a bounce sound as it overrides the entire channel
				// which causes footsteps on ai's to not honor their shader parms
				SetSoundVolume( f );
			}
			nextSoundTime = gameLocal.time + 500;
		}
		// tels:
		DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("Moveable %s might call script_collide %s because m_collideScriptCounter=%i and v=%f and time=%f > m_nextCollideScriptTime=%f.\r",
				name.c_str(), m_scriptCollide.c_str(), m_collideScriptCounter, v, gameLocal.time, m_nextCollideScriptTime );
 		if ( m_collideScriptCounter != 0 && v > m_minScriptVelocity && gameLocal.time > m_nextCollideScriptTime ) 
		{ 
	 		if ( m_collideScriptCounter > 0)
			{
			// if positive, decrement it, so -1 stays as it is (for 0, we never come here)
	 		m_collideScriptCounter --;
			}

			// call the script
			const function_t* pScriptFun = scriptObject.GetFunction( m_scriptCollide.c_str() );
			if (pScriptFun == NULL)
    		{
				// Local function not found, check in global namespace
				pScriptFun = gameLocal.program.FindFunction( m_scriptCollide.c_str() );
		    }
			if (pScriptFun != NULL)
			{
				DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("Moveable %s calling script_collide %s.\r",
					name.c_str(), m_scriptCollide.c_str());
				idThread *pThread = new idThread( pScriptFun );
				pThread->CallFunctionArgs( pScriptFun, true, "e", this );
				pThread->DelayedStart( 0 );
			}
			else
			{
				// script function not found!
				DM_LOG(LC_ENTITY, LT_ERROR)LOGSTRING("Moveable %s could not find script_collide %s.\r",
					name.c_str(), m_scriptCollide.c_str());
	 			m_collideScriptCounter = 0;
			}

		m_nextCollideScriptTime = gameLocal.time + 300;
		}

	}

	ent = gameLocal.entities[ collision.c.entityNum ];

	if ( canDamage && damage.Length() && gameLocal.time > nextDamageTime ) 
	{
		if ( ent && v > minDamageVelocity ) 
		{
			f = v > maxDamageVelocity ? 1.0f : idMath::Sqrt( v - minDamageVelocity ) * ( 1.0f / idMath::Sqrt( maxDamageVelocity - minDamageVelocity ) );
			dir = velocity;
			dir.NormalizeFast();
			ent->Damage( this, GetPhysics()->GetClipModel()->GetOwner(), dir, damage, f, CLIPMODEL_ID_TO_JOINT_HANDLE(collision.c.id), const_cast<trace_t *>(&collision) );
			nextDamageTime = gameLocal.time + 1000;
		}
	}

	// Darkmod: Collision stims and a tactile alert if it collides with an AI
	if ( ent )
	{
		ProcCollisionStims( ent, collision.c.id );

		if( ent->IsType( idAI::Type ) )
		{
			idAI *alertee = static_cast<idAI *>(ent);
			alertee->TactileAlert( this );
		}
	}

	if ( fxCollide.Length() && gameLocal.time > nextCollideFxTime ) {
		idEntityFx::StartFx( fxCollide, &collision.c.point, NULL, this, false );
		nextCollideFxTime = gameLocal.time + 3500;
	}

	return false;
}

/*
============
idMoveable::Killed
============
*/
void idMoveable::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) 
{
	bool bPlayerResponsible(false);

	if ( unbindOnDeath ) {
		Unbind();
	}

	// tels: call base class method to switch to broken model
	idEntity::BecomeBroken( inflictor );

	if ( explode ) {
		if ( brokenModel == "" ) {
			PostEventMS( &EV_Remove, 1000 );
		}
	}

	if ( renderEntity.gui[ 0 ] ) {
		renderEntity.gui[ 0 ] = NULL;
	}

	ActivateTargets( this );

	fl.takedamage = false;

	if ( attacker && attacker->IsType( idPlayer::Type ) )
		bPlayerResponsible = ( attacker == gameLocal.GetLocalPlayer() );
	else if( attacker && attacker->m_SetInMotionByActor.GetEntity() )
		bPlayerResponsible = ( attacker->m_SetInMotionByActor.GetEntity() == gameLocal.GetLocalPlayer() );

	gameLocal.m_MissionData->MissionEvent( COMP_DESTROY, this, bPlayerResponsible );
}

/*
================
idMoveable::AllowStep
================
*/
bool idMoveable::AllowStep( void ) const {
	return allowStep;
}

/*
================
idMoveable::BecomeNonSolid
================
*/
void idMoveable::BecomeNonSolid( void ) {
	// set CONTENTS_RENDERMODEL so bullets still collide with the moveable
	physicsObj.SetContents( CONTENTS_CORPSE | CONTENTS_RENDERMODEL );
	physicsObj.SetClipMask( MASK_SOLID | CONTENTS_CORPSE | CONTENTS_MOVEABLECLIP );
	
	// SR CONTENTS_RESPONSE FIX:
	if( m_StimResponseColl->HasResponse() )
		physicsObj.SetContents( physicsObj.GetContents() | CONTENTS_RESPONSE );
}

/*
================
idMoveable::EnableDamage
================
*/
void idMoveable::EnableDamage( bool enable, float duration ) {
	canDamage = enable;
	if ( duration ) {
		PostEventSec( &EV_EnableDamage, duration, ( !enable ) ? 0.0f : 1.0f );
	}
}

/*
================
idMoveable::InitInitialSpline
================
*/
void idMoveable::InitInitialSpline( int startTime ) {
	int initialSplineTime;

	initialSpline = GetSpline();
	initialSplineTime = spawnArgs.GetInt( "initialSplineTime", "300" );

	if ( initialSpline != NULL ) {
		initialSpline->MakeUniform( initialSplineTime );
		initialSpline->ShiftTime( startTime - initialSpline->GetTime( 0 ) );
		initialSplineDir = initialSpline->GetCurrentFirstDerivative( startTime );
		initialSplineDir *= physicsObj.GetAxis().Transpose();
		initialSplineDir.Normalize();
		BecomeActive( TH_THINK );
	}
}

/*
================
idMoveable::FollowInitialSplinePath
================
*/
bool idMoveable::FollowInitialSplinePath( void ) {
	if ( initialSpline != NULL ) {
		if ( gameLocal.time < initialSpline->GetTime( initialSpline->GetNumValues() - 1 ) ) {
			idVec3 splinePos = initialSpline->GetCurrentValue( gameLocal.time );
			idVec3 linearVelocity = ( splinePos - physicsObj.GetOrigin() ) * USERCMD_HZ;
			physicsObj.SetLinearVelocity( linearVelocity );

			idVec3 splineDir = initialSpline->GetCurrentFirstDerivative( gameLocal.time );
			idVec3 dir = initialSplineDir * physicsObj.GetAxis();
			idVec3 angularVelocity = dir.Cross( splineDir );
			angularVelocity.Normalize();
			angularVelocity *= idMath::ACos16( dir * splineDir / splineDir.Length() ) * USERCMD_HZ;
			physicsObj.SetAngularVelocity( angularVelocity );
			return true;
		} else {
			delete initialSpline;
			initialSpline = NULL;
		}
	}
	return false;
}

/*
================
idMoveable::Think
================
*/
void idMoveable::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		if ( !FollowInitialSplinePath() && !isPushed) {
			BecomeInactive( TH_THINK );
		}
	}

	idEntity::Think();

	UpdateSlidingSounds();
}

/*
================
idMoveable::GetRenderModelMaterial
================
*/
const idMaterial *idMoveable::GetRenderModelMaterial( void ) const {
	if ( renderEntity.customShader ) {
		return renderEntity.customShader;
	}
	if ( renderEntity.hModel && renderEntity.hModel->NumSurfaces() ) {
		 return renderEntity.hModel->Surface( 0 )->shader;
	}
	return NULL;
}

/*
================
idMoveable::WriteToSnapshot
================
*/
void idMoveable::WriteToSnapshot( idBitMsgDelta &msg ) const {
	physicsObj.WriteToSnapshot( msg );
}

/*
================
idMoveable::ReadFromSnapshot
================
*/
void idMoveable::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	physicsObj.ReadFromSnapshot( msg );
	if ( msg.HasChanged() ) {
		UpdateVisuals();
	}
}

void idMoveable::SetIsPushed(bool isNowPushed, const idVec3& pushDirection)
{
	isPushed = isNowPushed;
	this->pushDirection = pushDirection;
	lastPushOrigin = GetPhysics()->GetOrigin();

	// Update our think flags to allow UpdateMoveables to be called. 
	if (isPushed)
	{
		BecomeActive(TH_THINK);
	}
}

bool idMoveable::IsPushed()
{
	return isPushed;
}

void idMoveable::UpdateSlidingSounds()
{
	if (isPushed)
	{
		const idVec3& curVelocity = GetPhysics()->GetLinearVelocity();
		const idVec3& gravityNorm = GetPhysics()->GetGravityNormal();

		idVec3 xyVelocity = curVelocity - (curVelocity * gravityNorm) * gravityNorm;

		// Only consider the xyspeed if the velocity is in pointing in the same direction as we're being pushed
		float xySpeed = (idMath::Fabs(xyVelocity * pushDirection) > 0.2f) ? xyVelocity.NormalizeFast() : 0;

		//gameRenderWorld->DrawText( idStr(xySpeed), GetPhysics()->GetAbsBounds().GetCenter(), 0.1f, colorWhite, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec );
		//gameRenderWorld->DebugArrow(colorWhite, GetPhysics()->GetAbsBounds().GetCenter(), GetPhysics()->GetAbsBounds().GetCenter() + xyVelocity, 1, gameLocal.msec );

		if (wasPushedLastFrame && xySpeed <= SLIDING_VELOCITY_THRESHOLD)
		{
			// We are still being pushed, but we are not fast enough
			StopSound(SND_CHANNEL_BODY3, false);
			BecomeInactive(TH_THINK);

			isPushed = false;
			wasPushedLastFrame = false;
		}
		else if (!wasPushedLastFrame && xySpeed > SLIDING_VELOCITY_THRESHOLD)
		{
			if (lastPushOrigin.Compare(GetPhysics()->GetOrigin(), 0.05f))
			{
				// We did not really move, despite what the velocity says
				StopSound(SND_CHANNEL_BODY3, false);
				BecomeInactive(TH_THINK);

				isPushed = false;
			}
			else
			{
				// We just got into pushed state and are fast enough
				StartSound("snd_sliding", SND_CHANNEL_BODY3, 0, false, NULL);

				// Update the state flag for the next round
				wasPushedLastFrame = true;
			}
		}

		lastPushOrigin = GetPhysics()->GetOrigin();
	}
	else if (wasPushedLastFrame)
	{
		// We are not pushed anymore
		StopSound(SND_CHANNEL_BODY3, false);
		BecomeInactive(TH_THINK);

		// Update the state flag for the next round
		wasPushedLastFrame = false;
	}
}

/*
================
idMoveable::Event_BecomeNonSolid
================
*/
void idMoveable::Event_BecomeNonSolid( void ) {
	BecomeNonSolid();
}

/*
================
idMoveable::Event_Activate
================
*/
void idMoveable::Event_Activate( idEntity *activator ) {
	float delay;
	idVec3 init_velocity, init_avelocity;

	Show();

	if ( !spawnArgs.GetInt( "notPushable" ) ) {
        physicsObj.EnableImpact();
	}

	physicsObj.Activate();

	spawnArgs.GetVector( "init_velocity", "0 0 0", init_velocity );
	spawnArgs.GetVector( "init_avelocity", "0 0 0", init_avelocity );

	delay = spawnArgs.GetFloat( "init_velocityDelay", "0" );
	if ( delay == 0.0f ) {
		physicsObj.SetLinearVelocity( init_velocity );
	} else {
		PostEventSec( &EV_SetLinearVelocity, delay, init_velocity );
	}

	delay = spawnArgs.GetFloat( "init_avelocityDelay", "0" );
	if ( delay == 0.0f ) {
		physicsObj.SetAngularVelocity( init_avelocity );
	} else {
		PostEventSec( &EV_SetAngularVelocity, delay, init_avelocity );
	}

	InitInitialSpline( gameLocal.time );
}

/*
================
idMoveable::Event_SetOwnerFromSpawnArgs
================
*/
void idMoveable::Event_SetOwnerFromSpawnArgs( void ) {
	idStr owner;

	if ( spawnArgs.GetString( "owner", "", owner ) ) {
		ProcessEvent( &EV_SetOwner, gameLocal.FindEntity( owner ) );
	}
}

/*
================
idMoveable::Event_IsAtRest
================
*/
void idMoveable::Event_IsAtRest( void ) {
	idThread::ReturnInt( physicsObj.IsAtRest() );
}

/*
================
idMoveable::Event_EnableDamage
================
*/
void idMoveable::Event_EnableDamage( float enable ) {
	canDamage = ( enable != 0.0f );
}


/*
===============================================================================

  idBarrel
	
===============================================================================
*/

CLASS_DECLARATION( idMoveable, idBarrel )
END_CLASS

/*
================
idBarrel::idBarrel
================
*/
idBarrel::idBarrel() {
	radius = 1.0f;
	barrelAxis = 0;
	lastOrigin.Zero();
	lastAxis.Identity();
	additionalRotation = 0.0f;
	additionalAxis.Identity();
	fl.networkSync = true;
}

/*
================
idBarrel::Save
================
*/
void idBarrel::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( radius );
	savefile->WriteInt( barrelAxis );
	savefile->WriteVec3( lastOrigin );
	savefile->WriteMat3( lastAxis );
	savefile->WriteFloat( additionalRotation );
	savefile->WriteMat3( additionalAxis );
}

/*
================
idBarrel::Restore
================
*/
void idBarrel::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( radius );
	savefile->ReadInt( barrelAxis );
	savefile->ReadVec3( lastOrigin );
	savefile->ReadMat3( lastAxis );
	savefile->ReadFloat( additionalRotation );
	savefile->ReadMat3( additionalAxis );
}

/*
================
idBarrel::BarrelThink
================
*/
void idBarrel::BarrelThink( void ) {
	bool wasAtRest, onGround;
	float movedDistance, rotatedDistance, angle;
	idVec3 curOrigin, gravityNormal, dir;
	idMat3 curAxis, axis;

	wasAtRest = IsAtRest();

	// run physics
	RunPhysics();

	// only need to give the visual model an additional rotation if the physics were run
	if ( !wasAtRest ) {

		// current physics state
		onGround = GetPhysics()->HasGroundContacts();
		curOrigin = GetPhysics()->GetOrigin();
		curAxis = GetPhysics()->GetAxis();

		// if the barrel is on the ground
		if ( onGround ) {
			gravityNormal = GetPhysics()->GetGravityNormal();

			dir = curOrigin - lastOrigin;
			dir -= gravityNormal * dir * gravityNormal;
			movedDistance = dir.LengthSqr();

			// if the barrel moved and the barrel is not aligned with the gravity direction
			if ( movedDistance > 0.0f && idMath::Fabs( gravityNormal * curAxis[barrelAxis] ) < 0.7f ) {

				// barrel movement since last think frame orthogonal to the barrel axis
				movedDistance = idMath::Sqrt( movedDistance );
				dir *= 1.0f / movedDistance;
				movedDistance = ( 1.0f - idMath::Fabs( dir * curAxis[barrelAxis] ) ) * movedDistance;

				// get rotation about barrel axis since last think frame
				angle = lastAxis[(barrelAxis+1)%3] * curAxis[(barrelAxis+1)%3];
				angle = idMath::ACos( angle );
				// distance along cylinder hull
				rotatedDistance = angle * radius;

				// if the barrel moved further than it rotated about it's axis
				if ( movedDistance > rotatedDistance ) {

					// additional rotation of the visual model to make it look
					// like the barrel rolls instead of slides
					angle = 180.0f * (movedDistance - rotatedDistance) / (radius * idMath::PI);
					if ( gravityNormal.Cross( curAxis[barrelAxis] ) * dir < 0.0f ) {
						additionalRotation += angle;
					} else {
						additionalRotation -= angle;
					}
					dir = vec3_origin;
					dir[barrelAxis] = 1.0f;
					additionalAxis = idRotation( vec3_origin, dir, additionalRotation ).ToMat3();
				}
			}
		}

		// save state for next think
		lastOrigin = curOrigin;
		lastAxis = curAxis;
	}

	Present();
}

/*
================
idBarrel::Think
================
*/
void idBarrel::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		if ( !FollowInitialSplinePath() ) {
			BecomeInactive( TH_THINK );
		}
	}

	BarrelThink();
}

/*
================
idBarrel::GetPhysicsToVisualTransform
================
*/
bool idBarrel::GetPhysicsToVisualTransform( idVec3 &origin, idMat3 &axis ) {
	origin = vec3_origin;
	axis = additionalAxis;
	return true;
}

/*
================
idBarrel::Spawn
================
*/
void idBarrel::Spawn( void ) {
	const idBounds &bounds = GetPhysics()->GetBounds();

	// radius of the barrel cylinder
	radius = ( bounds[1][0] - bounds[0][0] ) * 0.5f;

	// always a vertical barrel with cylinder axis parallel to the z-axis
	barrelAxis = 2;

	lastOrigin = GetPhysics()->GetOrigin();
	lastAxis = GetPhysics()->GetAxis();

	additionalRotation = 0.0f;
	additionalAxis.Identity();
}

/*
================
idBarrel::ClientPredictionThink
================
*/
void idBarrel::ClientPredictionThink( void ) {
	Think();
}


/*
===============================================================================

idExplodingBarrel

===============================================================================
*/
const idEventDef EV_Respawn( "<respawn>" );
const idEventDef EV_TriggerTargets( "<triggertargets>" );

CLASS_DECLARATION( idBarrel, idExplodingBarrel )
	EVENT( EV_Activate,					idExplodingBarrel::Event_Activate )
	EVENT( EV_Respawn,					idExplodingBarrel::Event_Respawn )
	EVENT( EV_Explode,					idExplodingBarrel::Event_Explode )
	EVENT( EV_TriggerTargets,			idExplodingBarrel::Event_TriggerTargets )
END_CLASS

/*
================
idExplodingBarrel::idExplodingBarrel
================
*/
idExplodingBarrel::idExplodingBarrel() {
	spawnOrigin.Zero();
	spawnAxis.Zero();
	state = NORMAL;
	particleModelDefHandle = -1;
	lightDefHandle = -1;
	memset( &particleRenderEntity, 0, sizeof( particleRenderEntity ) );
	memset( &light, 0, sizeof( light ) );
	particleTime = 0;
	lightTime = 0;
	time = 0.0f;
}

/*
================
idExplodingBarrel::~idExplodingBarrel
================
*/
idExplodingBarrel::~idExplodingBarrel() {
	if ( particleModelDefHandle >= 0 ){
		gameRenderWorld->FreeEntityDef( particleModelDefHandle );
	}
	if ( lightDefHandle >= 0 ) {
		gameRenderWorld->FreeLightDef( lightDefHandle );
	}
}

/*
================
idExplodingBarrel::Save
================
*/
void idExplodingBarrel::Save( idSaveGame *savefile ) const {
	savefile->WriteVec3( spawnOrigin );
	savefile->WriteMat3( spawnAxis );

	savefile->WriteInt( state );
	savefile->WriteInt( particleModelDefHandle );
	savefile->WriteInt( lightDefHandle );

	savefile->WriteRenderEntity( particleRenderEntity );
	savefile->WriteRenderLight( light );

	savefile->WriteInt( particleTime );
	savefile->WriteInt( lightTime );
	savefile->WriteFloat( time );
}

/*
================
idExplodingBarrel::Restore
================
*/
void idExplodingBarrel::Restore( idRestoreGame *savefile ) {
	savefile->ReadVec3( spawnOrigin );
	savefile->ReadMat3( spawnAxis );

	savefile->ReadInt( (int &)state );
	savefile->ReadInt( (int &)particleModelDefHandle );
	savefile->ReadInt( (int &)lightDefHandle );

	savefile->ReadRenderEntity( particleRenderEntity );
	savefile->ReadRenderLight( light );

	savefile->ReadInt( particleTime );
	savefile->ReadInt( lightTime );
	savefile->ReadFloat( time );
}

/*
================
idExplodingBarrel::Spawn
================
*/
void idExplodingBarrel::Spawn( void ) {
	health = spawnArgs.GetInt( "health", "5" );
	fl.takedamage = true;
	spawnOrigin = GetPhysics()->GetOrigin();
	spawnAxis = GetPhysics()->GetAxis();
	state = NORMAL;
	particleModelDefHandle = -1;
	lightDefHandle = -1;
	lightTime = 0;
	particleTime = 0;
	time = spawnArgs.GetFloat( "time" );
	memset( &particleRenderEntity, 0, sizeof( particleRenderEntity ) );
	memset( &light, 0, sizeof( light ) );
}

/*
================
idExplodingBarrel::Think
================
*/
void idExplodingBarrel::Think( void ) {
	idBarrel::BarrelThink();

	if ( lightDefHandle >= 0 ){
		if ( state == BURNING ) {
			// ramp the color up over 250 ms
			float pct = (gameLocal.time - lightTime) / 250.f;
			if ( pct > 1.0f ) {
				pct = 1.0f;
			}
			light.origin = physicsObj.GetAbsBounds().GetCenter();
			light.axis = mat3_identity;
			light.shaderParms[ SHADERPARM_RED ] = pct;
			light.shaderParms[ SHADERPARM_GREEN ] = pct;
			light.shaderParms[ SHADERPARM_BLUE ] = pct;
			light.shaderParms[ SHADERPARM_ALPHA ] = pct;
			gameRenderWorld->UpdateLightDef( lightDefHandle, &light );
		} else {
			if ( gameLocal.time - lightTime > 250 ) {
				gameRenderWorld->FreeLightDef( lightDefHandle );
				lightDefHandle = -1;
			}
			return;
		}
	}

	if ( !gameLocal.isClient && state != BURNING && state != EXPLODING ) {
		BecomeInactive( TH_THINK );
		return;
	}

	if ( particleModelDefHandle >= 0 ){
		particleRenderEntity.origin = physicsObj.GetAbsBounds().GetCenter();
		particleRenderEntity.axis = mat3_identity;
		gameRenderWorld->UpdateEntityDef( particleModelDefHandle, &particleRenderEntity );
	}
}

/*
================
idExplodingBarrel::AddParticles
================
*/
void idExplodingBarrel::AddParticles( const char *name, bool burn ) {
	if ( name && *name ) {
		if ( particleModelDefHandle >= 0 ){
			gameRenderWorld->FreeEntityDef( particleModelDefHandle );
		}
		memset( &particleRenderEntity, 0, sizeof ( particleRenderEntity ) );
		const idDeclModelDef *modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, name ) );
		if ( modelDef ) {
			particleRenderEntity.origin = physicsObj.GetAbsBounds().GetCenter();
			particleRenderEntity.axis = mat3_identity;
			particleRenderEntity.hModel = modelDef->ModelHandle();
			float rgb = ( burn ) ? 0.0f : 1.0f;
			particleRenderEntity.shaderParms[ SHADERPARM_RED ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_GREEN ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_BLUE ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_ALPHA ] = rgb;
			particleRenderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.realClientTime );
			particleRenderEntity.shaderParms[ SHADERPARM_DIVERSITY ] = ( burn ) ? 1.0f : gameLocal.random.RandomInt( 90 );
			if ( !particleRenderEntity.hModel ) {
				particleRenderEntity.hModel = renderModelManager->FindModel( name );
			}
			particleModelDefHandle = gameRenderWorld->AddEntityDef( &particleRenderEntity );
			if ( burn ) {
				BecomeActive( TH_THINK );
			}
			particleTime = gameLocal.realClientTime;
		}
	}
}

/*
================
idExplodingBarrel::AddLight
================
*/
void idExplodingBarrel::AddLight( const char *name, bool burn ) {
	if ( lightDefHandle >= 0 ){
		gameRenderWorld->FreeLightDef( lightDefHandle );
	}
	memset( &light, 0, sizeof ( light ) );
	light.axis = mat3_identity;
	light.lightRadius.x = spawnArgs.GetFloat( "light_radius" );
	light.lightRadius.y = light.lightRadius.z = light.lightRadius.x;
	light.origin = physicsObj.GetOrigin();
	light.origin.z += 128;
	light.pointLight = true;
	light.shader = declManager->FindMaterial( name );
	light.shaderParms[ SHADERPARM_RED ] = 2.0f;
	light.shaderParms[ SHADERPARM_GREEN ] = 2.0f;
	light.shaderParms[ SHADERPARM_BLUE ] = 2.0f;
	light.shaderParms[ SHADERPARM_ALPHA ] = 2.0f;
	lightDefHandle = gameRenderWorld->AddLightDef( &light );
	lightTime = gameLocal.realClientTime;
	BecomeActive( TH_THINK );
}

/*
================
idExplodingBarrel::ExplodingEffects
================
*/
void idExplodingBarrel::ExplodingEffects( void ) {
	const char *temp;

	StartSound( "snd_explode", SND_CHANNEL_ANY, 0, false, NULL );

	temp = spawnArgs.GetString( "model_damage" );
	if ( *temp != '\0' ) {
		SetModel( temp );
		Show();
	}

	temp = spawnArgs.GetString( "model_detonate" );
	if ( *temp != '\0' ) {
		AddParticles( temp, false );
	}

	temp = spawnArgs.GetString( "mtr_lightexplode" );
	if ( *temp != '\0' ) {
		AddLight( temp, false );
	}

	temp = spawnArgs.GetString( "mtr_burnmark" );
	if ( *temp != '\0' ) {
		gameLocal.ProjectDecal( GetPhysics()->GetOrigin(), GetPhysics()->GetGravity(), 128.0f, true, 96.0f, temp );
	}
}

/*
================
idExplodingBarrel::Killed
================
*/
void idExplodingBarrel::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {

	if ( IsHidden() || state == EXPLODING || state == BURNING ) {
		return;
	}

	float f = spawnArgs.GetFloat( "burn" );
	if ( f > 0.0f && state == NORMAL ) {
		state = BURNING;
		PostEventSec( &EV_Explode, f );
		StartSound( "snd_burn", SND_CHANNEL_ANY, 0, false, NULL );
		AddParticles( spawnArgs.GetString ( "model_burn", "" ), true );
		return;
	} else {
		state = EXPLODING;
		if ( gameLocal.isServer ) {
			idBitMsg	msg;
			byte		msgBuf[MAX_EVENT_PARAM_SIZE];

			msg.Init( msgBuf, sizeof( msgBuf ) );
			msg.WriteLong( gameLocal.time );
			ServerSendEvent( EVENT_EXPLODE, &msg, false, -1 );
		}		
	}

	// do this before applying radius damage so the ent can trace to any damagable ents nearby
	Hide();
	physicsObj.SetContents( 0 );

	const char *splash = spawnArgs.GetString( "def_splash_damage", "damage_explosion" );
	if ( splash && *splash ) {
		gameLocal.RadiusDamage( GetPhysics()->GetOrigin(), this, attacker, this, this, splash );
	}

	ExplodingEffects( );
	
	//FIXME: need to precache all the debris stuff here and in the projectiles
	const idKeyValue *kv = spawnArgs.MatchPrefix( "def_debris" );
	// bool first = true;
	while ( kv ) {
		const idDict *debris_args = gameLocal.FindEntityDefDict( kv->GetValue(), false );
		if ( debris_args ) {
			idEntity *ent;
			idVec3 dir;
			idDebris *debris;
			//if ( first ) {
				dir = physicsObj.GetAxis()[1];
			//	first = false;
			//} else {
				dir.x += gameLocal.random.CRandomFloat() * 4.0f;
				dir.y += gameLocal.random.CRandomFloat() * 4.0f;
				//dir.z = gameLocal.random.RandomFloat() * 8.0f;
			//}
			dir.Normalize();

			gameLocal.SpawnEntityDef( *debris_args, &ent, false );
			if ( !ent || !ent->IsType( idDebris::Type ) ) {
				gameLocal.Error( "'projectile_debris' is not an idDebris" );
			}

			debris = static_cast<idDebris *>(ent);
			debris->Create( this, physicsObj.GetOrigin(), dir.ToMat3() );
			debris->Launch();
			debris->GetRenderEntity()->shaderParms[ SHADERPARM_TIME_OF_DEATH ] = ( gameLocal.time + 1500 ) * 0.001f;
			debris->UpdateVisuals();
			
		}
		kv = spawnArgs.MatchPrefix( "def_debris", kv );
	}

	physicsObj.PutToRest();
	CancelEvents( &EV_Explode );
	CancelEvents( &EV_Activate );

	f = spawnArgs.GetFloat( "respawn" );
	if ( f > 0.0f ) {
		PostEventSec( &EV_Respawn, f );
	} else {
		PostEventMS( &EV_Remove, 5000 );
	}

	if ( spawnArgs.GetBool( "triggerTargets" ) ) {
		ActivateTargets( this );
	}
}

/*
================
idExplodingBarrel::Damage
================
*/
void idExplodingBarrel::Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir, 
					  const char *damageDefName, const float damageScale, const int location, trace_t *tr ) 
{

	const idDict *damageDef = gameLocal.FindEntityDefDict( damageDefName );
	if ( !damageDef ) {
		gameLocal.Error( "Unknown damageDef '%s'\n", damageDefName );
	}
	if ( damageDef->FindKey( "radius" ) && GetPhysics()->GetContents() != 0 && GetBindMaster() == NULL ) {
		PostEventMS( &EV_Explode, 400 );
	} else {
		idEntity::Damage( inflictor, attacker, dir, damageDefName, damageScale, location, tr );
	}
}

/*
================
idExplodingBarrel::Event_TriggerTargets
================
*/
void idExplodingBarrel::Event_TriggerTargets() {
	ActivateTargets( this );
}

/*
================
idExplodingBarrel::Event_Explode
================
*/
void idExplodingBarrel::Event_Explode() {
	if ( state == NORMAL || state == BURNING ) {
		state = BURNEXPIRED;
		Killed( NULL, NULL, 0, vec3_zero, 0 );
	}
}

/*
================
idExplodingBarrel::Event_Respawn
================
*/
void idExplodingBarrel::Event_Respawn() {
	int i;
	int minRespawnDist = spawnArgs.GetInt( "respawn_range", "256" );
	if ( minRespawnDist ) {
		float minDist = -1;
		for ( i = 0; i < gameLocal.numClients; i++ ) {
			if ( !gameLocal.entities[ i ] || !gameLocal.entities[ i ]->IsType( idPlayer::Type ) ) {
				continue;
			}
			idVec3 v = gameLocal.entities[ i ]->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
			float dist = v.Length();
			if ( minDist < 0 || dist < minDist ) {
				minDist = dist;
			}
		}
		if ( minDist < minRespawnDist ) {
			PostEventSec( &EV_Respawn, spawnArgs.GetInt( "respawn_again", "10" ) );
			return;
		}
	}
	const char *temp = spawnArgs.GetString( "model" );
	if ( temp && *temp ) {
		SetModel( temp );
	}
	health = spawnArgs.GetInt( "health", "5" );
	fl.takedamage = true;
	physicsObj.SetOrigin( spawnOrigin );
	physicsObj.SetAxis( spawnAxis );
	physicsObj.SetContents( CONTENTS_SOLID );
	// override with custom contents if present
	if( m_CustomContents != -1 )
		physicsObj.SetContents( m_CustomContents );
	// SR CONTENTS_RESPONSE FIX
	if( m_StimResponseColl->HasResponse() )
		physicsObj.SetContents( physicsObj.GetContents() | CONTENTS_RESPONSE );
	physicsObj.DropToFloor();
	state = NORMAL;
	Show();
	UpdateVisuals();
}

/*
================
idMoveable::Event_Activate
================
*/
void idExplodingBarrel::Event_Activate( idEntity *activator ) {
	Killed( activator, activator, 0, vec3_origin, 0 );
}

/*
================
idMoveable::WriteToSnapshot
================
*/
void idExplodingBarrel::WriteToSnapshot( idBitMsgDelta &msg ) const {
	idMoveable::WriteToSnapshot( msg );
	msg.WriteBits( IsHidden(), 1 );
}

/*
================
idMoveable::ReadFromSnapshot
================
*/
void idExplodingBarrel::ReadFromSnapshot( const idBitMsgDelta &msg ) {

	idMoveable::ReadFromSnapshot( msg );
	if ( msg.ReadBits( 1 ) ) {
		Hide();
	} else {
		Show();
	}
}

/*
================
idExplodingBarrel::ClientReceiveEvent
================
*/
bool idExplodingBarrel::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {

	switch( event ) {
		case EVENT_EXPLODE: {
			if ( gameLocal.realClientTime - msg.ReadLong() < spawnArgs.GetInt( "explode_lapse", "1000" ) ) {
				ExplodingEffects( );
			}
			return true;
		}
		default: {
			return idBarrel::ClientReceiveEvent( event, time, msg );
		}
	}
//	return false;
}
