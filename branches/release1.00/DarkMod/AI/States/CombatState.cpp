/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3668 $
 * $Date: 2009-08-14 13:03:57 -0400 (Fri, 14 Aug 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: CombatState.cpp 3668 2009-08-14 17:03:57Z greebo $", init_version);

#include "CombatState.h"
#include "../Memory.h"
#include "../../AIComm_Message.h"
#include "../Tasks/RandomHeadturnTask.h"
#include "../Tasks/ChaseEnemyTask.h"
#include "../Tasks/SingleBarkTask.h"
#include "../Tasks/MeleeCombatTask.h"
#include "../Tasks/RangedCombatTask.h"
#include "../Tasks/ChaseEnemyRangedTask.h"
#include "LostTrackOfEnemyState.h"
#include "AgitatedSearchingState.h"
#include "FleeState.h"
#include "../Library.h"

namespace ai
{

// Get the name of this state
const idStr& CombatState::GetName() const
{
	static idStr _name(STATE_COMBAT);
	return _name;
}

bool CombatState::CheckAlertLevel(idAI* owner)
{
	if (owner->AI_AlertIndex < 5)
	{
		// Alert index is too low for this state, fall back
		owner->GetMind()->EndState();
		// ishtvan: swap the expanded head model back in when exiting state
		owner->SwapHeadAFCM( true );
		return false;
	}

	// Alert Index is matching, return OK
	return true;
}

void CombatState::OnTactileAlert(idEntity* tactEnt)
{
	// do nothing as of now, we are already in combat mode
}

void CombatState::OnVisualAlert(idActor* enemy)
{
	// do nothing as of now, we are already in combat mode

}

void CombatState::OnAudioAlert()
{
	idAI* owner = _owner.GetEntity();
	assert(owner != NULL);

	Memory& memory = owner->GetMemory();

	memory.alertClass = EAlertAudio;
	memory.alertPos = owner->GetSndDir();

	if (!owner->AI_ENEMY_VISIBLE)
	{
		memory.lastTimeEnemySeen = gameLocal.time;
		owner->lastReachableEnemyPos = memory.alertPos;
		// gameRenderWorld->DebugArrow(colorRed, owner->GetEyePosition(), memory.alertPos, 2, 1000);
	}
}

void CombatState::OnFailedKnockoutBlow(idEntity* attacker, const idVec3& direction, bool hitHead)
{
	// Ignore failed knockout attempts in combat mode
}

void CombatState::OnPersonEncounter(idEntity* stimSource, idAI* owner)
{
	if (!stimSource->IsType(idActor::Type)) return; // No Actor, quit

	if (owner->IsFriend(stimSource))
	{
		// Remember last time a friendly AI was seen
		owner->GetMemory().lastTimeFriendlyAISeen = gameLocal.time;
	}
	// angua: ignore other people during combat
}

void CombatState::Init(idAI* owner)
{
	// Init base class first
	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("CombatState initialised.\r");
	assert(owner);

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner)) return;

	// Shortcut reference
	Memory& memory = owner->GetMemory();

	if (!owner->GetMind()->PerformCombatCheck()) return;

	if (owner->GetMoveType() == MOVETYPE_SIT || owner->GetMoveType() == MOVETYPE_SLEEP)
	{
		owner->GetUp();
	}

	// greebo: Check for weapons and flee if we are unarmed.
	_criticalHealth = owner->spawnArgs.GetInt("health_critical", "0");
	_meleePossible = owner->GetNumMeleeWeapons() > 0;
	_rangedPossible = owner->GetNumRangedWeapons() > 0;

	if (!_meleePossible && !_rangedPossible)
	{
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("I'm unarmed, I'm afraid!\r");
		owner->GetMind()->SwitchState(STATE_FLEE);
		return;
	}

	// greebo: Check for civilian AI, which will always flee in face of a combat (this is a temporary query)
	if (owner->spawnArgs.GetBool("is_civilian", "0"))
	{
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("I'm civilian. I'm afraid.\r");
		owner->GetMind()->SwitchState(STATE_FLEE);
		return;
	}

	// We have an enemy, store the enemy entity locally
	_enemy = owner->GetEnemy();
	idActor* enemy = _enemy.GetEntity();

	owner->StopMove(MOVE_STATUS_DONE);

	owner->movementSubsystem->ClearTasks();
	owner->senseSubsystem->ClearTasks();
	owner->actionSubsystem->ClearTasks();

	owner->DrawWeapon();

	// Fill the subsystems with their tasks

	// This will hold the message to be delivered with the bark, if appropriate
	CommMessagePtr message;
	
	// Only alert the bystanders if we didn't receive the alert by message ourselves
	if (!memory.alertedDueToCommunication)
	{
		message = CommMessagePtr(new CommMessage(
			CommMessage::DetectedEnemy_CommType, 
			owner, NULL, // from this AI to anyone 
			enemy,
			memory.lastEnemyPos
		));
	}


	// The communication system plays starting bark
	idPlayer* player(NULL);
	if (enemy->IsType(idPlayer::Type))
	{
		player = static_cast<idPlayer*>(enemy);
	}

	if (player && player->m_bShoulderingBody)
	{
		owner->commSubsystem->AddCommTask(
			CommunicationTaskPtr(new SingleBarkTask("snd_spotted_player_with_body", message))
		);
	}

	else if ((MS2SEC(gameLocal.time - memory.lastTimeFriendlyAISeen)) <= MAX_FRIEND_SIGHTING_SECONDS_FOR_ACCOMPANIED_ALERT_BARK)
	{
		owner->commSubsystem->AddCommTask(
			CommunicationTaskPtr(new SingleBarkTask("snd_to_combat_company", message))
		);
	}
	else
	{
		owner->commSubsystem->AddCommTask(
			CommunicationTaskPtr(new SingleBarkTask("snd_to_combat", message))
		);
	}

	// Ranged combat
	if (_rangedPossible)
	{
		if (_meleePossible && 
			(owner->GetPhysics()->GetOrigin()-enemy->GetPhysics()->GetOrigin()).LengthFast() < 3 * owner->GetMeleeRange())
		{
			ChaseEnemyTaskPtr chaseEnemy = ChaseEnemyTask::CreateInstance();
			chaseEnemy->SetEnemy(enemy);
			owner->movementSubsystem->PushTask(chaseEnemy);

			owner->actionSubsystem->PushTask(MeleeCombatTask::CreateInstance());
			_combatType = COMBAT_MELEE;
		}
		else
		{
			owner->actionSubsystem->PushTask(RangedCombatTask::CreateInstance());
			owner->movementSubsystem->PushTask(ChaseEnemyRangedTask::CreateInstance());
			_combatType = COMBAT_RANGED;
		}
	}
	// Melee combat
	else
	{
		// The movement subsystem should start running to the last enemy position
		ChaseEnemyTaskPtr chaseEnemy = ChaseEnemyTask::CreateInstance();
		chaseEnemy->SetEnemy(enemy);
		owner->movementSubsystem->PushTask(chaseEnemy);

		owner->actionSubsystem->PushTask(MeleeCombatTask::CreateInstance());
		_combatType = COMBAT_MELEE;
	}

	// Let the AI update their weapons (make them nonsolid)
	owner->UpdateAttachmentContents(false);
	// ishtvan: swap out the expanded head bounding box for easy KOs
	owner->SwapHeadAFCM( false );
}

