// vim:ts=4:sw=4:cindent
/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4011 $
 * $Date: 2010-07-01 23:06:18 -0400 (Thu, 01 Jul 2010) $
 * $Author: tels $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_MISC_H__
#define __GAME_MISC_H__

/* Tels: Define the number of supported LOD levels (0..LOD_LEVELS + hide)
 *		 About higher than 10 starts take performance instead of gaining it.
 *		 Do not go higher than 31, as each level needs 1 bit in a 32bit int.
 */
#define LOD_LEVELS 7

/*
===============================================================================

idSpawnableEntity

A simple, spawnable entity with a model and no functionable ability of it's own.
For example, it can be used as a placeholder during development, for marking
locations on maps for script, or for simple placed models without any behavior
that can be bound to other entities.  Should not be subclassed.
===============================================================================
*/

class idSpawnableEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idSpawnableEntity );

	void				Spawn( void );

private:
};

/*
===============================================================================

  Potential spawning position for players.
  The first time a player enters the game, they will be at an 'initial' spot.
  Targets will be fired when someone spawns in on them.

  When triggered, will cause player to be teleported to spawn spot.

===============================================================================
*/

class idPlayerStart : public idEntity {
public:
	CLASS_PROTOTYPE( idPlayerStart );

	enum {
		EVENT_TELEPORTPLAYER = idEntity::EVENT_MAXEVENTS,
		EVENT_MAXEVENTS
	};

						idPlayerStart( void );

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	virtual bool		ClientReceiveEvent( int event, int time, const idBitMsg &msg );

private:
	int					teleportStage;

	void				Event_TeleportPlayer( idEntity *activator );
	void				Event_TeleportStage( idEntity *player );
	void				TeleportPlayer( idPlayer *player );
};


/*
===============================================================================

  Non-displayed entity used to activate triggers when it touches them.
  Bind to a mover to have the mover activate a trigger as it moves.
  When target by triggers, activating the trigger will toggle the
  activator on and off. Check "start_off" to have it spawn disabled.
	
===============================================================================
*/

class idActivator : public idEntity {
public:
	CLASS_PROTOTYPE( idActivator );

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	virtual void		Think( void );

private:
	bool				stay_on;

	void				Event_Activate( idEntity *activator );
};


/*
===============================================================================

  Path entities for monsters to follow.

===============================================================================
*/
class idPathCorner : public idEntity {
public:
	CLASS_PROTOTYPE( idPathCorner );

	void				Spawn( void );

	static void			DrawDebugInfo( void );

	static idPathCorner *RandomPath( const idEntity *source, const idEntity *ignore, idAI* owner );

private:
	void				Event_RandomPath( void );
};

/*
===============================================================================

  Path entities for AI to flee to.

===============================================================================
*/
class tdmPathFlee : public idEntity {
public:
	CLASS_PROTOTYPE( tdmPathFlee );

	virtual ~tdmPathFlee();

	void				Spawn( void );

	static void		DrawDebugInfo( void );
};

/*
===============================================================================

  Object that fires targets and changes shader parms when damaged.

===============================================================================
*/

class idDamagable : public idEntity {
public:
	CLASS_PROTOTYPE( idDamagable );

						idDamagable( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );
	void				Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );

private:
	int					count;
	int					nextTriggerTime;

	void				BecomeBroken( idEntity *activator );
	void				Event_BecomeBroken( idEntity *activator );
	void				Event_RestoreDamagable( void );
};


/*
===============================================================================

  Hidden object that explodes when activated

===============================================================================
*/

class idExplodable : public idEntity {
public:
	CLASS_PROTOTYPE( idExplodable );

	void				Spawn( void );

private:
	void				Event_Explode( idEntity *activator );
};


/*
===============================================================================

  idSpring

===============================================================================
*/

class idSpring : public idEntity {
public:
	CLASS_PROTOTYPE( idSpring );

	void				Spawn( void );

