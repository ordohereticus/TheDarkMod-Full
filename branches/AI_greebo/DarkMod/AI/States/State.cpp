/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-30 18:53:28 +0200 (Di, 30 Okt 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: State.cpp 1435 2007-10-16 16:53:28Z greebo $", init_version);

#include "State.h"
#include "../Memory.h"
#include "../Tasks/SingleBarkTask.h"
#include "../../AIComm_Message.h"
#include "../../StimResponse/StimResponse.h"
#include "SearchingState.h"
#include "ReactingToStimulusState.h"
#include "CombatState.h"

namespace ai
{

#define AIUSE_WEAPON			"AIUSE_WEAPON"
#define AIUSE_LIGHTSOURCE		"AIUSE_LIGHTSOURCE"
#define AIUSE_BLOOD_EVIDENCE	"AIUSE_BLOOD_EVIDENCE"
#define AIUSE_SEAT				"AIUSE_SEAT"
#define AIUSE_COOK				"AIUSE_COOK"
#define AIUSE_EAT				"AIUSE_EAT"
#define AIUSE_PET				"AIUSE_PET"
#define AIUSE_MONSTER			"AIUSE_MONSTER"  // a random or caged monster, not a pet
#define AIUSE_UNDEAD			"AIUSE_UNDEAD" // An undead creature
#define AIUSE_CATTLE			"AIUSE_CATTLE"
#define AIUSE_PERSON			"AIUSE_PERSON"
#define AIUSE_PEST				"AIUSE_PEST"
#define AIUSE_DRINK			"AIUSE_DRINK"
#define AIUSE_DOOR				"AIUSE_DOOR"
#define AIUSE_MISSING_ITEM_MARKER "AIUSE_MISSING_ITEM_MARKER"

//----------------------------------------------------------------------------------------
// The following strings define classes of person, these are used if AIUse is AIUSE_PERSON 

// This is the key value
#define PERSONTYPE_KEY				"personType"

// And these are values in use, add to this list as needed
#define PERSONTYPE_GENERIC			"PERSONTYPE_GENERIC"
#define PERSONTYPE_NOBLE			"PERSONTYPE_NOBLE"
#define PERSONTYPE_CITYWATCH		"PERSONTYPE_CITYWATCH"
#define PERSONTYPE_MERC_PROGUARD	"PERSONTYPE_MERC_PROGUARD"
#define PERSONTYPE_BUILDER			"PERSONTYPE_BUILDER"
#define PERSONTYPE_PAGAN			"PERSONTYPE_PAGAN"
#define PERSONTYPE_THIEF			"PERSONTYPE_THIEF"

//----------------------------------------------------------------------------------------
// The following strings define genders of person, these are used if AIUse is AIUSE_PERSON 
// I don't want to get into the politics of gender identity here, this is just because the recorded
// voices will likely be in gendered languages.  As such, I'm just including the categories
// that are involved in word gender selection in many languages.
#define PERSONGENDER_KEY		"personGender"

#define PERSONGENDER_MALE		"PERSONGENDER_MALE"
#define PERSONGENDER_FEMALE		"PERSONGENDER_FEMALE"
#define PERSONGENDER_UNKNOWN	"PERSONGENDER_UNKNOWN"

//----------------------------------------------------------------------------------------
// The following key and values are used for identifying types of lights
#define AIUSE_LIGHTTYPE_KEY		"lightType"
#define AIUSE_LIGHTTYPE_TORCH	"AIUSE_LIGHTTYPE_TORCH"
#define AIUSE_LIGHTTYPE_GASLAMP	 "AIUSE_LIGHTTYPE_GASLAMP"
#define AIUSE_LIGHTTYPE_ELECTRIC "AIUSE_LIGHTTYPE_ELECTRIC"
#define AIUSE_LIGHTTYPE_MAGIC	 "AIUSE_LIGHTTYPE_MAGIC"
#define AIUSE_LIGHTTYPE_AMBIENT	 "AIUSE_LIGHTTYPE_AMBIENT"

//----------------------------------------------------------------------------------------
// The following key is used to identify the name of the switch entity used to turn on
// a AIUSE_LIGHTTYPE_ELECTRIC light.
#define AIUSE_LIGHTSWITCH_NAME_KEY	"switchName"

//----------------------------------------------------------------------------------------
// The following defines a key that should be non-0 if the device should be on
#define AIUSE_SHOULDBEON_KEY		"shouldBeOn"

//----------------------------------------------------------------------------------------
// The following defines a key that should be non-0 if the device should be closed
#define AIUSE_SHOULDBECLOSED_KEY		"shouldBeClosed"

void State::Init(idAI* owner)
{
	_owner = owner;
}

// Save/Restore methods
void State::Save(idSaveGame* savefile) const
{
	_owner.Save(savefile);
}

void State::Restore(idRestoreGame* savefile)
{
	_owner.Restore(savefile);
}

void State::OnVisualStim(idEntity* stimSource)
{
	idAI* owner = _owner.GetEntity();
	if (owner == NULL)
	{
		// Owner might not be initialised, serviceEvents is called after Mind::Think()
		return;
	}

	// Don't respond to NULL entities or when dead/knocked out and no enemy in sight
	if (stimSource == NULL || 
		owner->AI_KNOCKEDOUT || owner->AI_DEAD ||
		owner->GetEnemy() != NULL)
	{
		return;
	}

	// Get AI use of the stim
	idStr aiUse = stimSource->spawnArgs.GetString("AIUse");

	// Only respond if we can actually see it
	if (aiUse == AIUSE_LIGHTSOURCE)
	{
		// Special case for lights, we know it is off if there is no light. Also we can notice it
		// if we are not looking right at it.
		if (!owner->CanSeeExt(stimSource, false, false))
		{
			return;
		}
	}
	else if (!owner->CanSee(stimSource, true))
	{
		//DEBUG_PRINT ("I can't see the " + aiUse);
		return;
	}

	// Random chance
	float chance(gameLocal.random.RandomFloat());
	float chanceToNotice(0);
		
	if (aiUse == AIUSE_WEAPON)
	{
		chanceToNotice = owner->spawnArgs.GetFloat("chanceNoticeWeapon");
		if (chance < chanceToNotice)
		{
			OnVisualStimWeapon(stimSource);
		}
	}
	else if (aiUse == AIUSE_PERSON)
	{
		chanceToNotice = owner->spawnArgs.GetFloat("chanceNoticePerson");
		if (chance < chanceToNotice)
		{
			OnVisualStimPerson(stimSource);
		}
	}
	else if (aiUse == AIUSE_BLOOD_EVIDENCE)
	{
		chanceToNotice = owner->spawnArgs.GetFloat("chanceNoticeBlood");
		if (chance < chanceToNotice)
		{
			OnVisualStimBlood(stimSource);
		}
	}
	else if (aiUse == AIUSE_LIGHTSOURCE)
	{
		chanceToNotice = owner->spawnArgs.GetFloat("chanceNoticeLight");
		if (chance < chanceToNotice)
		{
			OnVisualStimLightSource(stimSource);
		}
	}
	else if (aiUse == AIUSE_MISSING_ITEM_MARKER)
	{
		chanceToNotice = owner->spawnArgs.GetFloat("chanceNoticeMissingItem");
		if (chance < chanceToNotice)
		{
			OnVisualStimMissingItem(stimSource);
		}
	}
	else if (aiUse == AIUSE_DOOR)
	{
		chanceToNotice = owner->spawnArgs.GetFloat("chanceNoticeDoor");
		if (chance < chanceToNotice)
		{
			OnVisualStimOpenDoor(stimSource);
		}
	}
}

void State::OnVisualStimWeapon(idEntity* stimSource)
{
	assert(stimSource != NULL); // must be fulfilled

	idAI* owner = _owner.GetEntity();
	assert(owner);

	// Memory shortcut
	Memory& memory = owner->GetMind()->GetMemory();

	// We've seen this object, don't respond to it again
	stimSource->ResponseIgnore(ST_VISUAL, owner);

	if (stimSource->IsType(idWeapon::Type))
	{
		// Is it a friendly weapon?  To find out we need to get its owner.
		idActor* objectOwner = static_cast<idWeapon*>(stimSource)->GetOwner();
		
		if (owner->IsFriend(objectOwner))
		{
			DM_LOG(LC_AI, LT_DEBUG).LogString("Ignoring visual stim from weapon with friendly owner\r");
			return;
		}
	}
	
	// Vocalize that see something out of place
	gameLocal.Printf("Hmm, that isn't right! A weapon!\n");
	if (gameLocal.time - memory.lastTimeVisualStimBark >= MINIMUM_SECONDS_BETWEEN_STIMULUS_BARKS)
	{
		memory.lastTimeVisualStimBark = gameLocal.time;
		owner->GetSubsystem(SubsysCommunication)->PushTask(
			TaskPtr(new SingleBarkTask("snd_somethingSuspicious"))
		);
	}

	// TWO more piece of evidence of something out of place: A weapon is not a good thing
	memory.countEvidenceOfIntruders += 2;

	// Raise alert level
	if (owner->AI_AlertNum < owner->thresh_combat-0.1)
	{
		owner->Event_SetAlertLevel(owner->thresh_combat - 0.1);
	}
	
	memory.alertPos = stimSource->GetPhysics()->GetOrigin();
	memory.alertType = EAlertVisual;

	// Do search as if there is an enemy that has escaped
	memory.alertRadius = LOST_ENEMY_ALERT_RADIUS;
	memory.alertSearchVolume = LOST_ENEMY_SEARCH_VOLUME; 
	memory.alertSearchExclusionVolume.Zero();
	
	owner->AI_VISALERT = false;
	
	// Do new reaction to stimulus
	memory.stimulusLocationItselfShouldBeSearched = true;
	memory.searchingDueToCommunication = false;
	
	// Switch state
	owner->GetMind()->PushStateIfHigherPriority(STATE_REACTING_TO_STIMULUS, PRIORITY_REACTING_TO_STIMULUS);
}

void State::OnVisualStimPerson(idEntity* stimSource)
{
	idAI* owner = _owner.GetEntity();
	assert(owner);

	bool ignoreStimulusFromNowOn = true;
	
	idActor* other = dynamic_cast<idActor*>(stimSource);

	if (other == NULL)
	{
		// No Actor, quit
		return;
	}

	// Are they dead or unconscious?
	if (other->health <= 0)
	{
		// React to finding body
		ignoreStimulusFromNowOn = OnVisualStimDeadPerson(other);
	}
	else if (other->IsKnockedOut())
	{
		// React to finding unconscious person
		ignoreStimulusFromNowOn = OnVisualStimUnconsciousPerson(other);
	}
	else
	{
		// Not knocked out, not dead, deal with it
		if (owner->IsEnemy(other))
		{
			// Living enemy
			gameLocal.Printf("I see a living enemy!\n");
			owner->SetEnemy(other);
			owner->AI_VISALERT = true;
			
			owner->GetMind()->PushStateIfHigherPriority(STATE_COMBAT, PRIORITY_COMBAT);
			// An enemy should not be ignored in the future
			ignoreStimulusFromNowOn = false;
		}
		else if (owner->IsFriend(other))
		{
			// Living friend
		}
		else
		{
			// Living neutral persons are not being handled, ignore it from now on
			ignoreStimulusFromNowOn = true;
		}
	}

	if (ignoreStimulusFromNowOn)
	{
		// We've seen this object, don't respond to it again
		stimSource->ResponseIgnore(ST_VISUAL, owner);
	}
}

bool State::OnVisualStimDeadPerson(idActor* person)
{
	assert(person); // must not be NULL

	idAI* owner = _owner.GetEntity();
	assert(owner);

	// Memory shortcut
	Memory& memory = owner->GetMind()->GetMemory();

	if (owner->IsEnemy(person))
	{
		// The dead person is your enemy, ignore from now on
		return true;
	}
	else 
	{
		// The dead person is neutral or friendly, this is suspicious
		gameLocal.Printf("I see dead people!\n");

		// We've seen this object, don't respond to it again
		person->ResponseIgnore(ST_VISUAL, owner);

		// Three more piece of evidence of something out of place: A dead body is a REALLY bad thing
		memory.countEvidenceOfIntruders += 3;
		
		// Determine what to say
		idStr soundName;
		idStr personGender = person->spawnArgs.GetString(PERSONGENDER_KEY);

		if (idStr(person->spawnArgs.GetString(PERSONTYPE_KEY)) == owner->spawnArgs.GetString(PERSONTYPE_KEY))
		{
			soundName = "snd_foundComradeBody";
		}
		else if (personGender == PERSONGENDER_FEMALE)
		{
			soundName = "snd_foundDeadFemale";
		}
		else
		{
			soundName = "snd_foundDeadMale";
		}

		// Speak a reaction
		if (gameLocal.time - memory.lastTimeVisualStimBark >= MINIMUM_SECONDS_BETWEEN_STIMULUS_BARKS)
		{
			memory.lastTimeVisualStimBark = gameLocal.time;
			owner->GetSubsystem(SubsysCommunication)->PushTask(
				TaskPtr(new SingleBarkTask(soundName))
			);
		}

		// Raise alert level
		if (owner->AI_AlertNum < owner->thresh_combat - 0.1f)
		{
			memory.alertPos = person->GetPhysics()->GetOrigin();
			memory.alertType = EAlertVisual;
			
			// Do search as if there is an enemy that has escaped
			memory.alertRadius = LOST_ENEMY_ALERT_RADIUS;
			memory.alertSearchVolume = LOST_ENEMY_SEARCH_VOLUME; 
			memory.alertSearchExclusionVolume.Zero();
			
			owner->AI_VISALERT = false;
			
			owner->Event_SetAlertLevel(owner->thresh_combat - 0.1);
		}
					
		// Do new reaction to stimulus
		memory.stimulusLocationItselfShouldBeSearched = true;
		memory.searchingDueToCommunication = false;
		
		// Callback for objectives
		owner->FoundBody(person);

		// Switch state
		owner->GetMind()->PushStateIfHigherPriority(STATE_REACTING_TO_STIMULUS, PRIORITY_REACTING_TO_STIMULUS);			
	}
}

bool State::OnVisualStimUnconsciousPerson(idActor* person)
{
	assert(person != NULL); // must not be NULL

	return true;
}

void State::OnVisualStimBlood(idEntity* stimSource)
{

}

void State::OnVisualStimLightSource(idEntity* stimSource)
{

}

void State::OnVisualStimMissingItem(idEntity* stimSource)
{

}

void State::OnVisualStimOpenDoor(idEntity* stimSource)
{

}

void State::OnAICommMessage(CAIComm_Message* message)
{
	assert(message); // Don't accept NULL messages

	// Get the message parameters
	CAIComm_Message::TCommType commType = message->getCommunicationType();
	
	idEntity* issuingEntity = message->getIssuingEntity();
	idEntity* recipientEntity = message->getRecipientEntity();
	idEntity* directObjectEntity = message->getDirectObjectEntity();
	idVec3 directObjectLocation = message->getDirectObjectLocation();

	if (issuingEntity != NULL)
	{
		DM_LOG(LC_AI, LT_INFO).LogString("Got incoming message from %s\r", issuingEntity->name.c_str());
	}

	idAI* owner = _owner.GetEntity();
	assert(owner != NULL);

	Memory& memory = owner->GetMind()->GetMemory();

	switch (commType)
	{
		case CAIComm_Message::Greeting_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: Greeting_CommType\r");
			// Have seen a friend
			memory.lastTimeFriendlyAISeen = gameLocal.time;

			// If not too upset, look at them
			if (owner->AI_AlertNum < owner->thresh_2)
			{
				owner->Event_LookAtEntity(issuingEntity, 3.0); // 3 seconds
			}
			break;
		case CAIComm_Message::FriendlyJoke_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: FriendlyJoke_CommType\r");
			// Have seen a friend
			memory.lastTimeFriendlyAISeen = gameLocal.time;

			if (directObjectEntity == owner)
			{
				gameLocal.Printf("Hah, yer no better!\n");
			}
			else
			{
				gameLocal.Printf("Ha, yer right, they be an ass\n");
			}
			break;
		case CAIComm_Message::Insult_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: Insult_CommType\r");
			if (directObjectEntity == owner)
			{
				gameLocal.Printf("Same to you, buddy\n");
			}
			else if (owner->IsEnemy(directObjectEntity))
			{
				gameLocal.Printf("Hah!\n");
			}
			else
			{
				gameLocal.Printf("I'm not gettin' involved\n");
			}
			break;
		case CAIComm_Message::RequestForHelp_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: RequestForHelp_CommType\r");
			if (owner->IsFriend(issuingEntity))
			{
				// Do we already have a target we are dealing with?
				if (owner->GetEnemy() != NULL)
				{
					gameLocal.Printf("I'm too busy, I have a target!\n");
					break;
				}

				if (directObjectEntity->IsType(idActor::Type))
				{
					// Bark
					owner->GetSubsystem(SubsysCommunication)->PushTask(
						SingleBarkTaskPtr(new SingleBarkTask("snd_assistFriend"))
					);

					gameLocal.Printf("Ok, I'm helping you.\n");

					owner->SetEnemy(static_cast<idActor*>(directObjectEntity));
					owner->GetMind()->PerformCombatCheck();
				}
			}
			else if (owner->AI_AlertNum < owner->thresh_1*0.5f)
			{
				owner->Event_SetAlertLevel(owner->thresh_1*0.5f);
			}
			break;
		case CAIComm_Message::RequestForMissileHelp_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: RequestForMissileHelp_CommType\r");
			// Respond if they are a friend and we have a ranged weapon
			if (owner->IsFriend(issuingEntity) && owner->GetNumRangedWeapons() > 0)
			{
				// Do we already have a target we are dealing with?
				if (owner->GetEnemy() != NULL)
				{
					gameLocal.Printf("I'm too busy, I have a target!\n");
					break;
				}

				if (directObjectEntity->IsType(idActor::Type))
				{
					gameLocal.Printf("I'll attack it with my ranged weapon!\n");

					// Bark
					owner->GetSubsystem(SubsysCommunication)->PushTask(
						SingleBarkTaskPtr(new SingleBarkTask("snd_assistFriend"))
					);
					
					owner->SetEnemy(static_cast<idActor*>(directObjectEntity));
					owner->GetMind()->PerformCombatCheck();
				}
			}
			else 
			{
				gameLocal.Printf("I don't have a ranged weapon or I am not getting involved.\n");
				if (owner->AI_AlertNum < owner->thresh_1*0.5f)
				{
					owner->Event_SetAlertLevel(owner->thresh_1*0.5f);
				}
			}
			break;
		case CAIComm_Message::RequestForMeleeHelp_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: RequestForMeleeHelp_CommType\r");
			// Respond if they are a friend and we have a ranged weapon
			if (owner->IsFriend(issuingEntity) && owner->GetNumMeleeWeapons() > 0)
			{
				// Do we already have a target we are dealing with?
				if (owner->GetEnemy() != NULL)
				{
					gameLocal.Printf("I'm too busy, I have a target!\n");
					break;
				}

				if (directObjectEntity->IsType(idActor::Type))
				{
					gameLocal.Printf("I'll attack it with my melee weapon!\n");

					// Bark
					owner->GetSubsystem(SubsysCommunication)->PushTask(
						SingleBarkTaskPtr(new SingleBarkTask("snd_assistFriend"))
					);
					
					owner->SetEnemy(static_cast<idActor*>(directObjectEntity));
					owner->GetMind()->PerformCombatCheck();
				}
			}
			else 
			{
				gameLocal.Printf("I don't have a melee weapon or I am not getting involved.\n");
				if (owner->AI_AlertNum < owner->thresh_1*0.5f)
				{
					owner->Event_SetAlertLevel(owner->thresh_1*0.5f);
				}
			}
			break;
		case CAIComm_Message::RequestForLight_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: RequestForLight_CommType\r");
			gameLocal.Printf("I don't know how to bring light!\n");
			break;
		case CAIComm_Message::DetectedSomethingSuspicious_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: DetectedSomethingSuspicious_CommType\r");
			OnMessageDetectedSomethingSuspicious(message);
			break;
		case CAIComm_Message::DetectedEnemy_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: DetectedEnemy_CommType\r");
			gameLocal.Printf("Somebody spotted an enemy... (%s)\n", directObjectEntity->name.c_str());
	
			if (owner->GetEnemy() != NULL)
			{
				gameLocal.Printf("I'm too busy with my own target!\n");
				return;
			}

			if (owner->IsFriend(issuingEntity) && owner->IsEnemy(directObjectEntity))
			{
				owner->Event_SetAlertLevel(owner->thresh_combat);
				
				gameLocal.Printf("They're my friend, I'll attack it too!\n");
				memory.alertPos = directObjectLocation;

				// Alert position is set, go into searching mode
				owner->GetMind()->PushStateIfHigherPriority(STATE_SEARCHING, PRIORITY_SEARCHING);
			}
			break;
		case CAIComm_Message::FollowOrder_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: FollowOrder_CommType\r");
			if (recipientEntity == owner && owner->IsFriend(issuingEntity))
			{
				gameLocal.Printf("But I don't know how to follow somebody!\n");
			}
			break;
		case CAIComm_Message::GuardLocationOrder_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: GuardLocationOrder_CommType\r");
			if (recipientEntity == owner && owner->IsFriend(issuingEntity))
			{
				gameLocal.Printf("But I don't know how to guard a location!\n");
			}
			break;
		case CAIComm_Message::GuardEntityOrder_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: GuardEntityOrder_CommType\r");
			if (recipientEntity == owner && owner->IsFriend(issuingEntity))
			{
				gameLocal.Printf("But I don't know how to guard an entity!\n");
			}
			break;
		case CAIComm_Message::PatrolOrder_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: PatrolOrder_CommType\r");
			if (recipientEntity == owner && owner->IsFriend(issuingEntity))
			{
				gameLocal.Printf("But I don't know how to switch my patrol route!\n");
			}
			break;
		case CAIComm_Message::SearchOrder_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: SearchOrder_CommType\r");
			if (recipientEntity == owner && owner->IsFriend(issuingEntity))
			{
				// Set alert pos to the position we were ordered to search
				memory.alertPos = directObjectLocation;
				memory.chosenHidingSpot = directObjectLocation;
				memory.numPossibleHidingSpotsSearched = 0;

				// Alert position is set, go into searching mode
				owner->GetMind()->PushStateIfHigherPriority(STATE_SEARCHING, PRIORITY_SEARCHING);
			}
			break;
		case CAIComm_Message::AttackOrder_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: AttackOrder_CommType\r");
			// Set this as our enemy and enter combat
			if (recipientEntity == owner && owner->IsFriend(issuingEntity))
			{
				gameLocal.Printf("Yes sir! Attacking your specified target!\n");

				if (directObjectEntity->IsType(idActor::Type))
				{
					owner->SetEnemy(static_cast<idActor*>(directObjectEntity));
					owner->GetMind()->SwitchStateIfHigherPriority(STATE_COMBAT, PRIORITY_COMBAT);
				}
			}
			else if (owner->AI_AlertNum < owner->thresh_1*0.5f)
			{
				owner->Event_SetAlertLevel(owner->thresh_1*0.5f);
			}
			break;
		case CAIComm_Message::GetOutOfTheWayOrder_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: GetOutOfTheWayOrder_CommType\r");
			break;
		case CAIComm_Message::ConveyWarning_EvidenceOfIntruders_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: ConveyWarning_EvidenceOfIntruders_CommType\r");
			if (issuingEntity->IsType(idAI::Type))
			{
				idAI* issuer = static_cast<idAI*>(issuingEntity);
				// Note: We deliberately don't care if the issuer is a friend or not
				float warningAmount = issuer->GetMind()->GetMemory().countEvidenceOfIntruders;
				
				if (memory.countEvidenceOfIntruders < warningAmount)
				{
					gameLocal.Printf("I've been warned about evidence of intruders.\n");
					memory.countEvidenceOfIntruders = warningAmount;

					if (owner->AI_AlertNum < owner->thresh_1*0.5f)
					{
						owner->Event_SetAlertLevel(owner->thresh_1*0.5f);
					}
				}
			}
			break;
		case CAIComm_Message::ConveyWarning_ItemsHaveBeenStolen_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: ConveyWarning_ItemsHaveBeenStolen_CommType\r");
			// Note: We deliberately don't care if the issuer is a friend or not
			if (!memory.itemsHaveBeenStolen)
			{
				gameLocal.Printf("I've been warned that items have been stolen.\n");
				memory.itemsHaveBeenStolen = true;

				if (owner->AI_AlertNum < owner->thresh_1*0.5f)
				{
					owner->Event_SetAlertLevel(owner->thresh_1*0.5f);
				}
			}
			break;
		case CAIComm_Message::ConveyWarning_EnemiesHaveBeenSeen_CommType:
			DM_LOG(LC_AI, LT_INFO).LogString("Message Type: ConveyWarning_EnemiesHaveBeenSeen_CommType\r");
			// Note: We deliberately don't care if the issuer is a friend or not
			if (!memory.enemiesHaveBeenSeen)
			{
				gameLocal.Printf("I've been warned that enemies have been seen.\n");
				memory.enemiesHaveBeenSeen = true;
				
				if (owner->AI_AlertNum < owner->thresh_1*0.5f)
				{
					owner->Event_SetAlertLevel(owner->thresh_1*0.5f);
				}
			}
			break;
	} // switch
}

void State::OnMessageDetectedSomethingSuspicious(CAIComm_Message* message)
{
	idEntity* issuingEntity = message->getIssuingEntity();
	idEntity* recipientEntity = message->getRecipientEntity();
	idEntity* directObjectEntity = message->getDirectObjectEntity();
	idVec3 directObjectLocation = message->getDirectObjectLocation();

	idAI* owner = _owner.GetEntity();
	assert(owner != NULL);

	Memory& memory = owner->GetMind()->GetMemory();

	gameLocal.Printf("Somebody else noticed something suspicious...\n");

	if (owner->GetEnemy() != NULL)
	{
		gameLocal.Printf ("I'm too busy with my own target!");
		return;
	}

	if (owner->IsFriend(issuingEntity))
	{
		// If not already searching something else
		if (GetName() == STATE_SEARCHING)
		{
			gameLocal.Printf ("I'm too busy searching something else\n");
			return;
		}
		
		gameLocal.Printf ("They're my friend, I'll look too!\n");
		
		// Get some search points from them.
		int numSpots = owner->GetSomeOfOtherEntitiesHidingSpotList(issuingEntity);

		if (numSpots > 0)
		{
			// What is the distance to the friend.  If it is greater than a certain amount, shout intention
			// to come help
			float distanceToIssuer = (issuingEntity->GetPhysics()->GetOrigin() - owner->GetPhysics()->GetOrigin()).LengthFast();
			if (distanceToIssuer >= MIN_DISTANCE_TO_ISSUER_TO_SHOUT_COMING_TO_ASSISTANCE)
			{
				// Bark
				owner->GetSubsystem(SubsysCommunication)->PushTask(
					SingleBarkTaskPtr(new SingleBarkTask("snd_assistFriend"))
				);
			}
			
			// If AI that called out has a higher alert num, raise ours
			// to match theres due to urgency in their voice
			float otherAlertNum = 0.0f;
			
			if (issuingEntity->IsType(idAI::Type))
			{
				otherAlertNum = static_cast<idAI*>(issuingEntity)->AI_AlertNum;
			}

			gameLocal.Printf("The AI who noticed something has an alert num of %f\n", otherAlertNum);
			if (otherAlertNum > owner->AI_AlertNum)
			{
				owner->Event_SetAlertLevel(otherAlertNum);
			}
			
			memory.searchingDueToCommunication = true;
			owner->GetMind()->PushStateIfHigherPriority(STATE_SEARCHING, PRIORITY_SEARCHING);
			return;
		}
		else
		{
			gameLocal.Printf("Hmpfh, no spots to help them with :(\n");
		}
		
	}
	else if (owner->AI_AlertNum < owner->thresh_1*0.5f)
	{
		owner->Event_SetAlertLevel(owner->thresh_1*0.5f);
	}
}

} // namespace ai
