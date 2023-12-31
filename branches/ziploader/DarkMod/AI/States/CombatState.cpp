/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3130 $
 * $Date: 2009-01-09 12:52:12 -0500 (Fri, 09 Jan 2009) $
 * $Author: angua $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: CombatState.cpp 3130 2009-01-09 17:52:12Z angua $", init_version);

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

	owner->GetSubsystem(SubsysMovement)->ClearTasks();
	owner->GetSubsystem(SubsysSenses)->ClearTasks();
	owner->GetSubsystem(SubsysCommunication)->ClearTasks();
	owner->GetSubsystem(SubsysAction)->ClearTasks();

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

	// The communication system 
	owner->GetSubsystem(SubsysCommunication)->PushTask(
		TaskPtr(new SingleBarkTask("snd_charge", message))
	);

	// Ranged combat
	if (_rangedPossible)
	{
		if (_meleePossible && 
			(owner->GetPhysics()->GetOrigin()-enemy->GetPhysics()->GetOrigin()).LengthFast() < 3 * owner->GetMeleeRange())
		{
			ChaseEnemyTaskPtr chaseEnemy = ChaseEnemyTask::CreateInstance();
			chaseEnemy->SetEnemy(enemy);
			owner->GetSubsystem(SubsysMovement)->PushTask(chaseEnemy);

			owner->GetSubsystem(SubsysAction)->PushTask(MeleeCombatTask::CreateInstance());
			_combatType = COMBAT_MELEE;
		}
		else
		{
			owner->GetSubsystem(SubsysAction)->PushTask(RangedCombatTask::CreateInstance());
			owner->GetSubsystem(SubsysMovement)->PushTask(ChaseEnemyRangedTask::CreateInstance());
			_combatType = COMBAT_RANGED;
		}
	}
	// Melee combat
	else
	{
		// The movement subsystem should start running to the last enemy position
		ChaseEnemyTaskPtr chaseEnemy = ChaseEnemyTask::CreateInstance();
		chaseEnemy->SetEnemy(enemy);
		owner->GetSubsystem(SubsysMovement)->PushTask(chaseEnemy);

		owner->GetSubsystem(SubsysAction)->PushTask(MeleeCombatTask::CreateInstance());
		_combatType = COMBAT_MELEE;
	}

	// Let the AI update their weapons (make them nonsolid)
	owner->UpdateAttachmentContents(false);
}

// Gets called each time the mind is thinking
void CombatState::Think(idAI* owner)
{
	if (owner->AI_ENEMY_DEAD)
	{
		owner->StopMove(MOVE_STATUS_DONE);
		owner->SetAlertLevel(owner->thresh_2 + (owner->thresh_3 - owner->thresh_2) * 0.9);
		owner->GetMind()->EndState();
		return;
	}
	// Ensure we are in the correct alert level
	if (!CheckAlertLevel(owner))
	{
		owner->GetMind()->EndState();
		return;
	}
	Memory& memory = owner->GetMemory();

	idActor* enemy = _enemy.GetEntity();
	if (enemy == NULL)
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("No enemy, terminating task!\r");
		owner->GetMind()->EndState();
		return;
	}

	// angua: look at ememy
	owner->Event_LookAtPosition(enemy->GetEyePosition(), gameLocal.msec);

	if (owner->health < _criticalHealth)
	{
		DM_LOG(LC_AI, LT_INFO)LOGSTRING("I'm badly hurt, I'm afraid!\r");
		owner->GetMind()->SwitchState(STATE_FLEE);
		return;
	}

	// Switch between melee and ranged combat based on enemy distance
	float enemyDist = (owner->GetPhysics()->GetOrigin()-enemy->GetPhysics()->GetOrigin()).LengthFast();

	if (_combatType == COMBAT_MELEE && _rangedPossible && enemyDist > 3 * owner->GetMeleeRange())
	{
		owner->GetSubsystem(SubsysMovement)->ClearTasks();
		owner->GetSubsystem(SubsysAction)->ClearTasks();

		owner->GetSubsystem(SubsysAction)->PushTask(RangedCombatTask::CreateInstance());
		owner->GetSubsystem(SubsysMovement)->PushTask(ChaseEnemyRangedTask::CreateInstance());
		_combatType = COMBAT_RANGED;
	}

	if (_combatType == COMBAT_RANGED && _meleePossible && enemyDist <= 3 * owner->GetMeleeRange())
	{
		owner->GetSubsystem(SubsysMovement)->ClearTasks();
		owner->GetSubsystem(SubsysAction)->ClearTasks();

		// Allocate a ChaseEnemyTask
		owner->GetSubsystem(SubsysMovement)->PushTask(TaskPtr(new ChaseEnemyTask(enemy)));

		owner->GetSubsystem(SubsysAction)->PushTask(MeleeCombatTask::CreateInstance());
		_combatType = COMBAT_MELEE;
	}

	// Check the distance to the enemy, the subsystem tasks need it.
	memory.canHitEnemy = owner->CanHitEntity(enemy, _combatType);

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
