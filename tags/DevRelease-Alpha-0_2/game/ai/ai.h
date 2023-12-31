/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 256 $
 * $Date: 2005-11-26 17:50:07 -0500 (Sat, 26 Nov 2005) $
 * $Author: $
 *
 * $Log$
 * Revision 1.6  2005/11/11 21:00:34  sparhawk
 * SDK 1.3 Merge
 *
 * Revision 1.5  2005/09/26 03:11:01  ishtvan
 * *) tactile alert fixed, added idAI::CheckTactile
 *
 * *) removed old tactile alert methods
 *
 * Revision 1.4  2005/08/22 04:58:21  ishtvan
 * eliminated unnecessary arguments in idAI::HearSound
 *
 * Revision 1.3  2005/04/23 01:42:47  ishtvan
 * Added AI awareness of their crouch/run/creep state
 *
 * Revision 1.2  2005/04/07 09:24:35  ishtvan
 * Added alerts: Visible, audible and tactile, as well as generalized alerts and acuities
 *
 * Revision 1.1.1.1  2004/10/30 15:52:32  sparhawk
 * Initial release
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __AI_H__
#define __AI_H__

/*
===============================================================================

	idAI

===============================================================================
*/

const float	SQUARE_ROOT_OF_2			= 1.414213562f;
const float	AI_TURN_PREDICTION			= 0.2f;
const float	AI_TURN_SCALE				= 60.0f;
const float	AI_SEEK_PREDICTION			= 0.3f;
const float	AI_FLY_DAMPENING			= 0.15f;
const float	AI_HEARING_RANGE			= 2048.0f;
const int	DEFAULT_FLY_OFFSET			= 68;


// used to declare the Dark Mod Acuity values array.
// THIS MUST BE CHANGED if you want more than 15 acuities
static const int s_MAXACUITIES = 15;

#define ATTACK_IGNORE			0
#define ATTACK_ON_DAMAGE		1
#define ATTACK_ON_ACTIVATE		2
#define ATTACK_ON_SIGHT			4

// defined in script/ai_base.script.  please keep them up to date.
typedef enum {
	MOVETYPE_DEAD,
	MOVETYPE_ANIM,
	MOVETYPE_SLIDE,
	MOVETYPE_FLY,
	MOVETYPE_STATIC,
	NUM_MOVETYPES
} moveType_t;

typedef enum {
	MOVE_NONE,
	MOVE_FACE_ENEMY,
	MOVE_FACE_ENTITY,

	// commands < NUM_NONMOVING_COMMANDS don't cause a change in position
	NUM_NONMOVING_COMMANDS,

	MOVE_TO_ENEMY = NUM_NONMOVING_COMMANDS,
	MOVE_TO_ENEMYHEIGHT,
	MOVE_TO_ENTITY, 
	MOVE_OUT_OF_RANGE,
	MOVE_TO_ATTACK_POSITION,
	MOVE_TO_COVER,
	MOVE_TO_POSITION,
	MOVE_TO_POSITION_DIRECT,
	MOVE_SLIDE_TO_POSITION,
	MOVE_WANDER,
	NUM_MOVE_COMMANDS
} moveCommand_t;

typedef enum {
	TALK_NEVER,
	TALK_DEAD,
	TALK_OK,
	TALK_BUSY,
	NUM_TALK_STATES
} talkState_t;

//
// status results from move commands
// make sure to change script/doom_defs.script if you add any, or change their order
//
typedef enum {
	MOVE_STATUS_DONE,
	MOVE_STATUS_MOVING,
	MOVE_STATUS_WAITING,
	MOVE_STATUS_DEST_NOT_FOUND,
	MOVE_STATUS_DEST_UNREACHABLE,
	MOVE_STATUS_BLOCKED_BY_WALL,
	MOVE_STATUS_BLOCKED_BY_OBJECT,
	MOVE_STATUS_BLOCKED_BY_ENEMY,
	MOVE_STATUS_BLOCKED_BY_MONSTER
} moveStatus_t;

#define	DI_NODIR	-1

// obstacle avoidance
typedef struct obstaclePath_s {
	idVec3				seekPos;					// seek position avoiding obstacles
	idEntity *			firstObstacle;				// if != NULL the first obstacle along the path
	idVec3				startPosOutsideObstacles;	// start position outside obstacles
	idEntity *			startPosObstacle;			// if != NULL the obstacle containing the start position 
	idVec3				seekPosOutsideObstacles;	// seek position outside obstacles
	idEntity *			seekPosObstacle;			// if != NULL the obstacle containing the seek position 
} obstaclePath_t;