	virtual void		Think( void );

private:
	idEntity *			ent1;
	idEntity *			ent2;
	int					id1;
	int					id2;
	idVec3				p1;
	idVec3				p2;
	idForce_Spring		spring;

	void				Event_LinkSpring( void );
};


/*
===============================================================================

  idForceField

===============================================================================
*/

class idForceField : public idEntity {
public:
	CLASS_PROTOTYPE( idForceField );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );

	virtual void		Think( void );

private:
	idForce_Field		forceField;

	void				Toggle( void );

	void				Event_Activate( idEntity *activator );
	void				Event_Toggle( void );
	void				Event_FindTargets( void );
};


/*
===============================================================================

  idAnimated

===============================================================================
*/

class idAnimated : public idAFEntity_Gibbable {
public:
	CLASS_PROTOTYPE( idAnimated );

	idAnimated();
	virtual ~idAnimated();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Spawn( void );
	virtual bool			LoadAF( void );
	bool					StartRagdoll( void );
	virtual bool			GetPhysicsToSoundTransform( idVec3 &origin, idMat3 &axis );

private:
	int						num_anims;
	int						current_anim_index;
	int						anim;
	int						blendFrames;
	jointHandle_t			soundJoint;
	idEntityPtr<idEntity>	activator;
	bool					activated;

	void					PlayNextAnim( void );

	void					Event_Activate( idEntity *activator );	
	void					Event_Start( void );
	void					Event_StartRagdoll( void );
	void					Event_AnimDone( int animIndex );
	void					Event_Footstep( void );
	void					Event_LaunchMissiles( const char *projectilename, const char *sound, const char *launchjoint, const char *targetjoint, int numshots, int framedelay );
	void					Event_LaunchMissilesUpdate( int launchjoint, int targetjoint, int numshots, int framedelay );
};


/*
===============================================================================

  idStaticEntity

===============================================================================
*/

class idStaticEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idStaticEntity );

						idStaticEntity( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );
	void				ShowEditingDialog( void );
	virtual void		Hide( void );
	virtual void		Show( void );
	void				Fade( const idVec4 &to, float fadeTime );
	virtual void		Think( void );

	/**
	 * Tels: Stop LOD changes. If doTeam is true, also disables it on teammembers.
	 */
	void				StopLOD( const bool doTeam);


	virtual void		WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void		ReadFromSnapshot( const idBitMsgDelta &msg );

private:
	void				Event_Activate( idEntity *activator );

	int					spawnTime;
	bool				active;
	idVec4				fadeFrom;
	idVec4				fadeTo;
	int					fadeStart;
	int					fadeEnd;
	bool				runGui;

	/**
	* Set to true if whether this static entity should hide
	* or change models/skins when outside a certain distance from the player
	**/
	bool				m_bDistDependent;

	/**
	* If true, the LOD distance check will only consider distance
	* orthogonal to gravity.  This can be useful for things like
	* turning on rainclouds high above the player.
	**/
	bool				m_bDistCheckXYOnly;
	
	/**
	* Timestamp and interval between distance checks, in milliseconds
	**/
	int					m_DistCheckTimeStamp;
	int					m_DistCheckInterval;
	
	/**
	* Tels: Distance squared beyond which the entity switches to LOD model/skin #1,#2,#3
	* if it is distance dependent
	* The last number is the distance squared beyond which the entity hides.
	**/
	float				m_DistLODSq[ LOD_LEVELS ];

	/**
	* Tels: LOD (0 - normal, 1,2,3,4,5 LOD, 6 hidden)
	**/
	int					m_LODLevel;

	/**
	* Tels: Models and skins to be used for the different LOD distances
	* for level 0 we use "model" and "skin"
	**/
	idStr				m_ModelLOD[ LOD_LEVELS ];
	idStr				m_SkinLOD[ LOD_LEVELS ];

	/** one bit for each LOD level, telling noshadows (1) or not (0) */
	int					m_noshadowsLOD;

	/* Tels: different LOD models might need different offsets to match
	*		 the position of the LOD 0 level. */
	idVec3				m_OffsetLOD[ LOD_LEVELS ];

	/**
	* Tels: fade out and fade in range in D3 units
	**/
	float				m_fLODFadeOutRange;
	float				m_fLODFadeInRange;

	/* Tels: Store the current model and skin to avoid flicker by not
	*		 switching from one model/skin to the same model/skin when
	*		 changing the LOD.
	*/
	idStr				m_ModelLODCur;
	idStr				m_SkinLODCur;
};


