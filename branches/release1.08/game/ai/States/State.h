/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5469 $ (Revision of last commit) 
 $Date: 2012-06-01 21:02:35 -0400 (Fri, 01 Jun 2012) $ (Date of last commit)
 $Author: grayman $ (Author of last commit)
 
******************************************************************************/

#ifndef __AI_STATE_H__
#define __AI_STATE_H__

#include "../../AIComm_Message.h"
#include "../../BinaryFrobMover.h"
#include "../Subsystem.h"
#include <boost/shared_ptr.hpp>

namespace ai
{

class State
{
protected:
	// The owning entity
	idEntityPtr<idAI> _owner;
	float _alertLevelDecreaseRate;

public:
	// Get the name of this state
	virtual const idStr& GetName() const = 0;

	// greebo: Sets the owner of this State, this is usually called by the mind
	void SetOwner(idAI* owner);

	// This is called when the state is about to be invoked the first time by Mind.
	virtual void Init(idAI* owner);

	/**
	 * greebo: This is called each time the Mind is thinking and gives 
	 *         the State an opportunity to monitor the Subsystems.
	 *         
	 * Note: This is basically called each frame, so don't do expensive things in here.
	 */
	virtual void Think(idAI* owner) = 0;

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Incoming events issued by the Subsystems
	virtual void OnSubsystemTaskFinished(idAI* owner, SubsystemId subSystem)
	{} // empty default implementation

	/**
	 * greebo: This handles an incoming tactile alert.
	 *
	 * @tactEnt: is the entity in question (actor, projectile, moveable,...)
	 */
	virtual void OnTactileAlert(idEntity* tactEnt);

	/**
	 * greebo: Gets called by idAI::PerformVisualScan when a visual alert is 
	 *         justified under the current circumstances.
	 *
	 * @enemy: The detected entity (usually the player). Must not be NULL.
	 */
	virtual void OnVisualAlert(idActor* enemy);

	/**
	 * greebo: Gets called when the AI is alerted by a suspicious sound.
	 */
	virtual void OnAudioAlert();

	// Handles incoming messages from other AI
	virtual void OnAICommMessage(CommMessage& message, float psychLoud);

	// greebo: An event called by the obstacle avoidance code to indicate that we have a frobmover ahead
	virtual void OnFrobDoorEncounter(CFrobDoor* frobDoor);

	virtual void NeedToUseElevator(const eas::RouteInfoPtr& routeInfo);

	virtual void OnBlindStim(idEntity* stimSource, bool skipVisibilityCheck);

	// Handles incoming visual stims coming from the given entity
	virtual void OnVisualStim(idEntity* stimSource);

	virtual bool ShouldProcessAlert(EAlertType newAlertType);

	// greebo: These get called by the above OnVisualStim() method. 
	// The passed <stimSource> pointer is guaranteed to be non-NULL.
	virtual void OnVisualStimWeapon(idEntity* stimSource, idAI* owner);
	virtual void OnVisualStimSuspicious(idEntity* stimSource, idAI* owner); // grayman #1327
	virtual void OnVisualStimRope( idEntity* stimSource, idAI* owner, idVec3 ropeStimSource ); // grayman #2872
	virtual void OnPersonEncounter(idEntity* stimSource, idAI* owner);
	virtual void OnVisualStimBlood(idEntity* stimSource, idAI* owner);
	virtual void OnVisualStimLightSource(idEntity* stimSource, idAI* owner);
	virtual void OnVisualStimMissingItem(idEntity* stimSource, idAI* owner);
	virtual void OnVisualStimBrokenItem(idEntity* stimSource, idAI* owner);
	virtual void OnVisualStimDoor(idEntity* stimSource, idAI* owner);
	virtual void OnHitByMoveable(idAI* owner, idEntity* tactEnt); // grayman #2816

	// greebo: Gets called by OnPersonEncounter on finding a dead body
	// returns TRUE when the stim should be ignored from now on, FALSE otherwise
	virtual bool OnDeadPersonEncounter(idActor* person, idAI* owner);
	virtual bool OnUnconsciousPersonEncounter(idActor* person, idAI* owner);

	/**
	 * greebo: Called if an attacker performed a failed knockout attack
	 *
	 * @attacker: The attacking entity (most probably the player)
	 * @direction: the direction of the attack
	 * @hitHead: is TRUE if the head has been hit, FALSE if the attacker hit other body parts
	 */
	virtual void OnFailedKnockoutBlow(idEntity* attacker, const idVec3& direction, bool hitHead);

	/**
	 * greebo: Called whenever the AI gets hit by a projectile.
	 */
	virtual void OnProjectileHit(idProjectile* projectile, idEntity* attacker, int damageTaken);

	/**
	* ishtvan: Called when targets are changed, does nothing on base class
	**/
	virtual void OnChangeTarget(idAI *owner) {};

	// Invoked when the movement subsystem considers this AI as blocked
	virtual void OnMovementBlocked(idAI* owner);

	virtual void ForgetSittingSleeping() {}; // grayman #3154

protected:
	/**
	 * greebo: Gets called by OnTactileAlert when the offending entity is 
	 *         an idProjectile. It does not alert the owning AI, as this is
	 *         handled in the calling OnTactileAlert method.
	 */
	virtual void OnProjectileHit(idProjectile* projectile);

	/**
	 * greebo: Method implemented by the States to check
	 *         for the correct alert index before continuing.
	 *
	 * returns TRUE by default. Backbone States override this method.
	 */
	virtual bool CheckAlertLevel(idAI* owner);

	virtual void UpdateAlertLevel();

	// Get the sound shader name for owner, when greeting otherAI
	virtual idStr GetGreetingSound(idAI* owner, idAI* otherAI);

	// Get the sound shader name for owner, when responding to a greeting from otherAI
	virtual idStr GetGreetingResponseSound(idAI* owner, idAI* otherAI);

	bool CheckTorch(idAI* owner, idLight* light); // grayman - check for potential torch drop

	bool SomeoneNearDoor(idAI* owner, CFrobDoor* door); // grayman #3104

private:
	void OnMessageDetectedSomethingSuspicious(CommMessage& message);

	// grayman #2603 - visual stim markers
	enum StimMarker
	{
		EAIuse_Default,
		EAIuse_Person,
		EAIuse_Weapon,
		EAIuse_Blood_Evidence,
		EAIuse_Lightsource,
		EAIuse_Missing_Item_Marker,
		EAIuse_Broken_Item,
		EAIuse_Door,
		EAIuse_Suspicious,	// grayman #1327
		EAIuse_Rope,		// grayman #2872
		ENumMarkers,		// invalid index
	};

	enum SBO_Level // grayman #2603 - "shouldBeOn" levels
	{
		ESBO_0,
		ESBO_1,
		ESBO_2,
		ENumSBOLevels,
	};

};
typedef boost::shared_ptr<State> StatePtr;

} // namespace ai

#endif /* __AI_STATE_H__ */