// path prediction
typedef enum {
	SE_BLOCKED			= BIT(0),
	SE_ENTER_LEDGE_AREA	= BIT(1),
	SE_ENTER_OBSTACLE	= BIT(2),
	SE_FALL				= BIT(3),
	SE_LAND				= BIT(4)
} stopEvent_t;

typedef struct predictedPath_s {
	idVec3				endPos;						// final position
	idVec3				endVelocity;				// velocity at end position
	idVec3				endNormal;					// normal of blocking surface
	int					endTime;					// time predicted
	int					endEvent;					// event that stopped the prediction
	const idEntity *	blockingEntity;				// entity that blocks the movement
} predictedPath_t;

//
// events
//
extern const idEventDef AI_BeginAttack;
extern const idEventDef AI_EndAttack;
extern const idEventDef AI_MuzzleFlash;
extern const idEventDef AI_CreateMissile;
extern const idEventDef AI_AttackMissile;
extern const idEventDef AI_FireMissileAtTarget;
extern const idEventDef AI_AttackMelee;
extern const idEventDef AI_DirectDamage;
extern const idEventDef AI_JumpFrame;
extern const idEventDef AI_EnableClip;
extern const idEventDef AI_DisableClip;
extern const idEventDef AI_EnableGravity;
extern const idEventDef AI_DisableGravity;
extern const idEventDef AI_TriggerParticles;
extern const idEventDef AI_RandomPath;

// DarkMod Events
extern const idEventDef AI_GetRelationEnt;
extern const idEventDef AI_IsEnemy;
extern const idEventDef AI_IsFriend;
extern const idEventDef AI_IsNeutral;
extern const idEventDef AI_GetSndDir;
extern const idEventDef AI_GetVisDir;
extern const idEventDef AI_GetTactEnt;
extern const idEventDef AI_VisScan;
extern const idEventDef AI_Alert;
extern const idEventDef AI_GetAcuity;
extern const idEventDef AI_SetAcuity;
extern const idEventDef AI_ClosestReachableEnemy;

class idPathCorner;

typedef struct particleEmitter_s {
	particleEmitter_s() {
		particle = NULL;
		time = 0;
		joint = INVALID_JOINT;
	};
	const idDeclParticle *particle;
	int					time;
	jointHandle_t		joint;
} particleEmitter_t;

class idMoveState {
public:
							idMoveState();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	moveType_t				moveType;
	moveCommand_t			moveCommand;
	moveStatus_t			moveStatus;
	idVec3					moveDest;
	idVec3					moveDir;			// used for wandering and slide moves
	idEntityPtr<idEntity>	goalEntity;
	idVec3					goalEntityOrigin;	// move to entity uses this to avoid checking the floor position every frame
	int						toAreaNum;
	int						startTime;
	int						duration;
	float					speed;				// only used by flying creatures
	float					range;
	float					wanderYaw;
	int						nextWanderTime;
	int						blockTime;
	idEntityPtr<idEntity>	obstacle;
	idVec3					lastMoveOrigin;
	int						lastMoveTime;
	int						anim;
};

class idAASFindCover : public idAASCallback {
public:
						idAASFindCover( const idVec3 &hideFromPos );
						~idAASFindCover();

	virtual bool		TestArea( const idAAS *aas, int areaNum );

private:
	pvsHandle_t			hidePVS;
	int					PVSAreas[ idEntity::MAX_PVS_AREAS ];
};

class idAASFindAreaOutOfRange : public idAASCallback {
public:
						idAASFindAreaOutOfRange( const idVec3 &targetPos, float maxDist );

	virtual bool		TestArea( const idAAS *aas, int areaNum );

private:
	idVec3				targetPos;
	float				maxDistSqr;
};

class idAASFindAttackPosition : public idAASCallback {
public:
						idAASFindAttackPosition( const idAI *self, const idMat3 &gravityAxis, idEntity *target, const idVec3 &targetPos, const idVec3 &fireOffset );
						~idAASFindAttackPosition();

	virtual bool		TestArea( const idAAS *aas, int areaNum );

private:
	const idAI			*self;
	idEntity			*target;
	idBounds			excludeBounds;
	idVec3				targetPos;
	idVec3				fireOffset;
	idMat3				gravityAxis;
	pvsHandle_t			targetPVS;
	int					PVSAreas[ idEntity::MAX_PVS_AREAS ];
};

class idAI : public idActor {
public:
	CLASS_PROTOTYPE( idAI );

							idAI();
							~idAI();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Spawn( void );
	void					HeardSound( idEntity *ent, const char *action );
	idActor					*GetEnemy( void ) const;
	void					TalkTo( idActor *actor );
	talkState_t				GetTalkState( void ) const;

	bool					GetAimDir( const idVec3 &firePos, idEntity *aimAtEnt, const idEntity *ignore, idVec3 &aimDir ) const;