/*
===============================================================================

idFuncEmitter

===============================================================================
*/

class idFuncEmitter : public idStaticEntity {
public:
	CLASS_PROTOTYPE( idFuncEmitter );

						idFuncEmitter( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );
	void				Event_Activate( idEntity *activator );

	virtual void		WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void		ReadFromSnapshot( const idBitMsgDelta &msg );

private:
	bool				hidden;

};


/*
===============================================================================

idFuncSmoke

===============================================================================
*/

class idFuncSmoke : public idEntity {
public:
	CLASS_PROTOTYPE( idFuncSmoke );

							idFuncSmoke();

	void					Spawn( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	virtual void			Think( void );
	void					Event_Activate( idEntity *activator );

private:
	int						smokeTime;
	const idDeclParticle *	smoke;
	bool					restart;
};


/*
===============================================================================

idFuncSplat

===============================================================================
*/

class idFuncSplat : public idFuncEmitter {
public:
	CLASS_PROTOTYPE( idFuncSplat );

	idFuncSplat( void );

	void				Spawn( void );

private:
	void				Event_Activate( idEntity *activator );
	void				Event_Splat();
};


/*
===============================================================================

idTextEntity

===============================================================================
*/

class idTextEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idTextEntity );

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	virtual void		Think( void );

private:
	idStr				text;
	bool				playerOriented;
};


/*
===============================================================================

idLocationEntity

===============================================================================
*/

class idLocationEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idLocationEntity );

	idLocationEntity( void );

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	const char *		GetLocation( void ) const;

public:
	/**
	* Soundprop: Loss multiplier for atmospheric attenuation
	**/
	float				m_SndLossMult;
	/**
	* Soundprop: Volume offset for sounds originating in location
	**/
	float				m_SndVolMod;
	/**
	* Objective system: Location's objective group name for objective checks
	**/
	idStr				m_ObjectiveGroup;

private:
};

class idLocationSeparatorEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idLocationSeparatorEntity );

	void				Spawn( void );

private:
};

class idVacuumSeparatorEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idVacuumSeparatorEntity );

						idVacuumSeparatorEntity( void );

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Event_Activate( idEntity *activator );	

private:
	qhandle_t			portal;
};

class idVacuumEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idVacuumEntity );

	void				Spawn( void );

private:
};


/*
===============================================================================

  idBeam

===============================================================================
*/

class idBeam : public idEntity {
public:
	CLASS_PROTOTYPE( idBeam );

						idBeam();

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	virtual void		Think( void );

	void				SetMaster( idBeam *masterbeam );
	void				SetBeamTarget( const idVec3 &origin );

	virtual void		Show( void );

	virtual void		WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void		ReadFromSnapshot( const idBitMsgDelta &msg );

private:
	void				Event_MatchTarget( void );
	void				Event_Activate( idEntity *activator );

	idEntityPtr<idBeam>	target;
	idEntityPtr<idBeam>	master;
};


/*
===============================================================================

  idLiquid

===============================================================================
*/

#ifndef MOD_WATERPHYSICS
class idRenderModelLiquid;

class idLiquid : public idEntity {
public:
	CLASS_PROTOTYPE( idLiquid );

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

private:
	void				Event_Touch( idEntity *other, trace_t *trace );


