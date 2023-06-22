/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 1435 $
 * $Date: 2007-10-16 18:53:28 +0200 (Di, 16 Okt 2007) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_IDLE_SENSORY_TASK_H__
#define __AI_IDLE_SENSORY_TASK_H__

#include <boost/shared_ptr.hpp>

namespace ai
{

// Define the name of this task
#define TASK_IDLE_SENSORY "Idle_Sensory"

class IdleSensoryTask;
typedef boost::shared_ptr<IdleSensoryTask> IdleSensoryTaskPtr;

class IdleSensoryTask :
	public Task
{
public:
	// Get the name of this task
	virtual const idStr& GetName() const;

	// Override the base Init method
	virtual void Init(idAI* owner, Subsystem& subsystem);

	virtual void Perform(Subsystem& subsystem);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	// Creates a new Instance of this task
	static IdleSensoryTaskPtr CreateInstance();

private:
	/**
	* SophisticatedZombie: This method handles a random chance of turning the AIs head
	* (visual gaze movement)
	*/
	void PerformRandomHeadTurnCheck();
};

} // namespace ai

#endif /* __AI_IDLE_SENSORY_TASK_H__ */