	void					TouchedByFlashlight( idActor *flashlight_owner );

							// Outputs a list of all monsters to the console.
	static void				List_f( const idCmdArgs &args );

							// Finds a path around dynamic obstacles.
	static bool				FindPathAroundObstacles( const idPhysics *physics, const idAAS *aas, const idEntity *ignore, const idVec3 &startPos, const idVec3 &seekPos, obstaclePath_t &path );
							// Frees any nodes used for the dynamic obstacle avoidance.
	static void				FreeObstacleAvoidanceNodes( void );
							// Predicts movement, returns true if a stop event was triggered.
	static bool				PredictPath( const idEntity *ent, const idAAS *aas, const idVec3 &start, const idVec3 &velocity, int totalTime, int frameTime, int stopEvent, predictedPath_t &path );
							// Return true if the trajectory of the clip model is collision free.
	static bool				TestTrajectory( const idVec3 &start, const idVec3 &end, float zVel, float gravity, float time, float max_height, const idClipModel *clip, int clipmask, const idEntity *ignore, const idEntity *targetEntity, int drawtime );
							// Finds the best collision free trajectory for a clip model.
	static bool				PredictTrajectory( const idVec3 &firePos, const idVec3 &target, float projectileSpeed, const idVec3 &projGravity, const idClipModel *clip, int clipmask, float max_height, const idEntity *ignore, const idEntity *targetEntity, int drawtime, idVec3 &aimDir );

	// Begin Dark Mod Functions:
	
	/**
	* Checks with the global Relationship Manager to see if the
	* other entity is an enemy of this AI.
	**/
	bool IsEnemy( idEntity *other );
	
	/**
	* Interface with Dark Mod Sound Propagation
	**/

	/**
	* Convert Sound Pressure Level from sound propagation
	* to psychoacoustic Loudness for the given AI
	* propVol is read from propParms, and 
	* loudness is set in propParms for later use.
	**/
	void SPLtoLoudness( SSprParms *propParms );
							
	/**
	* CheckHearing returns "true" if the sound is above
	* AI hearing threshold, without taking env. noise into 
	* account.
	**/
	bool CheckHearing( SSprParms *propParms );

	/**
	* Called by soundprop when AI hears a sound Assumes that CheckHearing
	* has been called and that the sound is above threshold without
	* considering environmental noise masking.
	**/
	void HearSound( SSprParms *propParms, float noise, idVec3 origin );

	/**
	* Return the last point at which the AI heard a sound
	* Returns (0,0,0) if the AI didn't hear a sound.
	* Check AI_HEARDSOUND to see if the vector is valid.
	**/
	idVec3 GetSndDir( void );

	/**
	* Return the last point at which an AI glimpsed something suspicious.
	* Returns (0,0,0) if the AI was not visually alerted.
	* Check AI_VISALERT to see if the vector is valid.
	**/
	idVec3 GetVisDir( void );

	/**
	* Returns the entity that the AI is in tactile contact with
	**/
	idEntity *GetTactEnt( void );

	/**
	* Visual Alerts
	**/

	/**
	* Do a visibility calculation based on 3 things:
	* The lightgem value, the distance to entity, and the movement velocity
	* of the entity.
	*
	* The visibility can also be integrated over a number
	* of frames if we need to do that for optimization later.
	**/
	float GetVisibility( idEntity *ent );

	/**
	* Checks enemies in the AI's FOV and calls Alert( "vis", amount )
	* The amount is calculated based on distance and the lightgem
	*
	* It also returns the most visible entity in case the AI script
	* goes into a combat state and needs to get the enemy to attack.
	*
	* For now the check is only done on the player, and only the player
	* may be returned.
	**/
	idActor *VisualScan( float time = 1/60 );

	/**
	* Translate the AI's bounds ahead in the direction that it's walking
	*	and call HadTactile if an enemy is within those bounds.
	**/
	void CheckTactile( idVec3 &dir );

	/**
	* Tactile Alerts:
	*
	* If no amount is entered, of the alert is defined in the global AI 
	* settings def file, and it also gets multiplied by the AI's specific
	* "acuity_tact" key in the spawnargs (defaults to 1.0)
	*
	* The amount is in alert units, so as usual 1 = barely noticible, 
	*	10 = twice as noticable, etc.
	**/
	void TactileAlert( idEntity *ent, float amount = -1 );

	/**
	* This is called in the frame if the AI bumped into another actor.
	* Checks the relationship to the AI, and calls TactileAlert appropriately.
	*
	* If the bumped actor is an enemy of the AI, the AI calls TactileAlert on itself
	*
	* If the bumped actor is an AI, and if this AI is an enemy of the bumped AI,
	* it calls TactileAlert on the bumped AI as well.
	**/
	void HadTactile( idActor *actor );

