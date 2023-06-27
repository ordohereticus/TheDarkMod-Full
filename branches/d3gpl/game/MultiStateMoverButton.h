/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 5066 $
 * $Date: 2011-12-03 14:49:11 -0500 (Sat, 03 Dec 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef __MULTISTATEMOVER_BUTTON_H_
#define __MULTISTATEMOVER_BUTTON_H_

#include "FrobButton.h"

enum EMMButtonType
{
	BUTTON_TYPE_RIDE = 0,
	BUTTON_TYPE_FETCH,
	NUM_BUTTON_TYPES,
};

/** 
 * greebo: A MultiStateMoverButton is a bit more intelligent
 * than an ordinary FrobButton as it is "communicating" with
 * the targetted elevator a bit at spawn time.
 */
class CMultiStateMoverButton : 
	public CFrobButton 
{
public:

	CLASS_PROTOTYPE( CMultiStateMoverButton );

	void					Spawn();

private:
	void					Event_RegisterSelfWithElevator();
};

#endif /* __MULTISTATEMOVER_BUTTON_H_ */