	idRenderModelLiquid *model;
};
#endif

/*
===============================================================================

  idShaking

===============================================================================
*/

class idShaking : public idEntity {
public:
	CLASS_PROTOTYPE( idShaking );

							idShaking();

	void					Spawn( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

private:
	idPhysics_Parametric	physicsObj;
	bool					active;

	void					BeginShaking( void );
	void					Event_Activate( idEntity *activator );
};


/*
===============================================================================

  idEarthQuake

===============================================================================
*/

class idEarthQuake : public idEntity {
public:
	CLASS_PROTOTYPE( idEarthQuake );
			
						idEarthQuake();

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	virtual void		Think( void );

private:
	int					nextTriggerTime;
	int					shakeStopTime;
	float				wait;
	float				random;
	bool				triggered;
	bool				playerOriented;
	bool				disabled;
	float				shakeTime;

	void				Event_Activate( idEntity *activator );
};


/*
===============================================================================

  idFuncPortal

===============================================================================
*/

class idFuncPortal : public idEntity {
public:
	CLASS_PROTOTYPE( idFuncPortal );
			
						idFuncPortal();

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );
	void				Think( void );

private:
	qhandle_t			portal;
	bool				state;

	/**
	* Set to true if the portal state depends on distance from player
	**/
	bool				m_bDistDependent;
	
	/**
	* Timestamp and interval between checks, in milliseconds
	**/
	int					m_TimeStamp;
	int					m_Interval;
	/**
	* Distance at which the portal shuts off, if it is distance dependent
	**/
	float				m_Distance;

	void				Event_Activate( idEntity *activator );
};

/*
===============================================================================

  idFuncAASPortal

===============================================================================
*/

class idFuncAASPortal : public idEntity {
public:
	CLASS_PROTOTYPE( idFuncAASPortal );
			
						idFuncAASPortal();

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

private:
	bool				state;

	void				Event_Activate( idEntity *activator );
};

/*
===============================================================================

  idFuncAASObstacle

===============================================================================
*/

class idFuncAASObstacle : public idEntity {
public:
	CLASS_PROTOTYPE( idFuncAASObstacle );
			
						idFuncAASObstacle();

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	// greebo: Public function to set the state directly
	// Note: Passing TRUE means that the AAS area is DISABLED
	void				SetAASState(bool newState);

private:
	bool				state;

	void				Event_Activate( idEntity *activator );
};


/*
===============================================================================

idFuncRadioChatter

===============================================================================
*/

class idFuncRadioChatter : public idEntity {
public:
	CLASS_PROTOTYPE( idFuncRadioChatter );

						idFuncRadioChatter();

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

private:
	float				time;
	void				Event_Activate( idEntity *activator );
	void				Event_ResetRadioHud( idEntity *activator );
};


/*
===============================================================================

  idPhantomObjects

===============================================================================
*/

class idPhantomObjects : public idEntity {
public:
	CLASS_PROTOTYPE( idPhantomObjects );
			
						idPhantomObjects();

	void				Spawn( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	virtual void		Think( void );

private:
	void				Event_Activate( idEntity *activator );
	void				Event_Throw( void );
	void				Event_ShakeObject( idEntity *object, int starttime );

	int					end_time;
	float				throw_time;
	float				shake_time;
	idVec3				shake_ang;
	float				speed;
	int					min_wait;
	int					max_wait;
	idEntityPtr<idActor>target;
	idList<int>			targetTime;
	idList<idVec3>		lastTargetPos;
};

/*
===============================================================================

idPortalSky

===============================================================================
*/

class idPortalSky : public idEntity {

public:

	CLASS_PROTOTYPE( idPortalSky );

	idPortalSky();

	virtual ~idPortalSky();

	void				Spawn( void );
	void				Event_PostSpawn();
	void				Event_Activate( idEntity *activator );
};


#endif /* !__GAME_MISC_H__ */