	/**
	* Generalized alerts and acuities
	**/

	/**
	* Alert the AI.  The first parameter is the alert type (same as acuity type)
	* The second parameter is the alert amount.
	* NOTE: For "alert units," an alert of 1 corresponds to just barely
	* seeing something or just barely hearing a whisper of a sound.
	**/
	void AlertAI( const char *type, float amount );

	/**
	* Returns the float val of the specific AI's acuity
	* Acuity type is a char, from the same list as alert types
	* That list is defined in DarkModGlobals.cpp
	**/
	float GetAcuity( const char *type );

	/**
	* Sets the AI acuity for a certain type of alert.
	**/
	void SetAcuity( const char *type, float acuity );

	/**
	* Get the volume modifier for a given movement type
	* Use the same function as idPlayer::GetMovementVolMod.
	* Unfortunately this is exactly the same as idPlayer::GetMovementVolMod
	* It's bad coding, but that's how D3 wrote the movement vars.
	**/
	float GetMovementVolMod( void );

public:
	/**
	* DarkMod AI Member Vars
	**/
	
	/**
	* Set to true if the AI has been alerted in this frame
	**/
	idScriptBool			AI_ALERTED;

	/**
	* Set these flags so we can tell if the AI is running or creeping
	* for the purposes of playing audible sounds and propagated sounds.
	**/
	idScriptBool			AI_CROUCH;
	idScriptBool			AI_RUN;
	idScriptBool			AI_CREEP;

protected:
	// navigation
	idAAS *					aas;
	int						travelFlags;

	idMoveState				move;
	idMoveState				savedMove;

	float					kickForce;
	bool					ignore_obstacles;
	float					blockedRadius;
	int						blockedMoveTime;
	int						blockedAttackTime;

	// turning
	float					ideal_yaw;
	float					current_yaw;
	float					turnRate;
	float					turnVel;
	float					anim_turn_yaw;
	float					anim_turn_amount;
	float					anim_turn_angles;

	// physics
	idPhysics_Monster		physicsObj;

	// flying
	jointHandle_t			flyTiltJoint;
	float					fly_speed;
	float					fly_bob_strength;
	float					fly_bob_vert;
	float					fly_bob_horz;
	int						fly_offset;					// prefered offset from player's view
	float					fly_seek_scale;
	float					fly_roll_scale;
	float					fly_roll_max;
	float					fly_roll;
	float					fly_pitch_scale;
	float					fly_pitch_max;
	float					fly_pitch;

	bool					allowMove;					// disables any animation movement
	bool					allowHiddenMovement;		// allows character to still move around while hidden
	bool					disableGravity;				// disables gravity and allows vertical movement by the animation
	bool					af_push_moveables;			// allow the articulated figure to push moveable objects
	
	// weapon/attack vars
	bool					lastHitCheckResult;
	int						lastHitCheckTime;
	int						lastAttackTime;
	float					melee_range;
	float					projectile_height_to_distance_ratio;	// calculates the maximum height a projectile can be thrown
	idList<idVec3>			missileLaunchOffset;

	const idDict *			projectileDef;
	mutable idClipModel		*projectileClipModel;
	float					projectileRadius;
	float					projectileSpeed;
	idVec3					projectileVelocity;
	idVec3					projectileGravity;
	idEntityPtr<idProjectile> projectile;
	idStr					attack;

	// chatter/talking
	const idSoundShader		*chat_snd;
	int						chat_min;
	int						chat_max;
	int						chat_time;
	talkState_t				talk_state;
	idEntityPtr<idActor>	talkTarget;

	// cinematics
	int						num_cinematics;
	int						current_cinematic;

	bool					allowJointMod;
	idEntityPtr<idEntity>	focusEntity;
	idVec3					currentFocusPos;
	int						focusTime;
	int						alignHeadTime;
	int						forceAlignHeadTime;
	idAngles				eyeAng;
	idAngles				lookAng;
	idAngles				destLookAng;
	idAngles				lookMin;
	idAngles				lookMax;
	idList<jointHandle_t>	lookJoints;
	idList<idAngles>		lookJointAngles;
	float					eyeVerticalOffset;
	float					eyeHorizontalOffset;
	float					eyeFocusRate;
	float					headFocusRate;
	int						focusAlignTime;

	// special fx
	float					shrivel_rate;
	int						shrivel_start;
	
	bool					restartParticles;			// should smoke emissions restart
	bool					useBoneAxis;				// use the bone vs the model axis
	idList<particleEmitter_t> particles;				// particle data

