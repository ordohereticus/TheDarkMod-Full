/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 866 $
 * $Date: 2007-03-23 17:25:02 -0400 (Fri, 23 Mar 2007) $
 * $Author: sparhawk $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: sound.cpp 866 2007-03-23 21:25:02Z sparhawk $", init_version);

#include "Game_local.h"

/*
===============================================================================

  SOUND

===============================================================================
*/

const idEventDef EV_Speaker_On( "On", NULL );
const idEventDef EV_Speaker_Off( "Off", NULL );
const idEventDef EV_Speaker_Timer( "<timer>", NULL );

CLASS_DECLARATION( idEntity, idSound )
	EVENT( EV_Activate,				idSound::Event_Trigger )
	EVENT( EV_Speaker_On,			idSound::Event_On )
	EVENT( EV_Speaker_Off,			idSound::Event_Off )
	EVENT( EV_Speaker_Timer,		idSound::Event_Timer )
END_CLASS


/*
================
idSound::idSound
================
*/
idSound::idSound( void ) {
	lastSoundVol = 0.0f;
	soundVol = 0.0f;
	shakeTranslate.Zero();
	shakeRotate.Zero();
	random = 0.0f;
	wait = 0.0f;
	timerOn = false;
	playingUntilTime = 0;
}

/*
================
idSound::Save
================
*/
void idSound::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( lastSoundVol );
	savefile->WriteFloat( soundVol );
	savefile->WriteFloat( random );
	savefile->WriteFloat( wait );
	savefile->WriteBool( timerOn );
	savefile->WriteVec3( shakeTranslate );
	savefile->WriteAngles( shakeRotate );
	savefile->WriteInt( playingUntilTime );
}

/*
================
idSound::Restore
================
*/
void idSound::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( lastSoundVol );
	savefile->ReadFloat( soundVol );
	savefile->ReadFloat( random );
	savefile->ReadFloat( wait );
	savefile->ReadBool( timerOn );
	savefile->ReadVec3( shakeTranslate );
	savefile->ReadAngles( shakeRotate );
	savefile->ReadInt( playingUntilTime );
}

