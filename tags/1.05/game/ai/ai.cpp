/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4737 $
 * $Date: 2011-03-30 01:50:23 -0400 (Wed, 30 Mar 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ai.cpp 4737 2011-03-30 05:50:23Z greebo $", init_version);

#include "../game_local.h"
#include "../../DarkMod/AI/Mind.h"
#include "../../DarkMod/AI/Subsystem.h"
#include "../../DarkMod/AI/MovementSubsystem.h"
#include "../../DarkMod/AI/Memory.h"
#include "../../DarkMod/AI/States/KnockedOutState.h"
#include "../../DarkMod/AI/States/DeadState.h"
#include "../../DarkMod/AI/States/ConversationState.h"
#include "../../DarkMod/AI/States/PainState.h"
#include "../../DarkMod/AI/States/IdleState.h"
#include "../../DarkMod/AI/States/ObservantState.h"
#include "../../DarkMod/AI/States/SuspiciousState.h"
#include "../../DarkMod/AI/States/SearchingState.h"
#include "../../DarkMod/AI/States/AgitatedSearchingState.h"
#include "../../DarkMod/AI/States/CombatState.h"
#include "../../DarkMod/AI/Tasks/SingleBarkTask.h"
#include "../../DarkMod/AI/Conversation/ConversationSystem.h"
#include "../../DarkMod/Relations.h"
#include "../../DarkMod/MissionData.h"
#include "../../DarkMod/StimResponse/StimResponseCollection.h"
#include "../../DarkMod/AbsenceMarker.h"
#include "../../DarkMod/BloodMarker.h"
#include "../../DarkMod/DarkModGlobals.h"
#include "../../DarkMod/MultiStateMover.h"
#include "../../DarkMod/MeleeWeapon.h"
#include "../../DarkMod/sndProp.h"
#include "../../DarkMod/EscapePointManager.h"
#include "../../DarkMod/PositionWithinRangeFinder.h"
#include "../../DarkMod/TimerManager.h"

// For handling the opening of doors and other binary Frob movers
#include "../../DarkMod/BinaryFrobMover.h"
#include "../../DarkMod/FrobDoor.h"
#include "../../DarkMod/FrobDoorHandle.h"
#include "tdmAASFindEscape.h"
#include "../../DarkMod/AI/AreaManager.h"

#include <boost/lexical_cast.hpp>

//TODO: Move these to AI def:

// Visual detection parameters

/**
* amount of time to normalize by for the % check, in seconds
*  this gets modified by 1 / the cvar dm_ai_sight.
**/
static const float s_VisNormtime = 0.2f;

/**
* In pitch darkness, player is invisible
**/
// full bright minimum distance is 0
static const float s_VisFDMin = 0.0f;

// full bright maximum distance is zero (because probability is always zero anyway)
static const float s_VisFDMax = 0.0f;

// TODO: Move this to def file or INI
static const float s_AITactDist = 1.0f;

const float s_DOOM_TO_METERS = 0.0254f;

// TDM: Maximum flee distance for any AI
const float MAX_FLEE_DISTANCE = 10000.0f;

class CRelations;
class CsndProp;
class CDarkModPlayer;
class CMissionData;

static const char *moveCommandString[ NUM_MOVE_COMMANDS ] = {
	"MOVE_NONE",
	"MOVE_FACE_ENEMY",
	"MOVE_FACE_ENTITY",
	"MOVE_TO_ENEMY",
	"MOVE_TO_ENEMYHEIGHT",
	"MOVE_TO_ENTITY",
	"MOVE_OUT_OF_RANGE",
	"MOVE_TO_ATTACK_POSITION",
	"MOVE_TO_COVER",
	"MOVE_TO_POSITION",
	"MOVE_TO_POSITION_DIRECT",
	"MOVE_SLIDE_TO_POSITION",
	"MOVE_WANDER"
};

/*
============
idAASFindCover::idAASFindCover
============
*/
idAASFindCover::idAASFindCover( const idActor* hidingActor, const idEntity* hideFromEnt, const idVec3 &hideFromPos ) {
	this->hidingActor = hidingActor; // This should not be NULL
	this->hideFromEnt = hideFromEnt; // May be NULL
	this->hideFromPos = hideFromPos;
}

/*
============
idAASFindCover::~idAASFindCover
============
*/
idAASFindCover::~idAASFindCover() {
}

/*
============
idAASFindCover::TestArea
============
*/
bool idAASFindCover::TestArea( const idAAS *aas, int areaNum )
{
	idVec3	areaCenter;
	trace_t	trace, trace2;

	if (areaNum == aas->PointAreaNum(hidingActor->GetPhysics()->GetOrigin()))
	{
		// We're in this AAS area; assume that our current position can never be cover.
		// (If it was, we probably wouldn't be trying to move into cover.)
		return false;
	}

	// Get location of feet
	// Assumes they're at the centre of the bounding box in the X and Y axes,
	// but at the bottom in the Z axis.
	idBounds bounds = hidingActor->GetPhysics()->GetAbsBounds();
	idVec3 feet = (bounds[0] + bounds[1]) * 0.5f;
	feet.z = bounds[0].z;

	// Get location to trace to
	areaCenter = aas->AreaCenter( areaNum );

	// Adjust areaCenter to factor in height of AI, so we trace from the estimated eye position
	areaCenter += hidingActor->GetEyePosition() - feet;

	gameLocal.clip.TracePoint(trace, hideFromPos, areaCenter, MASK_OPAQUE, hideFromEnt);
	if (trace.fraction < 1.0f)
	{
		// The trace was interrupted, so this location is probably cover.
		//gameRenderWorld->DebugLine( colorGreen, hideFromPos, areaCenter, 5000, true);

		// But before we say for certain, let's look at the floor as well.
		areaCenter = aas->AreaCenter( areaNum );
		gameLocal.clip.TracePoint(trace2, hideFromPos, areaCenter, MASK_OPAQUE, hideFromEnt);
		if (trace2.fraction < 1.0f)
		{
			// Yes, the feet are hidden too, so this is almost certainly cover
			//gameRenderWorld->DebugLine( colorGreen, hideFromPos, areaCenter, 5000, true);
			return true;
		}

		// Oops; the head is hidden but the feet are not, so this isn't very good cover at all.
		//gameRenderWorld->DebugLine( colorRed, hideFromPos, areaCenter, 5000, true);
		return false;
	}

	// The trace found a clear path, so this location is not cover.
	//gameRenderWorld->DebugLine( colorRed, hideFromPos, areaCenter, 5000, true);
	return false;
}

/*
============
idAASFindAreaOutOfRange::idAASFindAreaOutOfRange
============
*/
idAASFindAreaOutOfRange::idAASFindAreaOutOfRange( const idVec3 &targetPos, float maxDist ) {
	this->targetPos		= targetPos;
	this->maxDistSqr	= maxDist * maxDist;
}

/*
============
idAASFindAreaOutOfRange::TestArea
============
*/
bool idAASFindAreaOutOfRange::TestArea( const idAAS *aas, int areaNum ) {
	const idVec3 &areaCenter = aas->AreaCenter( areaNum );
	trace_t	trace;
	float dist;

	dist = ( targetPos.ToVec2() - areaCenter.ToVec2() ).LengthSqr();

	if ( ( maxDistSqr > 0.0f ) && ( dist < maxDistSqr ) ) {
		return false;
	}

	gameLocal.clip.TracePoint( trace, targetPos, areaCenter + idVec3( 0.0f, 0.0f, 1.0f ), MASK_OPAQUE, NULL );
	if ( trace.fraction < 1.0f ) {
		return false;
	}

	return true;
}

/*
============
idAASFindAttackPosition::idAASFindAttackPosition
============
*/
idAASFindAttackPosition::idAASFindAttackPosition(idAI *self, const idMat3 &gravityAxis, idEntity *target, const idVec3 &targetPos, const idVec3 &fireOffset)
{
	int	numPVSAreas;

	this->target		= target;
	this->targetPos		= targetPos;
	this->fireOffset	= fireOffset;
	this->self			= self;
	this->gravityAxis	= gravityAxis;

	excludeBounds		= idBounds( idVec3( -64.0, -64.0f, -8.0f ), idVec3( 64.0, 64.0f, 64.0f ) );
	excludeBounds.TranslateSelf( self->GetPhysics()->GetOrigin() );

	// setup PVS
	idBounds bounds( targetPos - idVec3( 16, 16, 0 ), targetPos + idVec3( 16, 16, 64 ) );
	numPVSAreas = gameLocal.pvs.GetPVSAreas( bounds, PVSAreas, idEntity::MAX_PVS_AREAS );
	targetPVS	= gameLocal.pvs.SetupCurrentPVS( PVSAreas, numPVSAreas );
}

/*
============
idAASFindAttackPosition::~idAASFindAttackPosition
============
*/
idAASFindAttackPosition::~idAASFindAttackPosition() {
	gameLocal.pvs.FreeCurrentPVS( targetPVS );
}

/*
============
idAASFindAttackPosition::TestArea
============
*/
bool idAASFindAttackPosition::TestArea( const idAAS *aas, int areaNum ) {
	idVec3	dir;
	idVec3	local_dir;
	idVec3	fromPos;
	idMat3	axis;
	idVec3	areaCenter;
	int		numPVSAreas;
	int		PVSAreas[ idEntity::MAX_PVS_AREAS ];

	areaCenter = aas->AreaCenter( areaNum );
	areaCenter[ 2 ] += 1.0f;

	if ( excludeBounds.ContainsPoint( areaCenter ) ) {
		// too close to where we already are
		return false;
	}

	numPVSAreas = gameLocal.pvs.GetPVSAreas( idBounds( areaCenter ).Expand( 16.0f ), PVSAreas, idEntity::MAX_PVS_AREAS );
	if ( !gameLocal.pvs.InCurrentPVS( targetPVS, PVSAreas, numPVSAreas ) ) {
		return false;
	}

	// calculate the world transform of the launch position
	dir = targetPos - areaCenter;
	gravityAxis.ProjectVector( dir, local_dir );
	local_dir.z = 0.0f;
	local_dir.ToVec2().Normalize();
	axis = local_dir.ToMat3();
	fromPos = areaCenter + fireOffset * axis;

	return self->GetAimDir( fromPos, target, self, dir );
}


/*
============
idAASFindObservationPosition::idAASFindObservationPosition
============
*/
idAASFindObservationPosition::idAASFindObservationPosition( const idAI *self, const idMat3 &gravityAxis, const idVec3 &targetPos, const idVec3 &eyeOffset, float maxDistanceFromWhichToObserve )
{
	int	numPVSAreas;

	this->targetPos		= targetPos;
	this->eyeOffset		= eyeOffset;
	this->self			= self;
	this->gravityAxis	= gravityAxis;
	this->maxObservationDistance = maxDistanceFromWhichToObserve;
	this->b_haveBestGoal = false;


	// setup PVS
	idBounds bounds( targetPos - idVec3( 16, 16, 0 ), targetPos + idVec3( 16, 16, 64 ) );
	numPVSAreas = gameLocal.pvs.GetPVSAreas( bounds, PVSAreas, idEntity::MAX_PVS_AREAS );
	targetPVS	= gameLocal.pvs.SetupCurrentPVS( PVSAreas, numPVSAreas );
}

/*
============
idAASFindObservationPosition::~idAASFindObservationPosition
============
*/
idAASFindObservationPosition::~idAASFindObservationPosition() {
	gameLocal.pvs.FreeCurrentPVS( targetPVS );
}

/*
============
idAASFindObservationPosition::TestArea
============
*/
bool idAASFindObservationPosition::TestArea( const idAAS *aas, int areaNum )
{
	idVec3	dir;
	idVec3	local_dir;
	idVec3	fromPos;
	idMat3	axis;
	idVec3	areaCenter;

	areaCenter = aas->AreaCenter( areaNum );
	areaCenter[ 2 ] += 1.0f;

	/*
	numPVSAreas = gameLocal.pvs.GetPVSAreas( idBounds( areaCenter ).Expand( 16.0f ), PVSAreas, idEntity::MAX_PVS_AREAS );
	if ( !gameLocal.pvs.InCurrentPVS( targetPVS, PVSAreas, numPVSAreas ) ) {
		return false;
	}
	*/

	// calculate the world transform of the view position
	dir = targetPos - areaCenter;
	gravityAxis.ProjectVector( dir, local_dir );
	local_dir.z = 0.0f;
	local_dir.ToVec2().Normalize();
	axis = local_dir.ToMat3();

	fromPos = areaCenter + eyeOffset * axis;

	// Run trace
	trace_t results;
	gameLocal.clip.TracePoint( results, fromPos, targetPos, MASK_SOLID, self );
	if (  results.fraction >= 1.0f )
	{
		// What is the observation distance?
		float distance = (fromPos - targetPos).Length();

		// Remember best result, even if outside max distance allowed
		if ((!b_haveBestGoal) || (distance < bestGoalDistance))
		{
			b_haveBestGoal = true;
			bestGoalDistance = distance;
			bestGoal.areaNum = areaNum;
			bestGoal.origin = areaCenter;
		}
		if (distance > maxObservationDistance)
		{
			// Can't use this point, its too far
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}

}

//------------------------------------------------------------------------------

bool idAASFindObservationPosition::getBestGoalResult
(
	float& out_bestGoalDistance,
	aasGoal_t& out_bestGoal
)
{
	if (b_haveBestGoal)
	{
		out_bestGoalDistance = bestGoalDistance;
		out_bestGoal = bestGoal;
		return true;
	}
	else
	{
		return false;
	}
}


/*
=====================
idAI::idAI
=====================
*/
idAI::idAI()
{
	aiNode.SetOwner(this);

	aas					= NULL;
	travelFlags			= TFL_WALK|TFL_AIR|TFL_DOOR;
	lastAreaReevaluationTime = -1;
	maxAreaReevaluationInterval = 2000; // msec
	doorRetryTime		= 120000; // msec

	kickForce			= 60.0f; // grayman #2568 - default Doom 3 value
	ignore_obstacles	= false;
	blockedRadius		= 0.0f;
	blockedMoveTime		= 750;
	blockedAttackTime	= 750;
	turnRate			= 360.0f;
	turnVel				= 0.0f;
	anim_turn_yaw		= 0.0f;
	anim_turn_amount	= 0.0f;
	anim_turn_angles	= 0.0f;
	reachedpos_bbox_expansion = 0.0f;
	aas_reachability_z_tolerance = 75;
	fly_offset			= 0;
	fly_seek_scale		= 1.0f;
	fly_roll_scale		= 0.0f;
	fly_roll_max		= 0.0f;
	fly_roll			= 0.0f;
	fly_pitch_scale		= 0.0f;
	fly_pitch_max		= 0.0f;
	fly_pitch			= 0.0f;
	allowMove			= false;
	allowHiddenMovement	= false;
	fly_speed			= 0.0f;
	fly_bob_strength	= 0.0f;
	fly_bob_vert		= 0.0f;
	fly_bob_horz		= 0.0f;
	lastHitCheckResult	= false;
	lastHitCheckTime	= 0;
	lastAttackTime		= 0;
	fire_range			= 0.0f;
	projectile_height_to_distance_ratio = 1.0f;
	activeProjectile.projEnt = NULL;
	curProjectileIndex	= -1;
	talk_state			= TALK_NEVER;
	talkTarget			= NULL;

	particles.Clear();
	restartParticles	= true;
	useBoneAxis			= false;

	wakeOnFlashlight	= false;
	lastUpdateEnemyPositionTime = -1;
	memset( &worldMuzzleFlash, 0, sizeof ( worldMuzzleFlash ) );
	worldMuzzleFlashHandle = -1;

	enemy				= NULL;
	lastVisibleEnemyPos.Zero();
	lastVisibleEnemyEyeOffset.Zero();
	lastVisibleReachableEnemyPos.Zero();
	lastReachableEnemyPos.Zero();
	enemyReachable		= false;
	shrivel_rate		= 0.0f;
	shrivel_start		= 0;
	fl.neverDormant		= false;		// AI's can go dormant
	current_yaw			= 0.0f;
	ideal_yaw			= 0.0f;

	num_cinematics		= 0;
	current_cinematic	= 0;

	allowEyeFocus		= true;
	allowPain			= true;
	allowJointMod		= true;
	focusEntity			= NULL;
	focusTime			= 0;
	alignHeadTime		= 0;
	forceAlignHeadTime	= 0;

	currentFocusPos.Zero();
	eyeAng.Zero();
	lookAng.Zero();
	destLookAng.Zero();
	lookMin.Zero();
	lookMax.Zero();

	eyeMin.Zero();
	eyeMax.Zero();
	muzzleFlashEnd		= 0;
	flashTime			= 0;
	flashJointWorld		= INVALID_JOINT;

	focusJoint			= INVALID_JOINT;
	orientationJoint	= INVALID_JOINT;
	flyTiltJoint		= INVALID_JOINT;

	eyeVerticalOffset	= 0.0f;
	eyeHorizontalOffset = 0.0f;
	eyeFocusRate		= 0.0f;
	headFocusRate		= 0.0f;
	focusAlignTime		= 0;
	m_tactileEntity		= NULL; // grayman #2345
	m_canResolveBlock	= true;	// grayman #2345
	m_leftQueue			= false; // grayman #2345

	m_SoundDir.Zero();
	m_LastSight.Zero();
	m_AlertLevelThisFrame = 0.0f;
	m_prevAlertIndex = 0;
	m_maxAlertLevel = 0;
	m_maxAlertIndex = 0;
	m_AlertedByActor = NULL;

	m_TactAlertEnt = NULL;
	m_AlertGraceActor = NULL;
	m_AlertGraceStart = 0;
	m_AlertGraceTime = 0;
	m_AlertGraceThresh = 0;
	m_AlertGraceCount = 0;
	m_AlertGraceCountLimit = 0;
	m_AudThreshold = 0.0f;
	m_oldVisualAcuity = 0.0f;

	/**
	* Darkmod: No hiding spot search by default
	*/
	m_HidingSpotSearchHandle = NULL_HIDING_SPOT_SEARCH_HANDLE;

	m_bCanDrown = true;
	m_AirCheckTimer = 0;
	m_AirTics = 0;
	m_AirTicksMax = 0;
	m_HeadBodyID = 0;
	m_HeadJointID = INVALID_JOINT;
	m_OrigHeadCM = NULL;
	m_bHeadCMSwapped = false;
	m_MouthOffset = vec3_zero;

	m_bCanBeKnockedOut = true;
	m_HeadCenterOffset = vec3_zero;
	m_FOVRot = mat3_identity;
	m_bKoAlertImmune = false;
	m_KoDotVert = 0;
	m_KoDotHoriz = 0;
	m_KoAlertDotHoriz = 0;
	m_KoRot = mat3_identity;

	m_bCanBeGassed = true;		// grayman #2468
	m_koState = KO_NOT;			// grayman #2604
	m_earlyThinkCounter = 5 + gameLocal.random.RandomInt(5);	// grayman #2654

	m_bCanOperateDoors = false;

	m_lipSyncActive		= false;

	m_bPushOffPlayer	= false;

	m_bCanBeFlatFooted	= false;
	m_bFlatFooted		= false;
	m_FlatFootedTimer	= 0;
	m_FlatFootedTime	= 0;

	m_FlatFootParryNum	= 0;
	m_FlatFootParryMax	= 0;
	m_FlatFootParryTimer = 0;
	m_FlatFootParryTime	= 0;
	m_MeleeCounterAttChance = 0.0f;
	m_bMeleePredictProximity = false;

	m_maxInterleaveThinkFrames = 0;
	m_minInterleaveThinkDist = 1000;
	m_maxInterleaveThinkDist = 3000;
	m_lastThinkTime = 0;
	m_nextThinkFrame = 0;

	INIT_TIMER_HANDLE(aiThinkTimer);
	INIT_TIMER_HANDLE(aiMindTimer);
	INIT_TIMER_HANDLE(aiAnimationTimer);
	INIT_TIMER_HANDLE(aiPushWithAFTimer);
	INIT_TIMER_HANDLE(aiUpdateEnemyPositionTimer);
	INIT_TIMER_HANDLE(aiScriptTimer);
	INIT_TIMER_HANDLE(aiAnimMoveTimer);
	INIT_TIMER_HANDLE(aiObstacleAvoidanceTimer);
	INIT_TIMER_HANDLE(aiPhysicsTimer);
	INIT_TIMER_HANDLE(aiGetMovePosTimer);
	INIT_TIMER_HANDLE(aiPathToGoalTimer);
	INIT_TIMER_HANDLE(aiGetFloorPosTimer);
	INIT_TIMER_HANDLE(aiPointReachableAreaNumTimer);
	INIT_TIMER_HANDLE(aiCanSeeTimer);

}

/*
=====================
idAI::~idAI
=====================
*/
idAI::~idAI()
{
	DeconstructScriptObject();
	scriptObject.Free();
	if ( worldMuzzleFlashHandle != -1 )
	{
		gameRenderWorld->FreeLightDef( worldMuzzleFlashHandle );
		worldMuzzleFlashHandle = -1;
	}

	/**
	* Darkmod: Get rid of current hiding spot search if there is one.
	*/
	destroyCurrentHidingSpotSearch();

	aiNode.Remove();

	if( m_OrigHeadCM )
		delete m_OrigHeadCM;
}

/*
=====================
idAI::Save
=====================
*/
void idAI::Save( idSaveGame *savefile ) const {
	int i;

	savefile->WriteInt( travelFlags );
	savefile->WriteInt(lastAreaReevaluationTime);
	savefile->WriteInt(maxAreaReevaluationInterval);
	savefile->WriteInt(doorRetryTime);

	move.Save( savefile );
	savedMove.Save( savefile );

	// greebo: save the movestack
	savefile->WriteInt(static_cast<int>(moveStack.size()));
	for (std::list<idMoveState>::const_iterator m = moveStack.begin(); m != moveStack.end(); ++m)
	{
		m->Save(savefile);
	}

	savefile->WriteFloat( kickForce );
	savefile->WriteBool( ignore_obstacles );
	savefile->WriteFloat( blockedRadius );
	savefile->WriteInt( blockedMoveTime );
	savefile->WriteInt( blockedAttackTime );

	savefile->WriteFloat( ideal_yaw );
	savefile->WriteFloat( current_yaw );
	savefile->WriteFloat( turnRate );
	savefile->WriteFloat( turnVel );
	savefile->WriteFloat( anim_turn_yaw );
	savefile->WriteFloat( anim_turn_amount );
	savefile->WriteFloat( anim_turn_angles );
	savefile->WriteVec3(sitting_turn_pivot);

	savefile->WriteFloat(reachedpos_bbox_expansion);
	savefile->WriteFloat(aas_reachability_z_tolerance);

	savefile->WriteStaticObject( physicsObj );

	savefile->WriteFloat( fly_speed );
	savefile->WriteFloat( fly_bob_strength );
	savefile->WriteFloat( fly_bob_vert );
	savefile->WriteFloat( fly_bob_horz );
	savefile->WriteInt( fly_offset );
	savefile->WriteFloat( fly_seek_scale );
	savefile->WriteFloat( fly_roll_scale );
	savefile->WriteFloat( fly_roll_max );
	savefile->WriteFloat( fly_roll );
	savefile->WriteFloat( fly_pitch_scale );
	savefile->WriteFloat( fly_pitch_max );
	savefile->WriteFloat( fly_pitch );

	savefile->WriteBool( allowMove );
	savefile->WriteBool( allowHiddenMovement );
	savefile->WriteBool( disableGravity );

	savefile->WriteBool( lastHitCheckResult );
	savefile->WriteInt( lastHitCheckTime );
	savefile->WriteInt( lastAttackTime );
	savefile->WriteFloat( fire_range );
	savefile->WriteFloat( projectile_height_to_distance_ratio );

	savefile->WriteInt( missileLaunchOffset.Num() );
	for( i = 0; i < missileLaunchOffset.Num(); i++ ) {
		savefile->WriteVec3( missileLaunchOffset[ i ] );
	}

	savefile->WriteInt(projectileInfo.Num());

	for (i = 0; i < projectileInfo.Num(); ++i)
	{
		const ProjectileInfo& info = projectileInfo[i];

		// Save projectile def names instead of dict* pointers, will be resolved at restore time
		// also leave clipmodel pointer alone, will be initialised to NULL on restore and
		// reloaded on demand
		savefile->WriteString(info.defName);
		savefile->WriteFloat(info.radius);
		savefile->WriteFloat(info.speed);
		savefile->WriteVec3(info.velocity);
		savefile->WriteVec3(info.gravity);
	}

	savefile->WriteInt(curProjectileIndex);

	// Active Projectile
	savefile->WriteString(activeProjectile.info.defName);
	savefile->WriteFloat(activeProjectile.info.radius);
	savefile->WriteFloat(activeProjectile.info.speed);
	savefile->WriteVec3(activeProjectile.info.velocity);
	savefile->WriteVec3(activeProjectile.info.gravity);
	activeProjectile.projEnt.Save(savefile);

	savefile->WriteString( attack );

	savefile->WriteInt( talk_state );
	talkTarget.Save( savefile );

	savefile->WriteInt( num_cinematics );
	savefile->WriteInt( current_cinematic );

	savefile->WriteBool( allowJointMod );
	focusEntity.Save( savefile );
	savefile->WriteVec3( currentFocusPos );
	savefile->WriteInt( focusTime );
	savefile->WriteInt( alignHeadTime );
	savefile->WriteInt( forceAlignHeadTime );
	savefile->WriteAngles( eyeAng );
	savefile->WriteAngles( lookAng );
	savefile->WriteAngles( destLookAng );
	savefile->WriteAngles( lookMin );
	savefile->WriteAngles( lookMax );

	savefile->WriteInt( lookJoints.Num() );
	for( i = 0; i < lookJoints.Num(); i++ ) {
		savefile->WriteJoint( lookJoints[ i ] );
		savefile->WriteAngles( lookJointAngles[ i ] );
	}
	savefile->WriteInt( lookJointsCombat.Num() );
	for( i = 0; i < lookJointsCombat.Num(); i++ ) {
		savefile->WriteJoint( lookJointsCombat[ i ] );
		savefile->WriteAngles( lookJointAnglesCombat[ i ] );
	}

	savefile->WriteFloat( shrivel_rate );
	savefile->WriteInt( shrivel_start );

	savefile->WriteInt( particles.Num() );
	for  ( i = 0; i < particles.Num(); i++ ) {
		savefile->WriteParticle( particles[i].particle );
		savefile->WriteInt( particles[i].time );
		savefile->WriteJoint( particles[i].joint );
	}
	savefile->WriteBool( restartParticles );
	savefile->WriteBool( useBoneAxis );

	enemy.Save( savefile );
	savefile->WriteVec3( lastVisibleEnemyPos );
	savefile->WriteVec3( lastVisibleEnemyEyeOffset );
	savefile->WriteVec3( lastVisibleReachableEnemyPos );
	savefile->WriteVec3( lastReachableEnemyPos );
	savefile->WriteBool( enemyReachable );
	savefile->WriteBool( wakeOnFlashlight );
	savefile->WriteInt(lastUpdateEnemyPositionTime);

	savefile->WriteAngles( eyeMin );
	savefile->WriteAngles( eyeMax );

	savefile->WriteFloat( eyeVerticalOffset );
	savefile->WriteFloat( eyeHorizontalOffset );
	savefile->WriteFloat( eyeFocusRate );
	savefile->WriteFloat( headFocusRate );
	savefile->WriteInt( focusAlignTime );
	savefile->WriteObject(m_tactileEntity);		// grayman #2345
	savefile->WriteBool(m_canResolveBlock);		// grayman #2345
	savefile->WriteBool(m_leftQueue);			// grayman #2345
	savefile->WriteJoint( flashJointWorld );
	savefile->WriteInt( muzzleFlashEnd );

	savefile->WriteJoint( focusJoint );
	savefile->WriteJoint( orientationJoint );
	savefile->WriteJoint( flyTiltJoint );

	// TDM Alerts:
	savefile->WriteInt( m_Acuities.Num() );
	for( i = 0; i < m_Acuities.Num(); i++ )
	{
		savefile->WriteFloat( m_Acuities[ i ] );
	}
	savefile->WriteFloat(m_oldVisualAcuity);
	savefile->WriteFloat( m_AudThreshold );
	savefile->WriteVec3( m_SoundDir );
	savefile->WriteVec3( m_LastSight );
	savefile->WriteFloat( m_AlertLevelThisFrame );
	savefile->WriteInt( m_prevAlertIndex );
	savefile->WriteFloat( m_maxAlertLevel);
	savefile->WriteInt( m_maxAlertIndex);
	savefile->WriteFloat(m_lastAlertLevel);
	savefile->WriteBool( m_bIgnoreAlerts );

	m_AlertedByActor.Save( savefile );

	for (i = 0; i < ai::EAlertTypeCount; i++)
	{
		savefile->WriteInt(alertTypeWeight[i]);
	}

	m_TactAlertEnt.Save( savefile );
	m_AlertGraceActor.Save( savefile );
	savefile->WriteInt( m_AlertGraceStart );
	savefile->WriteInt( m_AlertGraceTime );
	savefile->WriteFloat( m_AlertGraceThresh );
	savefile->WriteInt( m_AlertGraceCount );
	savefile->WriteInt( m_AlertGraceCountLimit );

	savefile->WriteInt(m_Messages.Num());
	for (i = 0; i < m_Messages.Num(); i++)
	{
		m_Messages[i]->Save(savefile);
	}

	savefile->WriteBool( GetPhysics() == static_cast<const idPhysics *>(&physicsObj) );

	savefile->WriteFloat(m_VisDistMax);

	savefile->WriteInt(m_HidingSpotSearchHandle);
	m_hidingSpots.Save(savefile);

	savefile->WriteInt(m_AirCheckTimer);
	savefile->WriteBool(m_bCanDrown);
	savefile->WriteInt(m_HeadBodyID);
	savefile->WriteJoint(m_HeadJointID);

	savefile->WriteBool( m_bHeadCMSwapped );
//	if( m_bHeadCMSwapped && m_OrigHeadCM )
//		m_OrigHeadCM->Save( savefile );

	savefile->WriteInt(m_AirTics);
	savefile->WriteInt(m_AirTicksMax);
	savefile->WriteInt(m_AirCheckInterval);

	savefile->WriteVec3(m_MouthOffset);
	savefile->WriteBool(m_bCanBeKnockedOut);
	savefile->WriteVec3(m_HeadCenterOffset);
	savefile->WriteMat3(m_FOVRot);
	savefile->WriteString(m_KoZone);
	savefile->WriteInt(m_KoAlertState);
	savefile->WriteInt(m_KoAlertImmuneState);
	savefile->WriteBool(m_bKoAlertImmune);
	savefile->WriteFloat(m_KoDotVert);
	savefile->WriteFloat(m_KoDotHoriz);
	savefile->WriteFloat(m_KoAlertDotVert);
	savefile->WriteFloat(m_KoAlertDotHoriz);
	savefile->WriteMat3(m_KoRot);

	savefile->WriteBool(m_bCanBeGassed);		// grayman #2468
	savefile->WriteInt( m_koState );			// grayman #2604
	savefile->WriteInt( m_earlyThinkCounter );	// grayman #2654

	savefile->WriteFloat(thresh_1);
	savefile->WriteFloat(thresh_2);
	savefile->WriteFloat(thresh_3);
	savefile->WriteFloat(thresh_4);
	savefile->WriteFloat(thresh_5);

	savefile->WriteFloat(m_gracetime_1);
	savefile->WriteFloat(m_gracetime_2);
	savefile->WriteFloat(m_gracetime_3);
	savefile->WriteFloat(m_gracetime_4);
	savefile->WriteFloat(m_gracefrac_1);
	savefile->WriteFloat(m_gracefrac_2);
	savefile->WriteFloat(m_gracefrac_3);
	savefile->WriteFloat(m_gracefrac_4);
	savefile->WriteFloat(m_gracecount_1);
	savefile->WriteFloat(m_gracecount_2);
	savefile->WriteFloat(m_gracecount_3);
	savefile->WriteFloat(m_gracecount_4);

	savefile->WriteFloat(atime1);
	savefile->WriteFloat(atime2);
	savefile->WriteFloat(atime3);
	savefile->WriteFloat(atime4);
	savefile->WriteFloat(atime_fleedone);

	savefile->WriteFloat(atime1_fuzzyness);
	savefile->WriteFloat(atime2_fuzzyness);
	savefile->WriteFloat(atime3_fuzzyness);
	savefile->WriteFloat(atime4_fuzzyness);
	savefile->WriteFloat(atime_fleedone_fuzzyness);

	savefile->WriteInt(m_timeBetweenHeadTurnChecks);
	savefile->WriteFloat(m_headTurnChanceIdle);
	savefile->WriteFloat(m_headTurnFactorAlerted);
	savefile->WriteFloat(m_headTurnMaxYaw);
	savefile->WriteFloat(m_headTurnMaxPitch);
	savefile->WriteInt(m_headTurnMinDuration);
	savefile->WriteInt(m_headTurnMaxDuration);

	savefile->WriteInt(static_cast<int>(backboneStates.size()));
	for (BackboneStateMap::const_iterator i = backboneStates.begin(); i != backboneStates.end(); ++i)
	{
		savefile->WriteInt(i->first);
		savefile->WriteString(i->second);
	}

	savefile->WriteInt(m_maxInterleaveThinkFrames);
	savefile->WriteFloat(m_minInterleaveThinkDist);
	savefile->WriteFloat(m_maxInterleaveThinkDist);

	savefile->WriteInt(m_lastThinkTime);
	savefile->WriteInt(m_nextThinkFrame);

	savefile->WriteBool(m_bPushOffPlayer);

	savefile->WriteBool(m_bCanBeFlatFooted);
	savefile->WriteBool(m_bFlatFooted);
	savefile->WriteInt(m_FlatFootedTimer);
	savefile->WriteInt(m_FlatFootedTime);
	savefile->WriteInt(m_FlatFootParryNum);
	savefile->WriteInt(m_FlatFootParryMax);
	savefile->WriteInt(m_FlatFootParryTimer);
	savefile->WriteInt(m_FlatFootParryTime);
	savefile->WriteFloat(m_MeleeCounterAttChance);
	savefile->WriteBool(m_bMeleePredictProximity);

	savefile->WriteBool(m_bCanOperateDoors);
	savefile->WriteBool(m_HandlingDoor);
	savefile->WriteBool(m_HandlingElevator);
	savefile->WriteBool(m_RestoreMove); // grayman #2706

	int size = unlockableDoors.size();
	savefile->WriteInt(size);
	for (FrobMoverList::const_iterator i = unlockableDoors.begin(); i != unlockableDoors.end(); ++i)
	{
		savefile->WriteObject(*i);
	}

	savefile->WriteInt(tactileIgnoreEntities.size());

	for (TactileIgnoreList::const_iterator i = tactileIgnoreEntities.begin(); i != tactileIgnoreEntities.end(); ++i)
	{
		savefile->WriteObject(*i);
	}

	mind->Save(savefile);

	senseSubsystem->Save(savefile);
	movementSubsystem->Save(savefile);
	commSubsystem->Save(savefile);
	actionSubsystem->Save(savefile);

	SAVE_TIMER_HANDLE(aiThinkTimer, savefile);
	SAVE_TIMER_HANDLE(aiMindTimer, savefile);
	SAVE_TIMER_HANDLE(aiAnimationTimer, savefile);
	SAVE_TIMER_HANDLE(aiPushWithAFTimer, savefile);
	SAVE_TIMER_HANDLE(aiUpdateEnemyPositionTimer, savefile);
	SAVE_TIMER_HANDLE(aiScriptTimer, savefile);
	SAVE_TIMER_HANDLE(aiAnimMoveTimer, savefile);
	SAVE_TIMER_HANDLE(aiObstacleAvoidanceTimer, savefile);
	SAVE_TIMER_HANDLE(aiPhysicsTimer, savefile);
	SAVE_TIMER_HANDLE(aiGetMovePosTimer, savefile);
	SAVE_TIMER_HANDLE(aiPathToGoalTimer, savefile);
	SAVE_TIMER_HANDLE(aiGetFloorPosTimer, savefile);
	SAVE_TIMER_HANDLE(aiPointReachableAreaNumTimer, savefile);
	SAVE_TIMER_HANDLE(aiCanSeeTimer, savefile);

}

/*
=====================
idAI::Restore
=====================
*/
void idAI::Restore( idRestoreGame *savefile ) {
	bool		restorePhysics;
	int			i;
	int			num;
	idBounds	bounds;

	savefile->ReadInt( travelFlags );
	savefile->ReadInt(lastAreaReevaluationTime);
	savefile->ReadInt(maxAreaReevaluationInterval);
	savefile->ReadInt(doorRetryTime);

	move.Restore( savefile );
	savedMove.Restore( savefile );

	// greebo: restore the movestack
	moveStack.clear();
	savefile->ReadInt(num);
	for (i = 0; i < num; i++)
	{
		moveStack.push_back(idMoveState());
		moveStack.back().Restore(savefile);
	}

	savefile->ReadFloat( kickForce );
	savefile->ReadBool( ignore_obstacles );
	savefile->ReadFloat( blockedRadius );
	savefile->ReadInt( blockedMoveTime );
	savefile->ReadInt( blockedAttackTime );

	savefile->ReadFloat( ideal_yaw );
	savefile->ReadFloat( current_yaw );
	savefile->ReadFloat( turnRate );
	savefile->ReadFloat( turnVel );
	savefile->ReadFloat( anim_turn_yaw );
	savefile->ReadFloat( anim_turn_amount );
	savefile->ReadFloat( anim_turn_angles );
	savefile->ReadVec3(sitting_turn_pivot);

	savefile->ReadFloat(reachedpos_bbox_expansion);
	savefile->ReadFloat(aas_reachability_z_tolerance);

	savefile->ReadStaticObject( physicsObj );

	savefile->ReadFloat( fly_speed );
	savefile->ReadFloat( fly_bob_strength );
	savefile->ReadFloat( fly_bob_vert );
	savefile->ReadFloat( fly_bob_horz );
	savefile->ReadInt( fly_offset );
	savefile->ReadFloat( fly_seek_scale );
	savefile->ReadFloat( fly_roll_scale );
	savefile->ReadFloat( fly_roll_max );
	savefile->ReadFloat( fly_roll );
	savefile->ReadFloat( fly_pitch_scale );
	savefile->ReadFloat( fly_pitch_max );
	savefile->ReadFloat( fly_pitch );

	savefile->ReadBool( allowMove );
	savefile->ReadBool( allowHiddenMovement );
	savefile->ReadBool( disableGravity );

	savefile->ReadBool( lastHitCheckResult );
	savefile->ReadInt( lastHitCheckTime );
	savefile->ReadInt( lastAttackTime );
	savefile->ReadFloat( fire_range );
	savefile->ReadFloat( projectile_height_to_distance_ratio );

	savefile->ReadInt( num );
	missileLaunchOffset.SetGranularity( 1 );
	missileLaunchOffset.SetNum( num );
	for( i = 0; i < num; i++ ) {
		savefile->ReadVec3( missileLaunchOffset[ i ] );
	}

	savefile->ReadInt(num);
	projectileInfo.SetNum(num);

	for (i = 0; i < projectileInfo.Num(); ++i)
	{
		ProjectileInfo& info = projectileInfo[i];

		savefile->ReadString(info.defName);
		// Resolve projectile def pointers from names
		info.def = (info.defName.Length() > 0) ? gameLocal.FindEntityDefDict(info.defName) : NULL;
		
		// leave clipmodel pointer alone, is already initialised to NULL
		savefile->ReadFloat(info.radius);
		savefile->ReadFloat(info.speed);
		savefile->ReadVec3(info.velocity);
		savefile->ReadVec3(info.gravity);
	}

	savefile->ReadInt(curProjectileIndex);

	// Active Projectile
	savefile->ReadString(activeProjectile.info.defName);

	// Resolve projectile def pointers from names
	if (activeProjectile.info.defName.Length() > 0)
	{
		activeProjectile.info.def = gameLocal.FindEntityDefDict(activeProjectile.info.defName);
	}
	else
	{
		activeProjectile.info.def = NULL;
	}

	savefile->ReadFloat(activeProjectile.info.radius);
	savefile->ReadFloat(activeProjectile.info.speed);
	savefile->ReadVec3(activeProjectile.info.velocity);
	savefile->ReadVec3(activeProjectile.info.gravity);
	activeProjectile.projEnt.Restore(savefile);

	savefile->ReadString( attack );

	savefile->ReadInt( i );
	talk_state = static_cast<talkState_t>( i );
	talkTarget.Restore( savefile );

	savefile->ReadInt( num_cinematics );
	savefile->ReadInt( current_cinematic );

	savefile->ReadBool( allowJointMod );
	focusEntity.Restore( savefile );
	savefile->ReadVec3( currentFocusPos );
	savefile->ReadInt( focusTime );
	savefile->ReadInt( alignHeadTime );
	savefile->ReadInt( forceAlignHeadTime );
	savefile->ReadAngles( eyeAng );
	savefile->ReadAngles( lookAng );
	savefile->ReadAngles( destLookAng );
	savefile->ReadAngles( lookMin );
	savefile->ReadAngles( lookMax );

	savefile->ReadInt( num );
	lookJoints.SetGranularity( 1 );
	lookJoints.SetNum( num );
	lookJointAngles.SetGranularity( 1 );
	lookJointAngles.SetNum( num );
	for( i = 0; i < num; i++ ) {
		savefile->ReadJoint( lookJoints[ i ] );
		savefile->ReadAngles( lookJointAngles[ i ] );
	}

	savefile->ReadInt( num );
	lookJointsCombat.SetGranularity( 1 );
	lookJointsCombat.SetNum( num );
	lookJointAnglesCombat.SetGranularity( 1 );
	lookJointAnglesCombat.SetNum( num );
	for( i = 0; i < num; i++ ) {
		savefile->ReadJoint( lookJointsCombat[ i ] );
		savefile->ReadAngles( lookJointAnglesCombat[ i ] );
	}

	savefile->ReadFloat( shrivel_rate );
	savefile->ReadInt( shrivel_start );

	savefile->ReadInt( num );
	particles.SetNum( num );
	for  ( i = 0; i < particles.Num(); i++ ) {
		savefile->ReadParticle( particles[i].particle );
		savefile->ReadInt( particles[i].time );
		savefile->ReadJoint( particles[i].joint );
	}
	savefile->ReadBool( restartParticles );
	savefile->ReadBool( useBoneAxis );

	enemy.Restore( savefile );
	savefile->ReadVec3( lastVisibleEnemyPos );
	savefile->ReadVec3( lastVisibleEnemyEyeOffset );
	savefile->ReadVec3( lastVisibleReachableEnemyPos );
	savefile->ReadVec3( lastReachableEnemyPos );
	savefile->ReadBool( enemyReachable );
	savefile->ReadBool( wakeOnFlashlight );
	savefile->ReadInt(lastUpdateEnemyPositionTime);

	savefile->ReadAngles( eyeMin );
	savefile->ReadAngles( eyeMax );

	savefile->ReadFloat( eyeVerticalOffset );
	savefile->ReadFloat( eyeHorizontalOffset );
	savefile->ReadFloat( eyeFocusRate );
	savefile->ReadFloat( headFocusRate );
	savefile->ReadInt( focusAlignTime );
	savefile->ReadObject(reinterpret_cast<idClass*&>(m_tactileEntity)); // grayman #2345
	savefile->ReadBool(m_canResolveBlock);	// grayman #2345
	savefile->ReadBool(m_leftQueue);		// grayman #2345

	savefile->ReadJoint( flashJointWorld );
	savefile->ReadInt( muzzleFlashEnd );

	savefile->ReadJoint( focusJoint );
	savefile->ReadJoint( orientationJoint );
	savefile->ReadJoint( flyTiltJoint );

	// TDM Alerts:
	savefile->ReadInt( num );
	m_Acuities.SetNum( num );
	for( i = 0; i < num; i++ )
	{
		savefile->ReadFloat( m_Acuities[ i ] );
	}
	savefile->ReadFloat(m_oldVisualAcuity);
	savefile->ReadFloat( m_AudThreshold );
	savefile->ReadVec3( m_SoundDir );
	savefile->ReadVec3( m_LastSight );
	savefile->ReadFloat( m_AlertLevelThisFrame );
	savefile->ReadInt( m_prevAlertIndex );
	savefile->ReadFloat( m_maxAlertLevel );
	savefile->ReadInt( m_maxAlertIndex);
	savefile->ReadFloat(m_lastAlertLevel);
	savefile->ReadBool( m_bIgnoreAlerts );

	m_AlertedByActor.Restore( savefile );
	for (i = 0; i < ai::EAlertTypeCount; i++)
	{
		savefile->ReadInt(alertTypeWeight[i]);
	}
	m_TactAlertEnt.Restore( savefile );
	m_AlertGraceActor.Restore( savefile );
	savefile->ReadInt( m_AlertGraceStart );
	savefile->ReadInt( m_AlertGraceTime );
	savefile->ReadFloat( m_AlertGraceThresh );
	savefile->ReadInt( m_AlertGraceCount );
	savefile->ReadInt( m_AlertGraceCountLimit );

	savefile->ReadInt(num);
	m_Messages.Clear();
	for (int i = 0; i < num; i++)
	{
		ai::CommMessagePtr message(new ai::CommMessage);
		message->Restore(savefile);
		m_Messages.Append(message);
	}

	savefile->ReadBool( restorePhysics );

	// Set the AAS if the character has the correct gravity vector
	idVec3 gravity = spawnArgs.GetVector( "gravityDir", "0 0 -1" );
	gravity *= g_gravity.GetFloat();
	if ( gravity == gameLocal.GetGravity() ) {
		SetAAS();
	}

	savefile->ReadFloat(m_VisDistMax);

	savefile->ReadInt(m_HidingSpotSearchHandle);
	m_hidingSpots.Restore(savefile);

	savefile->ReadInt(m_AirCheckTimer);
	savefile->ReadBool(m_bCanDrown);
	savefile->ReadInt(m_HeadBodyID);
	savefile->ReadJoint(m_HeadJointID);

	savefile->ReadBool( m_bHeadCMSwapped );
	// Ishtvan: if the head CM was swapped at save, swap it again
	// (I tried to do this more correctly, but the AF itself isn't saving its new clipmodel)
	if( m_bHeadCMSwapped )
	{
		m_OrigHeadCM = NULL;
		m_bHeadCMSwapped = false;
		SwapHeadAFCM( true );
	}

	savefile->ReadInt(m_AirTics);
	savefile->ReadInt(m_AirTicksMax);
	savefile->ReadInt(m_AirCheckInterval);

	savefile->ReadVec3(m_MouthOffset);
	savefile->ReadBool(m_bCanBeKnockedOut);
	savefile->ReadVec3(m_HeadCenterOffset);
	savefile->ReadMat3(m_FOVRot);
	savefile->ReadString(m_KoZone);
	savefile->ReadInt(m_KoAlertState);
	savefile->ReadInt(m_KoAlertImmuneState);
	savefile->ReadBool(m_bKoAlertImmune);
	savefile->ReadFloat(m_KoDotVert);
	savefile->ReadFloat(m_KoDotHoriz);
	savefile->ReadFloat(m_KoAlertDotVert);
	savefile->ReadFloat(m_KoAlertDotHoriz);
	savefile->ReadMat3(m_KoRot);

	savefile->ReadBool(m_bCanBeGassed); // grayman #2468
	savefile->ReadInt( i ); // grayman #2604
	m_koState = static_cast<koState_t>( i );
	savefile->ReadInt(m_earlyThinkCounter); // grayman #2654

	savefile->ReadFloat(thresh_1);
	savefile->ReadFloat(thresh_2);
	savefile->ReadFloat(thresh_3);
	savefile->ReadFloat(thresh_4);
	savefile->ReadFloat(thresh_5);

	savefile->ReadFloat(m_gracetime_1);
	savefile->ReadFloat(m_gracetime_2);
	savefile->ReadFloat(m_gracetime_3);
	savefile->ReadFloat(m_gracetime_4);
	savefile->ReadFloat(m_gracefrac_1);
	savefile->ReadFloat(m_gracefrac_2);
	savefile->ReadFloat(m_gracefrac_3);
	savefile->ReadFloat(m_gracefrac_4);
	savefile->ReadInt(m_gracecount_1);
	savefile->ReadInt(m_gracecount_2);
	savefile->ReadInt(m_gracecount_3);
	savefile->ReadInt(m_gracecount_4);

	savefile->ReadFloat(atime1);
	savefile->ReadFloat(atime2);
	savefile->ReadFloat(atime3);
	savefile->ReadFloat(atime4);
	savefile->ReadFloat(atime_fleedone);

	savefile->ReadFloat(atime1_fuzzyness);
	savefile->ReadFloat(atime2_fuzzyness);
	savefile->ReadFloat(atime3_fuzzyness);
	savefile->ReadFloat(atime4_fuzzyness);
	savefile->ReadFloat(atime_fleedone_fuzzyness);

	savefile->ReadInt(m_timeBetweenHeadTurnChecks);
	savefile->ReadFloat(m_headTurnChanceIdle);
	savefile->ReadFloat(m_headTurnFactorAlerted);
	savefile->ReadFloat(m_headTurnMaxYaw);
	savefile->ReadFloat(m_headTurnMaxPitch);
	savefile->ReadInt(m_headTurnMinDuration);
	savefile->ReadInt(m_headTurnMaxDuration);

	backboneStates.clear();
	savefile->ReadInt(num);

	for (int i = 0; i < num; ++i)
	{
		int state = 0;
		savefile->ReadInt(state);
		
		std::pair<BackboneStateMap::iterator, bool> result = backboneStates.insert(
			BackboneStateMap::value_type(static_cast<ai::EAlertState>(state), idStr())
		);

		savefile->ReadString(result.first->second);
	}
	
	savefile->ReadInt(m_maxInterleaveThinkFrames);
	savefile->ReadFloat(m_minInterleaveThinkDist);
	savefile->ReadFloat(m_maxInterleaveThinkDist);

	savefile->ReadInt(m_lastThinkTime);
	savefile->ReadInt(m_nextThinkFrame);

	savefile->ReadBool(m_bPushOffPlayer);

	savefile->ReadBool(m_bCanBeFlatFooted);
	savefile->ReadBool(m_bFlatFooted);
	savefile->ReadInt(m_FlatFootedTimer);
	savefile->ReadInt(m_FlatFootedTime);
	savefile->ReadInt(m_FlatFootParryNum);
	savefile->ReadInt(m_FlatFootParryMax);
	savefile->ReadInt(m_FlatFootParryTimer);
	savefile->ReadInt(m_FlatFootParryTime);
	savefile->ReadFloat(m_MeleeCounterAttChance);
	savefile->ReadBool(m_bMeleePredictProximity);

	savefile->ReadBool(m_bCanOperateDoors);
	savefile->ReadBool(m_HandlingDoor);
	savefile->ReadBool(m_HandlingElevator);
	savefile->ReadBool(m_RestoreMove); // grayman #2706

	int size;
	savefile->ReadInt(size);
	unlockableDoors.clear();
	for (int i = 0; i < size; i++)
	{
		CBinaryFrobMover* mover;
		savefile->ReadObject( reinterpret_cast<idClass *&>( mover ) );
		unlockableDoors.insert(mover);
	}

	savefile->ReadInt(size);
	tactileIgnoreEntities.clear();
	for (int i = 0; i < size; i++)
	{
		idEntity* tactEnt;
		savefile->ReadObject(reinterpret_cast<idClass *&>(tactEnt));
		tactileIgnoreEntities.insert(tactEnt);
	}

	mind = ai::MindPtr(new ai::Mind(this));
	mind->Restore(savefile);

	// Allocate and install the subsystems
	movementSubsystem = ai::MovementSubsystemPtr(new ai::MovementSubsystem(ai::SubsysMovement, this));
	senseSubsystem = ai::SubsystemPtr(new ai::Subsystem(ai::SubsysSenses, this));
	commSubsystem = ai::CommunicationSubsystemPtr(new ai::CommunicationSubsystem(ai::SubsysCommunication, this));
	actionSubsystem = ai::SubsystemPtr(new ai::Subsystem(ai::SubsysAction, this));

	senseSubsystem->Restore(savefile);
	movementSubsystem->Restore(savefile);
	commSubsystem->Restore(savefile);
	actionSubsystem->Restore(savefile);

	SetCombatModel();
	LinkCombat();

	InitMuzzleFlash();

	// Link the script variables back to the scriptobject
	LinkScriptVariables();

	if ( restorePhysics ) {
		RestorePhysics( &physicsObj );
	}

	RESTORE_TIMER_HANDLE(aiThinkTimer, savefile);
	RESTORE_TIMER_HANDLE(aiMindTimer, savefile);
	RESTORE_TIMER_HANDLE(aiAnimationTimer, savefile);
	RESTORE_TIMER_HANDLE(aiPushWithAFTimer, savefile);
	RESTORE_TIMER_HANDLE(aiUpdateEnemyPositionTimer, savefile);
	RESTORE_TIMER_HANDLE(aiScriptTimer, savefile);
	RESTORE_TIMER_HANDLE(aiAnimMoveTimer, savefile);
	RESTORE_TIMER_HANDLE(aiObstacleAvoidanceTimer, savefile);
	RESTORE_TIMER_HANDLE(aiPhysicsTimer, savefile);
	RESTORE_TIMER_HANDLE(aiGetMovePosTimer, savefile);
	RESTORE_TIMER_HANDLE(aiPathToGoalTimer, savefile);
	RESTORE_TIMER_HANDLE(aiGetFloorPosTimer, savefile);
	RESTORE_TIMER_HANDLE(aiPointReachableAreaNumTimer, savefile);
	RESTORE_TIMER_HANDLE(aiCanSeeTimer, savefile);
}

ai::Subsystem* idAI::GetSubsystem(ai::SubsystemId id)
{
	switch (id)
	{
	case ai::SubsysSenses:
		return senseSubsystem.get();
	case ai::SubsysMovement:
		return movementSubsystem.get();
	case ai::SubsysCommunication:
		return commSubsystem.get();
	case ai::SubsysAction:
		return actionSubsystem.get();
	default:
		gameLocal.Error("Request for unknown subsystem %d", static_cast<int>(id));
		return NULL;
	};
}

/*
=====================
idAI::Spawn
=====================
*/
void idAI::Spawn( void )
{
	const char			*jointname;
	const idKeyValue	*kv;
	idStr				jointName;
	idAngles			jointScale;
	jointHandle_t		joint;
	idVec3				local_dir;
	bool				talks;

	aiNode.AddToEnd(gameLocal.spawnedAI);

	// Allocate a new default mind
	mind = ai::MindPtr(new ai::Mind(this));

	// Allocate and install the subsystems
	movementSubsystem = ai::MovementSubsystemPtr(new ai::MovementSubsystem(ai::SubsysMovement, this));
	senseSubsystem = ai::SubsystemPtr(new ai::Subsystem(ai::SubsysSenses, this));
	commSubsystem = ai::CommunicationSubsystemPtr(new ai::CommunicationSubsystem(ai::SubsysCommunication, this));
	actionSubsystem = ai::SubsystemPtr(new ai::Subsystem(ai::SubsysAction, this));

	if ( !g_monsters.GetBool() ) {
		PostEventMS( &EV_Remove, 0 );
		return;
	}

	spawnArgs.GetInt(	"team",					"1",		team );
	spawnArgs.GetInt(	"rank",					"0",		rank );
	spawnArgs.GetInt(	"fly_offset",			"0",		fly_offset );
	spawnArgs.GetFloat( "fly_speed",			"100",		fly_speed );
	spawnArgs.GetFloat( "fly_bob_strength",		"50",		fly_bob_strength );
	spawnArgs.GetFloat( "fly_bob_vert",			"2",		fly_bob_vert );
	spawnArgs.GetFloat( "fly_bob_horz",			"2.7",		fly_bob_horz );
	spawnArgs.GetFloat( "fly_seek_scale",		"4",		fly_seek_scale );
	spawnArgs.GetFloat( "fly_roll_scale",		"90",		fly_roll_scale );
	spawnArgs.GetFloat( "fly_roll_max",			"60",		fly_roll_max );
	spawnArgs.GetFloat( "fly_pitch_scale",		"45",		fly_pitch_scale );
	spawnArgs.GetFloat( "fly_pitch_max",		"30",		fly_pitch_max );

	maxAreaReevaluationInterval = spawnArgs.GetInt( "max_area_reevaluation_interval", "2000");
	doorRetryTime = SEC2MS(spawnArgs.GetInt( "door_retry_time", "120"));

	spawnArgs.GetFloat( "fire_range",			"0",		fire_range );
	spawnArgs.GetFloat( "projectile_height_to_distance_ratio",	"1", projectile_height_to_distance_ratio );

	spawnArgs.GetFloat( "turn_rate",			"360",		turnRate );

	spawnArgs.GetVector("sitting_turn_pivot",	"-20 0 0",	sitting_turn_pivot);

	reachedpos_bbox_expansion = spawnArgs.GetFloat("reachedpos_bbox_expansion", "0");
	aas_reachability_z_tolerance = spawnArgs.GetFloat("aas_reachability_z_tolerance", "75");

	spawnArgs.GetBool( "talks",					"0",		talks );

	//// DarkMod: Alert level parameters
	// The default values of these spawnargs are normally set in tdm_ai_base.def, so the default values
	// here are somewhat superfluous. It's better than having defaults of 0 here though.
	spawnArgs.GetFloat( "alert_thresh1",		"1.5",		thresh_1 );
	spawnArgs.GetFloat( "alert_thresh2",		"6",		thresh_2 );
	spawnArgs.GetFloat( "alert_thresh3",		"8",		thresh_3 );
	spawnArgs.GetFloat( "alert_thresh4",		"18",		thresh_4 );
	spawnArgs.GetFloat( "alert_thresh5",		"23",		thresh_5 );
	// Grace period info for each alert level
	spawnArgs.GetFloat( "alert_gracetime1",		"2",		m_gracetime_1 );
	spawnArgs.GetFloat( "alert_gracetime2",		"2",		m_gracetime_2 );
	spawnArgs.GetFloat( "alert_gracetime3",		"3.5",		m_gracetime_3 );
	spawnArgs.GetFloat( "alert_gracetime4",		"2",		m_gracetime_4 );
	spawnArgs.GetFloat( "alert_gracefrac1",		"1.2",		m_gracefrac_1 );
	spawnArgs.GetFloat( "alert_gracefrac2",		"1.2",		m_gracefrac_2 );
	spawnArgs.GetFloat( "alert_gracefrac3",		"1.2",		m_gracefrac_3 );
	spawnArgs.GetFloat( "alert_gracefrac4",		"1.0",		m_gracefrac_4 );
	spawnArgs.GetInt  ( "alert_gracecount1",	"5",		m_gracecount_1 );
	spawnArgs.GetInt  ( "alert_gracecount2",	"5",		m_gracecount_2 );
	spawnArgs.GetInt  ( "alert_gracecount3",	"5",		m_gracecount_3 );
	spawnArgs.GetInt  ( "alert_gracecount4",	"4",		m_gracecount_4 );
	// De-alert times for each alert level
	spawnArgs.GetFloat( "alert_time1",			"4",		atime1 );
	spawnArgs.GetFloat( "alert_time2",			"6",		atime2 );
	spawnArgs.GetFloat( "alert_time3",			"30",		atime3 );
	spawnArgs.GetFloat( "alert_time4",			"120",		atime4 );
	spawnArgs.GetFloat( "alert_time1_fuzzyness",			"4",		atime1_fuzzyness );
	spawnArgs.GetFloat( "alert_time2_fuzzyness",			"6",		atime2_fuzzyness );
	spawnArgs.GetFloat( "alert_time3_fuzzyness",			"30",		atime3_fuzzyness );
	spawnArgs.GetFloat( "alert_time4_fuzzyness",			"120",		atime4_fuzzyness );

	spawnArgs.GetFloat( "alert_time_fleedone",				"60",		atime_fleedone );
	spawnArgs.GetFloat( "alert_time_fleedone_fuzzyness",	"10",		atime_fleedone_fuzzyness );

	// State setup
	backboneStates.clear();

	backboneStates[ai::ERelaxed]			= spawnArgs.GetString("state_name_0", STATE_IDLE);
	backboneStates[ai::EObservant]			= spawnArgs.GetString("state_name_1", STATE_OBSERVANT);
	backboneStates[ai::ESuspicious]			= spawnArgs.GetString("state_name_2", STATE_SUSPICIOUS);
	backboneStates[ai::EInvestigating]		= spawnArgs.GetString("state_name_3", STATE_SEARCHING);
	backboneStates[ai::EAgitatedSearching]	= spawnArgs.GetString("state_name_4", STATE_AGITATED_SEARCHING);
	backboneStates[ai::ECombat]				= spawnArgs.GetString("state_name_5", STATE_COMBAT);
	
	spawnArgs.GetInt( "max_interleave_think_frames",		"12",		m_maxInterleaveThinkFrames );
	spawnArgs.GetFloat( "min_interleave_think_dist",		"1000",		m_minInterleaveThinkDist);
	spawnArgs.GetFloat( "max_interleave_think_dist",		"3000",		m_maxInterleaveThinkDist);

	spawnArgs.GetBool( "ignore_alerts",						"0",		m_bIgnoreAlerts );

	if (spawnArgs.GetBool("canOperateElevators", "0"))
	{
		travelFlags |= TFL_ELEVATOR;
	}

	float headTurnSec;
	spawnArgs.GetFloat( "headturn_delay_min",				"3",		headTurnSec);
	m_timeBetweenHeadTurnChecks = SEC2MS(headTurnSec);

	spawnArgs.GetFloat( "headturn_chance_idle",				"0.1",		m_headTurnChanceIdle);
	spawnArgs.GetFloat( "headturn_factor_alerted",			"2",		m_headTurnFactorAlerted);
	spawnArgs.GetFloat( "headturn_yaw",						"60",		m_headTurnMaxYaw);
	spawnArgs.GetFloat( "headturn_pitch",					"40",		m_headTurnMaxPitch);

	spawnArgs.GetFloat( "headturn_duration_min",			"1",		headTurnSec);
	m_headTurnMinDuration = SEC2MS(headTurnSec);

	spawnArgs.GetFloat( "headturn_duration_max",			"3",		headTurnSec);
	m_headTurnMaxDuration = SEC2MS(headTurnSec);

	alertTypeWeight[ai::EAlertTypeNone] = 0;
	alertTypeWeight[ai::EAlertTypeSuspicious] = 5;
	alertTypeWeight[ai::EAlertTypeEnemy] = 50;
	alertTypeWeight[ai::EAlertTypeWeapon] = 35;
	alertTypeWeight[ai::EAlertTypeDeadPerson] = 41;
	alertTypeWeight[ai::EAlertTypeUnconsciousPerson] = 40;
	alertTypeWeight[ai::EAlertTypeBlood] = 30;
	alertTypeWeight[ai::EAlertTypeLightSource] = 10;
	alertTypeWeight[ai::EAlertTypeMissingItem] = 25;
	alertTypeWeight[ai::EAlertTypeBrokenItem] = 26;
	alertTypeWeight[ai::EAlertTypeDoor] = 20;
	alertTypeWeight[ai::EAlertTypeDamage] = 45;

	// DarkMod: Get the movement type audible volumes from the spawnargs
	spawnArgs.GetFloat( "stepvol_walk",			"0",		m_stepvol_walk );
	spawnArgs.GetFloat( "stepvol_run",			"0",		m_stepvol_run );
	spawnArgs.GetFloat( "stepvol_creep",		"0",		m_stepvol_creep );

	spawnArgs.GetFloat( "stepvol_crouch_walk",			"0",		m_stepvol_crouch_walk );
	spawnArgs.GetFloat( "stepvol_run",			"0",		m_stepvol_crouch_run );
	spawnArgs.GetFloat( "stepvol_creep",		"0",		m_stepvol_crouch_creep );

	if ( spawnArgs.GetString( "npc_name", NULL ) != NULL ) {
		if ( talks ) {
			talk_state = TALK_OK;
		} else {
			talk_state = TALK_BUSY;
		}
	} else {
		talk_state = TALK_NEVER;
	}

	spawnArgs.GetBool( "animate_z",				"0",		disableGravity );
	spawnArgs.GetFloat( "kick_force",			"60",		kickForce ); // grayman #2568 - use default Doom 3 value
	spawnArgs.GetBool( "ignore_obstacles",		"0",		ignore_obstacles );
	spawnArgs.GetFloat( "blockedRadius",		"-1",		blockedRadius );
	spawnArgs.GetInt( "blockedMoveTime",		"750",		blockedMoveTime );
	spawnArgs.GetInt( "blockedAttackTime",		"750",		blockedAttackTime );

	// DarkMod: Set the AI acuities from the spawnargs.

	m_Acuities.Clear();
	for( int ind=0; ind < g_Global.m_AcuityNames.Num(); ind++)
	{
		float tempFloat = spawnArgs.GetFloat( va("acuity_%s", g_Global.m_AcuityNames[ind].c_str()), "100" );
		// angua: divide by 100 to transform percent into fractions
		tempFloat *= 0.01f;
		m_Acuities.Append( tempFloat );
		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Acuities Array: index %d, name %s, value %f\r", ind, g_Global.m_AcuityNames[ind].c_str(), m_Acuities[ind]);
	}
	m_oldVisualAcuity = GetAcuity("vis");	// Tels fix #2408

	spawnArgs.GetFloat("alert_aud_thresh", va("%f",gameLocal.m_sndProp->m_SndGlobals.DefaultThreshold), m_AudThreshold );

	spawnArgs.GetInt(	"num_cinematics",		"0",		num_cinematics );
	current_cinematic = 0;

	LinkScriptVariables();

	/**
	* Initialize Darkmod AI vars
	**/
	AI_ALERTED = false;
	AI_CROUCH = false;
	AI_RUN = false;
	AI_CREEP = false;

	AI_LAY_DOWN_LEFT = true;

	AI_HEARDSOUND = false;
	AI_VISALERT = false;
	AI_TACTALERT = false;

	fl.takedamage		= !spawnArgs.GetBool( "noDamage" );
	enemy				= NULL;
	allowMove			= true;
	allowHiddenMovement = false;

	animator.RemoveOriginOffset( true );

	// create combat collision hull for exact collision detection
	SetCombatModel();

	lookMin	= spawnArgs.GetAngles( "look_min", "-80 -75 0" );
	lookMax	= spawnArgs.GetAngles( "look_max", "80 75 0" );

	lookJoints.SetGranularity( 1 );
	lookJointAngles.SetGranularity( 1 );
	kv = spawnArgs.MatchPrefix( "look_joint", NULL );
	while( kv ) {
		jointName = kv->GetKey();
		jointName.StripLeadingOnce( "look_joint " );
		joint = animator.GetJointHandle( jointName );
		if ( joint == INVALID_JOINT ) {
			gameLocal.Warning( "Unknown look_joint '%s' on entity %s", jointName.c_str(), name.c_str() );
		} else {
			jointScale = spawnArgs.GetAngles( kv->GetKey(), "0 0 0" );
			jointScale.roll = 0.0f;

			// if no scale on any component, then don't bother adding it.  this may be done to
			// zero out rotation from an inherited entitydef.
			if ( jointScale != ang_zero ) {
				lookJoints.Append( joint );
				lookJointAngles.Append( jointScale );
			}
		}
		kv = spawnArgs.MatchPrefix( "look_joint", kv );
	}

	lookJointsCombat.SetGranularity( 1 );
	lookJointAnglesCombat.SetGranularity( 1 );
	kv = spawnArgs.MatchPrefix( "combat_look_joint", NULL );
	while( kv ) {
		jointName = kv->GetKey();
		jointName.StripLeadingOnce( "combat_look_joint " );
		joint = animator.GetJointHandle( jointName );
		if ( joint == INVALID_JOINT ) {
			gameLocal.Warning( "Unknown combat_look_joint '%s' on entity %s", jointName.c_str(), name.c_str() );
		} else {
			jointScale = spawnArgs.GetAngles( kv->GetKey(), "0 0 0" );
			jointScale.roll = 0.0f;

			// if no scale on any component, then don't bother adding it.  this may be done to
			// zero out rotation from an inherited entitydef.
			if ( jointScale != ang_zero ) {
				lookJointsCombat.Append( joint );
				lookJointAnglesCombat.Append( jointScale );
			}
		}
		kv = spawnArgs.MatchPrefix( "combat_look_joint", kv );
	}

	// calculate joint positions on attack frames so we can do proper "can hit" tests
	CalculateAttackOffsets();

	eyeMin				= spawnArgs.GetAngles( "eye_turn_min", "-10 -30 0" );
	eyeMax				= spawnArgs.GetAngles( "eye_turn_max", "10 30 0" );
	eyeVerticalOffset	= spawnArgs.GetFloat( "eye_verticle_offset", "5" );
	eyeHorizontalOffset = spawnArgs.GetFloat( "eye_horizontal_offset", "-8" );
	eyeFocusRate		= spawnArgs.GetFloat( "eye_focus_rate", "0.5" );
	headFocusRate		= spawnArgs.GetFloat( "head_focus_rate", "0.1" );
	focusAlignTime		= SEC2MS( spawnArgs.GetFloat( "focus_align_time", "1" ) );

	// DarkMod: State of mind, allow the FM author to set initial values
	AI_AlertLevel			= spawnArgs.GetFloat( "alert_initial", "0" );

	flashJointWorld = animator.GetJointHandle( "flash" );

	if ( head.GetEntity() ) 
	{
		idAnimator *headAnimator = head.GetEntity()->GetAnimator();

		jointname = spawnArgs.GetString( "bone_focus" );
		if ( *jointname ) 
		{
			focusJoint = headAnimator->GetJointHandle( jointname );
			if ( focusJoint == INVALID_JOINT ) 
			{
				gameLocal.Warning( "Joint '%s' not found on head on '%s'", jointname, name.c_str() );
			}
		}
	} else 
	{
		jointname = spawnArgs.GetString( "bone_focus" );
		if ( *jointname ) 
		{
			focusJoint = animator.GetJointHandle( jointname );
			if ( focusJoint == INVALID_JOINT ) 
			{
				gameLocal.Warning( "Joint '%s' not found on '%s'", jointname, name.c_str() );
			}
		}
	}

	jointname = spawnArgs.GetString( "bone_orientation" );
	if ( *jointname ) {
		orientationJoint = animator.GetJointHandle( jointname );
		if ( orientationJoint == INVALID_JOINT ) {
			gameLocal.Warning( "Joint '%s' not found on '%s'", jointname, name.c_str() );
		}
	}

	jointname = spawnArgs.GetString( "bone_flytilt" );
	if ( *jointname ) {
		flyTiltJoint = animator.GetJointHandle( jointname );
		if ( flyTiltJoint == INVALID_JOINT ) {
			gameLocal.Warning( "Joint '%s' not found on '%s'", jointname, name.c_str() );
		}
	}

	InitMuzzleFlash();

	physicsObj.SetSelf( this );
	physicsObj.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ), 1.0f );

	physicsObj.SetMass( spawnArgs.GetFloat( "mass", "100" ) );
	kickForce = 2*physicsObj.GetMass(); // grayman #2568 - equation arrived at empirically

	physicsObj.SetStepUpIncrease(spawnArgs.GetFloat("step_up_increase", "0"));

	if ( spawnArgs.GetBool( "big_monster" ) ) {
		physicsObj.SetContents( 0 );
		physicsObj.SetClipMask( MASK_MONSTERSOLID & ~CONTENTS_BODY );
	} else {
		if ( use_combat_bbox ) {
			physicsObj.SetContents( CONTENTS_BODY|CONTENTS_SOLID );
		} else {
			physicsObj.SetContents( CONTENTS_BODY );
		}
		physicsObj.SetClipMask( MASK_MONSTERSOLID );
	}

// SR CONTENTS_RESPONSE fix:
	if( m_StimResponseColl->HasResponse() )
		physicsObj.SetContents( physicsObj.GetContents() | CONTENTS_RESPONSE );

	// move up to make sure the monster is at least an epsilon above the floor
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() + idVec3( 0, 0, CM_CLIP_EPSILON ) );

	if ( num_cinematics ) {
		physicsObj.SetGravity( vec3_origin );
	} else {
		idVec3 gravity = spawnArgs.GetVector( "gravityDir", "0 0 -1" );
		gravity *= g_gravity.GetFloat();
		physicsObj.SetGravity( gravity );
	}

	SetPhysics( &physicsObj );

	physicsObj.GetGravityAxis().ProjectVector( viewAxis[ 0 ], local_dir );
	current_yaw		= local_dir.ToYaw();
	ideal_yaw		= idMath::AngleNormalize180( current_yaw );

	move.blockTime = 0;

	SetAAS();

	InitProjectileInfo();

	particles.Clear();
	restartParticles = true;
	useBoneAxis = spawnArgs.GetBool( "useBoneAxis" );
	SpawnParticles( "smokeParticleSystem" );

	if ( num_cinematics || spawnArgs.GetBool( "hide" ) || spawnArgs.GetBool( "teleport" ) || spawnArgs.GetBool( "trigger_anim" ) ) {
		fl.takedamage = false;
		physicsObj.SetContents( 0 );
		physicsObj.GetClipModel()->Unlink();
		Hide();
	} else {
		// play a looping ambient sound if we have one
		StartSound( "snd_ambient", SND_CHANNEL_AMBIENT, 0, false, NULL );
	}

	if ( health <= 0 ) {
		gameLocal.Warning( "entity '%s' doesn't have health set", name.c_str() );
		health = 1;
	}

	// Dark Mod: set up drowning
	m_MouthOffset = spawnArgs.GetVector("mouth_offset");
	// set up drowning timer (add a random bit to make it asynchronous w/ respect to other AI)
	m_bCanDrown = spawnArgs.GetBool( "can_drown", "1" );
	m_AirCheckTimer = gameLocal.time + gameLocal.random.RandomInt( 8000 );
	m_AirTicksMax = spawnArgs.GetInt( "max_air_tics", "5" );
	m_AirTics = m_AirTicksMax;
	m_AirCheckInterval = static_cast<int>(1000.0f * spawnArgs.GetFloat( "air_check_interval", "4.0" ));
	// end drowning setup

	m_bPushOffPlayer = spawnArgs.GetBool("push_off_player", "1");

	m_bCanBeFlatFooted	= spawnArgs.GetBool("can_be_flatfooted", "1");
	m_FlatFootedTime	= spawnArgs.GetInt("flatfooted_time");
	m_FlatFootParryMax = spawnArgs.GetInt("flatfoot_parry_num");
	m_FlatFootParryTime = spawnArgs.GetInt("flatfoot_parry_time");
	// Convert percent chance to fractional
	m_MeleeCounterAttChance = spawnArgs.GetInt("melee_chance_to_counter") / 100.0f;
	m_bMeleePredictProximity = spawnArgs.GetBool("melee_predicts_proximity");

	if (spawnArgs.GetBool("melee_attacks_enabled_at_spawn_time", "0"))
	{
		SetAttackFlag(COMBAT_MELEE, true);
	}

	if (spawnArgs.GetBool("ranged_attacks_enabled_at_spawn_time", "0"))
	{
		SetAttackFlag(COMBAT_RANGED, true);
	}

	m_bCanOperateDoors = spawnArgs.GetBool("canOperateDoors", "0");
	m_HandlingDoor = false;
	m_RestoreMove = false; // grayman #2706

	m_HandlingElevator = false;

	// =============== Set up KOing and FOV ==============
	const char *HeadJointName = spawnArgs.GetString("head_jointname", "Head");

	m_HeadJointID = animator.GetJointHandle(HeadJointName);
	if( m_HeadJointID == INVALID_JOINT )
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Invalid head joint for joint %s on AI %s \r", HeadJointName, name.c_str());
		m_HeadBodyID = 0;
	}
	else
	{
		m_HeadBodyID = BodyForJoint(m_HeadJointID);
	}

	if( head.GetEntity() )
	{
		CopyHeadKOInfo();
	}

	ParseKnockoutInfo();

	// ================== End KO setup ====================

	// Sneak attack setup
	const char *tempc1;
	tempc1 = spawnArgs.GetString("sneak_attack_alert_state");
	m_SneakAttackThresh = spawnArgs.GetFloat( va("alert_thresh%s", tempc1), va("%f",idMath::INFINITY) );
	m_SneakAttackMult = spawnArgs.GetFloat( "sneak_attack_mult", "1.0" );

	BecomeActive( TH_THINK );

	// init the move variables
	StopMove( MOVE_STATUS_DONE );

	// Schedule a post-spawn event to parse the rest of the spawnargs
	PostEventMS( &EV_PostSpawn, 1 );

	m_lastThinkTime = gameLocal.time;

	CREATE_TIMER(aiThinkTimer, name, "Think");
	CREATE_TIMER(aiMindTimer, name, "Mind");
	CREATE_TIMER(aiAnimationTimer, name, "Animation");
	CREATE_TIMER(aiPushWithAFTimer, name, "PushWithAF");
	CREATE_TIMER(aiUpdateEnemyPositionTimer, name, "UpdateEnemyPosition");
	CREATE_TIMER(aiScriptTimer, name, "UpdateScript");
	CREATE_TIMER(aiAnimMoveTimer, name, "AnimMove");
	CREATE_TIMER(aiObstacleAvoidanceTimer, name, "ObstacleAvoidance");
	CREATE_TIMER(aiPhysicsTimer, name, "RunPhysics");
	CREATE_TIMER(aiGetMovePosTimer, name, "GetMovePos");
	CREATE_TIMER(aiPathToGoalTimer, name, "PathToGoal");
	CREATE_TIMER(aiGetFloorPosTimer, name, "GetFloorPos");
	CREATE_TIMER(aiPointReachableAreaNumTimer, name, "PointReachableAreaNum");
	CREATE_TIMER(aiCanSeeTimer, name, "CanSee");

	m_pathRank = rank; // grayman #2345 - rank for path-finding

	m_bCanBeGassed = !(spawnArgs.GetBool( "gas_immune", "0" )); // grayman #2468
}

void idAI::InitProjectileInfo()
{
	activeProjectile.projEnt = NULL;

	// greebo: Pre-cache the properties of each projectile def.
	for (const idKeyValue* kv = spawnArgs.MatchPrefix("def_projectile"); kv != NULL; kv = spawnArgs.MatchPrefix("def_projectile", kv))
	{
		const idStr& projectileName = kv->GetValue();
		
		if (projectileName.Length() == 0) continue;

		ProjectileInfo& info = projectileInfo.Alloc();

		// Pass to specialised routine
		InitProjectileInfoFromDict(info, projectileName);
	}

	// Roll a random projectile for starters
	curProjectileIndex = gameLocal.random.RandomInt(projectileInfo.Num());
}

void idAI::InitProjectileInfoFromDict(idAI::ProjectileInfo& info, const char* entityDef) const
{
	const idDict* dict = gameLocal.FindEntityDefDict(entityDef);

	if (dict == NULL)
	{
		gameLocal.Error("Cannot load projectile info for entityDef %s", entityDef);
	}
	
	InitProjectileInfoFromDict(info, dict);
}

void idAI::InitProjectileInfoFromDict(idAI::ProjectileInfo& info, const idDict* dict) const
{
	assert(dict != NULL);

	info.defName = dict->GetString("classname");
	info.def = dict;
	
	// Create a projectile of this specific type to retrieve its properties
	idProjectile* proj = SpawnProjectile(info.def);

	info.radius	= proj->GetPhysics()->GetClipModel()->GetBounds().GetRadius();
	info.velocity = idProjectile::GetVelocity(info.def);
	info.gravity = idProjectile::GetGravity(info.def);
	info.speed = info.velocity.Length();

	// dispose after this short use
	delete proj;
}

/*
===================
idAI::InitMuzzleFlash
===================
*/
void idAI::InitMuzzleFlash( void ) {
	const char			*shader;
	idVec3				flashColor;

	spawnArgs.GetString( "mtr_flashShader", "muzzleflash", &shader );
	spawnArgs.GetVector( "flashColor", "0 0 0", flashColor );
	float flashRadius = spawnArgs.GetFloat( "flashRadius" );
	flashTime = SEC2MS( spawnArgs.GetFloat( "flashTime", "0.25" ) );

	memset( &worldMuzzleFlash, 0, sizeof ( worldMuzzleFlash ) );

	worldMuzzleFlash.pointLight = true;
	worldMuzzleFlash.shader = declManager->FindMaterial( shader, false );
	worldMuzzleFlash.shaderParms[ SHADERPARM_RED ] = flashColor[0];
	worldMuzzleFlash.shaderParms[ SHADERPARM_GREEN ] = flashColor[1];
	worldMuzzleFlash.shaderParms[ SHADERPARM_BLUE ] = flashColor[2];
	worldMuzzleFlash.shaderParms[ SHADERPARM_ALPHA ] = 1.0f;
	worldMuzzleFlash.shaderParms[ SHADERPARM_TIMESCALE ] = 1.0f;
	worldMuzzleFlash.lightRadius[0] = flashRadius;
	worldMuzzleFlash.lightRadius[1]	= flashRadius;
	worldMuzzleFlash.lightRadius[2]	= flashRadius;

	worldMuzzleFlashHandle = -1;
}

/*
===================
idAI::List_f
===================
*/
void idAI::List_f( const idCmdArgs &args ) {
	int		e;
	idAI	*check;
	int		count;
	const char *statename;

	count = 0;

	gameLocal.Printf( "%-4s  %-20s %s\n", " Num", "EntityDef", "Name" );
	gameLocal.Printf( "------------------------------------------------\n" );
	for( e = 0; e < MAX_GENTITIES; e++ ) {
		check = static_cast<idAI *>(gameLocal.entities[ e ]);
		if ( !check || !check->IsType( idAI::Type ) ) {
			continue;
		}

		if ( check->state ) {
			statename = check->state->Name();
		} else {
			statename = "NULL state";
		}

		gameLocal.Printf( "%4i: %-20s %-20s %s  move: %d\n", e, check->GetEntityDefName(), check->name.c_str(), statename, check->allowMove );
		count++;
	}

	gameLocal.Printf( "...%d monsters\n", count );
}

/*
================
idAI::DormantBegin

called when entity becomes dormant
================
*/
void idAI::DormantBegin( void ) {
	// since dormant happens on a timer, we wont get to update particles to
	// hidden through the think loop, but we need to hide them though.
	if ( particles.Num() ) {
		for ( int i = 0; i < particles.Num(); i++ ) {
			particles[i].time = 0;
		}
	}

	if ( enemyNode.InList() ) {
		// remove ourselves from the enemy's enemylist
		enemyNode.Remove();
	}
	idActor::DormantBegin();
}

/*
================
idAI::DormantEnd

called when entity wakes from being dormant
================
*/
void idAI::DormantEnd( void ) {
	if ( enemy.GetEntity() && !enemyNode.InList() ) {
		// let our enemy know we're back on the trail
		enemyNode.AddToEnd( enemy.GetEntity()->enemyList );
	}

	if ( particles.Num() ) {
		for ( int i = 0; i < particles.Num(); i++ ) {
			particles[i].time = gameLocal.time;
		}
	}

	idActor::DormantEnd();
	m_lastThinkTime = gameLocal.time;
}

/*
=====================
idAI::Think
=====================
*/
void idAI::Think( void ) 
{
	START_SCOPED_TIMING(aiThinkTimer, scopedThinkTimer);
	if (cv_ai_opt_nothink.GetBool()) 
	{
		return; // Thinking is disabled.
	}

	// Interleaved thinking
	if (!ThinkingIsAllowed())
	{
		return;
	}

	SetNextThinkFrame();

	// if we are completely closed off from the player, don't do anything at all
	// angua: only go dormant while in idle
	// grayman #2536 - move alert check up in front
	if (AI_AlertIndex < 1)
	{
		bool outsidePVS = CheckDormant();
		if (outsidePVS && cv_ai_opt_disable.GetBool())
		{
			return;
		}
	}

	// grayman #2536 - if dormant and alert index > 0, wake up

	if ((AI_AlertIndex > 0) && fl.isDormant)
	{
		dormantStart = 0;
		fl.hasAwakened = true;
		fl.isDormant = false;
		DormantEnd();
	}
			
	// save old origin and velocity for crashlanding
	idVec3 oldOrigin = physicsObj.GetOrigin();
	idVec3 oldVelocity = physicsObj.GetLinearVelocity();

	if (thinkFlags & TH_THINK)
	{
		// clear out the enemy when he dies
		idActor* enemyEnt = enemy.GetEntity();
		if (enemyEnt != NULL)
		{
			if (enemyEnt->health <= 0)
			{
				EnemyDead();
			}
		}

		// Calculate the new view axis based on the turning settings
		current_yaw += deltaViewAngles.yaw;
		ideal_yaw = idMath::AngleNormalize180(ideal_yaw + deltaViewAngles.yaw);
		deltaViewAngles.Zero();
		viewAxis = idAngles(0, current_yaw, 0).ToMat3();

		// TDM: Fake lipsync
		if (m_lipSyncActive && !cv_ai_opt_nolipsync.GetBool() && GetSoundEmitter())
		{
			if (gameLocal.time < m_lipSyncEndTimer && head.GetEntity() != NULL)
			{
				// greebo: Get the number of frames from the head animator
				int numFrames = head.GetEntity()->GetAnimator()->NumFrames(m_lipSyncAnim);

				int frame = static_cast<int>(numFrames * idMath::Sqrt16(GetSoundEmitter()->CurrentAmplitude()));
				frame = idMath::ClampInt(0, numFrames, frame);
				headAnim.SetFrame(m_lipSyncAnim, frame);
			}
			else
			{
				// We're done; stop the animation
				StopLipSync();
			}
		}

		// Check for tactile alert due to AI movement
		CheckTactile();

		if (health > 0) // grayman #1488 - only do this if you're still alive
		{
			UpdateAir(); // Check air ticks (is interleaved and not checked each frame)
		}

		if (num_cinematics > 0)
		{
			// Active cinematics
			if ( !IsHidden() && torsoAnim.AnimDone( 0 ) ) {
				PlayCinematic();
			}
			RunPhysics();
		}
		else if (!allowHiddenMovement && IsHidden())
		{
			// hidden monsters
			UpdateScript();
		}
		else
		{
			// clear the ik before we do anything else so the skeleton doesn't get updated twice
			walkIK.ClearJointMods();

			// Update moves, depending on move type 
			switch (move.moveType)
			{
			case MOVETYPE_DEAD :
				// dead monsters
				UpdateScript();
				DeadMove();
				break;

			case MOVETYPE_FLY :
				// flying monsters
				UpdateEnemyPosition();
				UpdateScript();
				FlyMove();
				CheckBlink();
				break;

			case MOVETYPE_STATIC :
				// static monsters
				UpdateEnemyPosition();
				UpdateScript();
				StaticMove();
				CheckBlink();
				break;

			case MOVETYPE_ANIM :
				// animation based movement
				UpdateEnemyPosition();
				UpdateScript();
				if (!cv_ai_opt_noanims.GetBool())
				{
					AnimMove();
				}
				CheckBlink();
				break;

			case MOVETYPE_SLIDE :
				// velocity based movement
				UpdateEnemyPosition();
				UpdateScript();
				SlideMove();
				CheckBlink();
				break;

			case MOVETYPE_SIT :
				// static monsters
				UpdateEnemyPosition();
				UpdateScript();
				// moving not allowed, turning around sitting pivot
				SittingMove();
				CheckBlink();
				break;

			case MOVETYPE_SIT_DOWN :
				// static monsters
				UpdateEnemyPosition();
				UpdateScript();
				// moving and turning not allowed
				NoTurnMove();
				CheckBlink();
				break;

			case MOVETYPE_SLEEP :
				// static monsters
				UpdateEnemyPosition();
				UpdateScript();
				// moving and turning not allowed
				NoTurnMove();
				break;

			case MOVETYPE_LAY_DOWN :
				// static monsters
				UpdateEnemyPosition();
				UpdateScript();
				// moving and turning not allowed
				LayDownMove();
				break;

			case MOVETYPE_GET_UP :
				// static monsters
				UpdateEnemyPosition();
				UpdateScript();
				// moving not allowed
				SittingMove();
				CheckBlink();
				break;

			case MOVETYPE_GET_UP_FROM_LYING :
				// static monsters
				UpdateEnemyPosition();
				UpdateScript();
				// moving and turning not allowed
				LayDownMove();
				break;


			default:
				break;
			}
		}

		if (!cv_ai_opt_nomind.GetBool())
		{
			// greebo: We always rely on having a mind
			assert(mind);
			START_SCOPED_TIMING(aiMindTimer, scopedMindTimer);

			// Let the mind do the thinking (after the move updates)
			mind->Think();
		}

		// Clear DarkMod per frame vars now that the mind had time to think
		AI_ALERTED = false;
		m_AlertLevelThisFrame = 0;
		m_AlertedByActor = NULL;
		m_tactileEntity = NULL; // grayman #2345

		// clear pain flag so that we recieve any damage between now and the next time we run the script
		AI_PAIN = false;
		AI_SPECIAL_DAMAGE = 0;
		AI_PUSHED = false;
	}
	else if (thinkFlags & TH_PHYSICS)
	{
		// Thinking not allowed, but physics are still enabled
		RunPhysics();
	}

	if (m_bAFPushMoveables && !movementSubsystem->IsWaitingNonSolid()) // grayman #2345 - if you're waiting for someone to go by, you're non-solid, so you can't push anything
	{
		START_SCOPED_TIMING(aiPushWithAFTimer, scopedPushWithAFTimer)
		PushWithAF();
	}

	if (fl.hidden && allowHiddenMovement)
	{
		// UpdateAnimation won't call frame commands when hidden, so call them here when we allow hidden movement
		animator.ServiceAnims( gameLocal.previousTime, gameLocal.time );
	}

	UpdateMuzzleFlash();
	if (!cv_ai_opt_noanims.GetBool())
	{
		START_SCOPED_TIMING(aiAnimationTimer, scopedAnimationTimer)
		UpdateAnimation();
	}
	UpdateParticles();
	if (!cv_ai_opt_nopresent.GetBool())
	{
		Present();
	}
	UpdateDamageEffects();
	LinkCombat();

	if (health > 0)
	{
		idActor::CrashLand( physicsObj, oldOrigin, oldVelocity );
	}

	m_lastThinkTime = gameLocal.time;

	// Check the CVARs and print debug info, if appropriate
	ShowDebugInfo();
}

/*
=====================
idAI::ThinkingIsAllowed
=====================
*/
bool idAI::ThinkingIsAllowed()
{
	int frameNum = gameLocal.framenum;
	if (frameNum < m_nextThinkFrame)
	{
		// Ragdolls think every frame to avoid physics weirdness.
		if (health <= 0)
		{
			return true;
		}

		// angua: AI think every frame while sitting/laying down and getting up
		// otherwise, the AI might end up in a different sleeping position
		if (move.moveType == MOVETYPE_SIT_DOWN
			|| move.moveType == MOVETYPE_LAY_DOWN
			|| move.moveType == MOVETYPE_GET_UP
			|| move.moveType == MOVETYPE_GET_UP_FROM_LYING)
		{
			return true;
		}

		// skips PVS check, AI will also do interleaved thinking when in player view.
		bool skipPVScheck = cv_ai_opt_interleavethinkskippvscheck.GetBool();
		if (skipPVScheck)
		{
			return false;
		}

		// PVS check: let the AI think every frame as long as the player sees them.
		bool inPVS = gameLocal.InPlayerPVS(this);
		if (!inPVS)
		{
			return false;
		}
	}
	return true;
}



/*
=====================
idAI::SetNextThinkFrame
=====================
*/
void idAI::SetNextThinkFrame()
{
	int frameNum = gameLocal.framenum;
	int thinkFrame = GetThinkInterleave();
	int thinkDelta = 1;

	if (thinkFrame > 1)
	{
		// Let them think for the first few frames to initialize state and tasks
		if (m_earlyThinkCounter <= 0) // grayman #2654 - keep a separate counter
		{
			// grayman #2414 - think more often if
			//
			//   * working on a door-handling task
			//   * nearing a goal position
			//   * handling an elevator
			//
			// and, for #2345
			//
			//   * when resolving a block

			bool thinkMore = false;
			thinkDelta = thinkFrame;
			ai::Memory& memory = GetMemory();
			CFrobDoor *door = memory.doorRelated.currentDoor.GetEntity();

			if (door)
			{
				idVec3 origin = GetPhysics()->GetOrigin();
				idVec3 doorOrigin = door->GetPhysics()->GetOrigin();
				float distance = (doorOrigin - origin).LengthFast();

				if (distance <= TEMP_THINK_DISTANCE)
				{
					thinkMore = true; // avoid obstruction & damage
				}
			}
			else if (m_HandlingElevator)
			{
				thinkMore = true; // avoid death
			}
			else if ((move.moveCommand == MOVE_TO_POSITION) && (move.moveStatus == MOVE_STATUS_MOVING))
			{
				idVec3 origin = GetPhysics()->GetOrigin();
				float distance = (move.moveDest - origin).LengthFast();

				if (distance <= TEMP_THINK_FACTOR*thinkFrame)
				{
					thinkMore = true; // avoid confusion and becoming stuck
				}
			}
			else if (!movementSubsystem->IsWaiting() && !movementSubsystem->IsNotBlocked())
			{
				thinkMore = true; // avoid chaos
			}

			if (thinkMore)
			{
				thinkDelta = (thinkFrame < TEMP_THINK_INTERLEAVE ? thinkFrame : TEMP_THINK_INTERLEAVE);
			}
		}
		else
		{
			m_earlyThinkCounter--; // grayman #2654
		}
	}

	m_nextThinkFrame = frameNum + thinkDelta;
}

/*
=====================
idAI::GetThinkInterleave
=====================
*/
int idAI::GetThinkInterleave() const // grayman 2414 - add 'const'
{
	int maxFrames = m_maxInterleaveThinkFrames;
	if (cv_ai_opt_interleavethinkframes.GetInteger() > 0)
	{
		maxFrames = cv_ai_opt_interleavethinkframes.GetInteger();
	}

	if (maxFrames == 0)
	{
		return 0;
	}

	float minDist = m_minInterleaveThinkDist;
	float maxDist = m_maxInterleaveThinkDist;
	if (cv_ai_opt_interleavethinkmindist.GetFloat() > 0)
	{
		minDist = cv_ai_opt_interleavethinkmindist.GetFloat();
	}
	if (cv_ai_opt_interleavethinkmaxdist.GetFloat() > 0)
	{
		maxDist = cv_ai_opt_interleavethinkmaxdist.GetFloat();
	}

	if (maxDist < minDist)
	{
		gameLocal.Warning("Minimum distance for interleaved thinking is larger than maximum distance, switching optimization off.");
		return 0;
	}

	float playerDist = (physicsObj.GetOrigin() - gameLocal.GetLocalPlayer()->GetPhysics()->GetOrigin()).LengthFast();
	if (playerDist < minDist)
	{
		return 0;
	}
	else if (playerDist > maxDist)
	{
		return maxFrames;
	}
	else
	{
		float fraction = (playerDist - minDist) / (maxDist - minDist);
		int thinkFrames = 1 + static_cast<int>(fraction * maxFrames);
		return thinkFrames;
	}
}

/***********************************************************************

	AI script state management

***********************************************************************/

/*
=====================
idAI::LinkScriptVariables
=====================
*/
void idAI::LinkScriptVariables( void )
{
	// Call the base class first
	idActor::LinkScriptVariables();

	AI_TALK.LinkTo(				scriptObject, "AI_TALK" );
	AI_DAMAGE.LinkTo(			scriptObject, "AI_DAMAGE" );
	AI_PAIN.LinkTo(				scriptObject, "AI_PAIN" );
	AI_SPECIAL_DAMAGE.LinkTo(	scriptObject, "AI_SPECIAL_DAMAGE" );
	AI_KNOCKEDOUT.LinkTo(		scriptObject, "AI_KNOCKEDOUT" );
	AI_ENEMY_VISIBLE.LinkTo(	scriptObject, "AI_ENEMY_VISIBLE" );
	AI_ENEMY_IN_FOV.LinkTo(		scriptObject, "AI_ENEMY_IN_FOV" );
	AI_MOVE_DONE.LinkTo(		scriptObject, "AI_MOVE_DONE" );
	AI_ONGROUND.LinkTo(			scriptObject, "AI_ONGROUND" );
	AI_ACTIVATED.LinkTo(		scriptObject, "AI_ACTIVATED" );
	AI_FORWARD.LinkTo(			scriptObject, "AI_FORWARD" );
	AI_JUMP.LinkTo(				scriptObject, "AI_JUMP" );
	AI_BLOCKED.LinkTo(			scriptObject, "AI_BLOCKED" );
	AI_DEST_UNREACHABLE.LinkTo( scriptObject, "AI_DEST_UNREACHABLE" );
	AI_HIT_ENEMY.LinkTo(		scriptObject, "AI_HIT_ENEMY" );
	AI_OBSTACLE_IN_PATH.LinkTo(	scriptObject, "AI_OBSTACLE_IN_PATH" );
	AI_PUSHED.LinkTo(			scriptObject, "AI_PUSHED" );

	//this is only set in a given frame
	AI_ALERTED.LinkTo(			scriptObject, "AI_ALERTED" );

	AI_AlertLevel.LinkTo(			scriptObject, "AI_AlertLevel" );
	AI_AlertIndex.LinkTo(			scriptObject, "AI_AlertIndex" );

	//these are set until unset by the script
	AI_HEARDSOUND.LinkTo(		scriptObject, "AI_HEARDSOUND");
	AI_VISALERT.LinkTo(			scriptObject, "AI_VISALERT");
	AI_TACTALERT.LinkTo(		scriptObject, "AI_TACTALERT");

	AI_CROUCH.LinkTo(			scriptObject, "AI_CROUCH");
	AI_RUN.LinkTo(				scriptObject, "AI_RUN");
	AI_CREEP.LinkTo(			scriptObject, "AI_CREEP");

	AI_LAY_DOWN_LEFT.LinkTo(	scriptObject, "AI_LAY_DOWN_LEFT");
	AI_LAY_DOWN_FACE_DIR.LinkTo(scriptObject, "AI_LAY_DOWN_FACE_DIR");

	AI_SIT_DOWN_ANGLE.LinkTo(scriptObject, "AI_SIT_DOWN_ANGLE");
	AI_SIT_UP_ANGLE.LinkTo(scriptObject, "AI_SIT_UP_ANGLE");


}

/*
=====================
idAI::UpdateAIScript
=====================
*/
void idAI::UpdateScript()
{
	START_SCOPED_TIMING(aiScriptTimer, scopedScriptTimer);

	// greebo: This is overriding idActor::UpdateScript(), where all the state change stuff 
	// is executed, which is not needed for TDM AI

	if ( ai_debugScript.GetInteger() == entityNumber ) {
		scriptThread->EnableDebugInfo();
	} else {
		scriptThread->DisableDebugInfo();
	}

	// don't call script until it's done waiting
	if (!scriptThread->IsWaiting()) {
		scriptThread->Execute();
	}
    
	// clear the hit enemy flag so we catch the next time we hit someone
	AI_HIT_ENEMY = false;

	if ( allowHiddenMovement || !IsHidden() ) {
		// update the animstate if we're not hidden
		if (!cv_ai_opt_noanims.GetBool())
		{
			UpdateAnimState();
		}
	}
}

/***********************************************************************

	navigation

***********************************************************************/

/*
============
idAI::KickObstacles
============
*/
void idAI::KickObstacles( const idVec3 &dir, float force, idEntity *alwaysKick ) {
	int i, numListedClipModels;
	idBounds clipBounds;
	idEntity *obEnt;
	idClipModel *clipModel;
	idClipModel *clipModelList[ MAX_GENTITIES ];
	int clipmask;
	idVec3 org;
	idVec3 forceVec;
	idVec3 delta;
	idVec2 perpendicular;

	org = physicsObj.GetOrigin();

	// find all possible obstacles
	clipBounds = physicsObj.GetAbsBounds();
	clipBounds.TranslateSelf( dir * (clipBounds[1].x - clipBounds[0].x + clipBounds[1].y - clipBounds[0].y)/2); // grayman #2667
//	clipBounds.TranslateSelf( dir * 32.0f ); // grayman #2667 - old way assumed a humanoid
	clipBounds.ExpandSelf( 8.0f );
	clipBounds.AddPoint( org );
	clipmask = physicsObj.GetClipMask();
	numListedClipModels = gameLocal.clip.ClipModelsTouchingBounds( clipBounds, clipmask, clipModelList, MAX_GENTITIES );
	for ( i = 0; i < numListedClipModels; i++ ) {
		clipModel = clipModelList[i];
		obEnt = clipModel->GetEntity();
		if ( obEnt == alwaysKick ) {
			// we'll kick this one outside the loop
			continue;
		}

		if ( !clipModel->IsTraceModel() ) {
			continue;
		}

		if ( obEnt->IsType( idMoveable::Type ) && obEnt->GetPhysics()->IsPushable() )
		{
			delta = obEnt->GetPhysics()->GetOrigin() - org;
			delta.NormalizeFast();
			perpendicular.x = -delta.y;
			perpendicular.y = delta.x;
			delta.z += 0.5f;
			delta.ToVec2() += perpendicular * gameLocal.random.CRandomFloat() * 0.5f;
			forceVec = delta * force; // grayman #2568 - remove obEnt->mass from the equation
//			forceVec = delta * force * obEnt->GetPhysics()->GetMass(); // grayman #2568 - old way
			obEnt->ApplyImpulse( this, 0, obEnt->GetPhysics()->GetOrigin(), forceVec );
			if (obEnt->m_SetInMotionByActor.GetEntity() == NULL)
			{
				obEnt->m_SetInMotionByActor = this;
				obEnt->m_MovedByActor = this;
			}
		}
	}

	if ( alwaysKick )
	{
		delta = alwaysKick->GetPhysics()->GetOrigin() - org;
		delta.NormalizeFast();
		perpendicular.x = -delta.y;
		perpendicular.y = delta.x;
		delta.z += 0.5f;
		delta.ToVec2() += perpendicular * gameLocal.random.CRandomFloat() * 0.5f;
		forceVec = delta * force; // grayman #2568 - remove obEnt->mass from the equation
//		forceVec = delta * force * alwaysKick->GetPhysics()->GetMass(); // grayman #2568 - old way
		alwaysKick->ApplyImpulse( this, 0, alwaysKick->GetPhysics()->GetOrigin(), forceVec );
		if (alwaysKick->m_SetInMotionByActor.GetEntity() == NULL)
		{
			alwaysKick->m_SetInMotionByActor = this;
			alwaysKick->m_MovedByActor = this;
		}
	}
}

bool idAI::ReEvaluateArea(int areaNum)
{
	// Remember the time
	lastAreaReevaluationTime = gameLocal.time;

	// Let's see if we have a valid door info structure in our memory
	ai::DoorInfoPtr doorInfo = GetMemory().GetDoorInfo(areaNum);

	if (doorInfo != NULL)
	{
		//if (doorInfo->lastTimeTriedToOpen + doorRetryTime < gameLocal.time)
		{
			// Re-try the door after some time
			gameLocal.m_AreaManager.RemoveForbiddenArea(areaNum, this);
			return true;
		}
	}

	return false;
}

/*
============
ValidForBounds
============
*/
bool ValidForBounds( const idAASSettings *settings, const idBounds &bounds ) {
	int i;

	for ( i = 0; i < 3; i++ ) {
		if ( bounds[0][i] < settings->boundingBoxes[0][0][i] ) {
			return false;
		}
		if ( bounds[1][i] > settings->boundingBoxes[0][1][i] ) {
			return false;
		}
	}
	return true;
}

/*
=====================
idAI::SetAAS
=====================
*/
void idAI::SetAAS( void ) {
	idStr use_aas;

	spawnArgs.GetString( "use_aas", NULL, use_aas );
	aas = gameLocal.GetAAS( use_aas );
	if ( aas ) {
		const idAASSettings *settings = aas->GetSettings();
		if ( settings ) {
			if ( !ValidForBounds( settings, physicsObj.GetBounds() ) ) {
				gameLocal.Error( "%s cannot use use_aas %s\n", name.c_str(), use_aas.c_str() );
			}
			float height = settings->maxStepHeight;
			physicsObj.SetMaxStepHeight( height );
			return;
		} else {
			aas = NULL;
		}
	}
	gameLocal.Printf( "WARNING: %s has no AAS file\n", name.c_str() );
}

/*
=====================
idAI::DrawRoute
=====================
*/
void idAI::DrawRoute( void ) const {
	if ( aas && move.toAreaNum && move.moveCommand != MOVE_NONE && move.moveCommand != MOVE_WANDER && move.moveCommand != MOVE_FACE_ENEMY && move.moveCommand != MOVE_FACE_ENTITY && move.moveCommand != MOVE_TO_POSITION_DIRECT && move.moveCommand != MOVE_VECTOR ) 
	{
		if ( move.moveType == MOVETYPE_FLY ) {
			aas->ShowFlyPath( physicsObj.GetOrigin(), move.toAreaNum, move.moveDest );
		} else {
			aas->ShowWalkPath( physicsObj.GetOrigin(), move.toAreaNum, move.moveDest );
		}
	}
}

/*
=====================
idAI::ReachedPos
=====================
*/
bool idAI::ReachedPos( const idVec3 &pos, const moveCommand_t moveCommand) const {
	if ( move.moveType == MOVETYPE_SLIDE ) {
		idBounds bnds( idVec3( -4, -4.0f, -8.0f ), idVec3( 4.0f, 4.0f, 64.0f ) );
		bnds.TranslateSelf( physicsObj.GetOrigin() );
		if ( bnds.ContainsPoint( pos ) ) {
			return true;
		}
	} else {
		if ( ( moveCommand == MOVE_TO_ENEMY ) || ( moveCommand == MOVE_TO_ENTITY ) )
		{
			if ( physicsObj.GetAbsBounds().IntersectsBounds( idBounds( pos ).Expand( 8.0f ) ) ) {
				return true;
			}
		}
		else
		{
			// Use the AI bounding box check to see if we've reached the position
			return ReachedPosAABBCheck(pos);
		}
	}
	return false;
}

bool idAI::ReachedPosAABBCheck(const idVec3& pos) const
{
	// angua: use AI bounds for checking
	idBounds bnds(physicsObj.GetBounds());
	if (move.accuracy >= 0)
	{
		// if accuracy is set, replace x and y size of the bounds
		bnds[0][0] = -move.accuracy;
		bnds[0][1] = -move.accuracy;
		bnds[1][0] = move.accuracy;
		bnds[1][1] = move.accuracy;
	}

	bnds.TranslateSelf(physicsObj.GetOrigin());

	bnds.ExpandSelf(reachedpos_bbox_expansion);

	// angua: expand the bounds a bit downwards, so that they can actually reach target positions 
	// that are a reported as reachable by PathToGoal.
	bnds[0].z -= aas_reachability_z_tolerance;
	bnds[1].z += aas_reachability_z_tolerance;

	return (bnds.ContainsPoint(pos));
}

/*
=====================
idAI::PointReachableAreaNum
=====================
*/
int idAI::PointReachableAreaNum( const idVec3 &pos, const float boundsScale, const idVec3& offset) const
{
	START_SCOPED_TIMING(aiPointReachableAreaNumTimer, scopedPointReachableAreaNumTimer);

	if (aas == NULL) {
		return 0; // no AAS, no area number
	}

	idVec3 size = aas->GetSettings()->boundingBoxes[0][1] * boundsScale;

	// Construct the modifed bounds, based on the parameters (max.z = 32)
	idBounds bounds(-size, idVec3(size.x, size.y, 32.0f));
	idVec3 newPos = pos + offset;

/*	idBounds temp(bounds);
	temp.TranslateSelf(newPos);
	gameRenderWorld->DebugBox(colorGreen, idBox(temp),gameLocal.msec);*/

	int areaNum = 0;

	if (move.moveType == MOVETYPE_FLY)
	{
		// Flying monsters
		areaNum = aas->PointReachableAreaNum( newPos, bounds, AREA_REACHABLE_WALK | AREA_REACHABLE_FLY );
	} 
	else
	{
		// Non-flying monsters
		areaNum = aas->PointReachableAreaNum( newPos, bounds, AREA_REACHABLE_WALK );
	}

	return areaNum;
}

/*
=====================
idAI::PathToGoal
=====================
*/
bool idAI::PathToGoal( aasPath_t &path, int areaNum, const idVec3 &origin, int goalAreaNum, const idVec3 &goalOrigin, idActor* actor ) const {
	if ( !aas ) {
		return false;
	}

	START_SCOPED_TIMING(aiPathToGoalTimer, scopedPathToGoalTimer);

	idVec3 org = origin;
	aas->PushPointIntoAreaNum( areaNum, org );
	if ( !areaNum ) {
		return false;
	}

	idVec3 goal = goalOrigin;
	aas->PushPointIntoAreaNum( goalAreaNum, goal );
	if ( !goalAreaNum ) {
		return false;
	}

	// Sanity check the returned area. If the position isn't within the AI's height + aas_reachability_z_tolerance
	// reach, then report it as unreachable.
	const idVec3& grav = physicsObj.GetGravityNormal();

	float height = fabs((goal - aas->AreaCenter(goalAreaNum)) * grav);

	idBounds bounds = GetPhysics()->GetBounds();

	if (height > bounds[1][2] + aas_reachability_z_tolerance) {
		goalAreaNum = 0;
		return false;
	}
	
	bool returnval;
	gameLocal.m_AreaManager.DisableForbiddenAreas(this);
	if ( move.moveType == MOVETYPE_FLY ) {
		returnval = aas->FlyPathToGoal( path, areaNum, org, goalAreaNum, goal, travelFlags );
	} else {
		returnval = aas->WalkPathToGoal( path, areaNum, org, goalAreaNum, goal, travelFlags, actor );
	}
	gameLocal.m_AreaManager.EnableForbiddenAreas(this);

	// return returnval;
	// grayman #2708 - if returnval is true, return, but if false, check whether the AAS area is above
	// the AI's origin, as it might be if the AI is stuck in an AAS area next to a monster-clipped
	// table top, where it will be at the plane of the table top. If that's the case, return 'true'
	// to let the AI escape this type of AAS area. Normal pathfinding will never let him out.

	if (returnval)
	{
		return true;
	}

	idVec3 myOrigin = GetPhysics()->GetOrigin();
	idBounds areaBounds = aas->GetAreaBounds(areaNum);
	path.moveGoal = goal;
	return (myOrigin.z < areaBounds[0].z);
}


/*
=====================
idAI::TravelDistance

Returns the approximate travel distance from one position to the goal, or if no AAS, the straight line distance.

This is feakin' slow, so it's not good to do it too many times per frame.  It also is slower the further you
are from the goal, so try to break the goals up into shorter distances.
=====================
*/
float idAI::TravelDistance( const idVec3 &start, const idVec3 &end )  {
	int			fromArea;
	int			toArea;
	float		dist;
	idVec2		delta;
	aasPath_t	path;

	if ( !aas ) {
		// no aas, so just take the straight line distance
		delta = end.ToVec2() - start.ToVec2();
		dist = delta.LengthFast();

		if ( ai_debugMove.GetBool() ) {
			gameRenderWorld->DebugLine( colorBlue, start, end, gameLocal.msec, false );
			gameRenderWorld->DrawText( va( "%d", ( int )dist ), ( start + end ) * 0.5f, 0.1f, colorWhite, gameLocal.GetLocalPlayer()->viewAngles.ToMat3() );
		}

		return dist;
	}

	fromArea = PointReachableAreaNum( start );
	toArea = PointReachableAreaNum( end );

	if ( !fromArea || !toArea ) {
		// can't seem to get there
		return -1;
	}

	if ( fromArea == toArea ) {
		// same area, so just take the straight line distance
		delta = end.ToVec2() - start.ToVec2();
		dist = delta.LengthFast();

		if ( ai_debugMove.GetBool() ) {
			gameRenderWorld->DebugLine( colorBlue, start, end, gameLocal.msec, false );
			gameRenderWorld->DrawText( va( "%d", ( int )dist ), ( start + end ) * 0.5f, 0.1f, colorWhite, gameLocal.GetLocalPlayer()->viewAngles.ToMat3() );
		}

		return dist;
	}

	idReachability *reach;
	int travelTime;
	if ( !aas->RouteToGoalArea( fromArea, start, toArea, travelFlags, travelTime, &reach, NULL, this ) ) {
		return -1;
	}

	if ( ai_debugMove.GetBool() ) {
		if ( move.moveType == MOVETYPE_FLY ) {
			aas->ShowFlyPath( start, toArea, end );
		} else {
			aas->ShowWalkPath( start, toArea, end );
		}
	}

	return travelTime;
}

/*
=====================
idAI::StopMove
=====================
*/
void idAI::StopMove( moveStatus_t status ) {
	AI_MOVE_DONE		= true;
	AI_FORWARD			= false;
	m_pathRank			= 1000; // grayman #2345
	move.moveCommand	= MOVE_NONE;
	move.moveStatus		= status;
	move.toAreaNum		= 0;
	move.goalEntity		= NULL;
	move.moveDest		= physicsObj.GetOrigin();
	AI_DEST_UNREACHABLE	= (status == MOVE_STATUS_DEST_UNREACHABLE);
	AI_OBSTACLE_IN_PATH = false;
	AI_BLOCKED			= false;
	move.startTime		= gameLocal.time;
	move.duration		= 0;
	move.range			= 0.0f;
	move.speed			= 0.0f;
	move.anim			= 0;
	move.moveDir.Zero();
	move.lastMoveOrigin.Zero();
	move.lastMoveTime	= gameLocal.time;
	move.accuracy		= -1;
}

const idVec3& idAI::GetMoveDest() const
{
	return move.moveDest;
}

idEntity* idAI::GetTactileEntity(void) // grayman #2345
{
	return m_tactileEntity;
}

/*
=====================
idAI::FaceEnemy

Continually face the enemy's last known position.  MoveDone is always true in this case.
=====================
*/
bool idAI::FaceEnemy( void ) {
	idActor *enemyEnt = enemy.GetEntity();
	if ( !enemyEnt ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
		return false;
	}

	TurnToward( lastVisibleEnemyPos );
	move.goalEntity		= enemyEnt;
	move.moveDest		= physicsObj.GetOrigin();
	move.moveCommand	= MOVE_FACE_ENEMY;
	move.moveStatus		= MOVE_STATUS_WAITING;
	move.startTime		= gameLocal.time;
	move.speed			= 0.0f;
	move.accuracy		= -1;
	AI_MOVE_DONE		= true;
	AI_FORWARD			= false;
	m_pathRank			= 1000; // grayman #2345
	AI_DEST_UNREACHABLE = false;

	return true;
}

/*
=====================
idAI::FaceEntity

Continually face the entity position.  MoveDone is always true in this case.
=====================
*/
bool idAI::FaceEntity( idEntity *ent ) {
	if ( !ent ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
		return false;
	}

	idVec3 entityOrg = ent->GetPhysics()->GetOrigin();
	TurnToward( entityOrg );
	move.goalEntity		= ent;
	move.moveDest		= physicsObj.GetOrigin();
	move.moveCommand	= MOVE_FACE_ENTITY;
	move.moveStatus		= MOVE_STATUS_WAITING;
	move.startTime		= gameLocal.time;
	move.speed			= 0.0f;
	AI_MOVE_DONE		= true;
	AI_FORWARD			= false;
	m_pathRank			= 1000; // grayman #2345
	AI_DEST_UNREACHABLE = false;

	return true;
}

/*
=====================
idAI::DirectMoveToPosition
=====================
*/
bool idAI::DirectMoveToPosition( const idVec3 &pos ) {
	if ( ReachedPos( pos, move.moveCommand ) ) {
		StopMove( MOVE_STATUS_DONE );
		return true;
	}

	move.moveDest		= pos;
	move.goalEntity		= NULL;
	move.moveCommand	= MOVE_TO_POSITION_DIRECT;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= fly_speed;
	move.accuracy		= -1;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= true;
	m_pathRank			= rank; // grayman #2345

	if ( move.moveType == MOVETYPE_FLY ) {
		idVec3 dir = pos - physicsObj.GetOrigin();
		dir.Normalize();
		dir *= fly_speed;
		physicsObj.SetLinearVelocity( dir );
	}

	return true;
}

/*
=====================
idAI::MoveToEnemyHeight
=====================
*/
bool idAI::MoveToEnemyHeight( void ) {
	idActor	*enemyEnt = enemy.GetEntity();

	if ( !enemyEnt || ( move.moveType != MOVETYPE_FLY ) ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
		return false;
	}

	move.moveDest.z		= lastVisibleEnemyPos.z + enemyEnt->EyeOffset().z + fly_offset;
	move.goalEntity		= enemyEnt;
	move.moveCommand	= MOVE_TO_ENEMYHEIGHT;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= 0.0f;
	move.accuracy		= -1;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= false;
	m_pathRank			= 1000; // grayman #2345

	return true;
}

/*
=====================
idAI::MoveToEnemy
=====================
*/
bool idAI::MoveToEnemy( void ) {
	int			areaNum;
	aasPath_t	path;
	idActor		*enemyEnt = enemy.GetEntity();

	if ( !enemyEnt ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
		return false;
	}

	if ( ReachedPos( lastVisibleReachableEnemyPos, MOVE_TO_ENEMY ) ) {
		if ( !ReachedPos( lastVisibleEnemyPos, MOVE_TO_ENEMY ) || !AI_ENEMY_VISIBLE ) {
			StopMove( MOVE_STATUS_DEST_UNREACHABLE );
			AI_DEST_UNREACHABLE = true;
			return false;
		}
		StopMove( MOVE_STATUS_DONE );
		return true;
	}

	idVec3 pos = lastVisibleReachableEnemyPos;

	move.toAreaNum = 0;
	if ( aas ) {
		move.toAreaNum = PointReachableAreaNum( pos );
		aas->PushPointIntoAreaNum( move.toAreaNum, pos );

		areaNum	= PointReachableAreaNum( physicsObj.GetOrigin() );
		if ( !PathToGoal( path, areaNum, physicsObj.GetOrigin(), move.toAreaNum, pos, this ) ) {
			AI_DEST_UNREACHABLE = true;
			return false;
		}
	}

	if ( !move.toAreaNum ) {
		// if only trying to update the enemy position
		if ( move.moveCommand == MOVE_TO_ENEMY ) {
			if ( !aas ) {
				// keep the move destination up to date for wandering
				move.moveDest = pos;
			}
			return false;
		}

		if ( !NewWanderDir( pos ) ) {
			StopMove( MOVE_STATUS_DEST_UNREACHABLE );
			AI_DEST_UNREACHABLE = true;
			return false;
		}
	}

	if ( move.moveCommand != MOVE_TO_ENEMY ) {
		move.moveCommand	= MOVE_TO_ENEMY;
		move.startTime		= gameLocal.time;
	}

	move.moveDest		= pos;
	move.goalEntity		= enemyEnt;
	move.speed			= fly_speed;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.accuracy		= -1;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= true;
	m_pathRank			= rank; // grayman #2345

	return true;
}

/*
=====================
idAI::MoveToEntity
=====================
*/
bool idAI::MoveToEntity( idEntity *ent ) {
	int			areaNum;
	aasPath_t	path;
	idVec3		pos;

	if ( !ent ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
		return false;
	}

	pos = ent->GetPhysics()->GetOrigin();
	if ( ( move.moveType != MOVETYPE_FLY ) && ( ( move.moveCommand != MOVE_TO_ENTITY ) || ( move.goalEntityOrigin != pos ) ) ) {
		ent->GetFloorPos( 64.0f, pos );
	}

	if ( ReachedPos( pos, MOVE_TO_ENTITY ) ) {
		StopMove( MOVE_STATUS_DONE );
		return true;
	}

	move.toAreaNum = 0;
	if ( aas ) {
		move.toAreaNum = PointReachableAreaNum( pos );
		aas->PushPointIntoAreaNum( move.toAreaNum, pos );

		areaNum	= PointReachableAreaNum( physicsObj.GetOrigin() );
		if ( !PathToGoal( path, areaNum, physicsObj.GetOrigin(), move.toAreaNum, pos, this ) ) {
			AI_DEST_UNREACHABLE = true;
			return false;
		}
	}

	if ( !move.toAreaNum ) {
		// if only trying to update the entity position
		if ( move.moveCommand == MOVE_TO_ENTITY ) {
			if ( !aas ) {
				// keep the move destination up to date for wandering
				move.moveDest = pos;
			}
			return false;
		}

		if ( !NewWanderDir( pos ) ) {
			StopMove( MOVE_STATUS_DEST_UNREACHABLE );
			AI_DEST_UNREACHABLE = true;
			return false;
		}
	}

	if ( ( move.moveCommand != MOVE_TO_ENTITY ) || ( move.goalEntity.GetEntity() != ent ) ) {
		move.startTime		= gameLocal.time;
		move.goalEntity		= ent;
		move.moveCommand	= MOVE_TO_ENTITY;
	}

	move.moveDest			= pos;
	move.goalEntityOrigin	= ent->GetPhysics()->GetOrigin();
	move.moveStatus			= MOVE_STATUS_MOVING;
	move.speed				= fly_speed;
	move.accuracy			= -1;
	AI_MOVE_DONE			= false;
	AI_DEST_UNREACHABLE		= false;
	AI_FORWARD				= true;
	m_pathRank				= rank; // grayman #2345

	return true;
}

CMultiStateMover* idAI::OnElevator(bool mustBeMoving) const
{
	idEntity* ent = physicsObj.GetGroundEntity();

	// Return false if ground entity is not a mover
	if (ent == NULL || !ent->IsType(CMultiStateMover::Type)) return NULL;

	CMultiStateMover* mover = static_cast<CMultiStateMover*>(ent);
	if (mustBeMoving)
	{
		return (!mover->IsAtRest()) ? mover : NULL;
	}
	return mover;
}

bool idAI::Flee(idEntity* entityToFleeFrom, int algorithm, int distanceOption)
{
	EscapePointAlgorithm algorithmType = static_cast<EscapePointAlgorithm>(algorithm);
	
	if ( !aas || !entityToFleeFrom ) {
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	// The current AI origin
	const idVec3& org = physicsObj.GetOrigin();

	// These two will hold the travel destination info
	idVec3 moveDest;
	int moveAreaNum(-1);

	if (algorithmType != FIND_AAS_AREA_FAR_FROM_THREAT)
	{
		// Use the EscapePointManager to locate a pathFlee entity

		// Setup the escape conditions
		EscapeConditions conditions;
		
		conditions.fromEntity = entityToFleeFrom;
		conditions.aas = aas;
		conditions.fromPosition = org;
		conditions.self = this;
		conditions.distanceOption = static_cast<EscapeDistanceOption>(distanceOption);
		conditions.algorithm = algorithmType;
		conditions.minDistanceToThreat = 0.0f;

		// Request the escape goal from the manager
		EscapeGoal goal = gameLocal.m_EscapePointManager->GetEscapeGoal(conditions);

		if (goal.escapePointId == -1)
		{
			// Invalid escape point id returned
			return false;
		}

		// Get the actual point (this should never be NULL)
		EscapePoint* targetPoint = gameLocal.m_EscapePointManager->GetEscapePoint(goal.escapePointId);

		moveDest = targetPoint->origin;
		moveAreaNum = targetPoint->areaNum;
	}
	else 
	{
		// algorithm == FIND_AAS_AREA_FAR_FROM_THREAT

		int	areaNum = PointReachableAreaNum(org);
		
		// consider the entity the monster is getting close to as an obstacle
		aasObstacle_t obstacle;
		obstacle.absBounds = entityToFleeFrom->GetPhysics()->GetAbsBounds();

		idVec3 pos;

		if ( entityToFleeFrom == enemy.GetEntity() ) {
			pos = lastVisibleEnemyPos;
		} else {
			pos = entityToFleeFrom->GetPhysics()->GetOrigin();
		}

		// Setup the evaluator class
		tdmAASFindEscape findEscapeArea(pos, org, distanceOption, 100);
		aasGoal_t dummy;
		aas->FindNearestGoal(dummy, areaNum, org, pos, travelFlags, &obstacle, 1, findEscapeArea);

		aasGoal_t& goal = findEscapeArea.GetEscapeGoal();

		if (goal.areaNum == -1)
		{
			// Invalid escape point id returned
			return false;
		}

		moveDest = goal.origin;
		moveAreaNum = goal.areaNum;
	}

	StopMove(MOVE_STATUS_DONE);
	MoveToPosition(moveDest);
	return true;
/*
	if ( ReachedPos( moveDest, move.moveCommand ) ) {
		StopMove( MOVE_STATUS_DONE );
		return true;
	}
	
	if ( !move.toAreaNum && !NewWanderDir( moveDest ) ) 
	{
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}
*/
/* grayman - the section below can't be reached because
   of the 'return true' above, so I'm commenting it out.
   The section above this, just below the 'return', was
   already commented out.

	move.moveDest		= moveDest;
	move.toAreaNum		= moveAreaNum;
	move.goalEntity		= entityToFleeFrom;
	move.moveCommand	= MOVE_FLEE;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.range			= MAX_FLEE_DISTANCE;
	move.speed			= fly_speed;
	move.startTime		= gameLocal.time;
	move.accuracy		= -1;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= true;
	m_pathRank			= rank; // grayman #2345

	return true;
 */
}

/*
=====================
idAI::GetPositionWithinRange
=====================
*/
aasGoal_t idAI::GetPositionWithinRange(const idVec3& targetPos)
{
	idVec3 org = physicsObj.GetOrigin();
	idVec3 eye = GetEyePosition();
	float fireRange = spawnArgs.GetFloat("fire_range", "0");

	PositionWithinRangeFinder findGoal(this, physicsObj.GetGravityAxis(), targetPos, eye - org, fireRange);

	aasGoal_t goal;
	int areaNum	= PointReachableAreaNum(org);

	aas->FindNearestGoal(goal, areaNum, org, targetPos, travelFlags, NULL, 0, findGoal);
	
	goal.areaNum = -1;

	float bestDistance;
	findGoal.GetBestGoalResult(bestDistance, goal);
	return goal;
}




/*
=====================
idAI::GetObservationPosition
=====================
*/
idVec3 idAI::GetObservationPosition (const idVec3& pointToObserve, const float visualAcuityZeroToOne)
{
	int				areaNum;
	aasObstacle_t	obstacle;
	aasGoal_t		goal;
	idBounds		bounds;
	idVec3			observeFromPos;
	aasPath_t	path;

	if ( !aas ) 
	{	
		observeFromPos = GetPhysics()->GetOrigin();
		AI_DEST_UNREACHABLE = true;
		return observeFromPos;
	}

	const idVec3 &org = physicsObj.GetOrigin();
	areaNum	= PointReachableAreaNum( org );

	// Raise point up just a bit so it isn't on the floor of the aas
	idVec3 pointToObserve2 = pointToObserve;
	pointToObserve2.z += 45.0;

	// What is the lighting along the line where the thing to be observed
	// might be.
	float maxDistanceToObserve = GetMaximumObservationDistanceForPoints(pointToObserve, pointToObserve2);
		
	idAASFindObservationPosition findGoal
	(
		this, 
		physicsObj.GetGravityAxis(), 
		pointToObserve2, 
		GetEyePosition() - org,  // Offset of eye from origin
		maxDistanceToObserve // Maximum distance from which we can observe
	);

	if (!aas->FindNearestGoal
	(
		goal, 
		areaNum, 
		org,
		pointToObserve2, // It is also the goal target
		travelFlags, 
		NULL, 
		0, 
		findGoal 
	) ) 
	{
		float bestDistance;

		// See if we can get to the point itself since noplace was good enough
		// for just looking from a distance due to lighting/occlusion/reachability.
		if (PathToGoal( path, areaNum, physicsObj.GetOrigin(), areaNum, org, this ) ) 
		{

			// Can reach the point itself, so walk right up to it
			observeFromPos = pointToObserve; 

			// Draw the AI Debug Graphics
			if (cv_ai_search_show.GetInteger() >= 1.0)
			{
				idVec4 markerColor (0.0, 1.0, 1.0, 1.0);
				idVec3 arrowLength (0.0, 0.0, 50.0);

				gameRenderWorld->DebugArrow
				(
					markerColor,
					observeFromPos + arrowLength,
					observeFromPos,
					2,
					cv_ai_search_show.GetInteger()
				);
			}

		}

		else if (findGoal.getBestGoalResult
		(
			bestDistance,
			goal
		))
		{

			// Use closest reachable observation point that we found
			observeFromPos = goal.origin;

			// Draw the AI Debug Graphics
			if (cv_ai_search_show.GetInteger() >= 1.0)
			{
				idVec4 markerColor (1.0, 1.0, 0.0, 1.0);
				idVec3 arrowLength (0.0, 0.0, 50.0);

				gameRenderWorld->DebugArrow
				(
					markerColor,
					observeFromPos,
					pointToObserve,
					2,
					cv_ai_search_show.GetInteger()
				);
			}
		}

		else
		{
			// No choice but to try to walk up to it as much as we can
			observeFromPos = pointToObserve; 

			// Draw the AI Debug Graphics
			if (cv_ai_search_show.GetInteger() >= 1.0)
			{
				idVec4 markerColor (1.0, 0.0, 0.0, 1.0);
				idVec3 arrowLength (0.0, 0.0, 50.0);

				gameRenderWorld->DebugArrow
				(
					markerColor,
					observeFromPos + arrowLength,
					observeFromPos,
					2,
					cv_ai_search_show.GetInteger()
				);
			}
		}
		return observeFromPos;
	}

	else
	{
		observeFromPos = goal.origin;
		AI_DEST_UNREACHABLE = false;

		// Draw the AI Debug Graphics
		if (cv_ai_search_show.GetInteger() >= 1.0)
		{
			idVec4 markerColor (0.0, 1.0, 0.0, 1.0);
			idVec3 arrowLength (0.0, 0.0, 50.0);

			gameRenderWorld->DebugArrow
			(
				markerColor,
				observeFromPos,
				pointToObserve,
				2,
				cv_ai_search_show.GetInteger()
			);
		}
		return observeFromPos;
	}
}

/*
=====================
idAI::MoveOutOfRange
=====================
*/
bool idAI::MoveOutOfRange( idEntity *ent, float range ) {
	int				areaNum;
	aasObstacle_t	obstacle;
	aasGoal_t		goal;
	idBounds		bounds;
	idVec3			pos;

	if ( !aas || !ent ) {
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	const idVec3 &org = physicsObj.GetOrigin();
	areaNum	= PointReachableAreaNum( org );

	// consider the entity the monster is getting close to as an obstacle
	obstacle.absBounds = ent->GetPhysics()->GetAbsBounds();

	if ( ent == enemy.GetEntity() ) {
		pos = lastVisibleEnemyPos;
	} else {
		pos = ent->GetPhysics()->GetOrigin();
	}

	idAASFindAreaOutOfRange findGoal( pos, range );
	if ( !aas->FindNearestGoal( goal, areaNum, org, pos, travelFlags, &obstacle, 1, findGoal ) ) {
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Best fleeing location is: %f %f %f in area %d\r", goal.origin.x, goal.origin.y, goal.origin.z, goal.areaNum);

	if ( ReachedPos( goal.origin, move.moveCommand ) ) {
		StopMove( MOVE_STATUS_DONE );
		return true;
	}

	move.moveDest		= goal.origin;
	move.toAreaNum		= goal.areaNum;
	move.goalEntity		= ent;
	move.moveCommand	= MOVE_OUT_OF_RANGE;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.range			= range;
	move.speed			= fly_speed;
	move.startTime		= gameLocal.time;
	move.accuracy		= -1;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= true;
	m_pathRank			= rank; // grayman #2345

	return true;
}

/*
=====================
idAI::MoveToAttackPosition
=====================
*/
bool idAI::MoveToAttackPosition( idEntity *ent, int attack_anim ) {
	int				areaNum;
	aasObstacle_t	obstacle;
	aasGoal_t		goal;
	idBounds		bounds;
	idVec3			pos;

	if ( !aas || !ent ) {
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	const idVec3 &org = physicsObj.GetOrigin();
	areaNum	= PointReachableAreaNum( org );

	// consider the entity the monster is getting close to as an obstacle
	obstacle.absBounds = ent->GetPhysics()->GetAbsBounds();

	if ( ent == enemy.GetEntity() ) {
		pos = lastVisibleEnemyPos;
	} else {
		pos = ent->GetPhysics()->GetOrigin();
	}

	// Make sure we have a active projectile or at least the def
	if (activeProjectile.info.def == NULL)
	{
		if (projectileInfo.Num() == 0)
		{
			gameLocal.Warning("AI %s has no projectile info, cannot move to attack position!", name.c_str());
			return false;
		}

		// Move the def pointer of the "next" projectile info into the active one
		activeProjectile.info = projectileInfo[curProjectileIndex];
	}

	idAASFindAttackPosition findGoal( this, physicsObj.GetGravityAxis(), ent, pos, missileLaunchOffset[ attack_anim ] );
	if ( !aas->FindNearestGoal( goal, areaNum, org, pos, travelFlags, &obstacle, 1, findGoal ) ) {
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	move.moveDest		= goal.origin;
	move.toAreaNum		= goal.areaNum;
	move.goalEntity		= ent;
	move.moveCommand	= MOVE_TO_ATTACK_POSITION;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.speed			= fly_speed;
	move.startTime		= gameLocal.time;
	move.anim			= attack_anim;
	move.accuracy		= -1;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= true;
	m_pathRank			= rank; // grayman #2345

	return true;
}

/*
=====================
idAI::MoveToPosition
=====================
*/
bool idAI::MoveToPosition( const idVec3 &pos, float accuracy )
{
	// Clear the "blocked" flag in the movement subsystem
	movementSubsystem->SetBlockedState(ai::MovementSubsystem::ENotBlocked);

	// Check if we already reached the position
	if ( ReachedPos( pos, move.moveCommand) ) {
		StopMove( MOVE_STATUS_DONE );
		return true;
	}

	if (GetMoveType() == MOVETYPE_SIT 
		|| GetMoveType() == MOVETYPE_SLEEP 
		|| GetMoveType() == MOVETYPE_SIT_DOWN 
		|| GetMoveType() == MOVETYPE_GET_UP 
		|| GetMoveType()== MOVETYPE_LAY_DOWN
		|| GetMoveType()== MOVETYPE_GET_UP_FROM_LYING)
	{
		GetUp();
		return true;
	}

	idVec3 org = pos;
	move.toAreaNum = 0;
	aasPath_t path;
	if ( aas ) {
		move.toAreaNum = PointReachableAreaNum( org );
		aas->PushPointIntoAreaNum( move.toAreaNum, org ); // if this point is outside this area, it will be moved to one of the area's edges

		int areaNum	= PointReachableAreaNum( physicsObj.GetOrigin() );

		if ( !PathToGoal( path, areaNum, physicsObj.GetOrigin(), move.toAreaNum, org, this ) ) {
			StopMove( MOVE_STATUS_DEST_UNREACHABLE );
			AI_DEST_UNREACHABLE = true;
			return false;
		}
	}

	if ( !move.toAreaNum && !NewWanderDir( org ) ) {
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	// Valid path to goal, check if we need to use an elevator
	if (path.type == PATHTYPE_ELEVATOR)
	{
		NeedToUseElevator(path.elevatorRoute);
	}

	move.moveDest		= org;
	move.goalEntity		= NULL;
	move.moveCommand	= MOVE_TO_POSITION;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= fly_speed;
	move.accuracy		= accuracy;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= true;
	m_pathRank			= rank; // grayman #2345

	return true;
}

/*
=====================
idAI::LookForCover
=====================
*/
bool idAI::LookForCover(aasGoal_t& hideGoal,idEntity *hideFromEnt, const idVec3 &hideFromPos) 
{
	if (aas == NULL) return false;

	idBounds bounds;
	aasObstacle_t obstacle;

	const idVec3 &org = physicsObj.GetOrigin();
	int areaNum	= PointReachableAreaNum( org );

	// consider the entity the monster tries to hide from as an obstacle
	obstacle.absBounds = hideFromEnt->GetPhysics()->GetAbsBounds();

	idAASFindCover findCover( this, hideFromEnt, hideFromPos );
	return aas->FindNearestGoal( hideGoal, areaNum, org, hideFromPos, travelFlags, &obstacle, 1, findCover, spawnArgs.GetInt("taking_cover_max_cost") );
}

/*
=====================
idAI::MoveToCover
=====================
*/
bool idAI::MoveToCover( idEntity *hideFromEnt, const idVec3 &hideFromPos ) {
	//common->Printf("MoveToCover called... ");

	aasGoal_t hideGoal;

	if ( !aas || !hideFromEnt ) {
		common->Printf("MoveToCover failed: null aas or entity\n");
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}
	
	if (!LookForCover(hideGoal, hideFromEnt, hideFromPos)) {
		//common->Printf("MoveToCover failed: destination unreachable\n");
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	if ( ReachedPos( hideGoal.origin, move.moveCommand ) ) {
		//common->Printf("MoveToCover succeeded: Already at hide position\n");
		StopMove( MOVE_STATUS_DONE );
		return true;
	}

	move.moveDest		= hideGoal.origin;
	move.toAreaNum		= hideGoal.areaNum;
	move.goalEntity		= hideFromEnt;
	move.moveCommand	= MOVE_TO_COVER;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= fly_speed;
	move.accuracy		= -1;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= true;
	m_pathRank			= rank; // grayman #2345

	//common->Printf("MoveToCover succeeded: Now moving into cover\n");

	return true;
}

/*
=====================
idAI::SlideToPosition
=====================
*/
bool idAI::SlideToPosition( const idVec3 &pos, float time ) {
	StopMove( MOVE_STATUS_DONE );

	move.moveDest		= pos;
	move.goalEntity		= NULL;
	move.moveCommand	= MOVE_SLIDE_TO_POSITION;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.duration		= idPhysics::SnapTimeToPhysicsFrame( SEC2MS( time ) );
	move.accuracy		= -1;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= false;
	m_pathRank			= 1000; // grayman #2345

	if ( move.duration > 0 ) {
		move.moveDir = ( pos - physicsObj.GetOrigin() ) / MS2SEC( move.duration );
		if ( move.moveType != MOVETYPE_FLY ) {
			move.moveDir.z = 0.0f;
		}
		move.speed = move.moveDir.LengthFast();
	}

	return true;
}

/*
=====================
idAI::WanderAround
=====================
*/
bool idAI::WanderAround( void ) {
	StopMove( MOVE_STATUS_DONE );

	move.moveDest = physicsObj.GetOrigin() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 256.0f;
	if ( !NewWanderDir( move.moveDest ) ) {
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	move.moveCommand	= MOVE_WANDER;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= fly_speed;
	move.accuracy		= -1;
	AI_MOVE_DONE		= false;
	AI_FORWARD			= true;
	m_pathRank			= rank; // grayman #2345

	return true;
}

/*
=====================
idAI::MoveDone
=====================
*/
bool idAI::MoveDone( void ) const {
	return ( move.moveCommand == MOVE_NONE );
}

void idAI::SetMoveType( int moveType ) {
	if ( ( moveType < 0 ) || ( moveType >= NUM_MOVETYPES ) ) {
		gameLocal.Error( "Invalid movetype %d", moveType );
	}

	move.moveType = static_cast<moveType_t>( moveType );
	if ( move.moveType == MOVETYPE_FLY ) {
		travelFlags = TFL_WALK|TFL_AIR|TFL_FLY|TFL_DOOR;
	} else {
		travelFlags = TFL_WALK|TFL_AIR|TFL_DOOR;
	}
}

/*
================
idAI::StepDirection
================
*/
bool idAI::StepDirection( float dir ) {
	predictedPath_t path;
	idVec3 org;

	move.wanderYaw = dir;
	move.moveDir = idAngles( 0, move.wanderYaw, 0 ).ToForward();

	org = physicsObj.GetOrigin();

	idAI::PredictPath( this, aas, org, move.moveDir * 48.0f, 1000, 1000, ( move.moveType == MOVETYPE_FLY ) ? SE_BLOCKED : ( SE_ENTER_OBSTACLE | SE_BLOCKED | SE_ENTER_LEDGE_AREA ), path );

	if ( path.blockingEntity && ( ( move.moveCommand == MOVE_TO_ENEMY ) || ( move.moveCommand == MOVE_TO_ENTITY ) ) && ( path.blockingEntity == move.goalEntity.GetEntity() ) ) {
		// don't report being blocked if we ran into our goal entity
		return true;
	}

	// SZ: January 7, 2006: Wandering uses this, and currently it fails the test if it would bump into another AI.
	// If we are wandering, we want to make sure we can still bump into the player or enemy AIs, both of which are idActor
	// based.
	if (path.blockingEntity && (move.moveCommand == MOVE_WANDER))
	{
		// What type of entity is it?
		if
		(
			(path.blockingEntity->IsType(idActor::Type) )
		)
		{
			// Bump into enemies all you want while wandering
			if (IsEnemy(path.blockingEntity))
			{
				return true;
			}
		}

	} // End wandering case

	if ( ( move.moveType == MOVETYPE_FLY ) && ( path.endEvent == SE_BLOCKED ) ) {
		float z;

		move.moveDir = path.endVelocity * 1.0f / 48.0f;

		// trace down to the floor and see if we can go forward
		idAI::PredictPath( this, aas, org, idVec3( 0.0f, 0.0f, -1024.0f ), 1000, 1000, SE_BLOCKED, path );

		idVec3 floorPos = path.endPos;
		idAI::PredictPath( this, aas, floorPos, move.moveDir * 48.0f, 1000, 1000, SE_BLOCKED, path );
		if ( !path.endEvent ) {
			move.moveDir.z = -1.0f;
			return true;
		}

		// trace up to see if we can go over something and go forward
		idAI::PredictPath( this, aas, org, idVec3( 0.0f, 0.0f, 256.0f ), 1000, 1000, SE_BLOCKED, path );

		idVec3 ceilingPos = path.endPos;

		for( z = org.z; z <= ceilingPos.z + 64.0f; z += 64.0f ) {
			idVec3 start;
			if ( z <= ceilingPos.z ) {
				start.x = org.x;
				start.y = org.y;
                start.z = z;
			} else {
				start = ceilingPos;
			}
			idAI::PredictPath( this, aas, start, move.moveDir * 48.0f, 1000, 1000, SE_BLOCKED, path );
			if ( !path.endEvent ) {
				move.moveDir.z = 1.0f;
				return true;
			}
		}
		return false;
	}

	return ( path.endEvent == 0 );
}
/*
================
idAI::MoveAlongVector
================
*/
bool idAI::MoveAlongVector( float yaw ) 
{
	StopMove( MOVE_STATUS_DONE );
	move.moveDir = idAngles( 0, yaw, 0 ).ToForward();
	move.moveDest = physicsObj.GetOrigin() + move.moveDir * 256.0f;

	move.moveCommand	= MOVE_VECTOR;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= fly_speed;
	AI_MOVE_DONE		= false;
	AI_FORWARD			= true;
	m_pathRank			= rank; // grayman #2345

	return true;
}
/*
================
idAI::NewWanderDir
================
*/
bool idAI::NewWanderDir( const idVec3 &dest ) {
	float	deltax, deltay;
	float	d[ 3 ];
	float	tdir, olddir, turnaround;

	move.nextWanderTime = gameLocal.time + ( gameLocal.random.RandomInt(500) + 500 );

	olddir = idMath::AngleNormalize360( ( int )( current_yaw / 45 ) * 45 );
	turnaround = idMath::AngleNormalize360( olddir - 180 );

	idVec3 org = physicsObj.GetOrigin();
	deltax = dest.x - org.x;
	deltay = dest.y - org.y;
	if ( deltax > 10 ) {
		d[ 1 ]= 0;
	} else if ( deltax < -10 ) {
		d[ 1 ] = 180;
	} else {
		d[ 1 ] = DI_NODIR;
	}

	if ( deltay < -10 ) {
		d[ 2 ] = 270;
	} else if ( deltay > 10 ) {
		d[ 2 ] = 90;
	} else {
		d[ 2 ] = DI_NODIR;
	}

	// try direct route
	if ( d[ 1 ] != DI_NODIR && d[ 2 ] != DI_NODIR ) {
		if ( d[ 1 ] == 0 ) {
			tdir = d[ 2 ] == 90 ? 45 : 315;
		} else {
			tdir = d[ 2 ] == 90 ? 135 : 215;
		}

		if ( tdir != turnaround && StepDirection( tdir ) ) {
			return true;
		}
	}

	// try other directions
	if ( ( gameLocal.random.RandomInt() & 1 ) || fabs( deltay ) > fabs( deltax ) ) {
		tdir = d[ 1 ];
		d[ 1 ] = d[ 2 ];
		d[ 2 ] = tdir;
	}

	if ( d[ 1 ] != DI_NODIR && d[ 1 ] != turnaround && StepDirection( d[1] ) ) {
		return true;
	}

	if ( d[ 2 ] != DI_NODIR && d[ 2 ] != turnaround	&& StepDirection( d[ 2 ] ) ) {
		return true;
	}

	// there is no direct path to the player, so pick another direction
	if ( olddir != DI_NODIR && StepDirection( olddir ) ) {
		return true;
	}

	 // randomly determine direction of search
	if ( gameLocal.random.RandomInt() & 1 ) {
		for( tdir = 0; tdir <= 315; tdir += 45 ) {
			if ( tdir != turnaround && StepDirection( tdir ) ) {
                return true;
			}
		}
	} else {
		for ( tdir = 315; tdir >= 0; tdir -= 45 ) {
			if ( tdir != turnaround && StepDirection( tdir ) ) {
				return true;
			}
		}
	}

	if ( turnaround != DI_NODIR && StepDirection( turnaround ) ) {
		return true;
	}

	// can't move
	StopMove( MOVE_STATUS_DEST_UNREACHABLE );
	return false;
}

/*
=====================
idAI::GetMovePos
=====================
*/
bool idAI::GetMovePos(idVec3 &seekPos)
{
	START_SCOPED_TIMING(aiGetMovePosTimer, scopedGetMovePosTimer);

	const idVec3& org = physicsObj.GetOrigin();
	seekPos = org;

	switch( move.moveCommand ) {
	case MOVE_NONE :
		seekPos = move.moveDest;
		return false;
		break;

	case MOVE_FACE_ENEMY :
	case MOVE_FACE_ENTITY :
		seekPos = move.moveDest;
		return false;
		break;

	case MOVE_TO_POSITION_DIRECT :
		seekPos = move.moveDest;
		if ( ReachedPos(move.moveDest, move.moveCommand))
		{
			StopMove( MOVE_STATUS_DONE );
		}
		return false;
		break;

	case MOVE_SLIDE_TO_POSITION :
		seekPos = org;
		return false;
		break;

	case MOVE_VECTOR :
		seekPos = move.moveDest;
		return true;
		break;
	default:
		break; // Handled below (note the returns in all cases above)
		// (default case added to suppress GCC warnings)
	}

	if (move.moveCommand == MOVE_TO_ENTITY) {
		MoveToEntity(move.goalEntity.GetEntity());
	}

	move.moveStatus = MOVE_STATUS_MOVING;
	bool result = false;

	// Check if the blocking time has already expired
	if (gameLocal.time > move.blockTime)
	{
		if (move.moveCommand == MOVE_WANDER)
		{
			move.moveDest = org + viewAxis[0] * physicsObj.GetGravityAxis() * 256.0f;
		} 
		else
		{
			// Check if we already reached the destination position
			if (ReachedPos(move.moveDest, move.moveCommand))
			{
				// Yes, stop the move, move status is DONE
				StopMove(MOVE_STATUS_DONE);
				seekPos	= org;
				return false; // nothing to do, return false
			}
		}

		if (aas != NULL && move.toAreaNum != 0)
		{
			// Get the area number we're currently in.
			int areaNum = PointReachableAreaNum(org);

			// Try to setup a path to the goal
			aasPath_t path;
			if (PathToGoal(path, areaNum, org, move.toAreaNum, move.moveDest, this))
			{
				seekPos = path.moveGoal;
				result = true; // We have a valid Path to the goal
				move.nextWanderTime = 0;

				// angua: check whether there is a door in the path
				if (path.firstDoor != NULL)
				{
					const idVec3& doorOrg = path.firstDoor->GetPhysics()->GetOrigin();
					const idVec3& org = GetPhysics()->GetOrigin();
					idVec3 dir = doorOrg - org;
					dir.z = 0;
					float dist = dir.LengthFast();
					if (dist < 500)
					{
						GetMind()->GetState()->OnFrobDoorEncounter(path.firstDoor);
					}	
				}
			}
			else
			{
				AI_DEST_UNREACHABLE = true;
			}
		}
	}

	if (!result)
	{
		// No path to the goal found, wander around
		if (gameLocal.time > move.nextWanderTime || !StepDirection(move.wanderYaw))
		{
			result = NewWanderDir(move.moveDest);
			if (!result)
			{
				StopMove(MOVE_STATUS_DEST_UNREACHABLE);
				AI_DEST_UNREACHABLE = true;
				seekPos	= org;
				return false;
			}
		}
		else
		{
			result = true;
		}

		// Set the seek position to something far away, but in the right direction
		seekPos = org + move.moveDir * 2048.0f;

		if (ai_debugMove.GetBool())	{
			gameRenderWorld->DebugLine( colorYellow, org, seekPos, gameLocal.msec, true );
		}
	}
	else
	{
		// result == true, we have a valid path
		AI_DEST_UNREACHABLE = false;
	}

	if (result && ai_debugMove.GetBool()) {
		gameRenderWorld->DebugLine( colorCyan, physicsObj.GetOrigin(), seekPos );
	}

	return result;
}


/*
=====================
The Dark Mod
idAI::CanSee virtual override
=====================
*/
bool idAI::CanSee( idEntity *ent, bool useFOV ) const
{
	START_SCOPED_TIMING(aiCanSeeTimer, scopedCanSeeTimer);

	// Test if it is occluded, and use field of vision in the check (true as second parameter)
	bool cansee = idActor::CanSee( ent, useFOV );

	// Also consider lighting and visual acuity of AI
	if (cansee)
	{
		cansee = !IsEntityHiddenByDarkness(ent, 0.1f);	// tels: hard-coded threshold of 0.1f
	}

	// Return result
	return cansee;
}

/*
=====================
The Dark Mod
idAI::CanSeeExt

This method can ignore lighting conditions and/or field of vision.
=====================
*/
bool idAI::CanSeeExt( idEntity *ent, const bool useFOV, const bool useLighting ) const
{
	// Test if it is occluded
	bool cansee = idActor::CanSee( ent, useFOV );

	if (cansee && useLighting)
	{
		cansee = !IsEntityHiddenByDarkness(ent, 0.1f);	// tels: hard-coded threshold of 0.1f
	}

	// Return result
	return cansee;
}

/*
=====================
The Dark Mod
idAI::CanSeePositionExt

This metohd can ignore lighting conditions and/or field of vision.
=====================
*/
bool idAI::CanSeePositionExt( idVec3 position, bool useFOV, bool useLighting )
{
	if ( useFOV && !CheckFOV( position ) )
	{
		return false;
	}

	idVec3 ownOrigin = physicsObj.GetOrigin();

	bool canSee = EntityCanSeePos (this, ownOrigin, position);

	if (canSee && useLighting)
	{
		idVec3 bottomPoint = position;
		idVec3 topPoint = position - (physicsObj.GetGravityNormal() * 32.0);

		float maxDistanceToObserve = GetMaximumObservationDistanceForPoints(bottomPoint, topPoint);

		if ((position - ownOrigin).Length() > maxDistanceToObserve)
		{
			canSee = false;
		}

		// Draw debug graphic?
		if (cv_ai_visdist_show.GetFloat() > 1.0)
		{
			idVec3 midPoint = bottomPoint + ((topPoint-bottomPoint) / 2.0);
			idVec3 observeFrom = GetEyePosition();

			if (!canSee)
			{
				idVec4 markerColor (1.0, 0.0, 0.0, 0.0);
				idVec4 markerColor2 (1.0, 0.0, 1.0, 0.0);
				idVec3 arrowLength = midPoint - observeFrom;
				arrowLength.Normalize();
				arrowLength *= maxDistanceToObserve;

				// Distance we could see
				gameRenderWorld->DebugArrow
				(
					markerColor,
					observeFrom,
					observeFrom + arrowLength,
					2,
					cv_ai_visdist_show.GetInteger()
				);

				// Gap to where we want to see
				gameRenderWorld->DebugArrow
				(
					markerColor2,
					observeFrom + arrowLength,
					midPoint,
					2,
					cv_ai_visdist_show.GetInteger()
				);
			}
			else
			{
				idVec4 markerColor (0.0, 1.0, 0.0, 0.0);

				// We can see there
				gameRenderWorld->DebugArrow
				(
					markerColor,
					observeFrom,
					midPoint,
					2,
					cv_ai_visdist_show.GetInteger()
				);
			}
		}
	}

	return canSee;
}

/*
=====================
idAI::EntityCanSeePos
=====================
*/
bool idAI::EntityCanSeePos( idActor *actor, const idVec3 &actorOrigin, const idVec3 &pos )
{
	pvsHandle_t handle = gameLocal.pvs.SetupCurrentPVS( actor->GetPVSAreas(), actor->GetNumPVSAreas() );

	if ( !gameLocal.pvs.InCurrentPVS( handle, GetPVSAreas(), GetNumPVSAreas() ) ) {
		gameLocal.pvs.FreeCurrentPVS( handle );
		return false;
	}

	gameLocal.pvs.FreeCurrentPVS( handle );

	idVec3 eye = actorOrigin + actor->EyeOffset();

	idVec3 point = pos;
	point[2] += 1.0f;

	physicsObj.DisableClip();

	trace_t results;
	gameLocal.clip.TracePoint( results, eye, point, MASK_SOLID, actor );
	if ( results.fraction >= 1.0f || ( gameLocal.GetTraceEntity( results ) == this ) ) {
		physicsObj.EnableClip();
		return true;
	}

	const idBounds &bounds = physicsObj.GetBounds();
	point[2] += bounds[1][2] - bounds[0][2];

	gameLocal.clip.TracePoint( results, eye, point, MASK_SOLID, actor );
	physicsObj.EnableClip();
	if ( results.fraction >= 1.0f || ( gameLocal.GetTraceEntity( results ) == this ) ) {
		return true;
	}

	return false;
}

/*
=====================
idAI::BlockedFailSafe
=====================
*/
void idAI::BlockedFailSafe( void ) {
	if ( !ai_blockedFailSafe.GetBool() || blockedRadius < 0.0f ) {
		return;
	}
	if ( !physicsObj.OnGround() || enemy.GetEntity() == NULL ||
			( physicsObj.GetOrigin() - move.lastMoveOrigin ).LengthSqr() > Square( blockedRadius ) ) {
		move.lastMoveOrigin = physicsObj.GetOrigin();
		move.lastMoveTime = gameLocal.time;
	}
	if ( move.lastMoveTime < gameLocal.time - blockedMoveTime ) {
		if ( lastAttackTime < gameLocal.time - blockedAttackTime ) {
			AI_BLOCKED = true;
			move.lastMoveTime = gameLocal.time;
		}
	}
}

/***********************************************************************

	turning

***********************************************************************/

/*
=====================
idAI::Turn
=====================
*/
void idAI::Turn(const idVec3& pivotOffset) {
	float diff;
	float diff2;
	float turnAmount;
	animFlags_t animflags;

	if ( !turnRate ) {
		return;
	}

	// check if the animator has marker this anim as non-turning
	if ( !legsAnim.Disabled() && !legsAnim.AnimDone( 0 ) ) {
		animflags = legsAnim.GetAnimFlags();
	} else {
		animflags = torsoAnim.GetAnimFlags();
	}
	if ( animflags.ai_no_turn ) {
		return;
	}

	idVec3 startPos = viewAxis * pivotOffset;

	if ( anim_turn_angles && animflags.anim_turn ) {
		idMat3 rotateAxis;

		// set the blend between no turn and full turn
		float frac = anim_turn_amount / anim_turn_angles;
		animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 0, 1.0f - frac );
		animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 1, frac );
		animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 0, 1.0f - frac );
		animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 1, frac );

		// get the total rotation from the start of the anim
		animator.GetDeltaRotation( 0, gameLocal.time, rotateAxis );
		current_yaw = idMath::AngleNormalize180( anim_turn_yaw + rotateAxis[ 0 ].ToYaw() );
	} else {
		diff = idMath::AngleNormalize180( ideal_yaw - current_yaw );
		turnVel += AI_TURN_SCALE * diff * MS2SEC( gameLocal.msec );
		if ( turnVel > turnRate ) {
			turnVel = turnRate;
		} else if ( turnVel < -turnRate ) {
			turnVel = -turnRate;
		}
		turnAmount = turnVel * MS2SEC( gameLocal.time - m_lastThinkTime );
		if ( ( diff >= 0.0f ) && ( turnAmount >= diff ) ) {
			turnVel = diff / MS2SEC( gameLocal.msec );
			turnAmount = diff;
		} else if ( ( diff <= 0.0f ) && ( turnAmount <= diff ) ) {
			turnVel = diff / MS2SEC( gameLocal.msec );
			turnAmount = diff;
		}
		current_yaw += turnAmount;
		current_yaw = idMath::AngleNormalize180( current_yaw );
		diff2 = idMath::AngleNormalize180( ideal_yaw - current_yaw );
		if ( idMath::Fabs( diff2 ) < 0.1f ) {
			current_yaw = ideal_yaw;
		}
	}

	viewAxis = idAngles( 0, current_yaw, 0 ).ToMat3();

	idVec3 endPos = viewAxis * pivotOffset;

	GetPhysics()->SetOrigin(GetPhysics()->GetOrigin() - idVec3(endPos - startPos));

	if ( ai_debugMove.GetBool() ) {
		const idVec3 &org = physicsObj.GetOrigin();
		gameRenderWorld->DebugLine( colorRed, org, org + idAngles( 0, ideal_yaw, 0 ).ToForward() * 64, gameLocal.msec );
		gameRenderWorld->DebugLine( colorGreen, org, org + idAngles( 0, current_yaw, 0 ).ToForward() * 48, gameLocal.msec );
		gameRenderWorld->DebugLine( colorYellow, org, org + idAngles( 0, current_yaw + turnVel, 0 ).ToForward() * 32, gameLocal.msec );
	}
}

/*
=====================
idAI::FacingIdeal
=====================
*/
bool idAI::FacingIdeal( void ) {
	float diff;

	if ( !turnRate ) {
		return true;
	}

	diff = idMath::AngleNormalize180( current_yaw - ideal_yaw );
	if ( idMath::Fabs( diff ) < 0.01f ) {
		// force it to be exact
		current_yaw = ideal_yaw;
		return true;
	}

	return false;
}

/*
=====================
idAI::TurnToward
=====================
*/
bool idAI::TurnToward( float yaw ) {
	ideal_yaw = idMath::AngleNormalize180( yaw );
	bool result = FacingIdeal();
	return result;
}

/*
=====================
idAI::TurnToward
=====================
*/
bool idAI::TurnToward( const idVec3 &pos ) {
	idVec3 dir;
	idVec3 local_dir;
	float lengthSqr;

	dir = pos - physicsObj.GetOrigin();
	physicsObj.GetGravityAxis().ProjectVector( dir, local_dir );
	local_dir.z = 0.0f;
	lengthSqr = local_dir.LengthSqr();
	if ( lengthSqr > Square( 2.0f ) || ( lengthSqr > Square( 0.1f ) && enemy.GetEntity() == NULL ) ) {
		ideal_yaw = idMath::AngleNormalize180( local_dir.ToYaw() );
	}

	bool result = FacingIdeal();
	return result;
}

/***********************************************************************

	Movement

***********************************************************************/

/*
================
idAI::ApplyImpulse
================
*/
void idAI::ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse ) {
	// FIXME: Jim take a look at this and see if this is a reasonable thing to do
	// instead of a spawnArg flag.. Sabaoth is the only slide monster ( and should be the only one for D3 )
	// and we don't want him taking physics impulses as it can knock him off the path
	if ( move.moveType != MOVETYPE_STATIC && move.moveType != MOVETYPE_SLIDE ) {
		idActor::ApplyImpulse( ent, id, point, impulse );
	}
}

/*
=====================
idAI::GetMoveDelta
=====================
*/
void idAI::GetMoveDelta( const idMat3 &oldaxis, const idMat3 &axis, idVec3 &delta )
{
	// Get the delta from the animation system (for one frame) and transform it
	// animator.GetDelta( gameLocal.time - gameLocal.msec, gameLocal.time, delta);

	// angua: distance from last thinking time
	animator.GetDelta( m_lastThinkTime, gameLocal.time, delta);

	delta = axis * delta;

	if ( modelOffset != vec3_zero )
	{
		// the pivot of the monster's model is around its origin, and not around the bounding
		// box's origin, so we have to compensate for this when the model is offset so that
		// the monster still appears to rotate around it's origin.
		idVec3 oldModelOrigin = modelOffset * oldaxis;
		idVec3 modelOrigin = modelOffset * axis;	
		
		delta += oldModelOrigin - modelOrigin;
	}

	delta *= physicsObj.GetGravityAxis();
}

/*
=====================
idAI::CheckObstacleAvoidance
=====================
*/
void idAI::CheckObstacleAvoidance( const idVec3 &goalPos, idVec3 &newPos )
{
	START_SCOPED_TIMING(aiObstacleAvoidanceTimer, scopedObstacleAvoidanceTimer);

	newPos = goalPos;	// grayman #2345 - initialize newPos in case nothing changes.
						// Without this, there's a return from this function
						// that leaves newPos = [0,0,0], so the AI thinks he
						// has to go to the world origin, which is a bug.

	if (ignore_obstacles || cv_ai_opt_noobstacleavoidance.GetBool())
	{
		move.obstacle = NULL;
		return;
	}

	if (cv_ai_goalpos_show.GetBool()) 
	{
		idVec4 color(colorCyan);
		color.x = gameLocal.random.RandomFloat();
		color.y = gameLocal.random.RandomFloat();
		color.z = gameLocal.random.RandomFloat();
		gameRenderWorld->DebugArrow(color, goalPos, goalPos + idVec3(0,0,15), 2, 16);
	}

	const idVec3& origin = physicsObj.GetOrigin();

	idEntity* obstacle = NULL;
	obstaclePath_t	path;

	AI_OBSTACLE_IN_PATH = false;
	bool foundPath = FindPathAroundObstacles( &physicsObj, aas, enemy.GetEntity(), origin, goalPos, path, this );

	if ( ai_showObstacleAvoidance.GetBool())
	{
		gameRenderWorld->DebugLine( colorBlue, goalPos + idVec3( 1.0f, 1.0f, 0.0f ), goalPos + idVec3( 1.0f, 1.0f, 64.0f ), gameLocal.msec );
		gameRenderWorld->DebugLine( foundPath ? colorYellow : colorRed, path.seekPos, path.seekPos + idVec3( 0.0f, 0.0f, 64.0f ), gameLocal.msec );
	}

	if (!foundPath)
	{
		// couldn't get around obstacles

		if (path.doorObstacle != NULL) 
		{
			// We have a frobmover in our way, raise a signal to the current state
			mind->GetState()->OnFrobDoorEncounter(path.doorObstacle);
		}

		if (path.firstObstacle)
		{
			AI_OBSTACLE_IN_PATH = true;

			if ( physicsObj.GetAbsBounds().Expand( 2.0f ).IntersectsBounds( path.firstObstacle->GetPhysics()->GetAbsBounds() ) )
			{
				// We are already touching the first obstacle
				obstacle = path.firstObstacle;
			}
		}
		else if (path.startPosObstacle)
		{
			AI_OBSTACLE_IN_PATH = true;
			if ( physicsObj.GetAbsBounds().Expand( 2.0f ).IntersectsBounds( path.startPosObstacle->GetPhysics()->GetAbsBounds() ) )
			{
				// We are touching the startpos obstacle
				obstacle = path.startPosObstacle;
			}
		}
		else
		{
			// No firstObstacle, no startPosObstacle, must be blocked by wall
			move.moveStatus = MOVE_STATUS_BLOCKED_BY_WALL;
		}
#if 0
	} else if ( path.startPosObstacle ) {
		// check if we're past where the our origin was pushed out of the obstacle
		dir = goalPos - origin;
		dir.Normalize();
		dist = ( path.seekPos - origin ) * dir;
		if ( dist < 1.0f ) {
			AI_OBSTACLE_IN_PATH = true;
			obstacle = path.startPosObstacle;
		}
#endif
	}
	else
	{
		// We found a path around obstacles
		if (path.doorObstacle != NULL) // grayman #2712 - do we have to handle a door?
		{
			// We have a frobmover in our way, raise a signal to the current state
			mind->GetState()->OnFrobDoorEncounter(path.doorObstacle);
		}

		// still check for the seekPosObstacle
		if (path.seekPosObstacle)
		{
	/*		gameRenderWorld->DebugBox(foundPath ? colorGreen : colorRed, idBox(path.seekPosObstacle->GetPhysics()->GetBounds(), 
													  path.seekPosObstacle->GetPhysics()->GetOrigin(), 
													  path.seekPosObstacle->GetPhysics()->GetAxis()), 16);
	 */

			// greebo: Check if we have a frobdoor entity at our seek position
			if (path.seekPosObstacle->IsType(CFrobDoor::Type)) 
			{
				// We have a frobmover in our way, raise a signal to the current state
				mind->GetState()->OnFrobDoorEncounter(static_cast<CFrobDoor*>(path.seekPosObstacle));
			}

			// if the AI is very close to the path.seekPos already and path.seekPosObstacle != NULL
			// then we want to push the path.seekPosObstacle entity out of the way
			AI_OBSTACLE_IN_PATH = true;

			// check if we're past where the goalPos was pushed out of the obstacle
			idVec3 dir = goalPos - origin;
			dir.NormalizeFast();
			float dist = (path.seekPos - origin) * dir;
			if (dist < 1.0f)
			{
				obstacle = path.seekPosObstacle;
			}
		}
	}

	// If we don't have an obstacle, set the seekpos and return
	if (obstacle == NULL)
	{
		newPos = path.seekPos;
		move.obstacle = NULL;
		return;
	}

	// if we had an obstacle, set our move status based on the type, and kick it out of the way if it's a moveable
	if (obstacle->IsType(idActor::Type))
	{
		// monsters aren't kickable
		move.moveStatus = (obstacle == enemy.GetEntity()) ? MOVE_STATUS_BLOCKED_BY_ENEMY : MOVE_STATUS_BLOCKED_BY_MONSTER;
	}
	else
	{
		// If it's a door handle, switch the obstacle to the door so we don't get all hung
		// up on door handles
		if (obstacle->IsType(CFrobDoorHandle::Type))
		{
			// Make the obstacle the door itself
			obstacle = static_cast<CFrobDoorHandle*>(obstacle)->GetDoor();
		}

		// Handle doors
		if (obstacle->IsType(CFrobDoor::Type))
		{
			// Try to open doors
			CFrobDoor* p_door = static_cast<CFrobDoor*>(obstacle);
		
			// We have a frobmover in our way, raise a signal to the current state
			mind->GetState()->OnFrobDoorEncounter(p_door);
		}
		
		// Try backing away
		newPos = obstacle->GetPhysics()->GetOrigin();
		idVec3 obstacleDelta = obstacle->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();

		obstacleDelta.NormalizeFast();
		obstacleDelta *= 128.0;

		newPos = obstacle->GetPhysics()->GetOrigin() - obstacleDelta;
		move.moveStatus = MOVE_STATUS_BLOCKED_BY_OBJECT;
	}

	move.obstacle = obstacle;
}

/*
=====================
idAI::CanPassThroughDoor - Is the AI allowed through this door? (grayman #2691) 
=====================
*/

bool idAI::CanPassThroughDoor(CFrobDoor* frobDoor)
{
	// grayman #2691 - quick test for spawnarg that appears on many furniture doors

	if (frobDoor->spawnArgs.GetBool("immune_to_target_setfrobable", "0"))
	{
		return false;
	}

	// grayman #2691 - can't pass through doors that don't rotate on the z-axis

	idVec3 rotationAxis = frobDoor->GetRotationAxis();
	if ((rotationAxis.z == 0) && ((rotationAxis.x != 0) || (rotationAxis.y != 0))) // grayman #2712 - handles sliding doors
	{
		return false;
	}

	idBounds door1Bounds = frobDoor->GetPhysics()->GetBounds();
	idBounds myBounds = GetPhysics()->GetBounds();
	idVec3 door1Size = door1Bounds.GetSize();
	idVec3 mySize = myBounds.GetSize();
	bool canPassDoor1 = (door1Size.x > mySize.x) || (door1Size.y > mySize.y);
	if (canPassDoor1)
	{
		return true;
	}

	// The AI can't fit through the first door. Is this door part of a double door?
	
	CFrobDoor* doubleDoor = frobDoor->GetDoubleDoor();
	if (doubleDoor != NULL)
	{
		idBounds door2Bounds = doubleDoor->GetPhysics()->GetBounds();
		idVec3 door2Size = door2Bounds.GetSize();
		bool canPassDoors = ((door1Size.x + door2Size.x) > mySize.x) || ((door1Size.y + door2Size.y) > mySize.y);
		if (canPassDoors)
		{
			return true;
		}
	}

	return false;
}

/*
=====================
idAI::DeadMove
=====================
*/
void idAI::DeadMove( void ) {
	idVec3				delta;
	monsterMoveResult_t	moveResult;

	idVec3 org = physicsObj.GetOrigin();

	GetMoveDelta( viewAxis, viewAxis, delta );
	physicsObj.SetDelta( delta );

	RunPhysics();

	moveResult = physicsObj.GetMoveResult();
	AI_ONGROUND = physicsObj.OnGround();
}

/*
=====================
idAI::AnimMove
=====================
*/
void idAI::AnimMove()
{
	START_SCOPED_TIMING(aiAnimMoveTimer, scopedAnimMoveTimer);

	idVec3				goalPos;
	idVec3				goalDelta;

	idVec3 oldorigin = physicsObj.GetOrigin();
	idMat3 oldaxis = viewAxis;

	AI_BLOCKED = false;

	if ( move.moveCommand < NUM_NONMOVING_COMMANDS ){
		move.lastMoveOrigin.Zero();
		move.lastMoveTime = gameLocal.time;
	}

	move.obstacle = NULL;
	if (move.moveCommand == MOVE_FACE_ENEMY && enemy.GetEntity())
	{
		TurnToward( lastVisibleEnemyPos );
		goalPos = oldorigin;
	}
	else if (move.moveCommand == MOVE_FACE_ENTITY && move.goalEntity.GetEntity())
	{
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
		goalPos = oldorigin;
	} 
	else if (GetMovePos(goalPos)) 
	{
		// greebo: We have a valid goalposition (not reached the target yet), check for obstacles
		if (move.moveCommand != MOVE_WANDER && move.moveCommand != MOVE_VECTOR) 
		{
			idVec3 newDest;
			CheckObstacleAvoidance( goalPos, newDest );

			TurnToward(newDest);
		} 
		else // MOVE_WANDER || MOVE_VECTOR
		{
			TurnToward(goalPos);
		}
	}

	// greebo: This should take care of rats running around in circles around their goal
	// due to not turning fast enough to their goalPos, sending them into a stable orbit.
	float oldTurnRate = turnRate;

	if ((goalPos - physicsObj.GetOrigin()).LengthSqr() < Square(50))
	{
		turnRate *= gameLocal.random.RandomFloat() + 1;
	}

	// greebo: Now actually turn towards the "ideal" yaw.
	Turn();

	// Revert the turnRate changes
	turnRate = oldTurnRate;

	// Determine the delta depending on the move type
	idVec3 delta(0,0,0);
	if (move.moveCommand == MOVE_SLIDE_TO_POSITION)
	{
		if ( gameLocal.time < move.startTime + move.duration ) {
			goalPos = move.moveDest - move.moveDir * MS2SEC( move.startTime + move.duration - gameLocal.time );
			delta = goalPos - oldorigin;
			delta.z = 0.0f;
		} else {
			delta = move.moveDest - oldorigin;
			delta.z = 0.0f;
			StopMove(MOVE_STATUS_DONE);
		}
	}
	else if (allowMove)
	{
		// Moving is allowed, get the delta
		GetMoveDelta( oldaxis, viewAxis, delta );
	}

	if ( move.moveCommand == MOVE_TO_POSITION ) {
		goalDelta = move.moveDest - oldorigin;
		float goalDist = goalDelta.LengthFast();
		if ( goalDist < delta.LengthFast() ) {
			delta = goalDelta;
		}
	}

	physicsObj.SetDelta( delta );
	physicsObj.ForceDeltaMove( disableGravity );

	{
		START_SCOPED_TIMING(aiPhysicsTimer, scopedPhysicsTimer);
		RunPhysics();
	}

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugLine( colorCyan, oldorigin, physicsObj.GetOrigin(), 5000 );
	}

	monsterMoveResult_t moveResult = physicsObj.GetMoveResult();
	if ( !m_bAFPushMoveables && attack.Length() && TestMelee() ) {
		DirectDamage( attack, enemy.GetEntity() );
	} else {
		idEntity *blockEnt = physicsObj.GetSlideMoveEntity();
		if ( blockEnt)
		{
			if (blockEnt->IsType( idMoveable::Type ) && blockEnt->GetPhysics()->IsPushable() )
			{
				KickObstacles( viewAxis[ 0 ], kickForce, blockEnt );
			}
		}
	}

	BlockedFailSafe();

	AI_ONGROUND = physicsObj.OnGround();

	const idVec3& org = physicsObj.GetOrigin();
	if (oldorigin != org) {
		TouchTriggers();
	}

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), move.moveDest, gameLocal.msec );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
		DrawRoute();
	}
}

/*
=====================
idAI::SittingMove
=====================
*/
void idAI::SittingMove()
{
	idVec3				goalPos;
	idVec3				goalDelta;
	monsterMoveResult_t	moveResult;
	idVec3				newDest;

	idVec3 oldorigin = physicsObj.GetOrigin();
	idMat3 oldaxis = viewAxis;

	AI_BLOCKED = false;

	RunPhysics();

	Turn(sitting_turn_pivot);

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugLine( colorCyan, oldorigin, physicsObj.GetOrigin(), 5000 );
	}

	moveResult = physicsObj.GetMoveResult();

	AI_ONGROUND = physicsObj.OnGround();

	const idVec3& org = physicsObj.GetOrigin();
	if (oldorigin != org) {
		TouchTriggers();
	}

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), move.moveDest, gameLocal.msec );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
		DrawRoute();
	}
}

void idAI::NoTurnMove()
{
	idVec3 oldorigin = physicsObj.GetOrigin();
	idMat3 oldaxis = viewAxis;

	AI_BLOCKED = false;

	RunPhysics();

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugLine( colorCyan, oldorigin, physicsObj.GetOrigin(), 5000 );
	}


	AI_ONGROUND = physicsObj.OnGround();

	const idVec3& org = physicsObj.GetOrigin();
	if (oldorigin != org) {
		TouchTriggers();
	}

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), move.moveDest, gameLocal.msec );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
		DrawRoute();
	}

}



void idAI::LayDownMove()
{
	idVec3 oldorigin(physicsObj.GetOrigin());
	idMat3 oldaxis(viewAxis);

	AI_BLOCKED = false;

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugLine( colorCyan, oldorigin, physicsObj.GetOrigin(), 5000 );
	}

	AI_ONGROUND = physicsObj.OnGround();

	// angua: Let the animation move the origin onto the bed
	idVec3 delta;
	GetMoveDelta( oldaxis, viewAxis, delta );

	physicsObj.SetDelta( delta );
	physicsObj.ForceDeltaMove( disableGravity );

	RunPhysics();

	const idVec3& org = physicsObj.GetOrigin();
	if (oldorigin != org) {
		TouchTriggers();
	}

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), move.moveDest, gameLocal.msec );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
		DrawRoute();
	}
}


/*
=====================
Seek
=====================
*/
idVec3 Seek( idVec3 &vel, const idVec3 &org, const idVec3 &goal, float prediction ) {
	idVec3 predictedPos;
	idVec3 goalDelta;
	idVec3 seekVel;

	// predict our position
	predictedPos = org + vel * prediction;
	goalDelta = goal - predictedPos;
	seekVel = goalDelta * MS2SEC( gameLocal.msec );

	return seekVel;
}

/*
=====================
idAI::SlideMove
=====================
*/
void idAI::SlideMove( void ) {
	idVec3				goalPos;
	idVec3				delta;
	idVec3				goalDelta;
	float				goalDist;
	monsterMoveResult_t	moveResult;
	idVec3				newDest;

	idVec3 oldorigin = physicsObj.GetOrigin();
	idMat3 oldaxis = viewAxis;

	AI_BLOCKED = false;

	if ( move.moveCommand < NUM_NONMOVING_COMMANDS ){
		move.lastMoveOrigin.Zero();
		move.lastMoveTime = gameLocal.time;
	}

	move.obstacle = NULL;
	if ( ( move.moveCommand == MOVE_FACE_ENEMY ) && enemy.GetEntity() ) {
		TurnToward( lastVisibleEnemyPos );
		goalPos = move.moveDest;
	} else if ( ( move.moveCommand == MOVE_FACE_ENTITY ) && move.goalEntity.GetEntity() ) {
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
		goalPos = move.moveDest;
	} else if ( GetMovePos( goalPos ) ) {
		CheckObstacleAvoidance( goalPos, newDest );
		TurnToward( newDest );
		goalPos = newDest;
	}

	if ( move.moveCommand == MOVE_SLIDE_TO_POSITION ) {
		if ( gameLocal.time < move.startTime + move.duration ) {
			goalPos = move.moveDest - move.moveDir * MS2SEC( move.startTime + move.duration - gameLocal.time );
		} else {
			goalPos = move.moveDest;
			StopMove( MOVE_STATUS_DONE );
		}
	}

	if ( move.moveCommand == MOVE_TO_POSITION ) {
		goalDelta = move.moveDest - oldorigin;
		goalDist = goalDelta.LengthFast();
		if ( goalDist < delta.LengthFast() ) {
			delta = goalDelta;
		}
	}

	idVec3 vel = physicsObj.GetLinearVelocity();
	float z = vel.z;
	idVec3  predictedPos = oldorigin + vel * AI_SEEK_PREDICTION;

	// seek the goal position
	goalDelta = goalPos - predictedPos;
	vel -= vel * AI_FLY_DAMPENING * MS2SEC( gameLocal.msec );
	vel += goalDelta * MS2SEC( gameLocal.msec );

	// cap our speed
	vel.Truncate( fly_speed );
	vel.z = z;
	physicsObj.SetLinearVelocity( vel );
	physicsObj.UseVelocityMove( true );
	RunPhysics();

	if ( ( move.moveCommand == MOVE_FACE_ENEMY ) && enemy.GetEntity() ) {
		TurnToward( lastVisibleEnemyPos );
	} else if ( ( move.moveCommand == MOVE_FACE_ENTITY ) && move.goalEntity.GetEntity() ) {
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
	} else if ( move.moveCommand != MOVE_NONE ) {
		if ( vel.ToVec2().LengthSqr() > 0.1f ) {
			TurnToward( vel.ToYaw() );
		}
	}

	Turn();

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugLine( colorCyan, oldorigin, physicsObj.GetOrigin(), 5000 );
	}

	moveResult = physicsObj.GetMoveResult();
	if ( !m_bAFPushMoveables && attack.Length() && TestMelee() ) {
		DirectDamage( attack, enemy.GetEntity() );
	} else {
		idEntity *blockEnt = physicsObj.GetSlideMoveEntity();
		if ( blockEnt && blockEnt->IsType( idMoveable::Type ) && blockEnt->GetPhysics()->IsPushable() ) {
			KickObstacles( viewAxis[ 0 ], kickForce, blockEnt );
		}
	}

	BlockedFailSafe();

	AI_ONGROUND = physicsObj.OnGround();

	idVec3 org = physicsObj.GetOrigin();
	if ( oldorigin != org ) {
		TouchTriggers();
	}

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), move.moveDest, gameLocal.msec );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
		DrawRoute();
	}
}

/*
=====================
idAI::AdjustFlyingAngles
=====================
*/
void idAI::AdjustFlyingAngles( void ) {
	idVec3	vel;
	float 	speed;
	float 	roll;
	float 	pitch;

	vel = physicsObj.GetLinearVelocity();

	speed = vel.Length();
	if ( speed < 5.0f ) {
		roll = 0.0f;
		pitch = 0.0f;
	} else {
		roll = vel * viewAxis[ 1 ] * -fly_roll_scale / fly_speed;
		if ( roll > fly_roll_max ) {
			roll = fly_roll_max;
		} else if ( roll < -fly_roll_max ) {
			roll = -fly_roll_max;
		}

		pitch = vel * viewAxis[ 2 ] * -fly_pitch_scale / fly_speed;
		if ( pitch > fly_pitch_max ) {
			pitch = fly_pitch_max;
		} else if ( pitch < -fly_pitch_max ) {
			pitch = -fly_pitch_max;
		}
	}

	fly_roll = fly_roll * 0.95f + roll * 0.05f;
	fly_pitch = fly_pitch * 0.95f + pitch * 0.05f;

	if ( flyTiltJoint != INVALID_JOINT ) {
		animator.SetJointAxis( flyTiltJoint, JOINTMOD_WORLD, idAngles( fly_pitch, 0.0f, fly_roll ).ToMat3() );
	} else {
		viewAxis = idAngles( fly_pitch, current_yaw, fly_roll ).ToMat3();
	}
}

/*
=====================
idAI::AddFlyBob
=====================
*/
void idAI::AddFlyBob( idVec3 &vel ) {
	idVec3	fly_bob_add;
	float	t;

	if ( fly_bob_strength ) {
		t = MS2SEC( gameLocal.time + entityNumber * 497 );
		fly_bob_add = ( viewAxis[ 1 ] * idMath::Sin16( t * fly_bob_horz ) + viewAxis[ 2 ] * idMath::Sin16( t * fly_bob_vert ) ) * fly_bob_strength;
		vel += fly_bob_add * MS2SEC( gameLocal.msec );
		if ( ai_debugMove.GetBool() ) {
			const idVec3 &origin = physicsObj.GetOrigin();
			gameRenderWorld->DebugArrow( colorOrange, origin, origin + fly_bob_add, 0 );
		}
	}
}

/*
=====================
idAI::AdjustFlyHeight
=====================
*/
void idAI::AdjustFlyHeight( idVec3 &vel, const idVec3 &goalPos ) {
	const idVec3	&origin = physicsObj.GetOrigin();
	predictedPath_t path;
	idVec3			end;
	idVec3			dest;
	trace_t			trace;
	idActor			*enemyEnt;
	bool			goLower;

	// make sure we're not flying too high to get through doors
	goLower = false;
	if ( origin.z > goalPos.z ) {
		dest = goalPos;
		dest.z = origin.z + 128.0f;
		idAI::PredictPath( this, aas, goalPos, dest - origin, 1000, 1000, SE_BLOCKED, path );
		if ( path.endPos.z < origin.z ) {
			idVec3 addVel = Seek( vel, origin, path.endPos, AI_SEEK_PREDICTION );
			vel.z += addVel.z;
			goLower = true;
		}

		if ( ai_debugMove.GetBool() ) {
			gameRenderWorld->DebugBounds( goLower ? colorRed : colorGreen, physicsObj.GetBounds(), path.endPos, gameLocal.msec );
		}
	}

	if ( !goLower ) {
		// make sure we don't fly too low
		end = origin;

		enemyEnt = enemy.GetEntity();
		if ( enemyEnt ) {
			end.z = lastVisibleEnemyPos.z + lastVisibleEnemyEyeOffset.z + fly_offset;
		} else {
			// just use the default eye height for the player
			end.z = goalPos.z + DEFAULT_FLY_OFFSET + fly_offset;
		}

		gameLocal.clip.Translation( trace, origin, end, physicsObj.GetClipModel(), mat3_identity, MASK_MONSTERSOLID, this );
		vel += Seek( vel, origin, trace.endpos, AI_SEEK_PREDICTION );
	}
}

/*
=====================
idAI::FlySeekGoal
=====================
*/
void idAI::FlySeekGoal( idVec3 &vel, idVec3 &goalPos ) {
	idVec3 seekVel;

	// seek the goal position
	seekVel = Seek( vel, physicsObj.GetOrigin(), goalPos, AI_SEEK_PREDICTION );
	seekVel *= fly_seek_scale;
	vel += seekVel;
}

/*
=====================
idAI::AdjustFlySpeed
=====================
*/
void idAI::AdjustFlySpeed( idVec3 &vel ) {
	float speed;

	// apply dampening
	vel -= vel * AI_FLY_DAMPENING * MS2SEC( gameLocal.msec );

	// gradually speed up/slow down to desired speed
	speed = vel.Normalize();
	speed += ( move.speed - speed ) * MS2SEC( gameLocal.msec );
	if ( speed < 0.0f ) {
		speed = 0.0f;
	} else if ( move.speed && ( speed > move.speed ) ) {
		speed = move.speed;
	}

	vel *= speed;
}

/*
=====================
idAI::FlyTurn
=====================
*/
void idAI::FlyTurn( void ) {
	if ( move.moveCommand == MOVE_FACE_ENEMY ) {
		TurnToward( lastVisibleEnemyPos );
	} else if ( ( move.moveCommand == MOVE_FACE_ENTITY ) && move.goalEntity.GetEntity() ) {
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
	} else if ( move.speed > 0.0f ) {
		const idVec3 &vel = physicsObj.GetLinearVelocity();
		if ( vel.ToVec2().LengthSqr() > 0.1f ) {
			TurnToward( vel.ToYaw() );
		}
	}
	Turn();
}

/*
=====================
idAI::FlyMove
=====================
*/
void idAI::FlyMove( void ) {
	idVec3	goalPos;
	idVec3	oldorigin;
	idVec3	newDest;

	AI_BLOCKED = false;
	if ( ( move.moveCommand != MOVE_NONE ) && ReachedPos( move.moveDest, move.moveCommand ) ) {
		StopMove( MOVE_STATUS_DONE );
	}

	if ( ai_debugMove.GetBool() ) {
		gameLocal.Printf( "%d: %s: %s, vel = %.2f, sp = %.2f, maxsp = %.2f\n", gameLocal.time, name.c_str(), moveCommandString[ move.moveCommand ], physicsObj.GetLinearVelocity().Length(), move.speed, fly_speed );
	}

	if ( move.moveCommand != MOVE_TO_POSITION_DIRECT ) {
		idVec3 vel = physicsObj.GetLinearVelocity();

		if ( GetMovePos( goalPos ) ) {
			CheckObstacleAvoidance( goalPos, newDest );
			goalPos = newDest;
		}

		if ( move.speed	) {
			FlySeekGoal( vel, goalPos );
		}

		// add in bobbing
		AddFlyBob( vel );

		if ( enemy.GetEntity() && ( move.moveCommand != MOVE_TO_POSITION ) ) {
			AdjustFlyHeight( vel, goalPos );
		}

		AdjustFlySpeed( vel );

		physicsObj.SetLinearVelocity( vel );
	}

	// turn
	FlyTurn();

	// run the physics for this frame
	oldorigin = physicsObj.GetOrigin();
	physicsObj.UseFlyMove( true );
	physicsObj.UseVelocityMove( false );
	physicsObj.SetDelta( vec3_zero );
	physicsObj.ForceDeltaMove( disableGravity );
	RunPhysics();

	monsterMoveResult_t	moveResult = physicsObj.GetMoveResult();
	if ( !m_bAFPushMoveables && attack.Length() && TestMelee() ) {
		DirectDamage( attack, enemy.GetEntity() );
	} else {
		idEntity *blockEnt = physicsObj.GetSlideMoveEntity();
		if ( blockEnt && blockEnt->IsType( idMoveable::Type ) && blockEnt->GetPhysics()->IsPushable() ) {
			KickObstacles( viewAxis[ 0 ], kickForce, blockEnt );
		} else if ( moveResult == MM_BLOCKED ) {
			move.blockTime = gameLocal.time + 500;
			AI_BLOCKED = true;
		}
	}

	idVec3 org = physicsObj.GetOrigin();
	if ( oldorigin != org ) {
		TouchTriggers();
	}

	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugLine( colorCyan, oldorigin, physicsObj.GetOrigin(), 4000 );
		gameRenderWorld->DebugBounds( colorOrange, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), move.moveDest, gameLocal.msec );
		gameRenderWorld->DebugLine( colorRed, org, org + physicsObj.GetLinearVelocity(), gameLocal.msec, true );
		gameRenderWorld->DebugLine( colorBlue, org, goalPos, gameLocal.msec, true );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
		DrawRoute();
	}
}

/*
=====================
idAI::StaticMove
=====================
*/
void idAI::StaticMove( void ) {
	idActor	*enemyEnt = enemy.GetEntity();

	if ( AI_DEAD || AI_KNOCKEDOUT ) {
		return;
	}

	if ( ( move.moveCommand == MOVE_FACE_ENEMY ) && enemyEnt ) {
		TurnToward( lastVisibleEnemyPos );
	} else if ( ( move.moveCommand == MOVE_FACE_ENTITY ) && move.goalEntity.GetEntity() ) {
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
	} else if ( move.moveCommand != MOVE_NONE ) {
		TurnToward( move.moveDest );
	}
	Turn();

	physicsObj.ForceDeltaMove( true ); // disable gravity
	RunPhysics();

	AI_ONGROUND = false;

	if ( !m_bAFPushMoveables && attack.Length() && TestMelee() ) {
		DirectDamage( attack, enemyEnt );
	}

	if ( ai_debugMove.GetBool() ) {
		const idVec3 &org = physicsObj.GetOrigin();
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugLine( colorBlue, org, move.moveDest, gameLocal.msec, true );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
	}
}

void idAI::Bark(const idStr& soundName)
{

	// Allocate a singlebarktask with the given sound and enqueue it
	commSubsystem->AddCommTask(
		ai::CommunicationTaskPtr(new ai::SingleBarkTask(soundName))
	);
}

void idAI::PlayFootStepSound()
{
	idStr				moveType, localSound, sound;
	idMaterial			*material = NULL;
	const idSoundShader	*sndShader = NULL;
	
	if ( !GetPhysics()->HasGroundContacts() ) {
		return;
	}

	// greebo: Don't play footsteps in ragdoll mode
	if (!GetPhysics()->IsType(idPhysics_Actor::Type))
	{
		return;
	}

	// DarkMod: make the string to identify the movement speed (crouch_run, creep, etc)
	// Currently only players have movement flags set up this way, not AI.  We could change that later.
	moveType.Clear();

	if (AI_CROUCH)
	{
		moveType = "_crouch";
	}

	if (AI_RUN)
	{
		moveType += "_run";
	}
	else if (AI_CREEP)
	{
		moveType += "_creep";
	}
	else
	{
		moveType += "_walk";
	}

	// start footstep sound based on material type
	material = const_cast<idMaterial*>(GetPhysics()->GetContact(0).material);
	if (material != NULL) 
	{
		DM_LOG(LC_SOUND,LT_DEBUG)LOGSTRING("AI %s stepped on entity %s, material %s \r", name.c_str(), gameLocal.entities[GetPhysics()->GetContact( 0 ).entityNum]->name.c_str(), material->GetName() );  
		g_Global.GetSurfName(material, localSound);
		localSound = "snd_footstep_" + localSound;

		DM_LOG(LC_SOUND,LT_DEBUG)LOGSTRING("Found surface type sound: %s\r", localSound.c_str() ); 
		sound = spawnArgs.GetString( localSound.c_str() );
	}

	waterLevel_t waterLevel = static_cast<idPhysics_Actor *>(GetPhysics())->GetWaterLevel();
	// If player is walking in liquid, replace the bottom surface sound with water sounds
	if (waterLevel == WATERLEVEL_FEET )
	{
		localSound = "snd_footstep_puddle";
		sound = spawnArgs.GetString( localSound.c_str() );
	}
	else if (waterLevel == WATERLEVEL_WAIST)
	{
		localSound = "snd_footstep_wading";
		sound = spawnArgs.GetString( localSound.c_str() );
	}
	// greebo: Added this to disable the walking sound when completely underwater
	// this should be replaced by snd_
	else if (waterLevel == WATERLEVEL_HEAD)
	{
		localSound = "snd_footstep_swim";
		sound = spawnArgs.GetString( localSound.c_str() );
	}

	if ( sound.IsEmpty() && waterLevel != WATERLEVEL_HEAD ) 
	{
		localSound = "snd_footstep";
	}
	
	sound = spawnArgs.GetString( localSound.c_str() );

	// if a sound was not found for that specific material, use default
	if( sound.IsEmpty() && waterLevel != WATERLEVEL_HEAD )
	{
		sound = spawnArgs.GetString( "snd_footstep" );
		localSound = "snd_footstep";
	}

	/***
	* AI footsteps always propagate as snd_footstep for now
	* If we want to add in AI soundprop based on movement speed later,
	*	here is the place to do it.
	**/
/*
	localSound += moveType;
	if( !gameLocal.m_sndProp->CheckSound( localSound.c_str(), false ) )
		localSound -= moveType;
*/

	if ( !sound.IsEmpty() ) 
	{
		// apply the movement type modifier to the volume
		sndShader = declManager->FindSound( sound.c_str() );
		SetSoundVolume( sndShader->GetParms()->volume + GetMovementVolMod() );
		StartSoundShader( sndShader, SND_CHANNEL_BODY, 0, false, NULL );
		SetSoundVolume( 0.0f );

		// propagate the suspicious sound to other AI
		PropSoundDirect( localSound, true, false );
	}
}

/***********************************************************************

	Damage

***********************************************************************/

/*
=====================
idAI::ReactionTo

DarkMod : Added call to AI Relationship Manager
		  We don't have to hardcode this, could be done
		  with scripts, but for now it's hardcoded for
		  testing purposes.
=====================
*/
int idAI::ReactionTo( const idEntity *ent )
{
	if ( ent->fl.hidden )
	{
		// ignore hidden entities
		return ATTACK_IGNORE;
	}

	if ( !ent->IsType( idActor::Type ) ) {
		return ATTACK_IGNORE;
	}

	const idActor *actor = static_cast<const idActor *>( ent );
	if ( actor->IsType( idPlayer::Type ) && static_cast<const idPlayer *>(actor)->noclip ) {
		// ignore players in noclip mode
		return ATTACK_IGNORE;
	}

	// actors will always fight if their teams are enemies
	if ( IsEnemy(actor) )
	{
		if ( actor->fl.notarget )
		{
			// don't attack on sight when attacker is notargeted
			return ATTACK_ON_DAMAGE | ATTACK_ON_ACTIVATE;
		}
		return ATTACK_ON_SIGHT | ATTACK_ON_DAMAGE | ATTACK_ON_ACTIVATE;
	}

	// monsters will fight when attacked by lower ranked monsters.  rank 0 never fights back.
	if ( rank && ( actor->rank < rank ) ) {
		return ATTACK_ON_DAMAGE;
	}

	// don't fight back
	return ATTACK_IGNORE;
}


/*
=====================
idAI::Pain
=====================
*/
bool idAI::Pain( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location, const idDict* damageDef )
{
	AI_PAIN = idActor::Pain( inflictor, attacker, damage, dir, location, damageDef );
	AI_DAMAGE = true;

	// force a blink
	blink_time = 0;

	// ignore damage from self
	if ( attacker != this )
	{
		if ( inflictor )
		{
			AI_SPECIAL_DAMAGE = inflictor->spawnArgs.GetInt( "special_damage" );
		} else
		{
			AI_SPECIAL_DAMAGE = 0;
		}

		// AI don't like being attacked
		ChangeEntityRelation(attacker, -10);

		// Switch to pain state if idle
		if (AI_AlertIndex == 0 && damage > 0 && 
			(damageDef == NULL || !damageDef->GetBool("no_pain_anim", "0")))
		{
			GetMind()->PushState(ai::StatePtr(new ai::PainState));
		}
	}

	return ( AI_PAIN != 0 );
}


/*
=====================
idAI::SpawnParticles
=====================
*/
void idAI::SpawnParticles( const char *keyName ) {
	const idKeyValue *kv = spawnArgs.MatchPrefix( keyName, NULL );
	while ( kv ) {
		particleEmitter_t pe;

		idStr particleName = kv->GetValue();

		if ( particleName.Length() ) {

			idStr jointName = kv->GetValue();
			int dash = jointName.Find('-');
			if ( dash > 0 ) {
				particleName = particleName.Left( dash );
				jointName = jointName.Right( jointName.Length() - dash - 1 );
			}

			SpawnParticlesOnJoint( pe, particleName, jointName );
			particles.Append( pe );
		}

		kv = spawnArgs.MatchPrefix( keyName, kv );
	}
}

/*
=====================
idAI::SpawnParticlesOnJoint
=====================
*/
const idDeclParticle *idAI::SpawnParticlesOnJoint( particleEmitter_t &pe, const char *particleName, const char *jointName ) {
	idVec3 origin;
	idMat3 axis;

	if ( *particleName == '\0' ) {
		memset( &pe, 0, sizeof( pe ) );
		return pe.particle;
	}

	pe.joint = animator.GetJointHandle( jointName );
	if ( pe.joint == INVALID_JOINT ) {
		gameLocal.Warning( "Unknown particleJoint '%s' on '%s'", jointName, name.c_str() );
		pe.time = 0;
		pe.particle = NULL;
	} else {
		animator.GetJointTransform( pe.joint, gameLocal.time, origin, axis );
		origin = renderEntity.origin + origin * renderEntity.axis;

		BecomeActive( TH_UPDATEPARTICLES );
		if ( !gameLocal.time ) {
			// particles with time of 0 don't show, so set the time differently on the first frame
			pe.time = 1;
		} else {
			pe.time = gameLocal.time;
		}
		pe.particle = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, particleName ) );
		gameLocal.smokeParticles->EmitSmoke( pe.particle, pe.time, gameLocal.random.CRandomFloat(), origin, axis );
	}

	return pe.particle;
}

/*
=====================
idAI::Killed
=====================
*/
void idAI::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location )
{
	idAngles ang;
	const char *modelDeath;
	bool bPlayerResponsible(false);

	// make sure the monster is activated
	EndAttack();

	if ( g_debugDamage.GetBool() ) {
		gameLocal.Printf( "Damage: joint: '%s', zone '%s'\n", animator.GetJointName( ( jointHandle_t )location ),
			GetDamageGroup( location ) );
	}

	if ( inflictor ) {
		AI_SPECIAL_DAMAGE = inflictor->spawnArgs.GetInt( "special_damage" );
	} else {
		AI_SPECIAL_DAMAGE = 0;
	}

	if ( AI_DEAD ) {
		AI_PAIN = true;
		AI_DAMAGE = true;
		return;
	}

	// stop all voice sounds
	StopSound( SND_CHANNEL_VOICE, false );
	if ( head.GetEntity() ) {
		head.GetEntity()->StopSound( SND_CHANNEL_VOICE, false );
		head.GetEntity()->GetAnimator()->ClearAllAnims( gameLocal.time, 100 );
	}

	disableGravity = false;
	move.moveType = MOVETYPE_DEAD;
	m_bAFPushMoveables = false;

	physicsObj.UseFlyMove( false );
	physicsObj.ForceDeltaMove( false );

	// end our looping ambient sound
	StopSound( SND_CHANNEL_AMBIENT, false );

	/* greebo: This is not needed anymore, I reckon.
	if ( attacker && attacker->IsType( idActor::Type ) ) {
		gameLocal.AlertAI( ( idActor * )attacker );
	}*/

	// activate targets
	ActivateTargets( attacker );

	RemoveAttachments();
	RemoveProjectile();
	StopMove( MOVE_STATUS_DONE );

	ClearEnemy();
	AI_DEAD	= true;

	// make monster nonsolid
	if (spawnArgs.GetBool("nonsolid_on_ragdoll", "1"))
	{
		physicsObj.SetContents(0);
		physicsObj.GetClipModel()->Unlink();
	}

	Unbind();

	idStr deathSound = MouthIsUnderwater() ? "snd_death_liquid" : "snd_death";

	StartSound( deathSound.c_str(), SND_CHANNEL_VOICE, 0, false, NULL );

	// Go to ragdoll mode immediately, if we don't have a death anim
	// If death anims are enabled, we need to wait with going to ragdoll until PostKilled()
	if (!spawnArgs.GetBool("enable_death_anim", "0"))
	{
		StartRagdoll();
	}

	// swaps the head CM back if a different one was swapped in while conscious
	SwapHeadAFCM( false );

	if ( spawnArgs.GetString( "model_death", "", &modelDeath ) ) {
		// lost soul is only case that does not use a ragdoll and has a model_death so get the death sound in here
		StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
		renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
		SetModel( modelDeath );
		physicsObj.SetLinearVelocity( vec3_zero );
		physicsObj.PutToRest();
		physicsObj.DisableImpact();
	}

	if (spawnArgs.GetBool("set_frobable_on_death", "1"))
	{
		// AI becomes frobable on death
		// greebo: Add a delay before the AI becomes actually frobable
		PostEventMS(&EV_SetFrobable, 750, 1);
	}
	
	restartParticles = false;

	mind->ClearStates();
	mind->PushState(STATE_DEAD);

	// drop items
	DropOnRagdoll();

	if ( attacker && (attacker == gameLocal.GetLocalPlayer()) && inflictor)
	{
		bPlayerResponsible = true;
	}
	else if( attacker && attacker->m_SetInMotionByActor.GetEntity() )
	{
		bPlayerResponsible = (attacker != gameLocal.world &&
			attacker->m_SetInMotionByActor.GetEntity() == gameLocal.GetLocalPlayer());
	}

	// Update TDM objective system
	gameLocal.m_MissionData->MissionEvent( COMP_KILL, this, attacker, bPlayerResponsible );
	// Mark the body as last moved by the player
	if( bPlayerResponsible )
		m_MovedByActor = gameLocal.GetLocalPlayer();

	// greebo: Set the lipsync flag to FALSE, we're dead
	m_lipSyncActive = false;

	DropBlood(inflictor);
}

void idAI::Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir, 
	const char *damageDefName, const float damageScale, const int location,
	trace_t *collision)
{
	// Save the current health, to see afterwards how much damage we've been taking
	int preHitHealth = health;

	idActor::Damage(inflictor, attacker, dir, damageDefName, damageScale, location, collision);

	if (inflictor != NULL && inflictor->IsType(idProjectile::Type))
	{
		int damageTaken = preHitHealth - health;

		// Send a signal to the current state that we've been hit by something
		GetMind()->GetState()->OnProjectileHit(static_cast<idProjectile*>(inflictor), attacker, damageTaken);
	}

	if (attacker != NULL && IsEnemy(attacker))
	{
		GetMemory().hasBeenAttackedByEnemy = true;
	}
}

void idAI::DropBlood(idEntity *inflictor)
{
	if (inflictor)
	{
		idStr damageDefName = inflictor->spawnArgs.RandomPrefix("def_damage", gameLocal.random);

		const idDeclEntityDef *def = gameLocal.FindEntityDef(damageDefName, false);
		if ( def == NULL ) 
		{
			return;
		}

		// blood splats are thrown onto nearby surfaces
		idStr splat = def->dict.RandomPrefix("mtr_killed_splat", gameLocal.random);

		if (!splat.IsEmpty()) 
		{
			SpawnBloodMarker(splat, splat + "_fading", 40);
		}
	}
}


void idAI::SpawnBloodMarker(const idStr& splat, const idStr& splatFading, float size)
{
	trace_t result;
	gameLocal.clip.TracePoint(result, GetPhysics()->GetOrigin(), GetPhysics()->GetOrigin() + 60 * idVec3(0, 0, -1), MASK_OPAQUE, this);

	idVec3 markerOrigin = result.endpos + idVec3(0, 0, 2);

	const idDict* markerDef = gameLocal.FindEntityDefDict("atdm:blood_marker", false);

	if (markerDef == NULL)
	{
		gameLocal.Error( "Failed to find definition of blood marker entity " );
		return;
	}

	idEntity* ent;
	gameLocal.SpawnEntityDef(*markerDef, &ent, false);

	if (!ent || !ent->IsType(CBloodMarker::Type)) 
	{
		gameLocal.Error( "Failed to spawn blood marker entity" );
		return;
	}

	CBloodMarker* bloodMarker = static_cast<CBloodMarker*>(ent);
	bloodMarker->SetOrigin(markerOrigin);
	bloodMarker->Init(splat, splatFading, size);
	bloodMarker->Event_GenerateBloodSplat();
}


/*
=====================
idAI::PostDeath
=====================
*/
void idAI::PostDeath()
{
	// For death anims, we need to wait with going to ragdoll until here
	if (spawnArgs.GetBool("enable_death_anim", "0"))
	{
		// Start going to ragdoll here, instead of in Killed() to enable death anims
		StartRagdoll();
	}

	headAnim.StopAnim(1);
	legsAnim.StopAnim(1);
	torsoAnim.StopAnim(1);

	headAnim.Disable();
	legsAnim.Disable();
	torsoAnim.Disable();
}

/***********************************************************************

	Targeting/Combat

***********************************************************************/

/*
=====================
idAI::PlayCinematic
=====================
*/
void idAI::PlayCinematic( void ) {
	const char *animname;

	if ( current_cinematic >= num_cinematics ) {
		if ( g_debugCinematic.GetBool() ) {
			gameLocal.Printf( "%d: '%s' stop\n", gameLocal.framenum, GetName() );
		}
		if ( !spawnArgs.GetBool( "cinematic_no_hide" ) ) {
			Hide();
		}
		current_cinematic = 0;
		ActivateTargets( gameLocal.GetLocalPlayer() );
		fl.neverDormant = false;

		return;
	}

	Show();
	current_cinematic++;

	allowJointMod = false;
	allowEyeFocus = false;

	spawnArgs.GetString( va( "anim%d", current_cinematic ), NULL, &animname );
	if ( !animname ) {
		gameLocal.Warning( "missing 'anim%d' key on %s", current_cinematic, name.c_str() );
		return;
	}

	if ( g_debugCinematic.GetBool() ) {
		gameLocal.Printf( "%d: '%s' start '%s'\n", gameLocal.framenum, GetName(), animname );
	}

	headAnim.animBlendFrames = 0;
	headAnim.lastAnimBlendFrames = 0;
	headAnim.BecomeIdle();

	legsAnim.animBlendFrames = 0;
	legsAnim.lastAnimBlendFrames = 0;
	legsAnim.BecomeIdle();

	torsoAnim.animBlendFrames = 0;
	torsoAnim.lastAnimBlendFrames = 0;
	ProcessEvent( &AI_PlayAnim, ANIMCHANNEL_TORSO, animname );

	// make sure our model gets updated
	animator.ForceUpdate();

	// update the anim bounds
	UpdateAnimation();
	UpdateVisuals();
	Present();

	if ( head.GetEntity() ) {
		// since the body anim was updated, we need to run physics to update the position of the head
		RunPhysics();

		// make sure our model gets updated
		head.GetEntity()->GetAnimator()->ForceUpdate();

		// update the anim bounds
		head.GetEntity()->UpdateAnimation();
		head.GetEntity()->UpdateVisuals();
		head.GetEntity()->Present();
	}


	fl.neverDormant = true;

}

/*
=====================
idAI::Activate

Notifies the script that a monster has been activated by a trigger or flashlight

DarkMod: Commented out calls to SetEnemy.  We don't want the AI calling setEnemy,
only the alert state scripts.
=====================
*/
void idAI::Activate( idEntity *activator )
{
	// Fire the TRIGGER response
	TriggerResponse(activator, ST_TRIGGER);

	if ( AI_DEAD || AI_KNOCKEDOUT ) {
		// ignore it when they're dead or KO'd
		return;
	}

	// make sure he's not dormant
	dormantStart = 0;

	if ( num_cinematics ) {
		PlayCinematic();
	} else {
		AI_ACTIVATED = true;

		/*idPlayer *player;
		if ( !activator || !activator->IsType( idPlayer::Type ) ) {
			player = gameLocal.GetLocalPlayer();
		} else {
			player = static_cast<idPlayer *>( activator );
		}

		if ( ReactionTo( player ) & ATTACK_ON_ACTIVATE )
		{
			//SetEnemy( player );
		}*/

		// update the script in cinematics so that entities don't start anims or show themselves a frame late.
		if ( cinematic ) {
            UpdateScript();

			// make sure our model gets updated
			animator.ForceUpdate();

			// update the anim bounds
			UpdateAnimation();
			UpdateVisuals();
			Present();

			if ( head.GetEntity() ) {
				// since the body anim was updated, we need to run physics to update the position of the head
				RunPhysics();

				// make sure our model gets updated
				head.GetEntity()->GetAnimator()->ForceUpdate();

				// update the anim bounds
				head.GetEntity()->UpdateAnimation();
				head.GetEntity()->UpdateVisuals();
				head.GetEntity()->Present();
			}
		}
	}
}

/*
=====================
idAI::EnemyDead
=====================
*/
void idAI::EnemyDead( void ) {
	ClearEnemy();
}

/*
=====================
idAI::TalkTo
=====================
*/
void idAI::TalkTo( idActor *actor ) {
	if ( talk_state != TALK_OK ) {
		return;
	}

	talkTarget = actor;
	if ( actor ) {
		AI_TALK = true;
	} else {
		AI_TALK = false;
	}
}

/*
=====================
idAI::GetEnemy
=====================
*/
idActor	*idAI::GetEnemy( void ) const {
	return enemy.GetEntity();
}

/*
=====================
idAI::GetTalkState
=====================
*/
talkState_t idAI::GetTalkState( void ) const {
	if ( ( talk_state != TALK_NEVER ) && AI_DEAD ) {
		return TALK_DEAD;
	}
	if ( IsHidden() ) {
		return TALK_NEVER;
	}
	return talk_state;
}

/*
=====================
idAI::TouchedByFlashlight
=====================
*/
void idAI::TouchedByFlashlight( idActor *flashlight_owner ) {
	if ( wakeOnFlashlight ) {
		Activate( flashlight_owner );
	}
}

/*
=====================
idAI::ClearEnemy
=====================
*/
void idAI::ClearEnemy( void ) {
	if ( move.moveCommand == MOVE_TO_ENEMY ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
	}

	enemyNode.Remove();
	enemy				= NULL;
	AI_ENEMY_IN_FOV		= false;
	AI_ENEMY_VISIBLE	= false;
}

/*
=====================
idAI::EnemyPositionValid
=====================
*/
bool idAI::EnemyPositionValid( void ) const {
	trace_t	tr;
	idVec3	muzzle;
	idMat3	axis;

	if ( !enemy.GetEntity() ) {
		return false;
	}

	if ( AI_ENEMY_VISIBLE ) {
		return true;
	}

	gameLocal.clip.TracePoint( tr, GetEyePosition(), lastVisibleEnemyPos + lastVisibleEnemyEyeOffset, MASK_OPAQUE, this );
	if ( tr.fraction < 1.0f ) {
		// can't see the area yet, so don't know if he's there or not
		return true;
	}

	return false;
}

/*
=====================
idAI::SetEnemyPosition
=====================
*/
void idAI::SetEnemyPosition()
{
	idActor* enemyEnt = enemy.GetEntity();

	if (enemyEnt == NULL)
	{
		return;
	}

	int			enemyAreaNum;
	int			areaNum;
	int			lastVisibleReachableEnemyAreaNum = -1;
	aasPath_t	path;
	idVec3		pos;
	bool		onGround;

	lastVisibleReachableEnemyPos = lastReachableEnemyPos;
	lastVisibleEnemyEyeOffset = enemyEnt->EyeOffset();
	lastVisibleEnemyPos = enemyEnt->GetPhysics()->GetOrigin();

	if (move.moveType == MOVETYPE_FLY)
	{
		// Flying AI assume the enemy to be always on ground
		pos = lastVisibleEnemyPos;
		onGround = true;
	} 
	else
	{
		// For non-flying AI, check if the enemy is on the ground
		onGround = enemyEnt->GetFloorPos(64.0f, pos);

		if (enemyEnt->OnLadder()) // greebo: TODO: What about ropes?
		{
			onGround = false;
		}
	}

	// greebo: "pos" now holds the enemy position used for pathing (floor pos or visible pos)

	if (!onGround)
	{
		// greebo: The AI considers a non-grounded entity to be unreachable
		// TODO: Check for climb height? The enemy might still be reachable?
		if (move.moveCommand == MOVE_TO_ENEMY)
		{
			AI_DEST_UNREACHABLE = true;
		}
		return;
	}

	if (aas != NULL)
	{
		// We have a valid AAS attached, try to reach the enemy

		// The default reachable area number is taken from the move command
		lastVisibleReachableEnemyAreaNum = move.toAreaNum;

		// Get the area number of the point the enemy has last been seen in (== enemy origin)
		enemyAreaNum = PointReachableAreaNum(lastVisibleEnemyPos, 1.0f);

		// If the area number is invalid, try to look it up again using the REACHABLE position
		if (!enemyAreaNum)
		{
			enemyAreaNum = PointReachableAreaNum(lastReachableEnemyPos, 1.0f);
			pos = lastReachableEnemyPos;
		}
		
		// Do we have a valid area number now?
		if (enemyAreaNum)
		{
			// We have a valid enemy area number

			// Get the own origin and area number
			const idVec3 &org = physicsObj.GetOrigin();
			areaNum = PointReachableAreaNum(org);

			// Try to set up a walk/fly path to the enemy
			if (PathToGoal(path, areaNum, org, enemyAreaNum, pos, this))
			{
				// Succeeded, we have a visible and reachable enemy position
				lastVisibleReachableEnemyPos = pos;
				lastVisibleReachableEnemyAreaNum = enemyAreaNum;

				if (move.moveCommand == MOVE_TO_ENEMY)
				{
					// Enemy is reachable
					AI_DEST_UNREACHABLE = false;
				}
			} 
			else if (move.moveCommand == MOVE_TO_ENEMY)
			{
				// No path to goal available, enemy is unreachable
				AI_DEST_UNREACHABLE = true;
			}
		}
		else
		{
			// The area number lookup failed, fallback to unreachable
			if (move.moveCommand == MOVE_TO_ENEMY)
			{
				AI_DEST_UNREACHABLE = true;
			}
			areaNum = 0;
		}
	}
	else
	{
		// We don't have a valid AAS, we can't tell if an enemy is reachable or not,
		// so just assume that he is.
		lastVisibleReachableEnemyPos = lastVisibleEnemyPos;
		if (move.moveCommand == MOVE_TO_ENEMY)
		{
			AI_DEST_UNREACHABLE = false;
		}
		enemyAreaNum = 0;
		areaNum = 0;
	}

	// General move command update
	if (move.moveCommand == MOVE_TO_ENEMY)
	{
		if (!aas)
		{
			// Invalid AAS keep the move destination up to date for wandering
			move.moveDest = lastVisibleReachableEnemyPos;
		}
		else if (enemyAreaNum)
		{
			// The previous pathing attempt succeeded, update the move command
			move.toAreaNum = lastVisibleReachableEnemyAreaNum;
			move.moveDest = lastVisibleReachableEnemyPos;
		}

		if (move.moveType == MOVETYPE_FLY)
		{
			predictedPath_t path;
			idVec3 end = move.moveDest;
			end.z += enemyEnt->EyeOffset().z + fly_offset;
			idAI::PredictPath( this, aas, move.moveDest, end - move.moveDest, 1000, 1000, SE_BLOCKED, path );
			move.moveDest = path.endPos;
			move.toAreaNum = PointReachableAreaNum( move.moveDest, 1.0f );
		}
	}
}

bool idAI::EntityInAttackCone(idEntity* ent)
{
	float	attack_cone;
	idVec3	delta;
	float	yaw;
	float	relYaw;
	
	if ( !ent ) {
		return false;
	}

	delta = ent->GetPhysics()->GetOrigin() - GetEyePosition();

	// get our gravity normal
	const idVec3 &gravityDir = GetPhysics()->GetGravityNormal();

	// infinite vertical vision, so project it onto our orientation plane
	delta -= gravityDir * ( gravityDir * delta );

	delta.Normalize();
	yaw = delta.ToYaw();

	attack_cone = spawnArgs.GetFloat( "attack_cone", "70" );
	relYaw = idMath::AngleNormalize180( ideal_yaw - yaw );

	return ( idMath::Fabs( relYaw ) < ( attack_cone * 0.5f ) );
}

bool idAI::CanHitEntity(idActor* entity, ECombatType combatType)
{
	if (entity == NULL || entity->IsKnockedOut() || entity->health <= 0) 
		return false;

	if (combatType == COMBAT_MELEE)
	{
		return TestMelee();
	}
	else if (combatType == COMBAT_RANGED)
	{
		return TestRanged();
	}
	else
	{
		if (GetNumRangedWeapons() > 0)
		{
			return TestRanged();
		}
		else if (GetNumMeleeWeapons() > 0)
		{
			return TestMelee();
		}
	}

	return false;
}

bool idAI::WillBeAbleToHitEntity(idActor* entity, ECombatType combatType)
{
	if (entity == NULL || entity->IsKnockedOut() || entity->health <= 0) 
		return false;

	if (combatType == COMBAT_MELEE)
	{
		return TestMeleeFuture();
	}
	else if (combatType == COMBAT_RANGED)
	{
		// not supported for ranged combat
		return false;
	}
	else
	{
		if (GetNumRangedWeapons() > 0)
		{
			return false;
		}
		else if (GetNumMeleeWeapons() > 0)
		{
			return TestMeleeFuture();
		}
	}

	return false;
}

bool idAI::CanBeHitByEntity(idActor* entity, ECombatType combatType)
{
	if (entity == NULL || entity->IsKnockedOut() || entity->health <= 0 ) 
		return false;

	if (combatType == COMBAT_MELEE)
	{
		if ( !entity->GetAttackFlag(COMBAT_MELEE) 
				|| !entity->melee_range )
			return false;

		const idVec3& org = physicsObj.GetOrigin();
		const idBounds& bounds = physicsObj.GetBounds();

		const idVec3& enemyOrg = entity->GetPhysics()->GetOrigin();
		const idBounds& enemyBounds = entity->GetPhysics()->GetBounds();

		idVec3 ourVel = physicsObj.GetLinearVelocity();
		idVec3 enemyVel = entity->GetPhysics()->GetLinearVelocity();
		idVec3 relVel = enemyVel - ourVel;
		float velDot = ourVel * enemyVel;

		idVec3 dir = enemyOrg - org;
		dir.z = 0;
		float dist = dir.LengthFast();

		float enemyReach = entity->melee_range;

		// generic factor to increase the enemy's threat range (accounts for sudden, quick advances)
		// (TODO: Make spawnwarg?)
		float threatFactor = 1.5f; 
		// velocity-based factor to increase the enemy's threat range
		float velFactor;
		if( velDot > 0 )
		{
			// enemy moving away
			velFactor = 1.0f;
		}
		else
		{
			// TODO: Make spawnargs?
			// max speed of 20 MPH
			float maxVel = 352;
			float maxThreatInc = 3.0;

			velFactor = idMath::ClampFloat(1.0f, maxThreatInc, 1.0f + (maxThreatInc-1.0f)*relVel.LengthFast()/maxVel);
		}

		enemyReach *= threatFactor * velFactor;

		float maxdist = enemyReach + bounds[1][0];

		if (dist < maxdist)
		{
			// within horizontal distance
			if ((org.z + enemyBounds[1][2] + entity->melee_range) > org.z &&
					(org.z + bounds[1][2]) > enemyOrg.z)
			{
				// within height
				// don't bother with trace for this test
				return true;
			}
		}
		
		return false;
	}
	else if (combatType == COMBAT_RANGED)
	{
		return false; // NYI
	}
	else
		return false;
}

void idAI::UpdateAttachmentContents(bool makeSolid)
{
	// ishtvan: Modified to work only with AF body representing the entity
	for (int i = 0; i < m_Attachments.Num(); i++)
	{
		idEntity* ent = m_Attachments[i].ent.GetEntity();

		if (ent == NULL || !m_Attachments[i].ent.IsValid()) continue;

		if (!ent->IsType(CMeleeWeapon::Type) && !ent->m_bAttachedAlertControlsSolidity) 
			continue;
		
		// Found a melee weapon or other attachment that should become solid on alert
		idAFBody *body;
		if (makeSolid)
		{
			if( (body = AFBodyForEnt(ent)) != NULL )
			{
				// TODO: Read the contents from a stored variable
				// in case they are something other than these standards?
				// CONTENTS_RENDERMODEL allows projectiles to hit it
				body->GetClipModel()->SetContents( CONTENTS_CORPSE | CONTENTS_RENDERMODEL );
				body->SetClipMask( CONTENTS_SOLID | CONTENTS_CORPSE );
			}
		}
		else
		{
			if( (body = AFBodyForEnt(ent)) != NULL )
			{
				body->GetClipModel()->SetContents( 0 );
				body->SetClipMask( 0 );
			}
		}
	}
}

/*
=====================
idAI::UpdateEnemyPosition
=====================
*/
void idAI::UpdateEnemyPosition()
{
	// Interleave this check
	if (gameLocal.time <= lastUpdateEnemyPositionTime + cv_ai_opt_update_enemypos_interleave.GetInteger())
	{
		return;
	}

	idActor* enemyEnt = enemy.GetEntity();
	enemyReachable = false;

	if (enemyEnt == NULL)
	{
		return;
	}

	// Set a new time stamp
	lastUpdateEnemyPositionTime = gameLocal.time;

	START_SCOPED_TIMING(aiUpdateEnemyPositionTimer, scopedUpdateEnemyPositionTimer)

	int				enemyAreaNum(-1);
	idVec3			enemyPos;
	bool			onGround;
	const idVec3&	org = physicsObj.GetOrigin();
	int				lastVisibleReachableEnemyAreaNum = -1;
	bool			enemyDetectable = false;

	if ( move.moveType == MOVETYPE_FLY )
	{
		enemyPos = enemyEnt->GetPhysics()->GetOrigin();
		// greebo: Flying AI always consider their enemies to be on the ground
		onGround = true;
	}
	else
	{
		// non-flying AI, get the floor position of the enemy
		START_TIMING(aiGetFloorPosTimer);
		onGround = enemyEnt->GetFloorPos(64.0f, enemyPos);
		STOP_TIMING(aiGetFloorPosTimer);

		if (enemyEnt->OnLadder())
		{
			onGround = false;
		}
	}

	int areaNum = PointReachableAreaNum(org, 1.0f);

	if (onGround)
	{
		// greebo: Enemy is on ground, hence reachable, try to setup a path
		if (aas != NULL)
		{
			// We have a valid AAS, try to get the area of the enemy (floorpos or origin)
			enemyAreaNum = PointReachableAreaNum(enemyPos, 1.0f);

			if (enemyAreaNum)
			{
				// We have a valid enemy area number
				// Get the own area number

				// Try to setup a path to the goal
				aasPath_t path;
				if (PathToGoal( path, areaNum, org, enemyAreaNum, enemyPos, this))
				{
					// Path successfully setup, store the position as "reachable"
					lastReachableEnemyPos = enemyPos;
					enemyReachable = true;
				}
			}
			else
			{
				// The area number lookup failed, fallback to unreachable
				if (move.moveCommand == MOVE_TO_ENEMY)
				{
					AI_DEST_UNREACHABLE = true;
				}
				areaNum = 0;
			}
		}
		else
		{
			// We don't have an AAS, we can't tell if an enemy is reachable or not,
			// so just assume that he is.
			enemyAreaNum = 0;
			lastReachableEnemyPos = enemyPos;
			enemyReachable = true;
		}
	}
	
	AI_ENEMY_IN_FOV		= false;
	AI_ENEMY_VISIBLE	= false;
	
	if (CanSee(enemyEnt, false))
	{
		if (cv_ai_show_enemy_visibility.GetBool())
		{
			// greebo: A trace to the enemy is possible (no FOV check!) and the entity is not hidden in darkness
			gameRenderWorld->DebugArrow(colorGreen, GetEyePosition(), GetEyePosition() + idVec3(0,0,10), 2, 100);
		}

		// Enemy is considered visible if not hidden in darkness and not obscured
		AI_ENEMY_VISIBLE = true;


		// Now perform the FOV check manually
		if (CheckFOV(enemyPos))
		{
			AI_ENEMY_IN_FOV = true;
			// TODO: call SetEnemyPosition here only?

			// Store the last time the enemy was visible
			mind->GetMemory().lastTimeEnemySeen = gameLocal.time;
		}
		enemyDetectable = true;
	}

	else	// enemy is not visible
	{
		if (cv_ai_show_enemy_visibility.GetBool())
		{
			// Enemy can't be seen (obscured or hidden in darkness)
			gameRenderWorld->DebugArrow(colorRed, GetEyePosition(), GetEyePosition() + idVec3(0,0,10), 2, 100);
		}

		// check if we heard any sounds in the last frame
		if (enemyEnt == gameLocal.GetAlertEntity())
		{
			float dist = (enemyEnt->GetPhysics()->GetOrigin() - org).LengthSqr();

			if (dist < Square(AI_HEARING_RANGE))
			{
				// Enemy within hearing distance, update position
				// greebo: TODO: This also updates lastVisibleReachableEnemyPos, is this ok?
				enemyDetectable= true;

			}
		}
	}

	if (enemyDetectable)
	{
		// angua: This was merged in from SetEnemyPosition
		// to avoid doing the same reachability testing stuff twice
		lastVisibleEnemyPos = enemyEnt->GetPhysics()->GetOrigin();
		lastVisibleEnemyEyeOffset = enemyEnt->EyeOffset();

		if (aas != NULL && enemyReachable)
		{
			// We have a visible and reachable enemy position
			lastVisibleReachableEnemyPos = enemyPos;
			lastVisibleReachableEnemyAreaNum = enemyAreaNum;
		}
		else
		{
			// We don't have a valid AAS, we can't tell if an enemy is reachable or not,
			// so just assume that he is.
			lastVisibleReachableEnemyPos = lastVisibleEnemyPos;
			enemyAreaNum = 0;
			areaNum = 0;
		}

		// General move command update
		if (move.moveCommand == MOVE_TO_ENEMY)
		{
			if (!aas)
			{
				// Invalid AAS keep the move destination up to date for wandering
				AI_DEST_UNREACHABLE = false;
				move.moveDest = lastVisibleReachableEnemyPos;
			}
			else if (enemyAreaNum)
			{
				// The previous pathing attempt succeeded, update the move command
				move.toAreaNum = lastVisibleReachableEnemyAreaNum;
				move.moveDest = lastVisibleReachableEnemyPos;
				AI_DEST_UNREACHABLE = !enemyReachable;
			}

			if (move.moveType == MOVETYPE_FLY)
			{
				predictedPath_t path;
				idVec3 end = move.moveDest;
				end.z += enemyEnt->EyeOffset().z + fly_offset;
				idAI::PredictPath( this, aas, move.moveDest, end - move.moveDest, 1000, 1000, SE_BLOCKED, path );
				move.moveDest = path.endPos;
				move.toAreaNum = PointReachableAreaNum( move.moveDest, 1.0f );
			}

			if (!onGround)
			{
				// greebo: The AI considers a non-grounded entity to be unreachable
				// TODO: Check for climb height? The enemy might still be reachable?
				AI_DEST_UNREACHABLE = true;
			}
		}
	}

	if (ai_debugMove.GetBool())
	{
		gameRenderWorld->DebugBounds( colorLtGrey, enemyEnt->GetPhysics()->GetBounds(), lastReachableEnemyPos, gameLocal.msec );
		gameRenderWorld->DebugBounds( colorWhite, enemyEnt->GetPhysics()->GetBounds(), lastVisibleReachableEnemyPos, gameLocal.msec );
	}
}

/*
=====================
idAI::SetEnemy
=====================
*/
bool idAI::SetEnemy(idActor* newEnemy)
{
	// Don't continue if we're dead or knocked out
	if (newEnemy == NULL || AI_DEAD || AI_KNOCKEDOUT)
	{
		ClearEnemy();
		return false; // not a valid enemy
	}

	// greebo: Check if the new enemy is different
	if (enemy.GetEntity() != newEnemy)
	{
		// Update the enemy pointer 
		enemy = newEnemy;
		enemyNode.AddToEnd(newEnemy->enemyList);

		// Check if the new enemy is dead
		if (newEnemy->health <= 0)
		{
			EnemyDead();
			return false; // not a valid enemy
		}

		int enemyAreaNum(-1);

		// greebo: Get the reachable position and area number of this enemy
		newEnemy->GetAASLocation(aas, lastReachableEnemyPos, enemyAreaNum);

		// SetEnemyPosition() can now try to setup a path, 
		// lastVisibleReachableEnemyPosition is set in ANY CASE by this method
		SetEnemyPosition();

		// greebo: This looks suspicious. It overwrites REACHABLE and
		// and VISIBLEREACHABLE with VISIBLE enemy position,
		// regardless of what happened before. WTF? TODO
		lastReachableEnemyPos = lastVisibleEnemyPos;
		lastVisibleReachableEnemyPos = lastReachableEnemyPos;

		// Get the area number of the enemy
		enemyAreaNum = PointReachableAreaNum(lastReachableEnemyPos, 1.0f);

		if (aas != NULL && enemyAreaNum)
		{
			aas->PushPointIntoAreaNum( enemyAreaNum, lastReachableEnemyPos );
			lastVisibleReachableEnemyPos = lastReachableEnemyPos;
		}

		GetMemory().lastTimeEnemySeen = gameLocal.time;

		return true; // valid enemy
	}
	else
	{
		return true; // still a valid enemy
	}
}

/*
============
idAI::FirstVisiblePointOnPath
============
*/
idVec3 idAI::FirstVisiblePointOnPath( const idVec3 origin, const idVec3 &target, int travelFlags ) {
	int i, areaNum, targetAreaNum, curAreaNum, travelTime;
	idVec3 curOrigin;
	idReachability *reach;

	if ( !aas ) {
		return origin;
	}

	areaNum = PointReachableAreaNum( origin );
	targetAreaNum = PointReachableAreaNum( target );

	if ( !areaNum || !targetAreaNum ) {
		return origin;
	}

	if ( ( areaNum == targetAreaNum ) || PointVisible( origin ) ) {
		return origin;
	}

	curAreaNum = areaNum;
	curOrigin = origin;

	for( i = 0; i < 10; i++ ) {

		if ( !aas->RouteToGoalArea( curAreaNum, curOrigin, targetAreaNum, travelFlags, travelTime, &reach, NULL, this ) ) {
			break;
		}

		if ( !reach ) {
			return target;
		}

		curAreaNum = reach->toAreaNum;
		curOrigin = reach->end;

		if ( PointVisible( curOrigin ) ) {
			return curOrigin;
		}
	}

	return origin;
}

/*
===================
idAI::CalculateAttackOffsets

calculate joint positions on attack frames so we can do proper "can hit" tests
===================
*/
void idAI::CalculateAttackOffsets( void ) {
	const idDeclModelDef	*modelDef;
	int						num;
	int						i;
	int						frame;
	const frameCommand_t	*command;
	idMat3					axis;
	const idAnim			*anim;
	jointHandle_t			joint;

	modelDef = animator.ModelDef();
	if ( !modelDef ) {
		return;
	}
	num = modelDef->NumAnims();

	// needs to be off while getting the offsets so that we account for the distance the monster moves in the attack anim
	animator.RemoveOriginOffset( false );

	// anim number 0 is reserved for non-existant anims.  to avoid off by one issues, just allocate an extra spot for
	// launch offsets so that anim number can be used without subtracting 1.
	missileLaunchOffset.SetGranularity( 1 );
	missileLaunchOffset.SetNum( num + 1 );
	missileLaunchOffset[ 0 ].Zero();

	for( i = 1; i <= num; i++ ) {
		missileLaunchOffset[ i ].Zero();
		anim = modelDef->GetAnim( i );
		if ( anim ) {
			frame = anim->FindFrameForFrameCommand( FC_LAUNCHMISSILE, &command );
			if ( frame >= 0 ) {
				joint = animator.GetJointHandle( command->string->c_str() );
				if ( joint == INVALID_JOINT ) {
					gameLocal.Error( "Invalid joint '%s' on 'launch_missile' frame command on frame %d of model '%s'", command->string->c_str(), frame, modelDef->GetName() );
				}
				GetJointTransformForAnim( joint, i, FRAME2MS( frame ), missileLaunchOffset[ i ], axis );
			}
		}
	}

	animator.RemoveOriginOffset( true );
}

/*
=====================
idAI::EnsureActiveProjectileInfo
=====================
*/
idAI::ProjectileInfo& idAI::EnsureActiveProjectileInfo()
{
	// Ensure valid projectile clipmodel
	if (activeProjectile.info.clipModel == NULL)
	{
		// Ensure we have a radius
		if (activeProjectile.info.radius < 0)
		{
			// At least we should have the def
			assert(activeProjectile.info.def != NULL);

			// Load values from the projectile
			InitProjectileInfoFromDict(activeProjectile.info, activeProjectile.info.def);
		}

		idBounds projectileBounds(vec3_origin);
		projectileBounds.ExpandSelf(activeProjectile.info.radius);

		activeProjectile.info.clipModel = new idClipModel(idTraceModel(projectileBounds));
	}

	return activeProjectile.info;
}

/*
=====================
idAI::GetAimDir
=====================
*/
bool idAI::GetAimDir( const idVec3 &firePos, idEntity *aimAtEnt, const idEntity *ignore, idVec3 &aimDir )
{
	idVec3	targetPos1;
	idVec3	targetPos2;
	idVec3	delta;
	float	max_height;
	bool	result;

	// if no aimAtEnt or projectile set
	if (aimAtEnt == NULL || projectileInfo.Num() == 0)
	{
		aimDir = viewAxis[ 0 ] * physicsObj.GetGravityAxis();
		return false;
	}

	// Ensure we have a valid clipmodel
	ProjectileInfo& info = EnsureActiveProjectileInfo();

	if ( aimAtEnt == enemy.GetEntity() ) {
		static_cast<idActor *>( aimAtEnt )->GetAIAimTargets( lastVisibleEnemyPos, targetPos1, targetPos2 );
	} else if ( aimAtEnt->IsType( idActor::Type ) ) {
		static_cast<idActor *>( aimAtEnt )->GetAIAimTargets( aimAtEnt->GetPhysics()->GetOrigin(), targetPos1, targetPos2 );
	} else {
		targetPos1 = aimAtEnt->GetPhysics()->GetAbsBounds().GetCenter();
		targetPos2 = targetPos1;
	}

	// try aiming for chest
	delta = firePos - targetPos1;
	max_height = delta.LengthFast() * projectile_height_to_distance_ratio;
	result = PredictTrajectory( firePos, targetPos1, info.speed, info.gravity, info.clipModel, MASK_SHOT_RENDERMODEL, max_height, ignore, aimAtEnt, ai_debugTrajectory.GetBool() ? 1000 : 0, aimDir );
	if ( result || !aimAtEnt->IsType( idActor::Type ) ) {
		return result;
	}

	// try aiming for head
	delta = firePos - targetPos2;
	max_height = delta.LengthFast() * projectile_height_to_distance_ratio;
	result = PredictTrajectory( firePos, targetPos2, info.speed, info.gravity, info.clipModel, MASK_SHOT_RENDERMODEL, max_height, ignore, aimAtEnt, ai_debugTrajectory.GetBool() ? 1000 : 0, aimDir );

	return result;
}

/*
=====================
idAI::BeginAttack
=====================
*/
void idAI::BeginAttack( const char *name ) {
	attack = name;
	lastAttackTime = gameLocal.time;
}

/*
=====================
idAI::EndAttack
=====================
*/
void idAI::EndAttack( void ) {
	attack = "";
}

/*
=====================
idAI::CreateProjectile
=====================
*/
idProjectile* idAI::CreateProjectile(const idVec3 &pos, const idVec3 &dir, int index)
{
	if (activeProjectile.projEnt.GetEntity() == NULL)
	{
		assert(curProjectileIndex >= 0 && curProjectileIndex < projectileInfo.Num()); // bounds check

		// projectile pointer still NULL, create a new one
		const idDict* def = projectileInfo[curProjectileIndex].def;

		// Fill the current projectile entity pointer, passing the arguments to the specialised routine
		// After this call, the activeProjectile.projEnt pointer will be initialised with the new projectile.
		CreateProjectileFromDict(pos, dir, def);

		// Re-roll the index for the next time
		curProjectileIndex = gameLocal.random.RandomInt(projectileInfo.Num());
	}

	return activeProjectile.projEnt.GetEntity();
}

idProjectile* idAI::CreateProjectileFromDict(const idVec3 &pos, const idVec3 &dir, const idDict* dict)
{
	if (activeProjectile.projEnt.GetEntity() == NULL)
	{
		// Store the def for later use
		activeProjectile.info.def = dict;
		activeProjectile.info.defName = dict->GetString("classname");

		// Fill the current projectile entity pointer
		activeProjectile.projEnt = SpawnProjectile(dict);
	}

	activeProjectile.projEnt.GetEntity()->Create(this, pos, dir);

	return activeProjectile.projEnt.GetEntity();
}

idProjectile* idAI::SpawnProjectile(const idDict* dict) const
{
	idEntity* ent;
	gameLocal.SpawnEntityDef(*dict, &ent, false);

	if (ent == NULL)
	{
		const char* clsname = dict->GetString("classname");
		gameLocal.Error("Could not spawn entityDef '%s'", clsname);
	}

	if (!ent->IsType(idProjectile::Type))
	{
		const char* clsname = ent->GetClassname();
		gameLocal.Error("'%s' is not an idProjectile", clsname);
	}

	return static_cast<idProjectile*>(ent);
}

/*
=====================
idAI::RemoveProjectile
=====================
*/
void idAI::RemoveProjectile()
{
	if (activeProjectile.projEnt.GetEntity())
	{
		activeProjectile.projEnt.GetEntity()->PostEventMS(&EV_Remove, 0);
		activeProjectile.projEnt = NULL;

		activeProjectile.info = ProjectileInfo();
	}
}

/*
=====================
idAI::LaunchProjectile
=====================
*/
idProjectile *idAI::LaunchProjectile( const char *jointname, idEntity *target, bool clampToAttackCone ) {
	idVec3				muzzle;
	idVec3				dir;
	idVec3				start;
	trace_t				tr;
	idBounds			projBounds;
	float				distance;
	const idClipModel	*projClip;
	float				attack_accuracy;
	float				attack_cone;
	float				projectile_spread;
	float				diff;
	float				angle;
	float				spin;
	idAngles			ang;
	int					num_projectiles;
	int					i;
	idMat3				axis;
	idVec3				tmp;

	if (projectileInfo.Num() == 0)
	{
		gameLocal.Warning( "%s (%s) doesn't have a projectile specified", name.c_str(), GetEntityDefName() );
		return NULL;
	}

	attack_accuracy = spawnArgs.GetFloat( "attack_accuracy", "7" );
	attack_cone = spawnArgs.GetFloat( "attack_cone", "70" );
	projectile_spread = spawnArgs.GetFloat( "projectile_spread", "0" );
	num_projectiles = spawnArgs.GetInt( "num_projectiles", "1" );

	GetMuzzle( jointname, muzzle, axis );

	// Ensure we have a set up projectile
	CreateProjectile(muzzle, axis[0]);

	idProjectile* lastProjectile = activeProjectile.projEnt.GetEntity();

	if ( target != NULL ) {
		tmp = target->GetPhysics()->GetAbsBounds().GetCenter() - muzzle;
		tmp.Normalize();
		axis = tmp.ToMat3();
	} else {
		axis = viewAxis;
	}

	// rotate it because the cone points up by default
	tmp = axis[2];
	axis[2] = axis[0];
	axis[0] = -tmp;

	// make sure the projectile starts inside the monster bounding box
	const idBounds &ownerBounds = physicsObj.GetAbsBounds();
	projClip = lastProjectile->GetPhysics()->GetClipModel();
	projBounds = projClip->GetBounds().Rotate( axis );

	// check if the owner bounds is bigger than the projectile bounds
	if ( ( ( ownerBounds[1][0] - ownerBounds[0][0] ) > ( projBounds[1][0] - projBounds[0][0] ) ) &&
		( ( ownerBounds[1][1] - ownerBounds[0][1] ) > ( projBounds[1][1] - projBounds[0][1] ) ) &&
		( ( ownerBounds[1][2] - ownerBounds[0][2] ) > ( projBounds[1][2] - projBounds[0][2] ) ) ) {
		if ( (ownerBounds - projBounds).RayIntersection( muzzle, viewAxis[ 0 ], distance ) ) {
			start = muzzle + distance * viewAxis[ 0 ];
		} else {
			start = ownerBounds.GetCenter();
		}
	} else {
		// projectile bounds bigger than the owner bounds, so just start it from the center
		start = ownerBounds.GetCenter();
	}

	gameLocal.clip.Translation( tr, start, muzzle, projClip, axis, MASK_SHOT_RENDERMODEL, this );
	muzzle = tr.endpos;

	// set aiming direction
	GetAimDir( muzzle, target, this, dir );
	ang = dir.ToAngles();

	// adjust his aim so it's not perfect.  uses sine based movement so the tracers appear less random in their spread.
	float t = MS2SEC( gameLocal.time + entityNumber * 497 );
	ang.pitch += idMath::Sin16( t * 5.1 ) * attack_accuracy;
	ang.yaw	+= idMath::Sin16( t * 6.7 ) * attack_accuracy;

	if ( clampToAttackCone ) {
		// clamp the attack direction to be within monster's attack cone so he doesn't do
		// things like throw the missile backwards if you're behind him
		diff = idMath::AngleDelta( ang.yaw, current_yaw );
		if ( diff > attack_cone ) {
			ang.yaw = current_yaw + attack_cone;
		} else if ( diff < -attack_cone ) {
			ang.yaw = current_yaw - attack_cone;
		}
	}

	axis = ang.ToMat3();

	float spreadRad = DEG2RAD( projectile_spread );

	for( i = 0; i < num_projectiles; i++ )
	{
		// spread the projectiles out
		angle = idMath::Sin( spreadRad * gameLocal.random.RandomFloat() );
		spin = (float)DEG2RAD( 360.0f ) * gameLocal.random.RandomFloat();
		dir = axis[ 0 ] + axis[ 2 ] * ( angle * idMath::Sin( spin ) ) - axis[ 1 ] * ( angle * idMath::Cos( spin ) );
		dir.Normalize();

		// create, launch and clear the projectile
		CreateProjectile( muzzle, dir );
		
		lastProjectile = activeProjectile.projEnt.GetEntity();
		lastProjectile->Launch( muzzle, dir, vec3_origin );
		activeProjectile.projEnt = NULL;
	}

	TriggerWeaponEffects( muzzle );

	lastAttackTime = gameLocal.time;

	return lastProjectile;
}

/*
================
idAI::DamageFeedback

callback function for when another entity recieved damage from this entity.  damage can be adjusted and returned to the caller.

FIXME: This gets called when we call idPlayer::CalcDamagePoints from idAI::AttackMelee, which then checks for a saving throw,
possibly forcing a miss.  This is harmless behavior ATM, but is not intuitive.
================
*/
void idAI::DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage ) {
	if ( ( victim == this ) && inflictor->IsType( idProjectile::Type ) ) {
		// monsters only get half damage from their own projectiles
		damage = ( damage + 1 ) / 2;  // round up so we don't do 0 damage

	} else if ( victim == enemy.GetEntity() ) {
		AI_HIT_ENEMY = true;
	}
}

/*
=====================
idAI::DirectDamage

Causes direct damage to an entity

kickDir is specified in the monster's coordinate system, and gives the direction
that the view kick and knockback should go
=====================
*/
void idAI::DirectDamage( const char *meleeDefName, idEntity *ent ) {
	const idDict *meleeDef;
	const char *p;
	const idSoundShader *shader;

	meleeDef = gameLocal.FindEntityDefDict( meleeDefName, false );
	if ( !meleeDef ) {
		gameLocal.Error( "Unknown damage def '%s' on '%s'", meleeDefName, name.c_str() );
	}

	if ( !ent->fl.takedamage ) {
		const idSoundShader *shader = declManager->FindSound(meleeDef->GetString( "snd_miss" ));
		StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
		return;
	}

	//
	// do the damage
	//
	p = meleeDef->GetString( "snd_hit" );
	if ( p && *p ) {
		shader = declManager->FindSound( p );
		StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
	}

	idVec3	kickDir;
	meleeDef->GetVector( "kickDir", "0 0 0", kickDir );

	idVec3	globalKickDir;
	globalKickDir = ( viewAxis * physicsObj.GetGravityAxis() ) * kickDir;

	ent->Damage( this, this, globalKickDir, meleeDefName, 1.0f, INVALID_JOINT );

	// end the attack if we're a multiframe attack
	EndAttack();
}

/*
=====================
idAI::TestMelee
=====================
*/
bool idAI::TestMelee( void ) const {
	trace_t trace;
	idActor *enemyEnt = enemy.GetEntity();

	if ( !enemyEnt || !melee_range ) {
		return false;
	}

	if (!GetAttackFlag(COMBAT_MELEE))
	{
		// greebo: Cannot attack with melee weapons yet
		return false;
	}

	//FIXME: make work with gravity vector
	const idVec3& org = physicsObj.GetOrigin();
	const idBounds& bounds = physicsObj.GetBounds();

	const idVec3& enemyOrg = enemyEnt->GetPhysics()->GetOrigin();
	const idBounds& enemyBounds = enemyEnt->GetPhysics()->GetBounds();

	idVec3 dir = enemyOrg - org;
	dir.z = 0;
	float dist = dir.LengthFast();

	float maxdist = melee_range + enemyBounds[1][0];

	if (dist < maxdist)
	{
		// angua: within horizontal distance
		if ((org.z + bounds[1][2] + melee_range) > enemyOrg.z &&
				(enemyOrg.z + enemyBounds[1][2]) > org.z)
		{
			// within height
			// check if there is something in between
			idVec3 start = GetEyePosition();
			idVec3 end = enemyEnt->GetEyePosition();

			gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_BOUNDINGBOX, this );
			if ( ( trace.fraction == 1.0f ) || ( gameLocal.GetTraceEntity( trace ) == enemyEnt ) ) 
			{
				return true;
			}
		}
	}

	return false;
}

/*
=====================
idAI::TestMeleeFuture
=====================
*/
bool idAI::TestMeleeFuture( void ) const {
	trace_t trace;
	idActor *enemyEnt = enemy.GetEntity();

	if ( !enemyEnt || !melee_range ) {
		return false;
	}

	if (!GetAttackFlag(COMBAT_MELEE))
	{
		// greebo: Cannot attack with melee weapons yet
		return false;
	}

	//FIXME: make work with gravity vector
	idVec3 org = physicsObj.GetOrigin();
	const idBounds& bounds = physicsObj.GetBounds();
	idVec3 vel = physicsObj.GetLinearVelocity();


	idVec3 enemyOrg = enemyEnt->GetPhysics()->GetOrigin();
	const idBounds& enemyBounds = enemyEnt->GetPhysics()->GetBounds();
	idVec3 enemyVel = enemyEnt->GetPhysics()->GetLinearVelocity();

	// update prediction
	float dt = m_MeleePredictedAttTime;
	idVec3 ds = dt * vel;
	org += ds;
	//bounds = bounds.TranslateSelf( ds );

	idVec3 dsEnemy = dt * enemyVel;
	enemyOrg += dsEnemy;
	//enemyBounds = enemyBounds.TranslateSelf( dsEnemy );

	// rest is the same as TestMelee
	idVec3 dir = enemyOrg - org;
	dir.z = 0;
	float dist = dir.LengthFast();

	float maxdist = melee_range + enemyBounds[1][0];

	if (dist < maxdist)
	{
		// angua: within horizontal distance
		if ((org.z + bounds[1][2] + melee_range) > enemyOrg.z &&
			(enemyOrg.z + enemyBounds[1][2]) > org.z)
		{
			// within height
			// check if there is something in between
			idVec3 start = GetEyePosition() + ds;
			idVec3 end = enemyEnt->GetEyePosition() + dsEnemy;

			gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_BOUNDINGBOX, this );
			if ( ( trace.fraction == 1.0f ) || ( gameLocal.GetTraceEntity( trace ) == enemyEnt ) ) 
			{
				return true;
			}
		}
	}

	return false;
}


/*
=====================
idAI::TestRanged
=====================
*/
bool idAI::TestRanged()
{
	idActor *enemyEnt = enemy.GetEntity();

	if ( !enemyEnt) 
	{
		return false;
	}

	if (!GetAttackFlag(COMBAT_RANGED))
	{
		// greebo: Cannot attack with ranged weapons yet
		return false;
	}

	// Calculate the point on enemy AI needs to see
	idVec3 enemyPoint;
	// stgatilov: Look at the UNLEANED player's eye position
	// to achieve this, we treat player as ordinary actor
	if (enemyEnt->IsType(idPlayer::Type))
		enemyPoint = enemyEnt->idActor::GetEyePosition();
	else
		enemyPoint = enemyEnt->GetEyePosition();

	// Test if the enemy is within range, in FOV and not occluded
	float dist = (GetEyePosition() - enemyPoint).LengthFast();
	return dist <= fire_range && CanSeePositionExt(enemyPoint, false, false);
}


/*
=====================
idAI::AttackMelee

jointname allows the endpoint to be exactly specified in the model,
as for the commando tentacle.  If not specified, it will be set to
the facing direction + melee_range.

kickDir is specified in the monster's coordinate system, and gives the direction
that the view kick and knockback should go

DarkMod : Took out saving throws.
=====================
*/
bool idAI::AttackMelee( const char *meleeDefName ) {
	const idDict *meleeDef;
	idActor *enemyEnt = enemy.GetEntity();
	const char *p;
	const idSoundShader *shader;

	meleeDef = gameLocal.FindEntityDefDict( meleeDefName, false );
	if ( !meleeDef ) {
		gameLocal.Error( "Unknown melee '%s'", meleeDefName );
	}

	if ( !enemyEnt ) {
		p = meleeDef->GetString( "snd_miss" );
		if ( p && *p ) {
			shader = declManager->FindSound( p );
			StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
		}
		return false;
	}

	// check for the "saving throw" automatic melee miss on lethal blow
	// stupid place for this.

/**
* Saving throws removed.  Uncomment the following for saving throws.
**/

/*** BEGIN SAVING THROWS SECTION *****
	bool forceMiss = false;
	if ( enemyEnt->IsType( idPlayer::Type ) && g_skill.GetInteger() < 2 ) {
		int	damage, armor;
		idPlayer *player = static_cast<idPlayer*>( enemyEnt );
		player->CalcDamagePoints( this, this, meleeDef, 1.0f, INVALID_JOINT, &damage, &armor );

		if ( enemyEnt->health <= damage ) {
			int	t = gameLocal.time - player->lastSavingThrowTime;
			if ( t > SAVING_THROW_TIME ) {
				player->lastSavingThrowTime = gameLocal.time;
				t = 0;
			}
			if ( t < 1000 ) {
				gameLocal.Printf( "Saving throw.\n" );
				forceMiss = true;
			}
		}
	}


	// make sure the trace can actually hit the enemy
	if ( forceMiss || !TestMelee() ) {
****** END SAVING THROWS SECTION *******/

	if ( !TestMelee() )
	{
		// missed
		p = meleeDef->GetString( "snd_miss" );
		if ( p && *p ) {
			shader = declManager->FindSound( p );
			StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
		}
		return false;
	}

	//
	// do the damage
	//
	p = meleeDef->GetString( "snd_hit" );
	if ( p && *p ) {
		shader = declManager->FindSound( p );
		StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
	}

	idVec3	kickDir;
	meleeDef->GetVector( "kickDir", "0 0 0", kickDir );

	idVec3	globalKickDir;
	globalKickDir = ( viewAxis * physicsObj.GetGravityAxis() ) * kickDir;

	enemyEnt->Damage( this, this, globalKickDir, meleeDefName, 1.0f, INVALID_JOINT );

	// cause a LARGE tactile alert in the enemy, if it is an AI
	if( enemyEnt->IsType(idAI::Type) )
	{
		static_cast<idAI *>(enemyEnt)->TactileAlert( this, 100 );
	}

	lastAttackTime = gameLocal.time;

	return true;
}

/*
================
idAI::PushWithAF
================
*/
void idAI::PushWithAF( void ) {
	int i, j;
	afTouch_t touchList[ MAX_GENTITIES ];
	idEntity *pushed_ents[ MAX_GENTITIES ];
	idEntity *ent;
	idVec3 vel( vec3_origin ), vGravNorm( vec3_origin );
	int num_pushed;

	num_pushed = 0;
	af.ChangePose( this, gameLocal.time );
	int num = af.EntitiesTouchingAF( touchList );

	for( i = 0; i < num; i++ ) {
		if ( touchList[ i ].touchedEnt->IsType( idProjectile::Type ) ) {
			// skip projectiles
			continue;
		}

		// make sure we havent pushed this entity already.  this avoids causing double damage
		for( j = 0; j < num_pushed; j++ ) {
			if ( pushed_ents[ j ] == touchList[ i ].touchedEnt ) {
				break;
			}
		}
		if ( j >= num_pushed )
		{
			ent = touchList[ i ].touchedEnt;
			pushed_ents[num_pushed++] = ent;
			vel = ent->GetPhysics()->GetAbsBounds().GetCenter() - touchList[ i ].touchedByBody->GetWorldOrigin();

			if ( ent->IsType(idPlayer::Type) && static_cast<idPlayer *>(ent)->noclip )
			{
				// skip player when noclip is on
				continue;
			}

			// ishtvan: don't push our own bind children (fix AI floating away when stuff is bound to them)
			if( ent->GetBindMaster() == this )
				continue;

			if( ent->IsType(idActor::Type) )
			{

				// Id code to stop from pushing the enemy back during melee
				// TODO: This will change with new melee system
				if ( attack.Length() )
				{
					// TODO: Don't need to do this right now, but keep in mind for future melee system
					ent->Damage( this, this, vel, attack, 1.0f, INVALID_JOINT );
				} else
				{
					// Ishtvan: Resolve velocity on to XY plane to stop from pushing AI up
					vGravNorm = physicsObj.GetGravityNormal();
					vel -= (vel * vGravNorm ) * vGravNorm;
					vel.Normalize();
					ent->GetPhysics()->SetLinearVelocity( 80.0f * vel, touchList[ i ].touchedClipModel->GetId() );
				}

				// Tactile Alert:

				// grayman #2345 - when an AI is non-solid, waiting for another AI
				// to pass by, there's no need to register a tactile alert from another AI

				if (!movementSubsystem->IsWaitingNonSolid())
				{
					if( ent->IsType(idPlayer::Type) )
					{
						// aesthetics: Dont react to dead player?
						if( ent->health > 0 )
							HadTactile( static_cast<idActor *>(ent) );
					}
					else if( ent->IsType(idAI::Type) && (ent->health > 0) && !static_cast<idAI *>(ent)->AI_KNOCKEDOUT )
					{
						if (!static_cast<idAI *>(ent)->movementSubsystem->IsWaitingNonSolid()) // grayman #2345 - don't call HadTactile() if the bumped AI is waiting
						{
							HadTactile( static_cast<idActor *>(ent) );
						}
					}
					else
					{
						// TODO: Touched a dead or unconscious body, should issue a body alert
						// Touched dead body code goes here: found body alert
					}
				}
			}
			// Ent was not an actor:
			else
			{
				vel.Normalize();
				ent->ApplyImpulse( this, touchList[i].touchedClipModel->GetId(), ent->GetPhysics()->GetOrigin(), cv_ai_bumpobject_impulse.GetFloat() * vel );
				if (ent->m_SetInMotionByActor.GetEntity() == NULL)
				{
					ent->m_SetInMotionByActor = this;
					ent->m_MovedByActor = this;
				}
			}
		}
	}
}

/***********************************************************************

	Misc

***********************************************************************/

/*
================
idAI::GetMuzzle
================
*/
void idAI::GetMuzzle( const char *jointname, idVec3 &muzzle, idMat3 &axis ) {
	jointHandle_t joint;

	if ( !jointname || !jointname[ 0 ] ) {
		muzzle = physicsObj.GetOrigin() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 14;
		muzzle -= physicsObj.GetGravityNormal() * physicsObj.GetBounds()[ 1 ].z * 0.5f;
	} else {
		joint = animator.GetJointHandle( jointname );
		if ( joint == INVALID_JOINT ) {
			gameLocal.Error( "Unknown joint '%s' on %s", jointname, GetEntityDefName() );
		}
		GetJointWorldTransform( joint, gameLocal.time, muzzle, axis );
	}
}

/*
================
idAI::TriggerWeaponEffects
================
*/
void idAI::TriggerWeaponEffects( const idVec3 &muzzle ) {
	idVec3 org;
	idMat3 axis;

	if ( !g_muzzleFlash.GetBool() ) {
		return;
	}

	// muzzle flash
	// offset the shader parms so muzzle flashes show up
	renderEntity.shaderParms[SHADERPARM_TIMEOFFSET] = -MS2SEC( gameLocal.time );
	renderEntity.shaderParms[ SHADERPARM_DIVERSITY ] = gameLocal.random.CRandomFloat();

	if ( flashJointWorld != INVALID_JOINT ) {
		GetJointWorldTransform( flashJointWorld, gameLocal.time, org, axis );

		if ( worldMuzzleFlash.lightRadius.x > 0.0f ) {
			worldMuzzleFlash.axis = axis;
			worldMuzzleFlash.shaderParms[SHADERPARM_TIMEOFFSET] = -MS2SEC( gameLocal.time );
			if ( worldMuzzleFlashHandle != - 1 ) {
				gameRenderWorld->UpdateLightDef( worldMuzzleFlashHandle, &worldMuzzleFlash );
			} else {
				worldMuzzleFlashHandle = gameRenderWorld->AddLightDef( &worldMuzzleFlash );
			}
			muzzleFlashEnd = gameLocal.time + flashTime;
			UpdateVisuals();
		}
	}
}

/*
================
idAI::UpdateMuzzleFlash
================
*/
void idAI::UpdateMuzzleFlash( void ) {
	if ( worldMuzzleFlashHandle != -1 ) {
		if ( gameLocal.time >= muzzleFlashEnd ) {
			gameRenderWorld->FreeLightDef( worldMuzzleFlashHandle );
			worldMuzzleFlashHandle = -1;
		} else {
			idVec3 muzzle;
			animator.GetJointTransform( flashJointWorld, gameLocal.time, muzzle, worldMuzzleFlash.axis );
			animator.GetJointTransform( flashJointWorld, gameLocal.time, muzzle, worldMuzzleFlash.axis );
			muzzle = physicsObj.GetOrigin() + ( muzzle + modelOffset ) * viewAxis * physicsObj.GetGravityAxis();
			worldMuzzleFlash.origin = muzzle;
			gameRenderWorld->UpdateLightDef( worldMuzzleFlashHandle, &worldMuzzleFlash );
		}
	}
}

/*
================
idAI::Hide
================
*/
void idAI::Hide( void ) {
	idActor::Hide();
	fl.takedamage = false;
	physicsObj.SetContents( 0 );
	physicsObj.GetClipModel()->Unlink();
	StopSound( SND_CHANNEL_AMBIENT, false );

	AI_ENEMY_IN_FOV		= false;
	AI_ENEMY_VISIBLE	= false;
	StopMove( MOVE_STATUS_DONE );
}

/*
================
idAI::Show
================
*/
void idAI::Show( void ) 
{
	idActor::Show();
	physicsObj.SetContents( m_preHideContents );

	physicsObj.GetClipModel()->Link( gameLocal.clip );
	fl.takedamage = !spawnArgs.GetBool( "noDamage" );
	StartSound( "snd_ambient", SND_CHANNEL_AMBIENT, 0, false, NULL );
}
/*
================
idAI::CanBecomeSolid
================
*/
bool idAI::CanBecomeSolid( void ) {
	idClipModel* clipModels[ MAX_GENTITIES ];

	int num = gameLocal.clip.ClipModelsTouchingBounds( physicsObj.GetAbsBounds(), MASK_MONSTERSOLID, clipModels, MAX_GENTITIES );
	for ( int i = 0; i < num; i++ ) {
		idClipModel* cm = clipModels[ i ];

		// don't check render entities
		if ( cm->IsRenderModel() ) {
			continue;
		}

		idEntity* hit = cm->GetEntity();
		if ( ( hit == this ) || !hit->fl.takedamage ) {
			continue;
		}

		if ( physicsObj.ClipContents( cm ) ) {
			return false;
		}
	}
	return true;

}

/*
=====================
idAI::UpdateParticles
=====================
*/
void idAI::UpdateParticles( void ) {
	if ( ( thinkFlags & TH_UPDATEPARTICLES) && !IsHidden() ) {
		idVec3 realVector;
		idMat3 realAxis;

		int particlesAlive = 0;
		for ( int i = 0; i < particles.Num(); i++ ) {
			if ( particles[i].particle && particles[i].time ) {
				particlesAlive++;
				if (af.IsActive()) {
					realAxis = mat3_identity;
					realVector = GetPhysics()->GetOrigin();
				} else {
					animator.GetJointTransform( particles[i].joint, gameLocal.time, realVector, realAxis );
					realAxis *= renderEntity.axis;
					realVector = physicsObj.GetOrigin() + ( realVector + modelOffset ) * ( viewAxis * physicsObj.GetGravityAxis() );
				}

				if ( !gameLocal.smokeParticles->EmitSmoke( particles[i].particle, particles[i].time, gameLocal.random.CRandomFloat(), realVector, realAxis )) {
					if ( restartParticles ) {
						particles[i].time = gameLocal.time;
					} else {
						particles[i].time = 0;
						particlesAlive--;
					}
				}
			}
		}
		if ( particlesAlive == 0 ) {
			BecomeInactive( TH_UPDATEPARTICLES );
		}
	}
}

/*
=====================
idAI::TriggerParticles
=====================
*/
void idAI::TriggerParticles( const char *jointName ) {
	jointHandle_t jointNum;

	jointNum = animator.GetJointHandle( jointName );
	for ( int i = 0; i < particles.Num(); i++ ) {
		if ( particles[i].joint == jointNum ) {
			particles[i].time = gameLocal.time;
			BecomeActive( TH_UPDATEPARTICLES );
		}
	}
}


/***********************************************************************

	Head & torso aiming

***********************************************************************/

/*
================
idAI::UpdateAnimationControllers
================
*/
bool idAI::UpdateAnimationControllers( void ) {
	idVec3		local;
	idVec3		focusPos;
	idQuat		jawQuat;
	idVec3		left;
	idVec3 		dir;
	idVec3 		orientationJointPos;
	idVec3 		localDir;
	idAngles 	newLookAng;
	idAngles	diff;
	idMat3		mat;
	idMat3		axis;
	idMat3		orientationJointAxis;
	idAFAttachment	*headEnt = head.GetEntity();
	idVec3		eyepos;
	idVec3		pos;
	int			i;
	idAngles	jointAng;
	float		orientationJointYaw;

	if ( AI_DEAD || AI_KNOCKEDOUT )
	{
		return idActor::UpdateAnimationControllers();
	}

	if ( orientationJoint == INVALID_JOINT ) {
		orientationJointAxis = viewAxis;
		orientationJointPos = physicsObj.GetOrigin();
		orientationJointYaw = current_yaw;
	} else {
		GetJointWorldTransform( orientationJoint, gameLocal.time, orientationJointPos, orientationJointAxis );
		orientationJointYaw = orientationJointAxis[ 2 ].ToYaw();
		orientationJointAxis = idAngles( 0.0f, orientationJointYaw, 0.0f ).ToMat3();
	}

	if ( focusJoint != INVALID_JOINT ) {
		if ( headEnt ) {
			headEnt->GetJointWorldTransform( focusJoint, gameLocal.time, eyepos, axis );
		} else {
			GetJointWorldTransform( focusJoint, gameLocal.time, eyepos, axis );
		}
		eyeOffset.z = eyepos.z - physicsObj.GetOrigin().z;
		if ( ai_debugMove.GetBool() ) {
			gameRenderWorld->DebugLine( colorRed, eyepos, eyepos + orientationJointAxis[ 0 ] * 32.0f, gameLocal.msec );
		}
	} else {
		eyepos = GetEyePosition();
	}

	if ( headEnt ) {
		CopyJointsFromBodyToHead();
	}

	// Update the IK after we've gotten all the joint positions we need, but before we set any joint positions.
	// Getting the joint positions causes the joints to be updated.  The IK gets joint positions itself (which
	// are already up to date because of getting the joints in this function) and then sets their positions, which
	// forces the heirarchy to be updated again next time we get a joint or present the model.  If IK is enabled,
	// or if we have a seperate head, we end up transforming the joints twice per frame.  Characters with no
	// head entity and no ik will only transform their joints once.  Set g_debuganim to the current entity number
	// in order to see how many times an entity transforms the joints per frame.
	idActor::UpdateAnimationControllers();

	idEntity *focusEnt = focusEntity.GetEntity();
	if ( !allowJointMod || !allowEyeFocus || ( gameLocal.time >= focusTime ) ) {
	    focusPos = GetEyePosition() + orientationJointAxis[ 0 ] * 512.0f;
	} else if ( focusEnt == NULL ) {
		// keep looking at last position until focusTime is up
		focusPos = currentFocusPos;
	} else if ( focusEnt == enemy.GetEntity() ) {
		focusPos = lastVisibleEnemyPos + lastVisibleEnemyEyeOffset - eyeVerticalOffset * enemy.GetEntity()->GetPhysics()->GetGravityNormal();
	} else if ( focusEnt->IsType( idActor::Type ) ) {
		focusPos = static_cast<idActor *>( focusEnt )->GetEyePosition() - eyeVerticalOffset * focusEnt->GetPhysics()->GetGravityNormal();
	} else {
		focusPos = focusEnt->GetPhysics()->GetOrigin();
	}

	currentFocusPos = currentFocusPos + ( focusPos - currentFocusPos ) * eyeFocusRate;

	// determine yaw from origin instead of from focus joint since joint may be offset, which can cause us to bounce between two angles
	dir = focusPos - orientationJointPos;
	newLookAng.yaw = idMath::AngleNormalize180( dir.ToYaw() - orientationJointYaw );
	newLookAng.roll = 0.0f;
	newLookAng.pitch = 0.0f;

#if 0
	gameRenderWorld->DebugLine( colorRed, orientationJointPos, focusPos, gameLocal.msec );
	gameRenderWorld->DebugLine( colorYellow, orientationJointPos, orientationJointPos + orientationJointAxis[ 0 ] * 32.0f, gameLocal.msec );
	gameRenderWorld->DebugLine( colorGreen, orientationJointPos, orientationJointPos + newLookAng.ToForward() * 48.0f, gameLocal.msec );
#endif

	// determine pitch from joint position
	dir = focusPos - eyepos;
	dir.NormalizeFast();
	orientationJointAxis.ProjectVector( dir, localDir );
	newLookAng.pitch = -idMath::AngleNormalize180( localDir.ToPitch() );
	newLookAng.roll	= 0.0f;

	diff = newLookAng - lookAng;

	if ( eyeAng != diff ) {
		eyeAng = diff;
		eyeAng.Clamp( eyeMin, eyeMax );
		idAngles angDelta = diff - eyeAng;
		if ( !angDelta.Compare( ang_zero, 0.1f ) ) {
			alignHeadTime = gameLocal.time;
		} else {
			alignHeadTime = gameLocal.time + static_cast<int>(( 0.5f + 0.5f * gameLocal.random.RandomFloat() ) * focusAlignTime);
		}
	}

	if ( idMath::Fabs( newLookAng.yaw ) < 0.1f ) {
		alignHeadTime = gameLocal.time;
	}

	if ( ( gameLocal.time >= alignHeadTime ) || ( gameLocal.time < forceAlignHeadTime ) ) {
		alignHeadTime = gameLocal.time + static_cast<int>(( 0.5f + 0.5f * gameLocal.random.RandomFloat() ) * focusAlignTime);
		destLookAng = newLookAng;
		destLookAng.Clamp( lookMin, lookMax );
	}

	diff = destLookAng - lookAng;
	if ( ( lookMin.pitch == -180.0f ) && ( lookMax.pitch == 180.0f ) ) {
		if ( ( diff.pitch > 180.0f ) || ( diff.pitch <= -180.0f ) ) {
			diff.pitch = 360.0f - diff.pitch;
		}
	}
	if ( ( lookMin.yaw == -180.0f ) && ( lookMax.yaw == 180.0f ) ) {
		if ( diff.yaw > 180.0f ) {
			diff.yaw -= 360.0f;
		} else if ( diff.yaw <= -180.0f ) {
			diff.yaw += 360.0f;
		}
	}
	lookAng = lookAng + diff * headFocusRate;
	lookAng.Normalize180();

	jointAng.roll = 0.0f;

	if (AI_AlertLevel >= thresh_5)
	{
		// use combat look joints in combat

		for( i = 0; i < lookJointsCombat.Num(); i++ ) 
		{
			jointAng.pitch	= lookAng.pitch * lookJointAnglesCombat[ i ].pitch;
			jointAng.yaw	= lookAng.yaw * lookJointAnglesCombat[ i ].yaw;
			animator.SetJointAxis( lookJointsCombat[ i ], JOINTMOD_WORLD, jointAng.ToMat3() );
		}
	}
	else
	{
		for( i = 0; i < lookJoints.Num(); i++ ) 
		{
			jointAng.pitch	= lookAng.pitch * lookJointAngles[ i ].pitch;
			jointAng.yaw	= lookAng.yaw * lookJointAngles[ i ].yaw;
			animator.SetJointAxis( lookJoints[ i ], JOINTMOD_WORLD, jointAng.ToMat3() );
		}
	}

	if ( move.moveType == MOVETYPE_FLY ) {
		// lean into turns
		AdjustFlyingAngles();
	}

	if ( headEnt ) {
		idAnimator *headAnimator = headEnt->GetAnimator();

		if ( allowEyeFocus ) {
			idMat3 eyeAxis = ( lookAng + eyeAng ).ToMat3();
			idMat3 headTranspose = headEnt->GetPhysics()->GetAxis().Transpose();
			axis =  eyeAxis * orientationJointAxis;
			left = axis[ 1 ] * eyeHorizontalOffset;
			eyepos -= headEnt->GetPhysics()->GetOrigin();
			headAnimator->SetJointPos( leftEyeJoint, JOINTMOD_WORLD_OVERRIDE, eyepos + ( axis[ 0 ] * 64.0f + left ) * headTranspose );
			headAnimator->SetJointPos( rightEyeJoint, JOINTMOD_WORLD_OVERRIDE, eyepos + ( axis[ 0 ] * 64.0f - left ) * headTranspose );
		} else {
			headAnimator->ClearJoint( leftEyeJoint );
			headAnimator->ClearJoint( rightEyeJoint );
		}
	} else {
		if ( allowEyeFocus ) {
			idMat3 eyeAxis = ( lookAng + eyeAng ).ToMat3();
			axis =  eyeAxis * orientationJointAxis;
			left = axis[ 1 ] * eyeHorizontalOffset;
			eyepos += axis[ 0 ] * 64.0f - physicsObj.GetOrigin();
			animator.SetJointPos( leftEyeJoint, JOINTMOD_WORLD_OVERRIDE, eyepos + left );
			animator.SetJointPos( rightEyeJoint, JOINTMOD_WORLD_OVERRIDE, eyepos - left );
		} else {
			animator.ClearJoint( leftEyeJoint );
			animator.ClearJoint( rightEyeJoint );
		}
	}

	return true;
}

/***********************************************************************

idCombatNode

***********************************************************************/

const idEventDef EV_CombatNode_MarkUsed( "markUsed" );

CLASS_DECLARATION( idEntity, idCombatNode )
	EVENT( EV_CombatNode_MarkUsed,				idCombatNode::Event_MarkUsed )
	EVENT( EV_Activate,							idCombatNode::Event_Activate )
END_CLASS

/*
=====================
idCombatNode::idCombatNode
=====================
*/
idCombatNode::idCombatNode( void ) {
	min_dist = 0.0f;
	max_dist = 0.0f;
	cone_dist = 0.0f;
	min_height = 0.0f;
	max_height = 0.0f;
	cone_left.Zero();
	cone_right.Zero();
	offset.Zero();
	disabled = false;
}

/*
=====================
idCombatNode::Save
=====================
*/
void idCombatNode::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( min_dist );
	savefile->WriteFloat( max_dist );
	savefile->WriteFloat( cone_dist );
	savefile->WriteFloat( min_height );
	savefile->WriteFloat( max_height );
	savefile->WriteVec3( cone_left );
	savefile->WriteVec3( cone_right );
	savefile->WriteVec3( offset );
	savefile->WriteBool( disabled );
}

/*
=====================
idCombatNode::Restore
=====================
*/
void idCombatNode::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( min_dist );
	savefile->ReadFloat( max_dist );
	savefile->ReadFloat( cone_dist );
	savefile->ReadFloat( min_height );
	savefile->ReadFloat( max_height );
	savefile->ReadVec3( cone_left );
	savefile->ReadVec3( cone_right );
	savefile->ReadVec3( offset );
	savefile->ReadBool( disabled );
}

/*
=====================
idCombatNode::Spawn
=====================
*/
void idCombatNode::Spawn( void ) {
	float fov;
	float yaw;
	float height;

	min_dist = spawnArgs.GetFloat( "min" );
	max_dist = spawnArgs.GetFloat( "max" );
	height = spawnArgs.GetFloat( "height" );
	fov = spawnArgs.GetFloat( "fov", "60" );
	offset = spawnArgs.GetVector( "offset" );

	const idVec3 &org = GetPhysics()->GetOrigin() + offset;
	min_height = org.z - height * 0.5f;
	max_height = min_height + height;

	const idMat3 &axis = GetPhysics()->GetAxis();
	yaw = axis[ 0 ].ToYaw();

	idAngles leftang( 0.0f, yaw + fov * 0.5f - 90.0f, 0.0f );
	cone_left = leftang.ToForward();

	idAngles rightang( 0.0f, yaw - fov * 0.5f + 90.0f, 0.0f );
	cone_right = rightang.ToForward();

	disabled = spawnArgs.GetBool( "start_off" );
}

/*
=====================
idCombatNode::IsDisabled
=====================
*/
bool idCombatNode::IsDisabled( void ) const {
	return disabled;
}

/*
=====================
idCombatNode::DrawDebugInfo
=====================
*/
void idCombatNode::DrawDebugInfo( void ) {
	idEntity		*ent;
	idCombatNode	*node;
	idPlayer		*player = gameLocal.GetLocalPlayer();
	idVec4			color;
	idBounds		bounds( idVec3( -16, -16, 0 ), idVec3( 16, 16, 0 ) );

	for( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		if ( !ent->IsType( idCombatNode::Type ) ) {
			continue;
		}

		node = static_cast<idCombatNode *>( ent );
		if ( node->disabled ) {
			color = colorMdGrey;
		} else if ( player && node->EntityInView( player, player->GetPhysics()->GetOrigin() ) ) {
			color = colorYellow;
		} else {
			color = colorRed;
		}

		idVec3 leftDir( -node->cone_left.y, node->cone_left.x, 0.0f );
		idVec3 rightDir( node->cone_right.y, -node->cone_right.x, 0.0f );
		idVec3 org = node->GetPhysics()->GetOrigin() + node->offset;

		bounds[ 1 ].z = node->max_height;

		leftDir.NormalizeFast();
		rightDir.NormalizeFast();

		const idMat3 &axis = node->GetPhysics()->GetAxis();
		float cone_dot = node->cone_right * axis[ 1 ];
		if ( idMath::Fabs( cone_dot ) > 0.1 ) {
			float cone_dist = node->max_dist / cone_dot;
			idVec3 pos1 = org + leftDir * node->min_dist;
			idVec3 pos2 = org + leftDir * cone_dist;
			idVec3 pos3 = org + rightDir * node->min_dist;
			idVec3 pos4 = org + rightDir * cone_dist;

			gameRenderWorld->DebugLine( color, node->GetPhysics()->GetOrigin(), ( pos1 + pos3 ) * 0.5f, gameLocal.msec );
			gameRenderWorld->DebugLine( color, pos1, pos2, gameLocal.msec );
			gameRenderWorld->DebugLine( color, pos1, pos3, gameLocal.msec );
			gameRenderWorld->DebugLine( color, pos3, pos4, gameLocal.msec );
			gameRenderWorld->DebugLine( color, pos2, pos4, gameLocal.msec );
			gameRenderWorld->DebugBounds( color, bounds, org, gameLocal.msec );
		}
	}
}

/*
=====================
idCombatNode::EntityInView
=====================
*/
bool idCombatNode::EntityInView( idActor *actor, const idVec3 &pos ) {
	if ( !actor || ( actor->health <= 0 ) ) {
		return false;
	}

	const idBounds &bounds = actor->GetPhysics()->GetBounds();
	if ( ( pos.z + bounds[ 1 ].z < min_height ) || ( pos.z + bounds[ 0 ].z >= max_height ) ) {
		return false;
	}

	const idVec3 &org = GetPhysics()->GetOrigin() + offset;
	const idMat3 &axis = GetPhysics()->GetAxis();
	idVec3 dir = pos - org;
	float  dist = dir * axis[ 0 ];

	if ( ( dist < min_dist ) || ( dist > max_dist ) ) {
		return false;
	}

	float left_dot = dir * cone_left;
	if ( left_dot < 0.0f ) {
		return false;
	}

	float right_dot = dir * cone_right;
	if ( right_dot < 0.0f ) {
		return false;
	}

	return true;
}

/*
=====================
idCombatNode::Event_Activate
=====================
*/
void idCombatNode::Event_Activate( idEntity *activator ) {
	disabled = !disabled;
}

/*
=====================
idCombatNode::Event_MarkUsed
=====================
*/
void idCombatNode::Event_MarkUsed( void ) {
	if ( spawnArgs.GetBool( "use_once" ) ) {
		disabled = true;
	}
}

// DarkMod sound propagation functions

void idAI::SPLtoLoudness( SSprParms *propParms )
{
	// put in frequency, duration and bandwidth effects here
	propParms->loudness = propParms->propVol;
}

bool idAI::CheckHearing( SSprParms *propParms )
{
	bool returnval(false);

	if (propParms->loudness > m_AudThreshold)
	{
		returnval = true;
	}

	return returnval;
}

void idAI::HearSound(SSprParms *propParms, float noise, const idVec3& origin)
{
	if (m_bIgnoreAlerts) return;

	DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("AI Hear Sound called\r");
	// TODO:
	// Modify loudness by propVol/noise ratio,
	// looking up a selectivity spawnarg on the AI to
	// see how well the AI distinguishes signal from noise

	//psychLoud = pow(2, (propParms->loudness - threshold)/10 );
	// this scale didn't make much sense for the alerts ingame
	// because the numbers would have been very close together for
	// very different amounts of alert.
	// It is better to keep it in dB.

	// and so alert units are born!

	/**
	* NOTE: an AlertLevel of 1 constitutes just barely seeing something
	* out of the corner of your eye, or just barely hearing a whisper
	* of a sound for a short instant.  An AlertLevel of 10 is seeing/hearing twice
	* as much, 20 is four times as much, etc.
	**/

	// angua: alert increase is scaled by alertFactor (defined on prpagated sound). 
	// This way, different sounds can result in different alert increase at the same volume
	float psychLoud = 1 + (propParms->loudness - m_AudThreshold) * propParms->alertFactor;
	
	// angua: alert increase can not exceed alertMax (defined on propagated sound)
	if (psychLoud > propParms->alertMax)
	{
		psychLoud = propParms->alertMax;
	}

	// don't alert the AI if they're deaf, or this is not a strong enough
	//	alert to overwrite another alert this frame
	if (GetAcuity("aud") > 0 && psychLoud > m_AlertLevelThisFrame)
	{
		AI_HEARDSOUND = true;
		m_SoundDir = origin;

		if (propParms->maker->IsType(idActor::Type))
		{
			m_AlertedByActor = static_cast<idActor *>(propParms->maker);
		}
		else
		{
			// greebo: Take the responsible actor for motion sound
			idActor* responsibleActor = propParms->maker->m_MovedByActor.GetEntity();
			if (responsibleActor != NULL)
			{
				m_AlertedByActor = responsibleActor;
			}
		}

		psychLoud *= GetAcuity("aud");

		// angua: the actor who produced this noise is either unknown or an enemy
		// alert now
		if (!m_AlertedByActor.GetEntity() || IsEnemy(m_AlertedByActor.GetEntity()))
		{
			AlertAI( "aud", psychLoud );

			// greebo: Notify the currently active state
			mind->GetState()->OnAudioAlert();
		}

		// Retrieve the messages from the other AI, if there are any
		if (propParms->makerAI != NULL)
		{
			for (int i = 0; i < propParms->makerAI->m_Messages.Num(); i++)
			{
				mind->GetState()->OnAICommMessage(*propParms->makerAI->m_Messages[i], psychLoud);
			}
		}

		if( cv_spr_show.GetBool() )
		{
			gameRenderWorld->DrawText( va("Alert: %.2f", psychLoud), 
				(GetEyePosition() - GetPhysics()->GetGravityNormal() * 55.0f), 0.25f, 
				colorGreen, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec * 30);
		}

		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("AI %s HEARD a sound\r", name.c_str() );

		if( cv_ai_debug.GetBool() )
			gameLocal.Printf("AI %s HEARD a sound\n", name.c_str() );
	}
}

void idAI::AlertAI(const char *type, float amount)
{
	DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("AlertAI called\r");

	if (m_bIgnoreAlerts)
	{
		return;
	}

	float acuity = GetAcuity(type);
	// Calculate the amount the current AI_AlertLevel is about to be increased
	// float alertInc = amount * acuity * 0.01f; // Acuity is defaulting to 100 (= 100%)
	// angua: alert amount already includes acuity

	float alertInc = amount;

	// Ignore actors in notarget mode
	idActor* actor = m_AlertedByActor.GetEntity();

	if (actor != NULL && actor->fl.notarget)
	{
		// Actor is set to notarget, quit
		return;
	}

	if (m_AlertGraceTime)
	{
		DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("Grace period active, testing... \r");
		if (gameLocal.time > m_AlertGraceStart + m_AlertGraceTime)
		{
			DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("Grace period found to have expired. Resetting. \r");
			m_AlertGraceTime = 0;
			m_AlertGraceActor = NULL;
			m_AlertGraceStart = 0;
			m_AlertGraceThresh = 0;
			m_AlertGraceCount = 0;
			m_AlertGraceCountLimit = 0;
		}
		else if (alertInc < m_AlertGraceThresh && 
				  actor != NULL &&
				  actor == m_AlertGraceActor.GetEntity() && 
				  m_AlertGraceCount < m_AlertGraceCountLimit)
		{
			DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("Grace period allowed, ignoring alert. \r");
			m_AlertGraceCount++;

			// Quick hack: Large lightgem values and visual alerts override the grace period count faster
			if (AI_VISALERT)
			{
				// greebo: Let the alert grace count increase by 12.5% of the current lightgem value
				// The maximum increase is therefore 32/8 = 4 based on DARKMOD_LG_MAX at the time of writing.
				if (actor->IsType(idPlayer::Type))
				{
					idPlayer* player = static_cast<idPlayer*>(actor);
					m_AlertGraceCount += static_cast<int>(idMath::Rint(player->GetCurrentLightgemValue() * 0.125f));
				}
			}
			return;
		}
		DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("Alert %f above threshold %f, or actor is not grace period actor\r", alertInc, m_AlertGraceThresh);
	}

	// set the last alert value so that simultaneous alerts only overwrite if they are greater than the value
	m_AlertLevelThisFrame = amount;

	// The grace check has failed, increase the AI_AlertLevel float by the increase amount
	float newAlertLevel = AI_AlertLevel + alertInc;
	SetAlertLevel(newAlertLevel);
	m_lastAlertLevel = newAlertLevel;

	DM_LOG(LC_AI, LT_DEBUG)LOGSTRING( "AI ALERT: AI %s alerted by alert type \"%s\",  amount %f (modified by acuity %f).  Total alert level now: %f\r", name.c_str(), type, amount, acuity, (float) AI_AlertLevel );

	if( cv_ai_debug.GetBool() )
		gameLocal.Printf("[TDM AI] ALERT: AI %s alerted by alert type \"%s\", base amount %f, modified by acuity %f percent.  Total alert level now: %f\n", name.c_str(), type, amount, acuity, (float) AI_AlertLevel );

	if (gameLocal.isNewFrame)
	{
		// AI has been alerted, set the boolean
		AI_ALERTED = true;
	}

	// Objectives callback
	gameLocal.m_MissionData->AlertCallback( this, m_AlertedByActor.GetEntity(), static_cast<int>(AI_AlertIndex) );
}

void idAI::SetAlertLevel(float newAlertLevel)
{
	// greebo: Clamp the (log) alert number to twice the combat threshold.
	if (newAlertLevel > thresh_5*2)
	{
		newAlertLevel = thresh_5*2;
	}
	else if (newAlertLevel < 0)
	{
		newAlertLevel = 0;
	}

	bool alertRising = (newAlertLevel > AI_AlertLevel);

	if (alertRising)
	{
		GetMemory().lastAlertRiseTime = gameLocal.time;
	}
	
	if (AI_DEAD || AI_KNOCKEDOUT) return;
	
	AI_AlertLevel = newAlertLevel;
	DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("idAI::SetAlertLevel %s Set AI_AlertLevel = %.2f\r", name.c_str(), newAlertLevel);

	if (AI_AlertLevel > m_maxAlertLevel)
	{
		m_maxAlertLevel = AI_AlertLevel;
	}
	
	// grace period vars
	float grace_time;
	float grace_frac;
	int grace_count;

	// How long should this alert level last, and which alert index should we be in now?
	if (newAlertLevel >= thresh_4)
	{
		// greebo: Only allow switching to combat if a valid enemy is set.
		if (newAlertLevel >= thresh_5)
		{
			if (GetEnemy() != NULL) 
			{
				// We have an enemy, raise the index
				m_prevAlertIndex = AI_AlertIndex;
				AI_AlertIndex = ai::ECombat;
			}
			else
			{
				// No enemy, can't switch to Combat mode
				m_prevAlertIndex = AI_AlertIndex;
				AI_AlertIndex = ai::EAgitatedSearching;
				// Set the alert level back to just below combat threshold
				AI_AlertLevel = thresh_5 - 0.01;
			}
		}
		else
		{
			m_prevAlertIndex = AI_AlertIndex;
			AI_AlertIndex = ai::EAgitatedSearching;
		}
		grace_time = m_gracetime_4;
		grace_frac = m_gracefrac_4;
		grace_count = m_gracecount_4;
	}
	else if (newAlertLevel >= thresh_3)
	{
		m_prevAlertIndex = AI_AlertIndex;
		AI_AlertIndex = ai::EInvestigating;
		grace_time = m_gracetime_3;
		grace_frac = m_gracefrac_3;
		grace_count = m_gracecount_3;
	}
	else if (newAlertLevel >= thresh_2)
	{
		m_prevAlertIndex = AI_AlertIndex;
		AI_AlertIndex = ai::ESuspicious;
		grace_time = m_gracetime_2;
		grace_frac = m_gracefrac_2;
		grace_count = m_gracecount_2;
	}
	else if (newAlertLevel >= thresh_1)
	{
		m_prevAlertIndex = AI_AlertIndex;
		AI_AlertIndex = ai::EObservant;
		grace_time = m_gracetime_1;
		grace_frac = m_gracefrac_1;
		grace_count = m_gracecount_1;
	}
	else
	{
		m_prevAlertIndex = AI_AlertIndex;
		AI_AlertIndex = ai::ERelaxed;
		grace_time = 0.0;
		grace_frac = 0.0;
		grace_count = 0;
	}

	// Tels: When the AI_AlertIndex increased, detach all bound entities
	// that have set "unbindonalertIndex" higher or equal:
    if (m_prevAlertIndex < AI_AlertIndex)
	{
		DetachOnAlert( AI_AlertIndex );
	}

	// greebo: Remember the highest alert index
	if (AI_AlertIndex > m_maxAlertIndex)
	{
		m_maxAlertIndex = AI_AlertIndex;
	}
	
	// Begin the grace period
	if (alertRising)
	{
		Event_SetAlertGracePeriod( grace_frac, grace_time, grace_count );
	}
}

bool idAI::AlertIndexIncreased() 
{
	return (AI_AlertIndex > m_prevAlertIndex);
}

float idAI::GetAcuity(const char *type) const
{
	float returnval(-1);

	// Try to lookup the ID in the hashindex. This corresponds to an entry in m_acuityNames.
	int ind = g_Global.m_AcuityHash.First( g_Global.m_AcuityHash.GenerateKey(type, false) );
//	DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Retrived Acuity index %d for type %s\r", ind, type);

	if (ind == -1 )
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("AI %s attempted to query nonexistant acuity type: %s", name.c_str(), type);
		gameLocal.Warning("[AI] AI %s attempted to query nonexistant acuity type: %s", name.c_str(), type);
		return returnval;
	}
	else if (ind > m_Acuities.Num())
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Acuity index %d exceed acuity array size %d!\r", ind, m_Acuities.Num());
		return returnval;
	}

	// Lookup the acuity value using the index from the hashindex
	returnval = m_Acuities[ind];

	// SZ: June 10, 2007
	// Acuities are now modified by alert level
	if (returnval > 0.0)
	{
		if (m_maxAlertLevel >= thresh_5)
		{
			returnval *= cv_ai_acuity_L5.GetFloat();
		}
		else if (m_maxAlertLevel >= thresh_4)
		{
			returnval *= cv_ai_acuity_L4.GetFloat();
		}
		else if (m_maxAlertLevel >= thresh_3)
		{
			returnval *= cv_ai_acuity_L3.GetFloat();
		}
	}

	// angua: drunken AI have reduced acuity, unless they have seen evidence of intruders
	if (spawnArgs.GetBool("drunk", "0") && HasSeenEvidence() == false)
	{
		returnval *= spawnArgs.GetFloat("drunk_acuity_factor", "1");
	}

	//DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Acuity %s = %f\r", type, returnval);

	return returnval;
}

void idAI::SetAcuity( const char *type, float acuity )
{
	int ind;

	ind = g_Global.m_AcuityHash.First( g_Global.m_AcuityHash.GenerateKey( type, false ) );

	if (ind == -1)
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Script on %s attempted to set nonexistant acuity type: %s\r",name.c_str(), type);
		gameLocal.Warning("[AI] Script on %s attempted to set nonexistant acuity type: %s",name.c_str(), type);
		goto Quit;
	}
	else if( ind > m_Acuities.Num() )
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Script on %s attempted to set acuity with an index %d greater than acuities array size %d!\r", ind, m_Acuities.Num() );
		goto Quit;
	}

	m_Acuities[ind] = acuity;

Quit:
	return;
}

idVec3 idAI::GetSndDir( void )
{
	return m_SoundDir;
}

idVec3 idAI::GetVisDir( void )
{
	return m_LastSight;
}

idEntity *idAI::GetTactEnt( void )
{
	return m_TactAlertEnt.GetEntity();
}

void idAI::PerformVisualScan(float timecheck)
{
	// Only perform enemy checks if we are in the player's PVS
	if (GetAcuity("vis") <= 0 || !gameLocal.InPlayerPVS(this))
	{
		return;
	}

	idActor* player = gameLocal.GetLocalPlayer();
	if (m_bIgnoreAlerts || player->fl.notarget)
	{
		// notarget
		return;
	}

	// Ignore dead actors or non-enemies
	if (player->health <= 0 || !IsEnemy(player))
	{
		return;
	}

	if (!CheckFOV(player->GetEyePosition()))
	{
		return;
	}

	// Check the candidate's visibility.
	float visFrac = GetVisibility(player);
	// Do the percentage check
	float randFrac = gameLocal.random.RandomFloat();
	float chance = timecheck / s_VisNormtime * cv_ai_sight_prob.GetFloat() * visFrac;
	if( randFrac > chance)
	{
		//DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Random number check failed: random %f > number %f\r", randFrac, (timecheck / s_VisNormtime) * visFrac );
		return;
	}

	// angua: does not take lighting and FOV into account
	if (!CanSeeExt(player, false, false))
	{
		return;
	}

	// greebo: At this point, the actor is identified as enemy and is visible
	// set AI_VISALERT and the vector for last sighted position
	// Store the position the enemy was visible
	m_LastSight = player->GetPhysics()->GetOrigin();
	AI_VISALERT = true;
	
	// Get the visual alert amount caused by the CVAR setting
	// float incAlert = GetPlayerVisualStimulusAmount();

	// angua: alert increase depends on brightness, distance and acuity
	float incAlert = 4 + 9 * visFrac;

	if (cv_ai_visdist_show.GetFloat() > 0) 
	{
		gameRenderWorld->DrawText("see you!", GetEyePosition() + idVec3(0,0,70), 0.2f, idVec4( 0.5f, 0.00f, 0.00f, 1.00f ), gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, 60 * gameLocal.msec);
		gameRenderWorld->DrawText(va("Alert increase: %.2f", incAlert), GetEyePosition() + idVec3(0,0,60), 0.2f, idVec4( 0.5f, 0.00f, 0.00f, 1.00f ), gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, 60 * gameLocal.msec);
	}

	float newAlertLevel = AI_AlertLevel + incAlert;
	if (newAlertLevel > thresh_5)
	{
		SetEnemy(player);
	}

	// If the alert amount is larger than everything else encountered this frame
	// ignore the previous alerts and remember this actor as enemy.
	if (incAlert > m_AlertLevelThisFrame)
	{
		// Remember this actor
		m_AlertedByActor = player;
		AlertAI("vis", incAlert);

		// Call the visual alert handler on the current state
		mind->GetState()->OnVisualAlert(player);
	}

	DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("AI %s SAW actor %s\r", name.c_str(), player->name.c_str() );

	return;
}


float idAI::GetVisibility( idEntity *ent ) const
{
	// Returns the probability that the entity will be seen within half a second
	// depends on light gem brightness, visual acuity and player distance
	
	float returnval(0);

	// for now, only players may have their visibility checked
	if (!ent->IsType( idPlayer::Type ))
	{
		return returnval;
	}
	idPlayer* player = static_cast<idPlayer*>(ent);

	// angua: probability for being seen (within half a second)
	// this depends only on the brightness of the light gem and the AI's visual acuity
	float clampVal = GetCalibratedLightgemValue();

	/**
	 * angua: within the clampDist, the probability stays constant
	 * the probability decreases linearly towards 0 between clampdist and savedist
	 * both distances are scaled with clampVal
	 */ 
	float clampdist = cv_ai_sightmindist.GetFloat() * clampVal;
	float safedist = clampdist + (cv_ai_sightmaxdist.GetFloat() - cv_ai_sightmindist.GetFloat()) * clampVal;

	idVec3 delta = GetEyePosition() - player->GetEyePosition();
	float dist = delta.Length()*s_DOOM_TO_METERS;

	if (dist < clampdist) 
	{
		returnval = clampVal;
	}
	else 
	{
		if (dist > safedist) 
		{
			returnval = 0;
		}
		else
		{
			returnval = clampVal * (1 - (dist-clampdist)/(safedist-clampdist) );
		}
	}

	if (cv_ai_visdist_show.GetFloat() > 0) 
	{
		idStr alertText(clampVal);
		alertText = "clampVal: "+ alertText;
		gameRenderWorld->DrawText(alertText.c_str(), GetEyePosition() + idVec3(0,0,1), 0.2f, idVec4( 0.15f, 0.15f, 0.15f, 1.00f ), gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec);
		idStr alertText2(clampdist);
		alertText2 = "clampdist: "+ alertText2;
		gameRenderWorld->DrawText(alertText2.c_str(), GetEyePosition() + idVec3(0,0,10), 0.2f, idVec4( 0.15f, 0.15f, 0.15f, 1.00f ), gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec);
		gameRenderWorld->DebugCircle(idVec4( 0.15f, 0.15f, 0.15f, 1.00f ), GetPhysics()->GetOrigin(),idVec3(0,0,1), clampdist / s_DOOM_TO_METERS, 100, gameLocal.msec);
		idStr alertText3(safedist);
		alertText3 = "savedist: "+ alertText3;
		gameRenderWorld->DrawText(alertText3.c_str(), GetEyePosition() + idVec3(0,0,20), 0.2f, idVec4( 0.15f, 0.15f, 0.15f, 1.00f ), gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec);
		gameRenderWorld->DebugCircle(idVec4( 0.15f, 0.15f, 0.15f, 1.00f ), GetPhysics()->GetOrigin(),idVec3(0,0,1), safedist / s_DOOM_TO_METERS, 100, gameLocal.msec);
		idStr alertText4(returnval);
		alertText4 = "returnval: "+ alertText4;
		gameRenderWorld->DrawText(alertText4.c_str(), GetEyePosition() + idVec3(0,0,30), 0.2f, idVec4( 0.15f, 0.15f, 0.15f, 1.00f ), gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec);
		idStr alertText5(dist);
		alertText5 = "distance: "+ alertText5;
		gameRenderWorld->DrawText(alertText5.c_str(), GetEyePosition() + idVec3(0,0,-10), 0.2f, idVec4( 0.15f, 0.15f, 0.15f, 1.00f ), gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec);
	}
	
	return returnval;
}

float idAI::GetCalibratedLightgemValue() const
{
	idPlayer* player = gameLocal.GetLocalPlayer();
	if (player == NULL) return 0.0f;

	float lgem = static_cast<float>(player->GetCurrentLightgemValue());

	float term0 = -0.003f;
	float term1 = 0.03f * lgem;
	float term2 = 0.001f * idMath::Pow16(lgem, 2);
	float term3 = 0.00013f * idMath::Pow16(lgem, 3);
	float term4 = - 0.000011f * idMath::Pow16(lgem, 4);
	float term5 = 0.0000001892f * idMath::Pow16(lgem, 5);

	float clampVal = term0 + term1 + term2 + term3 + term4 + term5;

	clampVal *= GetAcuity("vis");

	if (clampVal > 1)
	{
		clampVal = 1;
	}	

	// Debug output
	if (cv_ai_visdist_show.GetFloat() > 0) 
	{
		idStr alertText5(lgem);
		alertText5 = "lgem: "+ alertText5;
		gameRenderWorld->DrawText(alertText5.c_str(), GetEyePosition() + idVec3(0,0,40), 0.2f, idVec4( 0.15f, 0.15f, 0.15f, 1.00f ), gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec);
	}

	return clampVal;
}

void idAI::TactileAlert(idEntity* tactEnt, float amount)
{

	if (AI_DEAD || AI_KNOCKEDOUT || m_bIgnoreAlerts)
	{
		return;
	}

	if (tactEnt == NULL || CheckTactileIgnore(tactEnt))
	{
		return;
	}

	// The actor is either the touched entity or the originator of the tactile alert
	idActor* responsibleActor = 
		(tactEnt->IsType(idActor::Type)) ? static_cast<idActor*>(tactEnt) : tactEnt->m_SetInMotionByActor.GetEntity();

	if (responsibleActor == NULL)
	{
		return;
	}

	if (IsFriend(responsibleActor))
	{
		if (responsibleActor->health <= 0 || responsibleActor->IsKnockedOut())
		{
			// angua: We've found a friend that is dead or unconscious
			mind->GetState()->OnPersonEncounter(tactEnt, this);
		}
	}
	if (!IsEnemy(responsibleActor)) 
	{
		return; // not an enemy, no alert
	}

	// greebo: We touched an enemy, check if it's an unconscious body or corpse
	if (tactEnt->IsType(idAI::Type) && 
		(static_cast<idAI*>(tactEnt)->AI_DEAD || static_cast<idAI*>(tactEnt)->AI_KNOCKEDOUT))
	{
		// When AI_DEAD or AI_KNOCKEDOUT ignore this alert from now on to avoid
		// re-alerting us every time we touch it again and again
		TactileIgnore(tactEnt);
	}

	// Set the alert amount to the according tactile alert value
	if (amount == -1)
	{
		amount = cv_ai_tactalert.GetFloat();
	}

	// If we got this far, we give the alert
	// NOTE: Latest tactile alert always overrides other alerts
	m_TactAlertEnt = tactEnt;
	m_AlertedByActor = responsibleActor;

	amount *= GetAcuity("tact");
	AlertAI("tact", amount);

	// Notify the currently active state
	mind->GetState()->OnTactileAlert(tactEnt);

	// Set last visual contact location to this location as that is used in case
	// the target gets away
	m_LastSight = tactEnt->GetPhysics()->GetOrigin();

	// If no enemy set so far, set the last visible enemy position.
	if (GetEnemy() == NULL)
	{
		lastVisibleEnemyPos = tactEnt->GetPhysics()->GetOrigin();
	}

	AI_TACTALERT = true;

	if( cv_ai_debug.GetBool() )
	{
		// Note: This can spam the log a lot, so only put it in if cv_ai_debug.GetBool() is true
		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("AI %s FELT entity %s\r", name.c_str(), tactEnt->name.c_str() );
		gameLocal.Printf( "[DM AI] AI %s FELT entity %s\n", name.c_str(), tactEnt->name.c_str() );
	}
}

void idAI::TactileIgnore(idEntity* tactEnt)
{
	tactileIgnoreEntities.insert(tactEnt);
}

bool idAI::CheckTactileIgnore(idEntity* tactEnt)
{
	TactileIgnoreList::iterator i = tactileIgnoreEntities.find(tactEnt);
	if (i != tactileIgnoreEntities.end())
	{
		return true;
	}
	return false;
}




idActor *idAI::FindEnemy(bool useFOV)
{
	// Only perform enemy checks if we are in the player's PVS
	if (!gameLocal.InPlayerPVS(this))
	{
		return NULL;
	}

	// Go through all clients (=players)
	for (int i = 0; i < gameLocal.numClients ; i++)
	{
		idEntity* ent = gameLocal.entities[i];

		if (ent == NULL || ent->fl.notarget || !ent->IsType(idActor::Type))
		{
			// NULL, notarget or non-Actor, continue
			continue;
		}

		idActor* actor = static_cast<idActor*>(ent);

		// Ignore dead actors or non-enemies
		if (actor->health <= 0 || !IsEnemy(actor))
		{
			continue;
		}

		// angua: does not take lighting into account any more, 
		// this is done afterwards using GetVisibility
		if (CanSeeExt(actor, useFOV, false))
		{
			// Enemy actor found and visible, return it 
			return actor;
		}
	}

	return NULL;
}

idActor* idAI::FindEnemyAI(bool useFOV)
{
	pvsHandle_t pvs = gameLocal.pvs.SetupCurrentPVS( GetPVSAreas(), GetNumPVSAreas() );

	float bestDist = idMath::INFINITY;
	idActor* bestEnemy = NULL;

	for (idEntity* ent = gameLocal.activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
		if ( ent->fl.hidden || ent->fl.isDormant || ent->fl.notarget || !ent->IsType( idActor::Type ) ) {
			continue;
		}

		idActor* actor = static_cast<idActor *>( ent );
		if ( ( actor->health <= 0 ) || !( ReactionTo( actor ) & ATTACK_ON_SIGHT ) ) {
			continue;
		}

		if ( !gameLocal.pvs.InCurrentPVS( pvs, actor->GetPVSAreas(), actor->GetNumPVSAreas() ) ) {
			continue;
		}

		idVec3 delta = physicsObj.GetOrigin() - actor->GetPhysics()->GetOrigin();
		float dist = delta.LengthSqr();
		if ( ( dist < bestDist ) && CanSee( actor, useFOV != 0 ) ) {
			bestDist = dist;
			bestEnemy = actor;
		}
	}

	gameLocal.pvs.FreeCurrentPVS(pvs);
	return bestEnemy;
}

idActor* idAI::FindFriendlyAI(int requiredTeam)
{
// This is our return value
	idActor* candidate(NULL);
	// The distance of the nearest found AI
	float bestDist = idMath::INFINITY;

	// Setup the PVS areas of this entity using the PVSAreas set, this returns a handle
	pvsHandle_t pvs(gameLocal.pvs.SetupCurrentPVS( GetPVSAreas(), GetNumPVSAreas()));

	// Iterate through all active entities and find an AI with the given team.
	for (idEntity* ent = gameLocal.activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
		if ( ent == this || ent->fl.hidden || ent->fl.isDormant || !ent->IsType( idActor::Type ) ) {
			continue;
		}

		idActor* actor = static_cast<idActor *>(ent);
		if (actor->health <= 0) {
			continue;
		}

		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Taking actor %s into account\r", actor->name.c_str());

		if (requiredTeam != -1 && actor->team != requiredTeam) {
			// wrong team
			DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Taking actor %s has wrong team: %d\r", actor->name.c_str(), actor->team);
			continue;
		}

		if (!IsFriend(actor))
		{
			DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Actor %s is not on friendly team: %d or has specific relation that is not friendly\r", actor->name.c_str(), actor->team);
			// Not friendly
			continue;
		}

		if (!gameLocal.pvs.InCurrentPVS( pvs, actor->GetPVSAreas(), actor->GetNumPVSAreas())) {
			DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Actor %s is not in PVS\r", actor->name.c_str());
			// greebo: This actor is not in our PVS, skip it
			continue;
		}

		float dist = (physicsObj.GetOrigin() - actor->GetPhysics()->GetOrigin()).LengthSqr();
		if ( (dist < bestDist) && CanSee(actor, true) ) {
			// Actor can be seen and is nearer than the best candidate, save it
			bestDist = dist;
			candidate = actor;
		}
	}

	gameLocal.pvs.FreeCurrentPVS(pvs);
	
	return candidate;

}


/*---------------------------------------------------------------------------------*/

float idAI::GetMaximumObservationDistanceForPoints(const idVec3& p1, const idVec3& p2) const
{
	return LAS.queryLightingAlongLine(p1, p2, NULL, true) * cv_ai_sight_scale.GetFloat() * GetAcuity("vis");
}

float idAI::GetMaximumObservationDistance(idEntity* entity) const
{
	assert(entity != NULL); // don't accept NULL input

	float lightQuotient = entity->GetLightQuotient();
	
	return lightQuotient * cv_ai_sight_scale.GetFloat() * GetAcuity("vis");
}

/*---------------------------------------------------------------------------------*/

float idAI::GetPlayerVisualStimulusAmount() const
{
	float alertAmount = 0.0;

	// Quick fix for blind AI:
	if (GetAcuity("vis") > 0 )
	{
		// float visFrac = GetVisibility( p_playerEntity );
		// float lgem = (float) g_Global.m_DarkModPlayer->m_LightgemValue;
		// Convert to alert units ( 0.6931472 = ln(2) )
		// Old method, commented out
		// alertAmount = 4*log( visFrac * lgem ) / 0.6931472;

		// The current alert number is stored in logarithmic units
		float curAlertLog = AI_AlertLevel;

		// convert current alert from log to linear scale, add, then convert back
		// this might not be as good for performance, but it lets us keep all alerts
		// on the same scale.
		if (curAlertLog > 0)
		{
			// greebo: Convert log to lin units by using Alin = 2^[(Alog-1)/10]
			float curAlertLin = idMath::Pow16(2, (curAlertLog - 1)*0.1f);

			// Increase the linear alert number with the cvar
			curAlertLin += cv_ai_sight_mag.GetFloat();

			// greebo: Convert back to logarithmic alert units
			// The 1.44269 in the equation is the 1/ln(2) used to compensate for using the natural Log16 method.
			curAlertLog = 1 + 10.0f * idMath::Log16(curAlertLin) * 1.442695f;

			// Now calculate the difference in logarithmic units and return it
			alertAmount = curAlertLog - AI_AlertLevel;
		}
		else
		{
			alertAmount = 1;
		}
	}

	return alertAmount;
}

/*---------------------------------------------------------------------------------*/

bool idAI::IsEntityHiddenByDarkness(idEntity* p_entity, const float sightThreshold) const
{
	// Quick test using LAS at entity origin
	idPhysics* p_physics = p_entity->GetPhysics();

	if (p_physics == NULL) 
	{
		return false;	// Not in darkness
	}

	// Use lightgem if it is the player
	if (p_entity->IsType(idPlayer::Type))
	{
		// Get the alert increase amount (log) caused by the CVAR tdm_ai_sight_mag
		// greebo: Commented this out, this is not suitable to detect if player is hidden in darkness
		//float incAlert = GetPlayerVisualStimulusAmount();
		
		// greebo: Check the visibility of the player depending on lgem and distance
		float visFraction = GetCalibratedLightgemValue(); // returns values in [0..1]
/*
		// greebo: Debug output, comment me out
		gameRenderWorld->DrawText(idStr(visFraction), GetEyePosition() + idVec3(0,0,1), 0.11f, colorGreen, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec);
*/
		// Draw debug graphic
		/*if (cv_ai_visdist_show.GetFloat() > 1.0)
		{
			idVec4 markerColor (0.0, 0.0, 0.0, 0.0);

			float percToSeen = 1.0;
			if (sightThreshold > 0 && sightThreshold > incAlert)
			{
				percToSeen = (sightThreshold - incAlert) / sightThreshold;
			}

			// Scale red to green from not perceptable to quickly perceptable
			markerColor.x = idMath::Sin (percToSeen * (idMath::PI / 2.0));
			markerColor.y = idMath::Cos (percToSeen * (idMath::PI / 2.0));

			idVec3 observeFrom = GetEyePosition();

			gameRenderWorld->DebugArrow(markerColor, observeFrom, p_physics->GetOrigin(), 2, cv_ai_visdist_show.GetInteger());
		}*/

		// Very low threshold for visibility
		if (visFraction < sightThreshold)
		{
			// Not visible, entity is hidden in darkness
			return true;
		}
		else
		{
			// Visible, visual stim above threshold
			return false;
		}
	}
	else // Not the player
	{
		float maxDistanceToObserve = GetMaximumObservationDistance(p_entity);

		// Are we close enough to see it in the current light level?
		idVec3 observeFrom = GetEyePosition();
		idVec3 midPoint = p_entity->GetPhysics()->GetAbsBounds().GetCenter();

		if ((observeFrom - midPoint).LengthSqr() > maxDistanceToObserve*maxDistanceToObserve)
		{
			// Draw debug graphic?
			if (cv_ai_visdist_show.GetFloat() > 1.0f)
			{
				idVec3 arrowLength = midPoint - observeFrom;
				arrowLength.Normalize();
				arrowLength *= maxDistanceToObserve;

				// Distance we could see
				gameRenderWorld->DebugArrow(colorRed, observeFrom, observeFrom + arrowLength, 2, cv_ai_visdist_show.GetInteger());

				// Gap to where we want to see
				gameRenderWorld->DebugArrow(colorMagenta, observeFrom + arrowLength, midPoint, 2, cv_ai_visdist_show.GetInteger());
			}

			return true; // hidden by darkness
		}
		else
		{
			// Draw debug graphic?
			if (cv_ai_visdist_show.GetFloat() > 1.0f)
			{
				// We can see to target
				gameRenderWorld->DebugArrow(colorGreen, observeFrom, midPoint, 2, cv_ai_visdist_show.GetInteger());
			}

			return false; // not hidden by darkness
		}
	}
}

/*---------------------------------------------------------------------------------*/

idActor *idAI::FindNearestEnemy( bool useFOV )
{
	idEntity	*ent;
	idActor		*actor, *playerEnemy;
	idActor		*bestEnemy;
	float		bestDist;
	float		dist;
	idVec3		delta;
	pvsHandle_t pvs;

	pvs = gameLocal.pvs.SetupCurrentPVS( GetPVSAreas(), GetNumPVSAreas() );

	bestDist = idMath::INFINITY;
	bestEnemy = NULL;

	for ( ent = gameLocal.activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
		if ( ent->fl.hidden || ent->fl.isDormant || !ent->IsType( idActor::Type ) )
		{
			continue;
		}

		actor = static_cast<idActor *>( ent );
		if ( ( actor->health <= 0 ) || !( ReactionTo( actor ) & ATTACK_ON_SIGHT ) )
		{
			continue;
		}

		if ( !gameLocal.pvs.InCurrentPVS( pvs, actor->GetPVSAreas(), actor->GetNumPVSAreas() ) )
		{
			continue;
		}

		delta = physicsObj.GetOrigin() - actor->GetPhysics()->GetOrigin();
		dist = delta.LengthSqr();
		if ( ( dist < bestDist ) && CanSee( actor, useFOV ) ) {
			bestDist = dist;
			bestEnemy = actor;
		}
	}

	playerEnemy = FindEnemy(false);
	if( !playerEnemy )
		goto Quit;

	delta = physicsObj.GetOrigin() - playerEnemy->GetPhysics()->GetOrigin();
	dist = delta.LengthSqr();

	if( dist < bestDist )
		bestEnemy = playerEnemy;
		bestDist = dist;

Quit:
	gameLocal.pvs.FreeCurrentPVS( pvs );
	return bestEnemy;
}



void idAI::HadTactile( idActor *actor )
{
	if( !actor )
		goto Quit;

	if( IsEnemy( actor) )
		TactileAlert( actor );
	else
	{
		// TODO: FLAG BLOCKED BY FRIENDLY SO THE SCRIPT CAN DO SOMETHING ABOUT IT
	}

	// alert both AI if they bump into eachother
	if( actor->IsEnemy(this)
		&& actor->IsType(idAI::Type) )
	{
		static_cast<idAI *>(actor)->TactileAlert( this );
	}

Quit:
	return;
}

/*
=====================
idAI::GetMovementVolMod
=====================
*/

float idAI::GetMovementVolMod( void )
{
	float returnval;
	bool bCrouched(false);

	if( AI_CROUCH )
		bCrouched = true;

	// figure out which of the 6 cases we have:
	if( !AI_RUN && !AI_CREEP )
	{
		if( !bCrouched )
			returnval = m_stepvol_walk;
		else
			returnval = m_stepvol_crouch_walk;
	}

	// NOTE: running always has priority over creeping
	else if( AI_RUN )
	{
		if( !bCrouched )
			returnval = m_stepvol_run;
		else
			returnval = m_stepvol_crouch_run;
	}

	else if( AI_CREEP )
	{
		if( !bCrouched )
			returnval = m_stepvol_creep;
		else
			returnval = m_stepvol_crouch_creep;
	}

	else
	{
		// something unexpected happened
		returnval = 0;
	}

	return returnval;
}

/*
=====================
idAI::CheckTactile

Modified 5/25/06 , removed trace computation, found better way of checking
=====================
*/
void idAI::CheckTactile()
{
	// Only check tactile alerts if we aren't Dead, KO or already engaged in combat.

	// grayman #2345 - changed to handle the waiting, non-solid state for AI

	bool bumped = false;
	idEntity* blockingEnt = NULL;
	if (!AI_KNOCKEDOUT && !AI_DEAD && (AI_AlertLevel < thresh_5) && !movementSubsystem->IsWaitingNonSolid()) // no bump if I'm waiting
	{
		blockingEnt = physicsObj.GetSlideMoveEntity();
		if (blockingEnt) // grayman #2345 - note what we bumped into
		{
			if (blockingEnt->name != "world") // ignore bumping into the world
			{
				m_tactileEntity = blockingEnt;
			}
		}
		if (blockingEnt && blockingEnt->IsType(idPlayer::Type)) // player has no movement subsystem
		{
			// aesthetics: Dont react to dead player?
			if (blockingEnt->health > 0)
			{
				bumped = true;
			}
		}
		else if (blockingEnt && blockingEnt->IsType(idActor::Type))
		{
			idAI *e = static_cast<idAI*>(blockingEnt);
			if (e && !e->movementSubsystem->IsWaitingNonSolid()) // no bump if other entity is waiting
			{
				bumped = true;
			}
		}
	}

	if (bumped)
	{
		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("TACT: AI %s is bumping actor %s.\r", name.c_str(), blockingEnt->name.c_str() );
		HadTactile(static_cast<idActor*>(blockingEnt));
	}
}

bool idAI::HasSeenEvidence() const
{
	ai::Memory& memory = GetMemory();

	return memory.enemiesHaveBeenSeen
		|| memory.hasBeenAttackedByEnemy
		|| memory.itemsHaveBeenStolen
		|| memory.itemsHaveBeenBroken
		|| memory.unconsciousPeopleHaveBeenFound
		|| memory.deadPeopleHaveBeenFound
		|| spawnArgs.GetBool("alert_idle", "0");
}

/**
* ========================== BEGIN  TDM KNOCKOUT CODE =============================
**/

/*
=====================
idAI::TestKnockoutBlow
=====================
*/

bool idAI::TestKnockoutBlow( idEntity* attacker, const idVec3& dir, trace_t *tr, int location, bool bIsPowerBlow )
{
	DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Attempted KO of AI %s\r", name.c_str());

	if( AI_DEAD )
		return false; // already dead

	if( AI_KNOCKEDOUT )
	{
		AI_PAIN = true;
		AI_DAMAGE = true;

		return false; // already knocked out
	}

	const char* locationName = GetDamageGroup( location );

	DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("AI %s hit with KO object in joint %d corresponding to damage group %s\r", name.c_str(), location, locationName);

	// check if we're hitting the right zone (usually the head)
	if (idStr::Cmp(locationName, m_KoZone) != 0 )
	{
		// Signal the failed KO to the current state
		GetMind()->GetState()->OnFailedKnockoutBlow(attacker, dir, false);
		
		return false; // damage zone not matching
	}

	// Get the KO angles
	float minDotVert = m_KoDotVert;
	float minDotHoriz = m_KoDotHoriz;

	// Check if the AI is above the alert threshold for Immunity
	// Defined the name of the alert threshold in the AI def for generality
	if (AI_AlertIndex >= m_KoAlertImmuneState)
	{
		// abort KO if the AI is immune when alerted
		if( m_bKoAlertImmune )
		{
			return false; // AI is alerted, thus immune
		}

		minDotVert = m_KoAlertDotVert;
		minDotHoriz = m_KoAlertDotHoriz;
	}

	// check if we hit within the angles
	if( m_HeadJointID == INVALID_JOINT )
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Invalid head joint for joint found on AI %s when KO attempted \r", name.c_str());
		return false;
	}

	idVec3 KOSpot;
	idMat3 headAxis;
	// store head joint base position to KOSpot, axis to HeadAxis
	GetJointWorldTransform( m_HeadJointID, gameLocal.time, KOSpot, headAxis );

	KOSpot += headAxis * m_HeadCenterOffset;
	idMat3 headAxisR = headAxis * m_KoRot;

	idVec3 delta = KOSpot - tr->c.point;
	float lenDelta = delta.Length();

	// First, project delta into the horizontal plane of the head
	// Assume HeadAxis[1] is the up direction in head coordinates
	idVec3 deltaH = delta - headAxisR[1] * (headAxisR[1] * delta);
	float lenDeltaH = deltaH.Normalize();

	float dotHoriz = headAxisR[ 0 ] * deltaH;
	// cos (90-zenith) = adjacent / hypotenuse, applied to these vectors
	float dotVert = idMath::Fabs( lenDeltaH / lenDelta );

	// if hit was within the cone
	if (dotHoriz >= minDotHoriz && dotVert >= minDotVert)
	{
		// We just got knocked the taff out!
		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("AI %s was knocked out by a blow to the head\r", name.c_str());
		Event_KO_Knockout(attacker); // grayman #2468

		return true;
	}

	// Signal the failed KO to the current state
	GetMind()->GetState()->OnFailedKnockoutBlow(attacker, dir, true);
	
	return false; // knockout angles missed
}

void idAI::KnockoutDebugDraw( void )
{
	float AngVert(0), AngHoriz(0), radius(0);
	idVec3 KOSpot(vec3_zero), ConeDir(vec3_zero);
	idMat3 HeadAxis(mat3_zero);

	const char * testZone = m_KoZone.c_str();
	if( AI_KNOCKEDOUT || AI_DEAD || testZone[0] == '\0' )
	{
		return;
	}

	// Check if the AI is above the alert threshold for KOing
	// Defined the name of the alert threshold in the AI def for generality
	if( AI_AlertIndex >= m_KoAlertState)
	{
		// Do not display if immune
		if( m_bKoAlertImmune && AI_AlertIndex >= m_KoAlertImmuneState )
			return;

		// reduce the angle on alert, if needed
		AngVert = idMath::ACos( m_KoAlertDotVert );
		AngHoriz = idMath::ACos( m_KoAlertDotHoriz );
	}
	else
	{
		AngVert = idMath::ACos( m_KoDotVert );
		AngHoriz = idMath::ACos( m_KoDotHoriz );
	}

	if( AngVert == 0.0f )
		AngVert = 360.0f;
	if( AngHoriz == 0.0f )
		AngHoriz = 360.0f;

	if( m_HeadJointID == INVALID_JOINT )
	{
		return;
	}

	// store head joint base position to KOSpot, axis to HeadAxis
	GetJointWorldTransform( m_HeadJointID, gameLocal.time, KOSpot, HeadAxis );

	KOSpot += HeadAxis * m_HeadCenterOffset;
	idMat3 HeadAxisR = HeadAxis * m_KoRot;

	// Assumes the head joint is facing the same way as the look joint
	ConeDir = -HeadAxisR[0];

	// vertical angle in green
	radius = AngVert * 0.5f  * 30.0f;
	gameRenderWorld->DebugCone( colorGreen, KOSpot, 30.0f * ConeDir, 0, radius, gameLocal.msec );
	// horizontal angle in red
	radius = AngHoriz * 0.5f * 30.0f;
	gameRenderWorld->DebugCone( colorRed, KOSpot, 30.0f * ConeDir, 0, radius, gameLocal.msec );
}

/*
=====================
idAI::Event_KO_Knockout
=====================
*/

void idAI::Event_KO_Knockout( idEntity* inflictor )
{
	if (m_bCanBeKnockedOut) // grayman #2468 - new place to test ko immunity
	{
		m_koState = KO_BLACKJACK; // grayman #2604
		Knockout(inflictor);
	}
}

/*
=====================
idAI::Event_Gas_Knockout
=====================
*/

void idAI::Event_Gas_Knockout( idEntity* inflictor )
{
	if (m_bCanBeGassed) // grayman #2468 - new place to test gas immunity
	{
		m_koState = KO_GAS; // grayman #2604
		Knockout(inflictor);
	}
}

/*
=====================
idAI::Knockout

Based on idAI::Killed
=====================
*/

void idAI::Knockout( idEntity* inflictor )
{
	idAngles ang;

//	if( !m_bCanBeKnockedOut ) // grayman #2468 - this test has been moved up a level
//		return;

	if( AI_KNOCKEDOUT || AI_DEAD )
	{
		AI_PAIN = true;
		AI_DAMAGE = true;

		return;
	}

	EndAttack();

	// stop all voice sounds
	StopSound( SND_CHANNEL_VOICE, false );
	if ( head.GetEntity() )
	{
		head.GetEntity()->StopSound( SND_CHANNEL_VOICE, false );
		head.GetEntity()->GetAnimator()->ClearAllAnims( gameLocal.time, 100 );
	}

	disableGravity = false;
	move.moveType = MOVETYPE_DEAD;
	m_bAFPushMoveables = false;

	physicsObj.UseFlyMove( false );
	physicsObj.ForceDeltaMove( false );

	// end our looping ambient sound
	StopSound( SND_CHANNEL_AMBIENT, false );

	RemoveAttachments();
	RemoveProjectile();
	StopMove( MOVE_STATUS_DONE );

	ClearEnemy();

	AI_KNOCKEDOUT = true;

	// greebo: Switch the mind to KO state, this will trigger PostKnockout() when
	// the animation is done
	mind->ClearStates();
	mind->PushState(STATE_KNOCKED_OUT);

	// Update TDM objective system
	bool bPlayerResponsible = (inflictor != NULL && inflictor == gameLocal.GetLocalPlayer() );
	gameLocal.m_MissionData->MissionEvent(COMP_KO, this, inflictor, bPlayerResponsible);
	// Mark the body as last moved by the player
	if( bPlayerResponsible )
		m_MovedByActor = gameLocal.GetLocalPlayer();

	// greebo: Stop lipsyncing now
	m_lipSyncActive = false;
}

void idAI::PostKnockOut()
{
	// greebo: Removed StopAnim() for head channel, this caused the AI to open its eyes again
	//headAnim.StopAnim(1);

	legsAnim.StopAnim(1);
	torsoAnim.StopAnim(1);

	headAnim.Disable();
	legsAnim.Disable();
	torsoAnim.Disable();
	// make original self nonsolid
	physicsObj.SetContents( 0 );
	physicsObj.GetClipModel()->Unlink();

	Unbind();

	// swaps the head CM back if a different one was swapped in while conscious
	SwapHeadAFCM( false );

	if ( StartRagdoll() )
	{
		// grayman #2604 - play "gassed" sound if knocked out by gas
		switch (m_koState)
		{
		case KO_BLACKJACK:
			StartSound( "snd_knockout", SND_CHANNEL_VOICE, 0, false, NULL );
			break;
		case KO_GAS:
			StartSound( "snd_airGasp", SND_CHANNEL_VOICE, 0, false, NULL );
			break;
		case KO_NOT:
		default:
			break;
		}
	}

	const char *modelKOd;

	if ( spawnArgs.GetString( "model_knockedout", "", &modelKOd ) )
	{
		// lost soul is only case that does not use a ragdoll and has a model_death so get the death sound in here
		StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
		renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
		SetModel( modelKOd );
		physicsObj.SetLinearVelocity( vec3_zero );
		physicsObj.PutToRest();
		physicsObj.DisableImpact();
	}

	if (spawnArgs.GetBool("set_frobable_on_knockout", "1"))
	{
		// AI becomes frobable on KO
		// greebo: Add a delay before the AI becomes actually frobable
		PostEventMS(&EV_SetFrobable, 750, 1);
	}

	restartParticles = false;

	// drop items
	DropOnRagdoll();
}

/**
* ========================== END TDM KNOCKOUT CODE =============================
**/

/*
=====================
idAI::FoundBody
=====================
*/
void idAI::FoundBody( idEntity *body )
{
	bool bPlayerResp = false;
	if( m_MovedByActor.GetEntity() && m_MovedByActor.GetEntity() == gameLocal.GetLocalPlayer() )
		bPlayerResp = true;

	gameLocal.m_MissionData->MissionEvent( COMP_AI_FIND_BODY, body, bPlayerResp );
}

void idAI::AddMessage(const ai::CommMessagePtr& message)
{
	assert(message != NULL);
	m_Messages.Append(message);
}

void idAI::ClearMessages()
{
	m_Messages.Clear();
}

/*
=====================
idAI::CheckFOV
=====================
*/
bool idAI::CheckFOV( const idVec3 &pos ) const
{
	//DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("idAI::CheckFOV called \r");

	float	dotHoriz, dotVert, lenDelta, lenDeltaH;
	idVec3	delta, deltaH, deltaV, HeadCenter;
	idMat3	HeadAxis;

	// ugliness
	const_cast<idAI *>(this)->GetJointWorldTransform( m_HeadJointID, gameLocal.time, HeadCenter, HeadAxis );
	idMat3 HeadAxisR = m_FOVRot * HeadAxis;

	// Offset to get the center of the head
	HeadCenter += HeadAxis * m_HeadCenterOffset;
	delta = pos - HeadCenter;
	lenDelta = delta.Length(); // Consider LengthFast if error is not too bad

	// First, project delta into the horizontal plane of the head
	// Assume HeadAxis[1] is the up direction in head coordinates
	deltaH = delta - HeadAxisR[1] * (HeadAxisR[1] * delta);
	lenDeltaH = deltaH.Normalize(); // Consider NormalizeFast

	dotHoriz = HeadAxisR[ 0 ] * deltaH;
	// cos (90-zenith) = adjacent / hypotenuse, applied to these vectors
	// Technically this lets them see things behind them, but they won't because 
	// of the horizontal check.
	dotVert = idMath::Fabs( lenDeltaH / lenDelta );

	return ( dotHoriz >= m_fovDotHoriz && dotVert >= m_fovDotVert );
}

void idAI::FOVDebugDraw( void )
{
	float AngVert(0), AngHoriz(0), radius(0);
	idVec3 HeadCenter(vec3_zero), ConeDir(vec3_zero);
	idMat3 HeadAxis(mat3_zero);

	if( AI_KNOCKEDOUT || AI_DEAD || m_HeadJointID == INVALID_JOINT )
	{
		return;
	}

	// probably expensive, but that's okay since this is just for debug mode

	AngVert = idMath::ACos( m_fovDotVert );
	AngHoriz = idMath::ACos( m_fovDotHoriz );

	// store head joint base position to HeadCenter, axis to HeadAxis
	GetJointWorldTransform( m_HeadJointID, gameLocal.time, HeadCenter, HeadAxis );
	idMat3 HeadAxisR =  m_FOVRot * HeadAxis;
	// offset from head joint position to get the true head center
	HeadCenter += HeadAxis * m_HeadCenterOffset;

	ConeDir = HeadAxisR[0];

	// Diverge to keep reasonable cone size
	float coneLength;

	if (AngVert >= (idMath::PI / 4.0f))
	{
		// Fix radius and calculate length
		radius = 60.0f;
		coneLength = radius / idMath::Tan(AngVert);
	}
	else
	{
		// Fix length and calculate radius
		coneLength = 60.0f;
		// SZ: FOVAng is divergence off to one side (idActor::setFOV uses COS(fov/2.0) to calculate m_fovDotHoriz)
		radius = idMath::Tan(AngVert) * coneLength;
	}

	gameRenderWorld->DebugCone( colorBlue, HeadCenter, coneLength * ConeDir, 0, radius, gameLocal.msec );
	
	// now do the same for horizontal FOV angle, orange cone
	if (AngHoriz >= (idMath::PI / 4.0f))
	{
		// Fix radius and calculate length
		radius = 60.0f;
		coneLength = radius / idMath::Tan(AngHoriz);
	}
	else
	{
		// Fix length and calculate radius
		coneLength = 60.0f;
		// SZ: FOVAng is divergence off to one side (idActor::setFOV uses COS(fov/2.0) to calculate m_fovDotHoriz)
		radius = idMath::Tan(AngHoriz) * coneLength;
	}

	gameRenderWorld->DebugCone( colorOrange, HeadCenter, coneLength * ConeDir, 0, radius, gameLocal.msec );
}

moveStatus_t idAI::GetMoveStatus() const
{
	return move.moveStatus;
}

bool idAI::CanReachEnemy()
{
	aasPath_t	path;
	int			toAreaNum;
	int			areaNum;
	idVec3		pos;
	idActor		*enemyEnt;

	enemyEnt = enemy.GetEntity();
	if ( !enemyEnt ) {
		return false;
	}

	if ( move.moveType != MOVETYPE_FLY ) {
		if ( enemyEnt->OnLadder() ) {
			return false;
		}
		enemyEnt->GetAASLocation( aas, pos, toAreaNum );
	}  else {
		pos = enemyEnt->GetPhysics()->GetOrigin();
		toAreaNum = PointReachableAreaNum( pos );
	}

	if ( !toAreaNum ) {
		return false;
	}

	const idVec3 &org = physicsObj.GetOrigin();
	areaNum	= PointReachableAreaNum( org );
	return PathToGoal(path, areaNum, org, toAreaNum, pos, this);
}

bool idAI::MouthIsUnderwater( void )
{
	bool bReturnVal( false );
	idVec3 MouthOffset, MouthPosition;

	idEntity *headEnt = head.GetEntity();

	// *.def file will store the coordinates of the mouth relative ("mouth_offset") to the head origin
	// or relative to the eyes ("eye_height"). This should be defined by the modeler.

	// check for attached head
	if( headEnt )
	{
		MouthPosition = headEnt->GetPhysics()->GetOrigin();

		// add in the mouth offset oriented by head axis
		MouthPosition += headEnt->GetPhysics()->GetAxis() * m_MouthOffset;
	}
	else if( af.IsLoaded() && AI_KNOCKEDOUT )
	{
		MouthPosition = af.GetPhysics()->GetOrigin( m_HeadBodyID );

		// add in the mouth offset oriented by head axis
		MouthPosition += af.GetPhysics()->GetAxis( m_HeadBodyID ) * m_MouthOffset;
	}
	else
	{
		MouthPosition = GetEyePosition();
		MouthPosition.z += m_MouthOffset.z; // grayman #1488 - check at the mouth, not the eyes
	}

	// check if the mouth position is underwater

	int contents = gameLocal.clip.Contents( MouthPosition, NULL, mat3_identity, -1, this );
	bReturnVal = (contents & MASK_WATER) > 0;
	
#if 0
	// grayman #1488
	//
	// For debugging drowning deaths - indicate onscreen whether underwater or breathing, plus health level
	
	idStr str;
	idVec4 color;
	idStr healthLevel;

	if (bReturnVal)
	{
		str = "Drowning";
		color = colorRed;
	}
	else
	{
		str = "Breathing";
		color = colorGreen;
	}
	sprintf( healthLevel, " (%d)", health );
	gameRenderWorld->DrawText((str + healthLevel).c_str(),(GetEyePosition() - GetPhysics()->GetGravityNormal()*60.0f), 
		0.25f, color, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, 150 * gameLocal.msec);
#endif

	return bReturnVal;
}

void idAI::UpdateAir()
{
	// Are we able to drown at all? Has the time for next air check already come?
	if (!m_bCanDrown || gameLocal.time < m_AirCheckTimer)
	{
		return;
	}

	if (MouthIsUnderwater())
	{
		// don't let KO'd AI hold their breath
		if (AI_KNOCKEDOUT)
		{
			m_AirTics = 0;
		}

		m_AirTics--;
	}
	else
	{
		// regain breath twice as fast as losing
		m_AirTics += 2;

		if (m_AirTics > m_AirTicksMax)
		{
			m_AirTics = m_AirTicksMax;
		}
	}

	if (m_AirTics < 0)
	{
		m_AirTics = 0;

		// do the damage, damage_noair is already defined for the player
		Damage(NULL, NULL, vec3_origin, "damage_noair", 1.0f, 0);
	}

	// set the timer for next air check
	m_AirCheckTimer += m_AirCheckInterval;
}

int	idAI::getAirTicks() const {
	return m_AirTics;
}

void idAI::setAirTicks(int airTicks) {
	m_AirTics = airTicks;
	// Clamp to maximum value
	if( m_AirTics > m_AirTicksMax ) {
		m_AirTics = m_AirTicksMax;
	}
}

/*
===================== Lipsync =====================
*/
int idAI::PlayAndLipSync(const char *soundName, const char *animName)
{
	// Play sound
	int duration;
	StartSound( soundName, SND_CHANNEL_VOICE, 0, false, &duration );

	if (cv_ai_bark_show.GetBool())
	{
		gameRenderWorld->DrawText( va("%s", soundName), GetEyePosition(), 0.25f, colorBlue, 
			gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, duration );
	}

	// Do we want to lipsync this sound?
	StopLipSync(); // Assume not

	const char *sound;
	if (spawnArgs.GetString( soundName, "", &sound ))
	{
		const idSoundShader* shader = declManager->FindSound( sound );

		if (shader != NULL && idStr::Cmp(shader->GetDescription(), "nolipsync") != 0)
		{
			// The sound exists and isn't marked "nolipsync", so start the lipsync
			
			// Get the default animation name if necessary
			if (animName == NULL || animName[0]=='\0')
			{
				// Not specified; get the default from a spawnarg.
				// If even the spawnarg doesn't exist, revert to talk.
				animName = spawnArgs.GetString("lipsync_anim_name", "talk");
			}
			
			m_lipSyncActive = true;
			m_lipSyncAnim = GetAnim( ANIMCHANNEL_HEAD, animName );
			m_lipSyncEndTimer = gameLocal.time + duration;
			headAnim.CycleAnim( m_lipSyncAnim );
		}
	}

	return duration;
}

void idAI::Event_PlayAndLipSync( const char *soundName, const char *animName )
{
	// grayman - might have to add drowning check here if anyone
	// ever uses this function in a script.

	idThread::ReturnInt(static_cast<int>(MS2SEC(PlayAndLipSync(soundName, animName))));
}

void idAI::StopLipSync()
{
	if (m_lipSyncActive)
	{
		// Make sure mouth is closed
		headAnim.SetFrame( m_lipSyncAnim, 0 );
		// Halt animation
		headAnim.SetState("Head_Idle", 4);
	}
	m_lipSyncActive = false;
}

/*
===================== Sheathing/drawing weapons =====================
*/

void idAI::DrawWeapon() 
{
	/*const function_t* func = scriptObject.GetFunction("DrawWeapon");
	if (func) {
		idThread* thread = new idThread(func);
		thread->CallFunction(this, func, true);
		thread->DelayedStart(0);
	}*/
	// greebo: Replaced the above thread spawn with an animstate switch
	SetAnimState(ANIMCHANNEL_TORSO, "Torso_DrawWeapon", 4);
}

void idAI::SheathWeapon() 
{
	/*const function_t* func = scriptObject.GetFunction("SheathWeapon");
	if (func) {
		idThread* thread = new idThread(func);
		thread->CallFunction(this, func, true);
		thread->DelayedStart(0);
	}*/
	// greebo: Replaced the above thread spawn with an animstate switch
	SetAnimState(ANIMCHANNEL_TORSO, "Torso_SheathWeapon", 4);
}

void idAI::DropOnRagdoll( void )
{
	idEntity *ent = NULL;
	int mask(0);
	// Id style def_drops
	const idKeyValue *kv = spawnArgs.MatchPrefix( "def_drops", NULL );
	
	// old D3 code for spawning things at AI's feet when they die
	while( kv )
	{
		idDict args;

		args.Set( "classname", kv->GetValue() );
		args.Set( "origin", physicsObj.GetOrigin().ToString() );
		gameLocal.SpawnEntityDef( args );
		kv = spawnArgs.MatchPrefix( "def_drops", kv );
	}

	// Drop TDM style attachments
	for( int i=0; i<m_Attachments.Num(); i++ )
	{
		ent = m_Attachments[i].ent.GetEntity();
		if( !ent || !m_Attachments[i].ent.IsValid() )
			continue;

		// deactivate any melee weapon actions at this point
		if( ent->IsType(CMeleeWeapon::Type) )
		{
			CMeleeWeapon *pWeap = static_cast<CMeleeWeapon *>(ent);
			pWeap->DeactivateAttack();
			pWeap->DeactivateParry();
			pWeap->ClearOwner();
		}
		
		// greebo: Check if we should set some attachments to nonsolid
		// this applies for instance to the builder guard's pauldrons which
		// cause twitching and self-collisions when going down
		if (ent->spawnArgs.GetBool( "drop_set_nonsolid" ))
		{
			int curContents = ent->GetPhysics()->GetContents();

			// ishtvan: Also clear the CONTENTS_CORPSE flag (maybe this was a typo in original code?)
			ent->GetPhysics()->SetContents(curContents & ~(CONTENTS_SOLID|CONTENTS_CORPSE));

			// ishtvan: If this entity was added to the AF, set the AF body nonsolid as well
			idAFBody *EntBody = AFBodyForEnt( ent );
			if( EntBody != NULL )
			{
				int curBodyContents = EntBody->GetClipModel()->GetContents();
				EntBody->GetClipModel()->SetContents( curBodyContents  & ~(CONTENTS_SOLID|CONTENTS_CORPSE) );
			}

			// also have to iterate thru stuff attached to this attachment
			// ishtvan: left this commentd out because I'm not sure if GetTeamChildren is bugged or not
			// don't want to accidentally set all attachments to the AI to nonsolid
			/*
			idList<idEntity *> AttChildren;
			ent->GetTeamChildren( &AttChildren );
			gameLocal.Printf("TEST: drop_set_nonsolid, Num team children = %d", AttChildren.Num() );
			for( int i=0; i < AttChildren.Num(); i++ )
			{
				idPhysics *pChildPhys = AttChildren[i]->GetPhysics();
				if( pChildPhys == NULL )
					continue;

				int childContents = pChildPhys->GetContents();
				pChildPhys->SetContents( childContents & ~(CONTENTS_SOLID|CONTENTS_CORPSE) );
			}
			*/
		}

		bool bDrop = ent->spawnArgs.GetBool( "drop_when_ragdoll" );
		
		if( !bDrop ) {
			continue;
		}

		bool bDropWhenDrawn = ent->spawnArgs.GetBool( "drop_when_drawn" );
		bool bSetSolid = ent->spawnArgs.GetBool( "drop_add_contents_solid" );
		bool bSetCorpse = ent->spawnArgs.GetBool( "drop_add_contents_corpse" );
		bool bSetFrob = ent->spawnArgs.GetBool( "drop_set_frobable" );
		bool bExtinguish = ent->spawnArgs.GetBool("extinguish_on_drop", "0");

		if( bDropWhenDrawn )
		{
			DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("Testing drop weapon %s\r", ent->name.c_str() );
			
			bool bIsMelee = ent->spawnArgs.GetBool( "is_weapon_melee" );

			if( bIsMelee && !GetAttackFlag(COMBAT_MELEE) )
			{
				DM_LOG(LC_AI,LT_DEBUG)LOGSTRING("Melee weapon was not drawn\r" );
				continue;
			}

			bool bIsRanged = ent->spawnArgs.GetBool( "is_weapon_ranged" );

			if( bIsRanged && !GetAttackFlag(COMBAT_RANGED) )
			{
				continue;
			}
		}

		// Proceed with droppage
		DetachInd( i );

		if( bSetSolid )
			mask = CONTENTS_SOLID;
		if( bSetCorpse )
			mask = mask | CONTENTS_CORPSE;

		if( mask )
			ent->GetPhysics()->SetContents( ent->GetPhysics()->GetContents() | mask );

		if( bSetFrob )
			ent->m_bFrobable = true;

		// greebo: Check if we should extinguish the attachment, like torches
		if ( bExtinguish )
		{
			// Get the delay in milliseconds
			int delay = SEC2MS(ent->spawnArgs.GetInt("extinguish_on_drop_delay", "3"));
			if (delay < 0) {
				delay = 0;
			}

			// Schedule the extinguish event
			ent->PostEventMS(&EV_ExtinguishLights, delay);
		}

		ent->GetPhysics()->Activate();
		ent->m_droppedByAI = true; // grayman #1330
	}

	// also perform some of these same operations on attachments to our head,
	// because they are stored differently than attachments to us
	if( head.GetEntity() )
		head.GetEntity()->DropOnRagdoll();
}

int idAI::StartSearchForHidingSpotsWithExclusionArea
(
	const idVec3& hideFromLocation,
	const idVec3& minBounds, 
	const idVec3& maxBounds, 
	const idVec3& exclusionMinBounds, 
	const idVec3& exclusionMaxBounds, 
	int hidingSpotTypesAllowed, 
	idEntity* p_ignoreEntity
)
{
	DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("StartSearchForHidingSpots called.\r");

	// Destroy any current search
	destroyCurrentHidingSpotSearch();

	// Make caller's search bounds
	idBounds searchBounds (minBounds, maxBounds);
	idBounds searchExclusionBounds (exclusionMinBounds, exclusionMaxBounds);

	// greebo: Remember the initial alert position
	ai::Memory& memory = GetMemory();
	memory.alertSearchCenter = memory.alertPos;

	// Get aas
	if (aas != NULL)
	{
		// Allocate object that handles the search
		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Making finder\r");
		bool b_searchCompleted = false;
		m_HidingSpotSearchHandle = CHidingSpotSearchCollection::Instance().getOrCreateSearch
		(
			hideFromLocation, 
			aas, 
			HIDING_OBJECT_HEIGHT,
			searchBounds,
			searchExclusionBounds,
			hidingSpotTypesAllowed,
			p_ignoreEntity,
			gameLocal.framenum,
			b_searchCompleted
		);

		// Wait at least one frame for other AIs to indicate they want to share
		// this search. Return result indicating search is not done yet.
		return 1;
	}
	else
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Cannot perform Event_StartSearchForHidingSpotsWithExclusionArea if no AAS is set for the AI\r");
	
		// Search is done since there is no search
		return 0;
	}
}

int idAI::ContinueSearchForHidingSpots()
{
	DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("ContinueSearchForHidingSpots called.\r");

	// Get hiding spot search instance from handle
	CDarkmodAASHidingSpotFinder* p_hidingSpotFinder = NULL;
	if (m_HidingSpotSearchHandle != NULL_HIDING_SPOT_SEARCH_HANDLE)
	{
		p_hidingSpotFinder = CHidingSpotSearchCollection::Instance().getSearchByHandle(
			m_HidingSpotSearchHandle
		);
	}

	// Make sure search still around
	if (p_hidingSpotFinder == NULL)
	{
		// No hiding spot search to continue
		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("No current hiding spot search to continue\r");
		return 0;
	}
	else
	{
		// Call finder method to continue search
		bool moreProcessingToDo = p_hidingSpotFinder->continueSearchForHidingSpots
		(
			p_hidingSpotFinder->hidingSpotList,
			cv_ai_max_hiding_spot_tests_per_frame.GetInteger(),
			gameLocal.framenum
		);

		// Return result
		if (moreProcessingToDo)
		{
			return 1;
		}

		// No more processing to do at this point
		
		unsigned int refCount;

		// Get finder we just referenced
		CDarkmodAASHidingSpotFinder* p_hidingSpotFinder = 
			CHidingSpotSearchCollection::Instance().getSearchAndReferenceCountByHandle 
			(
				m_HidingSpotSearchHandle,
				refCount
			);

		m_hidingSpots.clear();
		// greebo: Now retrieve our share from the completed hiding spot search
		// Given that three other AI are referencing this hiding spot finder, this AI draws a third.
		p_hidingSpotFinder->hidingSpotList.getOneNth(refCount, m_hidingSpots);

		// Done with search object, dereference so other AIs know how many
		// AIs will still be retrieving points from the search
		CHidingSpotSearchCollection::Instance().dereference (m_HidingSpotSearchHandle);
		m_HidingSpotSearchHandle = NULL_HIDING_SPOT_SEARCH_HANDLE;

		// DEBUGGING
		if (cv_ai_search_show.GetInteger() >= 1.0)
		{
			// Clear the debug draw list and then fill with our results
			p_hidingSpotFinder->debugClearHidingSpotDrawList();
			p_hidingSpotFinder->debugAppendHidingSpotsToDraw (m_hidingSpots);
			p_hidingSpotFinder->debugDrawHidingSpots (cv_ai_search_show.GetInteger());
		}

		DM_LOG(LC_AI, LT_DEBUG)LOGSTRING("Hiding spot search completed\r");
		return 0;
	}
}

idVec3 idAI::GetNthHidingSpotLocation(int hidingSpotIndex)
{
	idVec3 outLocation(0,0,0);

	int numSpots = m_hidingSpots.getNumSpots();

	// In bounds?
	if (hidingSpotIndex >= 0 && hidingSpotIndex < numSpots)
	{
		idBounds areaNodeBounds;
		darkModHidingSpot* p_spot = m_hidingSpots.getNthSpotWithAreaNodeBounds(hidingSpotIndex, areaNodeBounds);
		if (p_spot != NULL)
		{
			outLocation = p_spot->goal.origin;
		}

		if (cv_ai_search_show.GetInteger() >= 1.0)
		{
			idVec4 markerColor (1.0, 1.0, 1.0, 1.0);
			idVec3 arrowLength (0.0, 0.0, 50.0);

			// Debug draw the point to be searched
			gameRenderWorld->DebugArrow
			(
				markerColor,
				outLocation + arrowLength,
				outLocation,
				2,
				cv_ai_search_show.GetInteger()
			);

			// Debug draw the bounds of the area node containing the hiding spot point
			// This may be smaller than the containing AAS area due to octant subdivision.
			gameRenderWorld->DebugBounds
			(
				markerColor,
				areaNodeBounds,
				vec3_origin,
				cv_ai_search_show.GetInteger()
			);
		}

    }
	else
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("Index %d is out of bounds, there are %d hiding spots\r", hidingSpotIndex, numSpots);
	}

	// Return the location
	return outLocation;
}

int idAI::GetSomeOfOtherEntitiesHidingSpotList(idEntity* p_ownerOfSearch)
{
	// Test parameters
	if (p_ownerOfSearch == NULL) 
	{
		return 0;
	}

	// The other entity must be an AI
	idAI* p_otherAI = dynamic_cast<idAI*>(p_ownerOfSearch);
	if (p_otherAI == NULL)
	{
		// Not an AI
		return 0;
	}

	CDarkmodHidingSpotTree& p_othersTree = p_otherAI->m_hidingSpots;
	if (p_othersTree.getNumSpots() <= 1)
	{
		// No points
		return 0;
	}

	// We must clear our current hiding spot search
	destroyCurrentHidingSpotSearch();

	// Move points from their tree to ours
	p_othersTree.getOneNth(2, m_hidingSpots);

	// Done
	return m_hidingSpots.getNumSpots();
}


float idAI::GetArmReachLength()
{
	if (idAAS* aas = GetAAS())
	{
		idVec3 size = aas->GetSettings()->boundingBoxes[0][1];
		return size.z * 0.5;
	}
	else
	{
		return 41;
	}
}

void idAI::NeedToUseElevator(const eas::RouteInfoPtr& routeInfo)
{
	mind->GetState()->NeedToUseElevator(routeInfo);
}


bool idAI::CanUnlock(CBinaryFrobMover *frobMover)
{
	// Look through list of unlockable doors set by spawnarg
	FrobMoverList::iterator i = unlockableDoors.find(frobMover);
	if (i != unlockableDoors.end())
	{
		return true;
	}
	
	// Look through attachments
	int n = frobMover->m_UsedByName.Num();
	for (int i = 0; i < m_Attachments.Num(); i++)
	{
		idEntity* ent = m_Attachments[i].ent.GetEntity();

		if (ent == NULL || !m_Attachments[i].ent.IsValid())
		{
			continue;
		}
		
		for (int j = 0; j < n; j++)
		{
			if (ent->name == frobMover->m_UsedByName[j])
			{
				return true;
			}
		}
	}
	return false;
}

bool idAI::ShouldCloseDoor(CBinaryFrobMover *frobMover)
{
	if (frobMover->spawnArgs.GetBool("ai_should_not_close", "0"))
	{
		// this door should not be closed
		return false;
	}
	if (AI_AlertLevel >= thresh_5)
	{
		// don't close doors during combat
		return false;
	}
	if (frobMover->spawnArgs.GetBool("shouldBeClosed", "0"))
	{
		// this door should really be closed
		return true;
	}
	if (AI_AlertLevel >= thresh_3)
	{
		// don't close other doors while searching
		return false;
	}

	// in all other cases, close the door
	return true;
}

void idAI::PushMove()
{
	// Copy the current movestate structure to the stack
	moveStack.push_back(move);

	if (moveStack.size() > 100)
	{
		gameLocal.Warning("AI MoveStack contains more than 100 moves! (%s)\n", name.c_str());
	}
}

void idAI::PopMove()
{
	if (moveStack.empty())
	{
		return; // nothing to pop from
	}
	
	const idMoveState& saved = moveStack.back(); // Get a reference of the last element
	RestoreMove(saved);
	moveStack.pop_back(); // Remove the last element
}

void idAI::RestoreMove(const idMoveState& saved)
{
	idVec3 goalPos;
	idVec3 dest;

	switch( saved.moveCommand ) {
	case MOVE_NONE :
		StopMove( saved.moveStatus );
		break;

	case MOVE_FACE_ENEMY :
		FaceEnemy();
		break;

	case MOVE_FACE_ENTITY :
		FaceEntity( saved.goalEntity.GetEntity() );
		break;

	case MOVE_TO_ENEMY :
		MoveToEnemy();
		break;

	case MOVE_TO_ENEMYHEIGHT :
		MoveToEnemyHeight();
		break;

	case MOVE_TO_ENTITY :
		MoveToEntity( saved.goalEntity.GetEntity() );
		break;

	case MOVE_OUT_OF_RANGE :
		MoveOutOfRange( saved.goalEntity.GetEntity(), saved.range );
		break;

	case MOVE_TO_ATTACK_POSITION :
		MoveToAttackPosition( saved.goalEntity.GetEntity(), saved.anim );
		break;

	case MOVE_TO_COVER :
		MoveToCover( saved.goalEntity.GetEntity(), lastVisibleEnemyPos );
		break;

	case MOVE_TO_POSITION :
		MoveToPosition( saved.moveDest );
		break;

	case MOVE_TO_POSITION_DIRECT :
		DirectMoveToPosition( saved.moveDest );
		break;

	case MOVE_SLIDE_TO_POSITION :
		SlideToPosition( saved.moveDest, saved.duration );
		break;

	case MOVE_WANDER :
		WanderAround();
		break;
		
	default: break;
	}

	if ( GetMovePos( goalPos ) ) {
		//CheckObstacleAvoidance( goalPos, dest );
	}
}

void idAI::ShowDebugInfo()
{
	// DarkMod: Show debug info
	if( cv_ai_ko_show.GetBool() )
	{
		KnockoutDebugDraw();
	}

	if( cv_ai_fov_show.GetBool() )
	{
		FOVDebugDraw();
	}
	
	if ( cv_ai_dest_show.GetBool() )
	{
		gameRenderWorld->DebugArrow(colorYellow, physicsObj.GetOrigin(), move.moveDest, 5, gameLocal.msec);
	}

	if ( cv_ai_task_show.GetBool())
	{
		idStr str("State: ");
		str += mind->GetState()->GetName() + "\n";
		
		if (GetSubsystem(ai::SubsysSenses)->IsEnabled()) str += "Senses: " + GetSubsystem(ai::SubsysSenses)->GetDebugInfo() + "\n";
		if (GetSubsystem(ai::SubsysMovement)->IsEnabled()) str += "Movement: " + GetSubsystem(ai::SubsysMovement)->GetDebugInfo() + "\n";
		if (GetSubsystem(ai::SubsysCommunication)->IsEnabled()) str += "Comm: " + GetSubsystem(ai::SubsysCommunication)->GetDebugInfo() + "\n";
		if (GetSubsystem(ai::SubsysAction)->IsEnabled()) str += "Action: " + GetSubsystem(ai::SubsysAction)->GetDebugInfo() + "\n";

		gameRenderWorld->DrawText( str, (GetEyePosition() - physicsObj.GetGravityNormal()*35.0f), 0.25f, colorWhite, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec );
	}

	if( cv_ai_alertlevel_show.GetBool() )
	{
		gameRenderWorld->DrawText( va("Alert: %f; Index: %d", (float) AI_AlertLevel, (int)AI_AlertIndex), (GetEyePosition() - physicsObj.GetGravityNormal()*45.0f), 0.25f, colorGreen, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec );
		if (m_AlertGraceStart + m_AlertGraceTime - gameLocal.time > 0)
		{
			gameRenderWorld->DrawText( va("Grace time: %d; Alert count: %d / %d", 
				m_AlertGraceStart + m_AlertGraceTime - gameLocal.time, 
				m_AlertGraceCount, m_AlertGraceCountLimit), 
				GetEyePosition(), 0.25f, colorGreen, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec );
		}
	}

	if (cv_ai_animstate_show.GetBool())
	{
		idStr debugText("Torso: ");
		debugText += GetAnimState(ANIMCHANNEL_TORSO);
		debugText += " Waitstate: ";
		debugText += WaitState(ANIMCHANNEL_TORSO);
		debugText += "\nLegs: ";
		debugText += GetAnimState(ANIMCHANNEL_LEGS);
		debugText += " Waitstate: ";
		debugText += WaitState(ANIMCHANNEL_LEGS);
		debugText += "\nHead: ";
		debugText += GetAnimState(ANIMCHANNEL_HEAD);
		debugText += " Waitstate: ";
		debugText += WaitState(ANIMCHANNEL_LEGS);
		debugText += "\n";

		if (WaitState() != NULL)
		{
			debugText += idStr("Waitstate: ") + WaitState();
		}
		gameRenderWorld->DrawText( debugText, (GetEyePosition() - physicsObj.GetGravityNormal()*-25), 0.20f, colorMagenta, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec );
	}

	if (cv_ai_aasarea_show.GetBool() && aas != NULL)
	{
		idVec3 org = GetPhysics()->GetOrigin();
		int areaNum = PointReachableAreaNum(org);

		idBounds areaBounds = aas->GetAreaBounds(areaNum);
		idVec3 areaCenter = aas->AreaCenter(areaNum);

		idMat3 playerViewMatrix(gameLocal.GetLocalPlayer()->viewAngles.ToMat3());

		gameRenderWorld->DrawText(va("%d", areaNum), areaCenter, 0.2f, colorGreen, playerViewMatrix, 1, gameLocal.msec);
		gameRenderWorld->DebugBox(colorGreen, idBox(areaBounds), gameLocal.msec);
	}

	if (cv_ai_elevator_show.GetBool())
	{
		idMat3 playerViewMatrix(gameLocal.GetLocalPlayer()->viewAngles.ToMat3());

		gameRenderWorld->DrawText(m_HandlingElevator ? "Elevator" : "---", physicsObj.GetOrigin(), 0.2f, m_HandlingElevator ? colorRed : colorGreen, playerViewMatrix, 1, gameLocal.msec);
	}
}

const idStr& idAI::GetNextIdleAnim()
{
	return m_NextIdleAnim;
}

void idAI::SetNextIdleAnim(const idStr& nextIdleAnim)
{
	m_NextIdleAnim = nextIdleAnim;
}

bool idAI::SwitchToConversationState(const idStr& conversationName)
{
	ai::ConversationStatePtr state = 
		boost::static_pointer_cast<ai::ConversationState>(ai::ConversationState::CreateInstance());

	// Convert the name to an index
	int convIndex = gameLocal.m_ConversationSystem->GetConversationIndex(conversationName);

	if (convIndex == -1) 
	{
		DM_LOG(LC_CONVERSATION, LT_ERROR)LOGSTRING("Could not find conversation '%s'.\r", conversationName.c_str());
		return false;
	}

	// Let the state know which conversation is about to be played
	state->SetConversation(convIndex);

	// Switch to this state
	GetMind()->PushState(state);

	return true;
}

void idAI::RemoveTarget(idEntity* target)
{
	idEntity::RemoveTarget( target );

	if (!GetMind()->IsEmpty())
	{
		GetMind()->GetState()->OnChangeTarget(this);
	}
}

void idAI::AddTarget(idEntity* target)
{
	idEntity::AddTarget( target );

	if (!GetMind()->IsEmpty())
	{
		GetMind()->GetState()->OnChangeTarget(this);
	}
}

void idAI::SitDown()
{
	idStr waitState(WaitState());
	if (GetMoveType() != MOVETYPE_ANIM)
	{
		return;
	}
	SetMoveType(MOVETYPE_SIT_DOWN);
	SetWaitState("sit_down");
}

void idAI::GetUp()
{
	moveType_t moveType = GetMoveType();

	if (moveType == MOVETYPE_SIT)
	{
		SetMoveType(MOVETYPE_GET_UP);
		SetWaitState("get_up");

	}
	else if (moveType == MOVETYPE_SLEEP)
	{
		SetMoveType(MOVETYPE_GET_UP_FROM_LYING);
		SetWaitState("get_up_from_lying_down");

		// Reset visual, hearing and tactile acuity
		SetAcuity("vis", m_oldVisualAcuity);		// Tels: fix #2408
		SetAcuity("aud", GetAcuity("aud") * 2);
		SetAcuity("tact", GetAcuity("tact") * 2);
	}
}


void idAI::LayDown()
{
	if (GetMoveType() != MOVETYPE_ANIM)
	{
		return;
	}

	AI_LAY_DOWN_FACE_DIR = idAngles( 0, current_yaw, 0 ).ToForward();

	SetMoveType(MOVETYPE_LAY_DOWN);
	SetWaitState("lay_down");

	// Tels: Sleepers are blind
	m_oldVisualAcuity = GetAcuity("vis");
	SetAcuity("vis", 0);

	// Reduce hearing and tactile acuity by 50%
	// TODO: use spawn args
	SetAcuity("aud", GetAcuity("aud") * 0.5);
	SetAcuity("tact", GetAcuity("tact") * 0.5);
}


float idAI::StealthDamageMult()
{
	// multiply up if unalert
	// Damage to unconscious AI also considered sneak attack
	if( AI_AlertLevel < m_SneakAttackThresh || IsKnockedOut() )
		return m_SneakAttackMult;
	else
		return 1.0;
}

void idAI::SwapHeadAFCM(bool bUseLargerCM)
{
	if (m_HeadJointID == -1) return; // safety check for AI without head

	if (bUseLargerCM && !m_bHeadCMSwapped && spawnArgs.FindKey("blackjack_headbox_mins"))
	{
		idAFBody* headBody = af.GetPhysics()->GetBody(af.BodyForJoint(m_HeadJointID));

		idClipModel *oldClip = headBody->GetClipModel();
		idVec3	CMorig	= oldClip->GetOrigin();
		idMat3	CMaxis	= oldClip->GetAxis();
		int		CMid	= oldClip->GetId();

		oldClip->Unlink();
		// store a copy of the original cm since it will be deleted
		m_OrigHeadCM = new idClipModel(oldClip);

		idBounds HeadBounds;
		spawnArgs.GetVector( "blackjack_headbox_mins", NULL, HeadBounds[0] );
		spawnArgs.GetVector( "blackjack_headbox_maxs", NULL, HeadBounds[1] );

		idTraceModel trm;
		trm.SetupBox( HeadBounds );

		idClipModel *NewClip = new idClipModel( trm );

		// AF bodies want to have their origin at the center of mass
		float MassOut;
		idVec3 COM;
		idMat3 inertiaTensor;
		NewClip->GetMassProperties( 1.0f, MassOut, COM, inertiaTensor );
		NewClip->TranslateOrigin( -COM );
		CMorig += COM * CMaxis;
		
		NewClip->SetContents( oldClip->GetContents() );
		NewClip->Link( gameLocal.clip, this, CMid, CMorig, CMaxis );
		headBody->SetClipModel( NewClip );

		m_bHeadCMSwapped = true;
	}
	// swap back to original CM when going unconscious, if we swapped earlier
	else if( !bUseLargerCM && m_bHeadCMSwapped )
	{
		idAFBody* headBody = af.GetPhysics()->GetBody(af.BodyForJoint(m_HeadJointID));

		idClipModel *oldClip = headBody->GetClipModel();
		idVec3	CMorig	= oldClip->GetOrigin();
		idMat3	CMaxis	= oldClip->GetAxis();
		int		CMid	= oldClip->GetId();

		m_OrigHeadCM->Link( gameLocal.clip, this, CMid, CMorig, CMaxis );
		headBody->SetClipModel( m_OrigHeadCM );

		m_bHeadCMSwapped = false;
		m_OrigHeadCM = NULL;
	}
}

void idAI::CopyHeadKOInfo( void )
{
	idEntity *ent = head.GetEntity();
	if( !ent )
		return;

	// Change this if the list below changes:
	const int numArgs = 14;
	const char *copyArgs[ numArgs ] = { "ko_immune", "ko_spot_offset", "ko_zone", 
		"ko_alert_state", "ko_alert_immune", "ko_alert_immune_state",  "ko_angle_vert", "ko_angle_horiz",
		"ko_angle_alert_vert", "ko_angle_alert_horiz", "ko_rotation", "fov",
		"fov_vert", "fov_rotation"};

	const idKeyValue *tempkv;
	const char *argName;

	// for each spawnarg, overwrite it if it exists on the head
	for( int i=0; i < numArgs; i++ )
	{
		argName = copyArgs[i];
		tempkv = ent->spawnArgs.FindKey( argName );
		if( tempkv != NULL )
			spawnArgs.Set( argName, tempkv->GetValue().c_str() );
	}
}

void idAI::ParseKnockoutInfo()
{
	m_bCanBeKnockedOut = !( spawnArgs.GetBool("ko_immune", "0") );
	m_HeadCenterOffset = spawnArgs.GetVector("ko_spot_offset");
	m_KoZone = spawnArgs.GetString("ko_zone");
	m_KoAlertState = spawnArgs.GetInt("ko_alert_state");
	m_KoAlertImmuneState = spawnArgs.GetInt("ko_alert_immune_state");
	m_bKoAlertImmune = spawnArgs.GetBool("ko_alert_immune");
	idAngles tempAngles = spawnArgs.GetAngles("ko_rotation");
	m_KoRot = tempAngles.ToMat3();
	
	float tempAng;
	tempAng = spawnArgs.GetFloat("ko_angle_vert", "360");
	m_KoDotVert = (float)cos( DEG2RAD( tempAng * 0.5f ) );
	tempAng = spawnArgs.GetFloat("ko_angle_horiz", "360");
	m_KoDotHoriz = (float)cos( DEG2RAD( tempAng * 0.5f ) );
	
	// Only set the alert angles if the spawnargs exist
	const char *tempc1, *tempc2;
	tempc1 = spawnArgs.GetString("ko_angle_alert_vert");
	tempc2 = spawnArgs.GetString("ko_angle_alert_horiz");
	if( tempc1[0] != '\0' )
	{
		tempAng = atof( tempc1 );
		m_KoAlertDotVert = (float)cos( DEG2RAD( tempAng * 0.5f ) );
	}
	else
		m_KoAlertDotVert = m_KoDotVert;
	if( tempc2[0] != '\0' )
	{
		tempAng = atof( tempc2 );
		m_KoAlertDotHoriz = (float)cos( DEG2RAD( tempAng * 0.5f ) );
	}
	else
		m_KoAlertDotHoriz = m_KoDotHoriz;

	// ishtvan: Also set the FOV again, as this may be copied from the head
	// TODO: This was done once already in idActor, do we still need it there or can we remove FOV from idActor?
	float fovDegHoriz, fovDegVert;	
	spawnArgs.GetFloat( "fov", "150", fovDegHoriz );
	// If fov_vert is -1, it will be set the same as horizontal
	spawnArgs.GetFloat( "fov_vert", "-1", fovDegVert );
	SetFOV( fovDegHoriz, fovDegVert );
	tempAngles = spawnArgs.GetAngles("fov_rotation");
	m_FOVRot = tempAngles.ToMat3();
}