	renderLight_t			worldMuzzleFlash;			// positioned on world weapon bone
	int						worldMuzzleFlashHandle;
	jointHandle_t			flashJointWorld;
	int						muzzleFlashEnd;
	int						flashTime;

	// joint controllers
	idAngles				eyeMin;
	idAngles				eyeMax;
	jointHandle_t			focusJoint;
	jointHandle_t			orientationJoint;

	// enemy variables
	idEntityPtr<idActor>	enemy;
	idVec3					lastVisibleEnemyPos;
	idVec3					lastVisibleEnemyEyeOffset;
	idVec3					lastVisibleReachableEnemyPos;
	idVec3					lastReachableEnemyPos;
	bool					wakeOnFlashlight;

	// script variables
	idScriptBool			AI_TALK;
	idScriptBool			AI_DAMAGE;
	idScriptBool			AI_PAIN;
	idScriptFloat			AI_SPECIAL_DAMAGE;
	idScriptBool			AI_DEAD;
	idScriptBool			AI_ENEMY_VISIBLE;
	idScriptBool			AI_ENEMY_IN_FOV;
	idScriptBool			AI_ENEMY_DEAD;
	idScriptBool			AI_MOVE_DONE;
	idScriptBool			AI_ONGROUND;
	idScriptBool			AI_ACTIVATED;
	idScriptBool			AI_FORWARD;
	idScriptBool			AI_JUMP;
	idScriptBool			AI_ENEMY_REACHABLE;
	idScriptBool			AI_BLOCKED;
	idScriptBool			AI_OBSTACLE_IN_PATH;
	idScriptBool			AI_DEST_UNREACHABLE;
	idScriptBool			AI_HIT_ENEMY;
	idScriptBool			AI_PUSHED;

	/**
	* The following variables are set as soon as the AI
	* gets a certain type of alert, and never unset by the
	* game code.  They are only unset in scripting.  This is
	* to facilitate different script reactions to different kinds
	* of alerts.
	*
	* It's also done this way so that the AI will know if it has been
	* alerted even if it happened in a frame that the script did not check.
	*
	* This is to facilitate optimization by having the AI check for alerts
	* every N frames rather than every frame.
	**/


	/**
	* Set to true if the AI heard a suspicious sound.
	**/
	idScriptBool			AI_HEARDSOUND;

	/**
	* Set to true if the AI saw something suspicious.
	**/
	idScriptBool			AI_VISALERT;

	/**
	* Set to true if the AI was pushed by or bumped into an enemy.
	**/
	idScriptBool			AI_TACTALERT;

	/**
	* The current alert number of the AI.
	* This is checked by scripting to see if the AI should
	* change alertstates.  This var is very important!
	**/
	idScriptFloat			AI_AlertNum;

	/**
	* Array containing the various AI acuities (visual, aural, tactile, etc)
	**/
	float					m_Acuities[s_MAXACUITIES];

	/**
	* Static visual distance cutoff that is calculated dynamically
	* from the other visual acuity settings.
	**/
	float					m_VisDistMax;

	/**
	* The loudest direction for the last suspicious sound the AI heard
	* is set to NULL if the AI has not yet heard a suspicious sound
	* Note suspicious sounds that are omnidirectional do not set this.
	* If no sound has been propagated it will be (0,0,0).
	**/
	idVec3					m_SoundDir;

	/**
	* Position of the last visual alert
	**/
	idVec3					m_LastSight;

	/**
	* The entity that last issued a tactile alert
	**/
	idEntity *				m_TactAlertEnt;

	//
	// ai/ai.cpp
	//
	void					SetAAS( void );
	virtual	void			DormantBegin( void );	// called when entity becomes dormant
	virtual	void			DormantEnd( void );		// called when entity wakes from being dormant
	void					Think( void );
	void					Activate( idEntity *activator );
	int						ReactionTo( const idEntity *ent );
	bool					CheckForEnemy( void );
	void					EnemyDead( void );
	virtual bool			CanPlayChatterSounds( void ) const;
	void					SetChatSound( void );
	void					PlayChatter( void );
	virtual void			Hide( void );
	virtual void			Show( void );
	idVec3					FirstVisiblePointOnPath( const idVec3 origin, const idVec3 &target, int travelFlags ) const;
	void					CalculateAttackOffsets( void );
	void					PlayCinematic( void );