/*
================
idSound::Spawn
================
*/
void idSound::Spawn( void ) {
	spawnArgs.GetVector( "move", "0 0 0", shakeTranslate );
	spawnArgs.GetAngles( "rotate", "0 0 0", shakeRotate );
	spawnArgs.GetFloat( "random", "0", random );
	spawnArgs.GetFloat( "wait", "0", wait );

	if ( ( wait > 0.0f ) && ( random >= wait ) ) {
		random = wait - 0.001;
		gameLocal.Warning( "speaker '%s' at (%s) has random >= wait", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	soundVol		= 0.0f;
	lastSoundVol	= 0.0f;

	if ( ( shakeRotate != ang_zero ) || ( shakeTranslate != vec3_zero ) ) {
		BecomeActive( TH_THINK );
	}

	if ( !refSound.waitfortrigger && ( wait > 0.0f ) ) {
		timerOn = true;
		PostEventSec( &EV_Speaker_Timer, wait + gameLocal.random.CRandomFloat() * random );
	} else {
		timerOn = false;
	}
}

/*
================
idSound::Event_Trigger

this will toggle the idle idSound on and off
================
*/
void idSound::Event_Trigger( idEntity *activator ) {
	if ( wait > 0.0f ) {
		if ( timerOn ) {
			timerOn = false;
			CancelEvents( &EV_Speaker_Timer );
		} else {
			timerOn = true;
			DoSound( true );
			PostEventSec( &EV_Speaker_Timer, wait + gameLocal.random.CRandomFloat() * random );
		}
	} else {
		if ( gameLocal.isMultiplayer ) {
			if ( refSound.referenceSound && ( gameLocal.time < playingUntilTime ) ) {
				DoSound( false );
			} else {
				DoSound( true );
			}
		} else {
			if ( refSound.referenceSound && refSound.referenceSound->CurrentlyPlaying() ) {
				DoSound( false );
			} else {
				DoSound( true );
			}
		}
	}
}

/*
================
idSound::Event_Timer
================
*/
void idSound::Event_Timer( void ) {
	DoSound( true );
	PostEventSec( &EV_Speaker_Timer, wait + gameLocal.random.CRandomFloat() * random );
}

/*
================
idSound::Think
================
*/
void idSound::Think( void ) {
	idAngles	ang;

	// run physics
	RunPhysics();

	// clear out our update visuals think flag since we never call Present
	BecomeInactive( TH_UPDATEVISUALS );
}

/*
===============
idSound::UpdateChangableSpawnArgs
===============
*/
void idSound::UpdateChangeableSpawnArgs( const idDict *source ) {

	idEntity::UpdateChangeableSpawnArgs( source );

	if ( source ) {
		FreeSoundEmitter( true );
		spawnArgs.Copy( *source );
		idSoundEmitter *saveRef = refSound.referenceSound;
		gameEdit->ParseSpawnArgsToRefSound( &spawnArgs, &refSound );
		refSound.referenceSound = saveRef;

		idVec3 origin;
		idMat3 axis;

		if ( GetPhysicsToSoundTransform( origin, axis ) ) {
			refSound.origin = GetPhysics()->GetOrigin() + origin * axis;
		} else {
			refSound.origin = GetPhysics()->GetOrigin();
		}

		spawnArgs.GetFloat( "random", "0", random );
		spawnArgs.GetFloat( "wait", "0", wait );

		if ( ( wait > 0.0f ) && ( random >= wait ) ) {
			random = wait - 0.001;
			gameLocal.Warning( "speaker '%s' at (%s) has random >= wait", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
		}

		if ( !refSound.waitfortrigger && ( wait > 0.0f ) ) {
			timerOn = true;
			DoSound( false );
			CancelEvents( &EV_Speaker_Timer );
			PostEventSec( &EV_Speaker_Timer, wait + gameLocal.random.CRandomFloat() * random );
		} else  if ( !refSound.waitfortrigger && !(refSound.referenceSound && refSound.referenceSound->CurrentlyPlaying() ) ) {
			// start it if it isn't already playing, and we aren't waitForTrigger
			DoSound( true );
			timerOn = false;
		}
	}
}

/*
===============
idSound::SetSound
===============
*/
void idSound::SetSound( const char *sound, int channel ) {
	const idSoundShader *shader = declManager->FindSound( sound );
	if ( shader != refSound.shader ) {
		FreeSoundEmitter( true );
	}
	gameEdit->ParseSpawnArgsToRefSound(&spawnArgs, &refSound);
	refSound.shader = shader;
	// start it if it isn't already playing, and we aren't waitForTrigger
	if ( !refSound.waitfortrigger && !(refSound.referenceSound && refSound.referenceSound->CurrentlyPlaying() ) ) {
		DoSound( true );
	}
}

/*
================
idSound::DoSound
================
*/
void idSound::DoSound( bool play ) {
	if ( play ) {
		StartSoundShader( refSound.shader, SND_CHANNEL_ANY, refSound.parms.soundShaderFlags, true, &playingUntilTime );
		playingUntilTime += gameLocal.time;
	} else {
		StopSound( SND_CHANNEL_ANY, true );
		playingUntilTime = 0;
	}
}

/*
================
idSound::Event_On
================
*/
void idSound::Event_On( void ) {
	if ( wait > 0.0f ) {
		timerOn = true;
		PostEventSec( &EV_Speaker_Timer, wait + gameLocal.random.CRandomFloat() * random );
	}
	DoSound( true );
}

/*
================
idSound::Event_Off
================
*/
void idSound::Event_Off( void ) {
	if ( timerOn ) {
		timerOn = false;
		CancelEvents( &EV_Speaker_Timer );
	}
	DoSound( false );
}

/*
===============
idSound::ShowEditingDialog
===============
*/
void idSound::ShowEditingDialog( void ) {
	common->InitTool( EDITOR_SOUND, &spawnArgs );
}

