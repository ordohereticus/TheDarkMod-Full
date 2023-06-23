/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_COMBAT_STATE_H__
#define __AI_COMBAT_STATE_H__

#include "../../../game/ai/ai.h"
#include "State.h"

namespace ai
{

#define STATE_COMBAT "Combat"

class CombatState :
	public State
{
	// The AI's enemy
	idEntityPtr<idActor> _enemy;
	int _criticalHealth;
	bool _meleePossible;
	bool _rangedPossible;

	ECombatType _combatType;


public:
	// Get the name of this state
	virtual const idStr& GetName() const;

	// This is called when the state is first attached to the AI's Mind.
	virtual void Init(idAI* owner);

	// Gets called each time the mind is thinking
	virtual void Think(idAI* owner);

	// Override the alert functions
	virtual void OnTactileAlert(idEntity* tactEnt);
	virtual void OnVisualAlert(idActor* enemy);
	virtual void OnAudioAlert(idActor* enemy);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	static StatePtr CreateInstance();

protected:
	// Override base class method
	virtual bool CheckAlertLevel(idAI* owner);
};

} // namespace ai

#endif /* __AI_COMBAT_STATE_H__ */