	// movement
	virtual void			ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse );
	void					GetMoveDelta( const idMat3 &oldaxis, const idMat3 &axis, idVec3 &delta );
	void					CheckObstacleAvoidance( const idVec3 &goalPos, idVec3 &newPos );
	void					DeadMove( void );
	void					AnimMove( void );
	void					SlideMove( void );
	void					AdjustFlyingAngles( void );
	void					AddFlyBob( idVec3 &vel );
	void					AdjustFlyHeight( idVec3 &vel, const idVec3 &goalPos );
	void					FlySeekGoal( idVec3 &vel, idVec3 &goalPos );
	void					AdjustFlySpeed( idVec3 &vel );
	void					FlyTurn( void );
	void					FlyMove( void );
	void					StaticMove( void );

	// damage
	virtual bool			Pain( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );
	virtual void			Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );

	// navigation
	void					KickObstacles( const idVec3 &dir, float force, idEntity *alwaysKick );
	bool					ReachedPos( const idVec3 &pos, const moveCommand_t moveCommand ) const;
	float					TravelDistance( const idVec3 &start, const idVec3 &end ) const;
	int						PointReachableAreaNum( const idVec3 &pos, const float boundsScale = 2.0f ) const;
	bool					PathToGoal( aasPath_t &path, int areaNum, const idVec3 &origin, int goalAreaNum, const idVec3 &goalOrigin ) const;
	void					DrawRoute( void ) const;
	bool					GetMovePos( idVec3 &seekPos );
	bool					MoveDone( void ) const;
	bool					EntityCanSeePos( idActor *actor, const idVec3 &actorOrigin, const idVec3 &pos );
	void					BlockedFailSafe( void );

	// movement control
	void					StopMove( moveStatus_t status );
	bool					FaceEnemy( void );
	bool					FaceEntity( idEntity *ent );
	bool					DirectMoveToPosition( const idVec3 &pos );
	bool					MoveToEnemyHeight( void );
	bool					MoveOutOfRange( idEntity *entity, float range );
	bool					MoveToAttackPosition( idEntity *ent, int attack_anim );
	bool					MoveToEnemy( void );
	bool					MoveToEntity( idEntity *ent );
	bool					MoveToPosition( const idVec3 &pos );
	bool					MoveToCover( idEntity *entity, const idVec3 &pos );
	bool					SlideToPosition( const idVec3 &pos, float time );
	bool					WanderAround( void );
	bool					StepDirection( float dir );
	bool					NewWanderDir( const idVec3 &dest );

	// effects
	const idDeclParticle	*SpawnParticlesOnJoint( particleEmitter_t &pe, const char *particleName, const char *jointName );
	void					SpawnParticles( const char *keyName );
	bool					ParticlesActive( void );

	// turning
	bool					FacingIdeal( void );
	void					Turn( void );
	bool					TurnToward( float yaw );
	bool					TurnToward( const idVec3 &pos );

	// enemy management
	void					ClearEnemy( void );
	bool					EnemyPositionValid( void ) const;
	void					SetEnemyPosition( void );
	void					UpdateEnemyPosition( void );
	void					SetEnemy( idActor *newEnemy );
/**
* DarkMod: Ishtvan note:
* Before I added this, this code was only called in
* Event_FindEnemy, so it could be used by scripting, but
* not by the SDK.  I just moved the code to a new function,
* and made Event_FindEnemy call this.
*
* This was because I needed to use FindEnemy in the visibility
* calculation.
**/
	idActor * FindEnemy( bool useFOV ) ;

/**
* Similarly to FindEnemy, this was previously only an Event_ scripting
* function.  I moved it over to a new SDK function and had the Event_ 
* call it, in case we want to use this later.  It returns the closest
* AI or Player enemy.
*
* It was originally used to get tactile alerts, but is no longer used for that
* IMO we should leave it in though, as we might use it for something later,
* like determining what targets to engage with ranged weapons.
**/	
	idActor * FindNearestEnemy( bool useFOV = true );				
		
			

	// attacks
	void					CreateProjectileClipModel( void ) const;
	idProjectile			*CreateProjectile( const idVec3 &pos, const idVec3 &dir );
	void					RemoveProjectile( void );
	idProjectile			*LaunchProjectile( const char *jointname, idEntity *target, bool clampToAttackCone );
	virtual void			DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage );
	void					DirectDamage( const char *meleeDefName, idEntity *ent );
	bool					TestMelee( void ) const;
	bool					AttackMelee( const char *meleeDefName );
	void					BeginAttack( const char *name );
	void					EndAttack( void );
	void					PushWithAF( void );

	// special effects
	void					GetMuzzle( const char *jointname, idVec3 &muzzle, idMat3 &axis );
	void					InitMuzzleFlash( void );
	void					TriggerWeaponEffects( const idVec3 &muzzle );
	void					UpdateMuzzleFlash( void );
	virtual bool			UpdateAnimationControllers( void );
	void					UpdateParticles( void );
	void					TriggerParticles( const char *jointName );

	// AI script state management
	void					LinkScriptVariables( void );
	void					UpdateAIScript( void );

	//
	// ai/ai_events.cpp
	//
	void					Event_Activate( idEntity *activator );

