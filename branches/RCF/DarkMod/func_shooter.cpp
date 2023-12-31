/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 22:25:02 +0100 (Fr, 23 Mar 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/
#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: func_shooter.cpp 870 2007-03-27 14:21:59Z greebo $", init_version);

#include "func_shooter.h"
#include "StimResponse/StimResponseCollection.h"

// Script event interface
const idEventDef EV_ShooterSetState( "shooterSetState", "d" );
const idEventDef EV_ShooterFireProjectile( "shooterFireProjectile", NULL );
const idEventDef EV_ShooterGetState( "shooterGetState", NULL, 'd' );
const idEventDef EV_ShooterSetAmmo( "shooterSetAmmo", "d" );
const idEventDef EV_ShooterGetAmmo( "shooterGetAmmo", NULL, 'd' );

// Event definitions
CLASS_DECLARATION( idStaticEntity, tdmFuncShooter )
EVENT( EV_Activate,					tdmFuncShooter::Event_Activate )
EVENT( EV_ShooterSetState,			tdmFuncShooter::Event_ShooterSetState )
EVENT( EV_ShooterGetState,			tdmFuncShooter::Event_ShooterGetState )
EVENT( EV_ShooterFireProjectile,	tdmFuncShooter::Event_ShooterFireProjectile )
EVENT( EV_ShooterSetAmmo,			tdmFuncShooter::Event_ShooterSetAmmo )
EVENT( EV_ShooterGetAmmo,			tdmFuncShooter::Event_ShooterGetAmmo )
END_CLASS

/*
===============
tdmFuncShooter::tdmFuncShooter
===============
*/
tdmFuncShooter::tdmFuncShooter( void ) :
	_active(false),
	_fireInterval(-1),
	_fireIntervalFuzzyness(0),
	_startDelay(0),
	_lastFireTime(0),
	_nextFireTime(0),
	_requiredStim(ST_DEFAULT),
	_lastStimVisit(0),
	_requiredStimTimeOut(0),
	_triggerRequired(false),
	_lastTriggerVisit(0),
	_triggerTimeOut(0),
	_ammo(-1),
	_useAmmo(false)
{}

/*
===============
tdmFuncShooter::Spawn
===============
*/
void tdmFuncShooter::Spawn( void ) {
	// Setup any stims/responses
	m_StimResponseColl->ParseSpawnArgsToStimResponse(&spawnArgs, this);

	_active = !spawnArgs.GetBool("start_off");
	_lastFireTime = 0;
	_fireInterval = spawnArgs.GetInt("fire_interval", "-1");
	_fireIntervalFuzzyness = spawnArgs.GetInt("fire_interval_fuzzyness", "0");
	_startDelay = spawnArgs.GetInt("start_delay", "0");
	
	idStr reqStimStr = spawnArgs.GetString("required_stim");

	if (!reqStimStr.IsEmpty()) {
		_requiredStim = CStimResponse::getStimType(reqStimStr);
		_requiredStimTimeOut = spawnArgs.GetInt("required_stim_timeout", "5000");
	}

	_triggerRequired = spawnArgs.GetBool("required_trigger");
	_triggerTimeOut = spawnArgs.GetInt("required_trigger_timeout", "4000");

	_ammo = spawnArgs.GetInt("ammo", "-1");
	_useAmmo = (_ammo != -1);

	if (_active && _fireInterval > 0) {
		BecomeActive( TH_THINK );
		setupNextFireTime();
		_nextFireTime += _startDelay;
	}

	// Always react to stims if a required stim is setup.
	if (_requiredStim != ST_DEFAULT) {
		//DM_LOG(LC_STIM_RESPONSE, LT_INFO)LOGSTRING("tdmFuncShooter is requiring stim %d\r", _requiredStim);
		GetPhysics()->SetContents( GetPhysics()->GetContents() | CONTENTS_RESPONSE );
	}
}

/*
===============
tdmFuncShooter::Save
===============
*/
void tdmFuncShooter::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( _active );
	savefile->WriteInt( _lastFireTime );
	savefile->WriteInt( _nextFireTime );
	savefile->WriteInt( _fireInterval );
	savefile->WriteInt( _fireIntervalFuzzyness );
	savefile->WriteInt( _startDelay );
	savefile->WriteInt( _requiredStim );
	savefile->WriteInt( _requiredStimTimeOut );
	savefile->WriteInt( _lastStimVisit );
	savefile->WriteInt( _lastTriggerVisit );
	savefile->WriteBool( _triggerRequired );
	savefile->WriteInt( _triggerTimeOut );
	savefile->WriteInt( _ammo );
	savefile->WriteBool( _useAmmo );
}

/*
===============
tdmFuncShooter::Restore
===============
*/
void tdmFuncShooter::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( _active );
	savefile->ReadInt( _lastFireTime );
	savefile->ReadInt( _nextFireTime );
	savefile->ReadInt( _fireInterval );
	savefile->ReadInt( _fireIntervalFuzzyness );
	savefile->ReadInt( _startDelay );

	int stimType;
	savefile->ReadInt( stimType );
	_requiredStim = static_cast<StimType>(stimType);

	savefile->ReadInt( _requiredStimTimeOut );
	savefile->ReadInt( _lastStimVisit );
	savefile->ReadInt( _lastTriggerVisit );
	savefile->ReadBool( _triggerRequired );
	savefile->ReadInt( _triggerTimeOut );
	savefile->ReadInt( _ammo );
	savefile->ReadBool( _useAmmo );
}

