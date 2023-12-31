/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2349 $
 * $Date: 2008-05-16 11:24:43 -0400 (Fri, 16 May 2008) $
 * $Author: angua $
 *
 ***************************************************************************/

#ifndef __AI_SWITCH_ON_LIGHT_H__
#define __AI_SWITCH_ON_LIGHT_H__

#include "State.h"

namespace ai
{

#define STATE_SWITCH_ON_LIGHT "SwitchOnLight"

class SwitchOnLightState :
	public State
{
private:
	// Default constructor
	SwitchOnLightState();

	idEntityPtr<idLight> _light;

	// time to wait after starting anim before the light is switched on
	int _waitEndTime;

	// is set to true when the AI reached the position and has started the anim
	bool _switchingOn;

	bool _lightOn;

public:
	// Constructor using light source as input parameter
	SwitchOnLightState(idLight* light);

	// Get the name of this state
	virtual const idStr& GetName() const;

	// This is called when the state is first attached to the AI's Mind.
	virtual void Init(idAI* owner);

	// Gets called each time the mind is thinking
	virtual void Think(idAI* owner);

	// Start switching on (stop move, start anim)
	void StartSwitchOn(idAI* owner, idLight* light);

	// Save/Restore methods
	virtual void Save(idSaveGame* savefile) const;
	virtual void Restore(idRestoreGame* savefile);

	static StatePtr CreateInstance();
};

} // namespace ai

#endif /* __AI_SWITCH_ON_LIGHT_H__ */
