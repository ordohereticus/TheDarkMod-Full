/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 125 $
 * $Date: 2005-07-01 17:21:23 -0400 (Fri, 01 Jul 2005) $
 * $Author: sophisticatedzombie $
 *
 * $Log$
 * Revision 1.2  2005/07/01 21:21:23  sophisticatedzombie
 * This is my check in of the mantling code on July 1, 2005.  I've tested it agains the .3 sdk, but not the .2 one.  Any takers?
 *
 * Revision 1.1.1.1  2004/10/30 15:52:34  sparhawk
 * Initial release
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __PHYSICS_PLAYER_H__
#define __PHYSICS_PLAYER_H__

/*
===================================================================================

	Player physics

	Simulates the motion of a player through the environment. Input from the
	player is used to allow a certain degree of control over the motion.

===================================================================================
*/

// movementType
typedef enum {
	PM_NORMAL,				// normal physics
	PM_DEAD,				// no acceleration or turning, but free falling
	PM_SPECTATOR,			// flying without gravity but with collision detection
	PM_FREEZE,				// stuck in place without control
	PM_NOCLIP				// flying without collision detection nor gravity
} pmtype_t;

typedef enum {
	WATERLEVEL_NONE,
	WATERLEVEL_FEET,
	WATERLEVEL_WAIST,
	WATERLEVEL_HEAD
} waterLevel_t;

#define	MAXTOUCH					32

typedef struct playerPState_s {
	idVec3					origin;
	idVec3					velocity;
	idVec3					localOrigin;
	idVec3					pushVelocity;
	float					stepUp;
	int						movementType;
	int						movementFlags;
	int						movementTime;
} playerPState_t;


// This enumreation defines the phases of the mantling movement
typedef enum
{
	notMantling_DarkModMantlePhase	= 0x00,
	hang_DarkModMantlePhase			= 0x01,
	pull_DarkModMantlePhase			= 0x02,
	shiftHands_DarkModMantlePhase	= 0x03,
	push_DarkModMantlePhase			= 0x04

} EDarkMod_MantlePhase;

class idPhysics_Player : public idPhysics_Actor {

public:
	CLASS_PROTOTYPE( idPhysics_Player );

