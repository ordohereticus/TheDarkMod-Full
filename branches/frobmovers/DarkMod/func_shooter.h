/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __GAME_FUNC_SHOOTER_H__
#define __GAME_FUNC_SHOOTER_H__

#include "StimResponse/StimResponse.h"

/**
* greebo: This entity fires projectiles in (periodic) intervals.
*		  All the key paramaters can be specified in the according entityDef.
*/
class tdmFuncShooter : 
	public idStaticEntity
{
public:
	CLASS_PROTOTYPE( tdmFuncShooter );

	// Constructor
	tdmFuncShooter( void );

	// Needed on game save/load
	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	// Gets called when this entity is actually being spawned
	void				Spawn( void );

	// Event interface
	void				Event_Activate( idEntity *activator );
	void				Event_ShooterSetState( bool state );
	void				Event_ShooterGetState();
	void				Event_ShooterSetAmmo( int newAmmo );
	void				Event_ShooterGetAmmo();
	void				Event_ShooterFireProjectile();

	// Snapshot interface
	virtual void		WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void		ReadFromSnapshot( const idBitMsgDelta &msg );

	// Overload the derived idEntity::Think method so that this object gets called each frame
	virtual void		Think();

	/**
	* Fires a projectile and sets the timer to gameLocal.time.
	*/
	virtual void		Fire();

	/**
	* Tries to stimulate this class with the given stimType.
	* This checks whether the incoming stim is the required one and
	* activated the shooter if this is the case.
	*/
	virtual void		stimulate(StimType stimId);

private:
	// Calculates the next time this shooter should fire
	void				setupNextFireTime();

	// is TRUE if the shooter can fire projectiles
	bool				_active;

	// The time interval between fires in ms
	int					_fireInterval;

	// This is the maximum tolerance the fireInterval can have
	int					_fireIntervalFuzzyness;

	// Start delay in ms
	int					_startDelay;

	// The last time the shooter fired a projectile
	int					_lastFireTime;

	// The next game time the shooter should fire
	int					_nextFireTime;

	// This is != (ST_DEFAULT == -1) if a stim is required in order to stay active
	StimType			_requiredStim;
	
	// This is set to the last known time the requiredStim has visited this entity
	int					_lastStimVisit;

	// This specifies the time that can pass before the shooter becomes inactive
	// if the requiredStim is not present.
	int					_requiredStimTimeOut;

	// TRUE if periodic triggering is needed for this shooter to stay active.
	bool				_triggerRequired;

	// This is set to the last known time the requiredStim has visited this entity
	int					_lastTriggerVisit;

	// If this time has passed without a trigger event, the shooter ceases to fire.
	int					_triggerTimeOut;

	// The maximum number of projectiles (ammo) for this shooter. 
	// A value of -1 means infinite ammonition
	int					_ammo;

	// This is true if the _ammo value should be considered or not
	bool				_useAmmo;
};

#endif /* !__GAME_FUNC_SHOOTER_H__ */

