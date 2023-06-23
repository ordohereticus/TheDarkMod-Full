/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_OBSERVANT_STATE_H__
#define __AI_OBSERVANT_STATE_H__

#include "State.h"

namespace ai
{

#define STATE_OBSERVANT "Observant"

class ObservantState :
	public State
{

public:
	// Get the name of this state
	virtual const idStr& GetName() const;

	// This is called when the state is first attached to the AI's Mind.
	virtual void Init(idAI* owner);

	// Gets called each time the mind is thinking
	virtual void Think(idAI* owner);

	static StatePtr CreateInstance();

protected:
	// Override base class method
	virtual bool CheckAlertLevel(idAI* owner);
};

} // namespace ai

#endif /* __AI_OBSERVANT_STATE_H__ */