							idPhysics_Player( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

							// initialisation
	void					SetSpeed( const float newWalkSpeed, const float newCrouchSpeed );
	void					SetMaxStepHeight( const float newMaxStepHeight );
	float					GetMaxStepHeight( void ) const;
	void					SetMaxJumpHeight( const float newMaxJumpHeight );
	void					SetMovementType( const pmtype_t type );
	void					SetPlayerInput( const usercmd_t &cmd, const idAngles &newViewAngles );
	void					SetKnockBack( const int knockBackTime );
	void					SetDebugLevel( bool set );
							// feed back from last physics frame
	waterLevel_t			GetWaterLevel( void ) const;
	int						GetWaterType( void ) const;
	bool					HasJumped( void ) const;
	bool					HasSteppedUp( void ) const;
	float					GetStepUp( void ) const;
	bool					IsCrouching( void ) const;
	bool					OnLadder( void ) const;
	const idVec3 &			PlayerGetOrigin( void ) const;	// != GetOrigin

public:	// common physics interface
	bool					Evaluate( int timeStepMSec, int endTimeMSec );
	void					UpdateTime( int endTimeMSec );
	int						GetTime( void ) const;

	void					GetImpactInfo( const int id, const idVec3 &point, impactInfo_t *info ) const;
	void					ApplyImpulse( const int id, const idVec3 &point, const idVec3 &impulse );
	bool					IsAtRest( void ) const;
	int						GetRestStartTime( void ) const;

	void					SaveState( void );
	void					RestoreState( void );

	void					SetOrigin( const idVec3 &newOrigin, int id = -1 );
	void					SetAxis( const idMat3 &newAxis, int id = -1 );

	void					Translate( const idVec3 &translation, int id = -1 );
	void					Rotate( const idRotation &rotation, int id = -1 );

	void					SetLinearVelocity( const idVec3 &newLinearVelocity, int id = 0 );

	const idVec3 &			GetLinearVelocity( int id = 0 ) const;

	void					SetPushed( int deltaTime );
	const idVec3 &			GetPushedLinearVelocity( const int id = 0 ) const;
	void					ClearPushedVelocity( void );

	void					SetMaster( idEntity *master, const bool orientated = true );

	void					WriteToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadFromSnapshot( const idBitMsgDelta &msg );

private:
	// player physics state
	playerPState_t			current;
	playerPState_t			saved;

	// properties
	float					walkSpeed;
	float					crouchSpeed;
	float					maxStepHeight;
	float					maxJumpHeight;
	int						debugLevel;				// if set, diagnostic output will be printed

	// player input
	usercmd_t				command;
	idAngles				viewAngles;

	// run-time variables
	int						framemsec;
	float					frametime;
	float					playerSpeed;
	idVec3					viewForward;
	idVec3					viewRight;

	// walk movement
	bool					walking;
	bool					groundPlane;
	trace_t					groundTrace;
	const idMaterial *		groundMaterial;

	// ladder movement
	bool					ladder;
	idVec3					ladderNormal;

	// results of last evaluate
	waterLevel_t			waterLevel;
	int						waterType;

private:
	float					CmdScale( const usercmd_t &cmd ) const;
	void					Accelerate( const idVec3 &wishdir, const float wishspeed, const float accel );
	bool					SlideMove( bool gravity, bool stepUp, bool stepDown, bool push );
	void					Friction( void );
	void					WaterJumpMove( void );
	void					WaterMove( void );
	void					FlyMove( void );
	void					AirMove( void );
	void					WalkMove( void );
	void					DeadMove( void );
	void					NoclipMove( void );
	void					SpectatorMove( void );
	void					LadderMove( void );
	void					CorrectAllSolid( trace_t &trace, int contents );
	void					CheckGround( void );
	void					CheckDuck( void );
	void					CheckLadder( void );
	bool					CheckJump( void );
	bool					CheckWaterJump( void );
	void					SetWaterLevel( void );
	void					DropTimers( void );
	void					MovePlayer( int msec );

	//#####################################################
	// Mantling handler
	// by SophisticatedZombie (Damon Hill)
	//
	//#####################################################

public:


	// This method returns
	// true if the player is mantling, false otherwise
	__inline bool IsMantling (void) const;
	
	// This returns the current mantling phase
	__inline EDarkMod_MantlePhase GetMantlePhase(void) const;

	// Cancels any current mantle
	inline void CancelMantle();

	// Checks to see if there is a mantleable target within reach
	// of the player's view. If so, starts the mantle... 
	// If the player is already mantling, this does nothing.
	void PerformMantle();


protected:


	// The current mantling phase
	EDarkMod_MantlePhase m_mantlePhase;

	// Point being mantled to...
	idVec3 m_mantlePullStartPos;
	idVec3 m_mantlePullEndPos;
	idVec3 m_mantlePushEndPos;

	// Pointer to the entity being mantled.
	// This is undefined if m_mantlePhase == notMantling_DarkModMantlePhase
	idEntity* p_mantledEntity;

	// How long will the current phase of the mantle operation take?
	// Uses the same time unit as other movement times.
	float m_mantleTime;

	// This method determines the mantle time required for each phase of the mantle.
	// I made this a function so you could implement things such as carry-weight,
	// tiredness, length of lift....
	float getMantleTimeForPhase (EDarkMod_MantlePhase mantlePhase);

	// Internal method to start the mantle operation
	void StartMantle
	(
		EDarkMod_MantlePhase initialMantlePhase,
		idVec3 eyePos,
		idVec3 startPos,
		idVec3 endPos
	);

	// Timer change methods
	void DropMantleTimers();
    
	// Movement methods
	void MantleMove();

	//#################################################
	// End mantling handler
	//#################################################


};

#endif /* !__PHYSICS_PLAYER_H__ */
