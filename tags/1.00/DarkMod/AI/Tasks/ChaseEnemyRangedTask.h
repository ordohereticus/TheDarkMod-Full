/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_CHASE_ENEMY_RANGED_TASK_H__
#define __AI_CHASE_ENEMY_RANGED_TASK_H__

#include "Task.h"

namespace ai
{

// Define the name of this task
#define TASK_CHASE_ENEMY_RANGED "ChaseEnemyRanged"

class ChaseEnemyRangedTask;
typedef boost::shared_ptr<ChaseEnemyRangedTask> ChaseEnemyRangedTaskPtr;

class ChaseEnemyRangedTask :
	public Task
{
	bool _hasGoal;

	idEntityPtr<idActor> _enemy;

public:
	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual bool Perform(Subsystem& subsystem);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static ChaseEnemyRangedTaskPtr CreateInstance();

	// Class-specific methods
	virtual void SetEnemy(idActor* enemy);
};

} // namespace ai

#endif /* __AI_CHASE_ENEMY_RANGED_TASK_H__ */
