/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2955 $
 * $Date: 2008-10-16 11:49:44 -0400 (Thu, 16 Oct 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: MeleeCombatTask.cpp 2955 2008-10-16 15:49:44Z greebo $", init_version);

#include "MeleeCombatTask.h"
#include "../Memory.h"
#include "../Library.h"

namespace ai
{

// Get the name of this task
const idStr& MeleeCombatTask::GetName() const
{
	static idStr _name(TASK_MELEE_COMBAT);
	return _name;
}

void MeleeCombatTask::Init(idAI* owner, Subsystem& subsystem)
{
	// Init the base class
	Task::Init(owner, subsystem);

	_enemy = owner->GetEnemy();
	_attackAnimStartTime = -1;
}

bool MeleeCombatTask::Perform(Subsystem& subsystem)
{
	DM_LOG(LC_AI, LT_INFO)LOGSTRING("Melee Combat Task performing.\r");

	idAI* owner = _owner.GetEntity();
	assert(owner != NULL);
/*
	idActor* enemy = _enemy.GetEntity();
	if (enemy == NULL)
	{
		DM_LOG(LC_AI, LT_ERROR)LOGSTRING("No enemy, terminating task!\r");
		return true; // terminate me
	}
*/
	// Can we damage the enemy already? (this flag is set by the sensory task)
	if (owner->GetMemory().canHitEnemy)
	{
		idStr waitState(owner->WaitState());

		// greebo: Let the animation wait time be 3 seconds at maximum
		if (waitState != "melee_attack" || gameLocal.time > _attackAnimStartTime + 3000)
		{
			_attackAnimStartTime = gameLocal.time;

			// Waitstate is not matching, this means that the animation 
			// can be started.
			StartAttack(owner);

			// greebo: Set the waitstate, this gets cleared by 
			// the script function when the animation is done.
			owner->SetWaitState("melee_attack");
		}
	}

	return false; // not finished yet
}

void MeleeCombatTask::StartAttack(idAI* owner)
{
	if (gameLocal.random.RandomFloat() < 0.5f)
	{
		// Quick melee
		owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_QuickMelee", 5);
	}
	else
	{
		// Long melee
		owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_LongMelee", 5);
	}
}

void MeleeCombatTask::OnFinish(idAI* owner)
{
	owner->SetAnimState(ANIMCHANNEL_TORSO, "Torso_Idle", 5);
	owner->SetWaitState("");
}

void MeleeCombatTask::Save(idSaveGame* savefile) const
{
	Task::Save(savefile);

	_enemy.Save(savefile);
	savefile->WriteInt(_attackAnimStartTime);
}

void MeleeCombatTask::Restore(idRestoreGame* savefile)
{
	Task::Restore(savefile);

	_enemy.Restore(savefile);
	savefile->ReadInt(_attackAnimStartTime);
}

MeleeCombatTaskPtr MeleeCombatTask::CreateInstance()
{
	return MeleeCombatTaskPtr(new MeleeCombatTask);
}

// Register this task with the TaskLibrary
TaskLibrary::Registrar meleeCombatTaskRegistrar(
	TASK_MELEE_COMBAT, // Task Name
	TaskLibrary::CreateInstanceFunc(&MeleeCombatTask::CreateInstance) // Instance creation callback
);

} // namespace ai