/*
================
tdmFuncShooter::Event_Activate
================
*/
void tdmFuncShooter::Event_Activate( idEntity *activator ) {
	if (_triggerRequired) {
		// This shooter requires constant triggering, save the time
		_lastTriggerVisit = gameLocal.time;
	}
	else if ( thinkFlags & TH_THINK ) {
		BecomeInactive( TH_THINK );
		_active = false;
	} else {
		BecomeActive( TH_THINK );
		_active = true;
		_ammo = spawnArgs.GetInt("ammo", "-1");
		_lastFireTime = gameLocal.time;
		setupNextFireTime();
		_nextFireTime += _startDelay;
	}
}

void tdmFuncShooter::Event_ShooterGetState() {
	idThread::ReturnInt(_active ? 1 : 0);
}

void tdmFuncShooter::Event_ShooterSetState( bool state ) {
	if (state == _active) {
		// Nothing to change
		return;
	}

	_active = state;

	if (_active) {
		// Reset the ammo on script activation (useAmmo can still override this)
		_ammo = spawnArgs.GetInt("ammo", "-1");
		setupNextFireTime();
		_nextFireTime += _startDelay;
	}
}

void tdmFuncShooter::Event_ShooterFireProjectile() {
	Fire();
}

void tdmFuncShooter::Event_ShooterSetAmmo( int newAmmo ) {
	_ammo = newAmmo;
}

void tdmFuncShooter::Event_ShooterGetAmmo() {
	idThread::ReturnInt(_ammo);
}

void tdmFuncShooter::setupNextFireTime() {
	// Calculate the next fire time
	int randomness = gameLocal.random.RandomFloat() * _fireIntervalFuzzyness - _fireIntervalFuzzyness/2;
	_nextFireTime = gameLocal.time + _fireInterval + randomness;
}

/*
================
tdmFuncShooter::WriteToSnapshot
================
*/
void tdmFuncShooter::WriteToSnapshot( idBitMsgDelta &msg ) const {
	msg.WriteBits( _active ? 1 : 0, 1 );
}

/*
================
tdmFuncShooter::ReadFromSnapshot
================
*/
void tdmFuncShooter::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	_active = msg.ReadBits( 1 ) != 0;
}

void tdmFuncShooter::stimulate(StimType stimId) {
	if (stimId == _requiredStim && _requiredStim != ST_DEFAULT) {
		//DM_LOG(LC_STIM_RESPONSE, LT_INFO)LOGSTRING("Stim is visiting at %d\r", gameLocal.time);
		// Save the time the stim is visiting
		_lastStimVisit = gameLocal.time;
	}
}

void tdmFuncShooter::Fire() {
	_lastFireTime = gameLocal.time;

	// Spawn a projectile
	idStr projectileDef = spawnArgs.GetString("def_projectile");
	if (!projectileDef.IsEmpty()) {
		const idDict* projectileDict = gameLocal.FindEntityDefDict(projectileDef);

		idEntity* ent = NULL;
		gameLocal.SpawnEntityDef(*projectileDict, &ent);

		idProjectile* projectile = dynamic_cast<idProjectile*>(ent);
		if (projectile != NULL) {
			// Get the default angle from the entity
			float angle = spawnArgs.GetFloat("angle");
			float pitch = spawnArgs.GetFloat("pitch", "0");

			// Check if the angle should be randomly chosen
			if (spawnArgs.GetBool("random_angle")) {
				angle = gameLocal.random.RandomFloat() * 360;
			}

			// Check for random pitch angle
			if (spawnArgs.GetBool("random_pitch")) {
				pitch = gameLocal.random.RandomFloat() * 180 - 90;
			}

			// Calculate the direction from "angle" and "pitch"
			idVec3 direction( cos(DEG2RAD(angle)), sin(DEG2RAD(angle)), sin(DEG2RAD(pitch)) );
			direction.NormalizeFast();

			// Check for a specified velocity on the shooter
			float velocity = spawnArgs.GetFloat("velocity", "0");

			if (velocity <= 0) {
				// Try to get a velocity from the projectile itself
				velocity = projectileDict->GetVector("velocity", "0 0 0").Length();
			}

			// Set the brand of the projectile, it should know its roots
			projectile->spawnArgs.Set("shooter", name.c_str());

			// Fire!
			projectile->Launch(GetPhysics()->GetOrigin(), direction, direction*velocity);

			if (spawnArgs.GetBool("override_projectile_angles", "0"))
			{
				// Read the angles from the projectile
				idAngles tempAngles(projectile->spawnArgs.GetAngles("angles"));

				// Add the angles of the shoot direction
				tempAngles.yaw += angle;
				tempAngles.pitch += pitch;
				tempAngles.roll += 0;
				
				// Convert the angles to an axis matrix and store it into the projectile
				projectile->GetPhysics()->SetAxis(tempAngles.ToMat3());
			}

			// Check the ammonition
			if (_useAmmo && --_ammo <= 0) {
				// Clamp the ammo value to zero
				_ammo = 0;
				// Inactivate the shooter as the max ammo was specified and has run out
				Event_ShooterSetState(false);
			}
		}
	}

	setupNextFireTime();
}

void tdmFuncShooter::Think() {
	if (_active && _fireInterval > 0 && gameLocal.time > _nextFireTime) {
		// greebo: Check before firing whether we have a required stim
		if (_requiredStim != ST_DEFAULT && _lastStimVisit > 0 && 
			_lastStimVisit + _requiredStimTimeOut >= gameLocal.time)
		{
			// We have a required stim, but it was not too far in the past => fire
			Fire();
		}
		else if (_triggerRequired && _lastTriggerVisit > 0 &&
				 _lastTriggerVisit + _triggerTimeOut >= gameLocal.time)
		{
			// Required constant triggering, last trigger was not too long ago => fire
			Fire();
		}
		else if (_requiredStim == ST_DEFAULT && !_triggerRequired) {
			// No required stim and no required trigger, fire away
			Fire();
		}
	}
}