// Gets called each time the mind is thinking
void CombatState::Think(idAI* owner)
{
	idActor* enemy = _enemy.GetEntity();
	if (enemy == NULL)
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("No enemy, terminating task!\r");
		owner->GetMind()->EndState();
		// ishtvan: swap the expanded head model back in when exiting state
		owner->SwapHeadAFCM( true );
		return;
	}

	if (enemy->AI_DEAD)
	{
		owner->ClearEnemy();
		owner->StopMove(MOVE_STATUS_DONE);

		// TODO: Check if more enemies are in range
		owner->SetAlertLevel(owner->thresh_2 + (owner->thresh_3 - owner->thresh_2) * 0.9);

		// Emit the killed player bark
		owner->commSubsystem->AddCommTask(
			CommunicationTaskPtr(new SingleBarkTask("snd_killed_enemy"))
		);

		owner->GetMind()->EndState();
		// ishtvan: swap the expanded head model back in when exiting state
		owner->SwapHeadAFCM( true );
		return;
	}

	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner))
	{
		owner->GetMind()->EndState();
		// ishtvan: swap the expanded head model back in when exiting state
		owner->SwapHeadAFCM( true );
		return;
	}
	Memory& memory = owner->GetMemory();

	if (!owner->IsEnemy(enemy))
	{
		// angua: the relation to the enemy has changed, this is not an enemy any more
		owner->StopMove(MOVE_STATUS_DONE);
		owner->SetAlertLevel(owner->thresh_2 + (owner->thresh_3 - owner->thresh_2) * 0.9);
		owner->ClearEnemy();
		owner->GetMind()->EndState();
		
		owner->movementSubsystem->ClearTasks();
		owner->senseSubsystem->ClearTasks();
		owner->actionSubsystem->ClearTasks();

		// ishtvan: swap the expanded head model back in when exiting state
		owner->SwapHeadAFCM( true );
		return;
	}

	// angua: look at ememy
	owner->Event_LookAtPosition(enemy->GetEyePosition(), gameLocal.msec);

	// Flee if we are damaged and the current melee action is finished
	if (owner->health < _criticalHealth && owner->m_MeleeStatus.m_ActionState == MELEEACTION_READY)
	{
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("I'm badly hurt, I'm afraid!\r");
		owner->GetMind()->SwitchState(STATE_FLEE);
		// ishtvan: swap the expanded head model back in when exiting state
		owner->SwapHeadAFCM( true );
		return;
	}

	if (owner->GetMoveType() != MOVETYPE_ANIM)
	{
		owner->GetUp();
		return;
	}

	// Switch between melee and ranged combat based on enemy distance
	float enemyDist = (owner->GetPhysics()->GetOrigin()-enemy->GetPhysics()->GetOrigin()).LengthFast();

	if (_combatType == COMBAT_MELEE && _rangedPossible && enemyDist > 3 * owner->GetMeleeRange())
	{
		owner->movementSubsystem->ClearTasks();
		owner->actionSubsystem->ClearTasks();

		owner->actionSubsystem->PushTask(RangedCombatTask::CreateInstance());
		owner->movementSubsystem->PushTask(ChaseEnemyRangedTask::CreateInstance());
		_combatType = COMBAT_RANGED;
	}

	if (_combatType == COMBAT_RANGED && _meleePossible && enemyDist <= 3 * owner->GetMeleeRange())
	{
		owner->movementSubsystem->ClearTasks();
		owner->actionSubsystem->ClearTasks();

		// Allocate a ChaseEnemyTask
		owner->movementSubsystem->PushTask(TaskPtr(new ChaseEnemyTask(enemy)));

		owner->actionSubsystem->PushTask(MeleeCombatTask::CreateInstance());
		_combatType = COMBAT_MELEE;
	}

	// Check the distance to the enemy, the subsystem tasks need it.
	memory.canHitEnemy = owner->CanHitEntity(enemy, _combatType);
	if( owner->m_bMeleePredictProximity )
		memory.willBeAbleToHitEnemy = owner->WillBeAbleToHitEntity(enemy, _combatType);

	// Check whether the enemy can hit us in the near future
	memory.canBeHitByEnemy = owner->CanBeHitByEntity(enemy, _combatType);

	if (!owner->AI_ENEMY_VISIBLE && 
		(( _combatType == COMBAT_MELEE  && !memory.canHitEnemy) || _combatType == COMBAT_RANGED))
	{
		// The enemy is not visible, let's keep track of him for a small amount of time
		if (gameLocal.time - memory.lastTimeEnemySeen < MAX_BLIND_CHASE_TIME)
		{
			// Cheat a bit and take the last reachable position as "visible & reachable"
			owner->lastVisibleReachableEnemyPos = owner->lastReachableEnemyPos;
		}
		else if (owner->ReachedPos(owner->lastVisibleReachableEnemyPos, MOVE_TO_POSITION)  
			|| gameLocal.time - memory.lastTimeEnemySeen > 2 * MAX_BLIND_CHASE_TIME)
		{
			// BLIND_CHASE_TIME has expired, we have lost the enemy!
			owner->GetMind()->SwitchState(STATE_LOST_TRACK_OF_ENEMY);
			// ishtvan: swap the expanded head model back in when exiting state
			owner->SwapHeadAFCM( true );
			return;
		}
	}
}

void CombatState::Save(idSaveGame* savefile) const
{
	State::Save(savefile);

	savefile->WriteInt(_criticalHealth);
	savefile->WriteBool(_meleePossible);
	savefile->WriteBool(_rangedPossible);

	savefile->WriteInt(static_cast<int>(_combatType));

	_enemy.Save(savefile);
}

void CombatState::Restore(idRestoreGame* savefile)
{
	State::Restore(savefile);

	savefile->ReadInt(_criticalHealth);
	savefile->ReadBool(_meleePossible);
	savefile->ReadBool(_rangedPossible);

	int temp;
	savefile->ReadInt(temp);
	_combatType = static_cast<ECombatType>(temp);

	_enemy.Restore(savefile);
}

StatePtr CombatState::CreateInstance()
{
	return StatePtr(new CombatState);
}

// Register this state with the StateLibrary
StateLibrary::Registrar combatStateRegistrar(
	STATE_COMBAT, // Task Name
	StateLibrary::CreateInstanceFunc(&CombatState::CreateInstance) // Instance creation callback
);

} // namespace ai
