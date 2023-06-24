/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2338 $
 * $Date: 2008-05-15 12:23:41 -0400 (Thu, 15 May 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __AI_AGITATED_SEARCHING_STATE_H__
#define __AI_AGITATED_SEARCHING_STATE_H__

#include "State.h"
#include "SearchingState.h"

/**
* greebo: AgitatedSearchingState is one alert index above SearchingState.
*
* Apart from a few minor things this is similar to the base class SearchingState.
* 
* See the base class for documentation.
*/

namespace ai
{

#define STATE_AGITATED_SEARCHING "AgitatedSearching"

class AgitatedSearchingState :
	protected SearchingState
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

#endif /* __AI_AGITATED_SEARCHING_STATE_H__ */
