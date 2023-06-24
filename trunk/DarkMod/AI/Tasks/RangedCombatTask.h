/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3550 $
 * $Date: 2009-07-19 07:03:12 -0400 (Sun, 19 Jul 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_RANGED_COMBAT_TASK_H__
#define __AI_RANGED_COMBAT_TASK_H__

#include "CombatTask.h"

namespace ai
{

// Define the name of this task
#define TASK_RANGED_COMBAT "RangedCombat"

class RangedCombatTask;
typedef boost::shared_ptr<RangedCombatTask> RangedCombatTaskPtr;

class RangedCombatTask :
	public CombatTask
{
public:
	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	// Creates a new Instance of this task
	static RangedCombatTaskPtr CreateInstance();

	virtual void OnFinish(idAI* owner);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);
};

} // namespace ai

#endif /* __AI_RANGED_COMBAT_TASK_H__ */
