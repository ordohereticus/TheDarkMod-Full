/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3865 $
 * $Date: 2010-03-21 07:06:10 -0400 (Sun, 21 Mar 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: ChaseEnemyRangedTask.cpp 3865 2010-03-21 11:06:10Z greebo $", init_version);

#include "ChaseEnemyRangedTask.h"
#include "../Memory.h"
#include "../Library.h"
#include "../States/UnreachableTargetState.h"

namespace ai
{

// Get the name of this task
const idStr& ChaseEnemyRangedTask::GetName() const
{
	static idStr _name(TASK_CHASE_ENEMY_RANGED);
	return _name;
}

void ChaseEnemyRangedTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Init the base class
	Task::Init(owner, subsystem);

	owner->AI_RUN = true;

	_enemy = owner->GetEnemy();
	_hasGoal = false;
}

bool ChaseEnemyRangedTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("ChaseEnemyRangedTask performing.\r");

	idAI* owner = _owner.GetEntity();
	assert(owner != NULL);

	Memory& memory = owner->GetMemory();

	idActor* enemy = _enemy.GetEntity();
	if (enemy == NULL)
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("No enemy, terminating task!\r");
		return true;
	}

	idStr waitState(owner->WaitState());

	// Can we damage the enemy already? (this flag is set by the combat state)
	if (memory.canHitEnemy)
	{
		_hasGoal = false;
		// Yes, stop the move!
		owner->StopMove(MOVE_STATUS_DONE);
		//gameLocal.Printf("Enemy is reachable!\n");
		// Turn to the player
		owner->TurnToward(enemy->GetEyePosition());
	}
	
	// try to get to the last visible enemy position
	else if (waitState != "ranged_attack" && owner->MoveToPosition(owner->lastVisibleEnemyPos))
	{
		_hasGoal = false;
		if (owner->AI_MOVE_DONE)
		{
			// Position has been reached, turn to player, if visible
			if (owner->AI_ENEMY_VISIBLE)
			{
				// Turn to the player
				owner->TurnToward(owner->GetEyePosition());
			}
		}
		else
		{
			// AI is moving, this is ok
		}
	}
	else 
	{
		// try to find an attack position using the AAS system
		if (waitState != "ranged_attack" && (!_hasGoal || owner->AI_MOVE_DONE))
		{
			// Can't reach the last visible enemy position,find another position within range
			aasGoal_t goal = owner->GetPositionWithinRange(enemy->GetEyePosition());
			if (goal.areaNum != -1)
			{
				// Found a suitable attack position, now move to it
				if (owner->MoveToPosition(goal.origin))
				{
					_hasGoal = true;
				}
			}
			else
			{
				// We did not find a reachable attack position 
				// it might be that the AI is not able to reach the enemy at all
				DM_LOG(LC_AI, LT_INFO)LOGSTRING("Destination unreachable!\r");
				gameLocal.Printf("Destination unreachable... \n");
				owner->StopMove(MOVE_STATUS_DEST_UNREACHABLE);
				owner->GetMind()->SwitchState(STATE_UNREACHABLE_TARGET);
				return true;
			}
		}
	}

	return false; // not finished yet
}

void ChaseEnemyRangedTask::SetEnemy(idActor* enemy)
{
	_enemy = enemy;
}

void ChaseEnemyRangedTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	savefile->WriteBool(_hasGoal);
	_enemy.Save(savefile);
}

void ChaseEnemyRangedTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	savefile->ReadBool(_hasGoal);
	_enemy.Restore(savefile);
}

ChaseEnemyRangedTaskPtr ChaseEnemyRangedTask::CreateInstance()
{
	return ChaseEnemyRangedTaskPtr(new ChaseEnemyRangedTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar chaseEnemyRangedTaskRegistrar(
	TASK_CHASE_ENEMY_RANGED, // Task Name
	TaskLibrary::CreateInstanceFunc(&ChaseEnemyRangedTask::CreateInstance) // Instance creation callback
);

} // namespace ai