/*****
* DarkMod: Event_Touch was modified to issue a tactile alert.
*
* Note: Event_Touch checks ReactionTo, which checks our DarkMod Relations.
* So it will only go off if the AI is bumped by an enemy that moves into it.
* This is NOT called when an AI moves into an enemy.
*
* AI bumping by inanimate objects is handled separately by CheckTactile.
****/
	void					Event_Touch( idEntity *other, trace_t *trace );

	void					Event_FindEnemy( int useFOV );
	void					Event_FindEnemyAI( int useFOV );
	void					Event_FindEnemyInCombatNodes( void );
	void					Event_ClosestReachableEnemyOfEntity( idEntity *team_mate );
	void					Event_HeardSound( int ignore_team );
	void					Event_SetEnemy( idEntity *ent );
	void					Event_ClearEnemy( void );
	void					Event_MuzzleFlash( const char *jointname );
	void					Event_CreateMissile( const char *jointname );
	void					Event_AttackMissile( const char *jointname );
	void					Event_FireMissileAtTarget( const char *jointname, const char *targetname );
	void					Event_LaunchMissile( const idVec3 &muzzle, const idAngles &ang );
	void					Event_AttackMelee( const char *meleeDefName );
	void					Event_DirectDamage( idEntity *damageTarget, const char *damageDefName );
	void					Event_RadiusDamageFromJoint( const char *jointname, const char *damageDefName );
	void					Event_BeginAttack( const char *name );
	void					Event_EndAttack( void );
	void					Event_MeleeAttackToJoint( const char *jointname, const char *meleeDefName );
	void					Event_RandomPath( void );
	void					Event_CanBecomeSolid( void );
	void					Event_BecomeSolid( void );
	void					Event_BecomeNonSolid( void );
	void					Event_BecomeRagdoll( void );
	void					Event_StopRagdoll( void );
	void					Event_SetHealth( float newHealth );
	void					Event_GetHealth( void );
	void					Event_AllowDamage( void );
	void					Event_IgnoreDamage( void );
	void					Event_GetCurrentYaw( void );
	void					Event_TurnTo( float angle );
	void					Event_TurnToPos( const idVec3 &pos );
	void					Event_TurnToEntity( idEntity *ent );
	void					Event_MoveStatus( void );
	void					Event_StopMove( void );
	void					Event_MoveToCover( void );
	void					Event_MoveToEnemy( void );
	void					Event_MoveToEnemyHeight( void );
	void					Event_MoveOutOfRange( idEntity *entity, float range );
	void					Event_MoveToAttackPosition( idEntity *entity, const char *attack_anim );
	void					Event_MoveToEntity( idEntity *ent );
	void					Event_MoveToPosition( const idVec3 &pos );
	void					Event_SlideTo( const idVec3 &pos, float time );
	void					Event_Wander( void );
	void					Event_FacingIdeal( void );
	void					Event_FaceEnemy( void );
	void					Event_FaceEntity( idEntity *ent );
	void					Event_WaitAction( const char *waitForState );
	void					Event_GetCombatNode( void );
	void					Event_EnemyInCombatCone( idEntity *ent, int use_current_enemy_location );
	void					Event_WaitMove( void );
	void					Event_GetJumpVelocity( const idVec3 &pos, float speed, float max_height );
	void					Event_EntityInAttackCone( idEntity *ent );
	void					Event_CanSeeEntity( idEntity *ent );
	void					Event_SetTalkTarget( idEntity *target );
	void					Event_GetTalkTarget( void );
	void					Event_SetTalkState( int state );
	void					Event_EnemyRange( void );
	void					Event_EnemyRange2D( void );
	void					Event_GetEnemy( void );
	void					Event_GetEnemyPos( void );
	void					Event_GetEnemyEyePos( void );
	void					Event_PredictEnemyPos( float time );
	void					Event_CanHitEnemy( void );
	void					Event_CanHitEnemyFromAnim( const char *animname );
	void					Event_CanHitEnemyFromJoint( const char *jointname );
	void					Event_EnemyPositionValid( void );
	void					Event_ChargeAttack( const char *damageDef );
	void					Event_TestChargeAttack( void );
	void					Event_TestAnimMoveTowardEnemy( const char *animname );
	void					Event_TestAnimMove( const char *animname );
	void					Event_TestMoveToPosition( const idVec3 &position );
	void					Event_TestMeleeAttack( void );
	void					Event_TestAnimAttack( const char *animname );
	void					Event_Shrivel( float shirvel_time );
	void					Event_Burn( void );
	void					Event_PreBurn( void );
	void					Event_ClearBurn( void );
	void					Event_SetSmokeVisibility( int num, int on );
	void					Event_NumSmokeEmitters( void );
	void					Event_StopThinking( void );
	void					Event_GetTurnDelta( void );
	void					Event_GetMoveType( void );
	void					Event_SetMoveType( int moveType );
	void					Event_SaveMove( void );
	void					Event_RestoreMove( void );
	void					Event_AllowMovement( float flag );
	void					Event_JumpFrame( void );
	void					Event_EnableClip( void );
	void					Event_DisableClip( void );
	void					Event_EnableGravity( void );
	void					Event_DisableGravity( void );
	void					Event_EnableAFPush( void );
	void					Event_DisableAFPush( void );
	void					Event_SetFlySpeed( float speed );
	void					Event_SetFlyOffset( int offset );
	void					Event_ClearFlyOffset( void );
	void					Event_GetClosestHiddenTarget( const char *type );
	void					Event_GetRandomTarget( const char *type );
	void					Event_TravelDistanceToPoint( const idVec3 &pos );
	void					Event_TravelDistanceToEntity( idEntity *ent );
	void					Event_TravelDistanceBetweenPoints( const idVec3 &source, const idVec3 &dest );
	void					Event_TravelDistanceBetweenEntities( idEntity *source, idEntity *dest );
	void					Event_LookAtEntity( idEntity *ent, float duration );
	void					Event_LookAtEnemy( float duration );
	void					Event_SetJointMod( int allowJointMod );
	void					Event_ThrowMoveable( void );
	void					Event_ThrowAF( void );
	void					Event_SetAngles( idAngles const &ang );
	void					Event_GetAngles( void );
	void					Event_RealKill( void );
	void					Event_Kill( void );
	void					Event_WakeOnFlashlight( int enable );
	void					Event_LocateEnemy( void );
	void					Event_KickObstacles( idEntity *kickEnt, float force );
	void					Event_GetObstacle( void );
	void					Event_PushPointIntoAAS( const idVec3 &pos );
	void					Event_GetTurnRate( void );
	void					Event_SetTurnRate( float rate );
	void					Event_AnimTurn( float angles );
	void					Event_AllowHiddenMovement( int enable );
	void					Event_TriggerParticles( const char *jointName );
	void					Event_FindActorsInBounds( const idVec3 &mins, const idVec3 &maxs );
	void 					Event_CanReachPosition( const idVec3 &pos );
	void 					Event_CanReachEntity( idEntity *ent );
	void					Event_CanReachEnemy( void );
	void					Event_GetReachableEntityPosition( idEntity *ent );

	/**
	* Frontend scripting functions for Dark Mod Relations Manager
	* See CRelations class definition for descriptions
	**/
	void					Event_GetRelationEnt( idEntity *ent );
	void					Event_IsEnemy( idEntity *ent );
	void					Event_IsFriend( idEntity *ent );
	void					Event_IsNeutral( idEntity *ent );
	
	/**
	* Script frontend for, idAI::GetAcuity and idAI::SetAcuity
	* and idAI::AlertAI
	**/
	void Event_Alert( const char *type, float amount );
	void Event_GetAcuity( const char *type );
	void Event_SetAcuity( const char *type, float val );

	/**
	* Scan for the player in FOV, and cause a visual alert if found
	* Currently only checks the player.
	* Will return the entity that caused the visual alert for 
	* scripting purposes.
	**/
	void Event_VisScan( void );
	
	/**
	* Return the last suspicious sound direction
	**/
	void Event_GetSndDir( void );

	/**
	* Return the last visual alert position
	**/
	void Event_GetVisDir( void );

	/**
	* Return the entity that the AI is in tactile contact with
	**/
	void Event_GetTactEnt( void );

	/**
	* This is needed for accurate AI-AI combat
	* It just calls the vanilla D3 event:
	* Event_ClosestReachableEnemyToEntity( idEntity *ent )
	* with the "this" pointer.
	*
	* For some reason this was left out of D3.
	**/
	void idAI::Event_ClosestReachableEnemy( void );

};

class idCombatNode : public idEntity {
public:
	CLASS_PROTOTYPE( idCombatNode );

						idCombatNode();

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );
	bool				IsDisabled( void ) const;
	bool				EntityInView( idActor *actor, const idVec3 &pos );
	static void			DrawDebugInfo( void );

private:
	float				min_dist;
	float				max_dist;
	float				cone_dist;
	float				min_height;
	float				max_height;
	idVec3				cone_left;
	idVec3				cone_right;
	idVec3				offset;
	bool				disabled;

	void				Event_Activate( idEntity *activator );
	void				Event_MarkUsed( void );
};

#endif /* !__AI_H__ */
