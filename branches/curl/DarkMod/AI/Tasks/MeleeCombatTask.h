/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3550 $
 * $Date: 2009-07-19 07:03:12 -0400 (Sun, 19 Jul 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_MELEE_COMBAT_TASK_H__
#define __AI_MELEE_COMBAT_TASK_H__

#include "CombatTask.h"

namespace ai
{

// Define the name of this task
#define TASK_MELEE_COMBAT "MeleeCombat"

class MeleeCombatTask;
typedef boost::shared_ptr<MeleeCombatTask> MeleeCombatTaskPtr;

class MeleeCombatTask :
	public CombatTask
{
	/** 
	* Set to true if we want to force an attack or parry at the next opportunity
	* I.e., we wait until we can do this action even if we could do the other first
	* These are cleared once the forced action is started
	**/
	bool				_bForceAttack;
	bool				_bForceParry;

	/**
	* Set to true when we have decided to parry but there is a delay before the anim starts
	**/
	bool				_bInPreParryDelayState;
	/**
	* Set to true when we have decided to stop parrying but there is a delay before the anim starts
	**/
	bool				_bInPostParryDelayState;
	/** 
	* Timer to keep track of when parry delay state started 
	* Re-used for both pre and post parry delays 
	**/
	int					_ParryDelayTimer;
	/** Cache the actual delays in ms over repeated frames **/
	int					_PreParryDelay;
	int					_PostParryDelay;

	/** Last enemy that attacked us **/
	idEntityPtr<idActor>	_PrevEnemy;
	/** Previous attack type we tried to parry (if any) **/
	EMeleeType				_PrevAttParried;
	/** Time of that previous attack **/
	int						_PrevAttTime;
	/** 
	* Number of times this attack type has been repeated in a row 
	* Gets cleared if a timer expires before the next repeated attack. 
	**/
	int						_NumAttReps;

public:
	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	// Creates a new Instance of this task
	static MeleeCombatTaskPtr CreateInstance();

	virtual void OnFinish(idAI* owner);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

private:
	void PerformReady(idAI* owner);
	void PerformAttack(idAI* owner);
	void PerformParry(idAI* owner);

	/**
	* Start up an attack or parry animation
	**/
	void StartAttack(idAI* owner);
	void StartParry(idAI* owner);
};

} // namespace ai

#endif /* __AI_MELEE_COMBAT_TASK_H__ */
