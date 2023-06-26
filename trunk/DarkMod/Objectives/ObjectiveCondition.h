/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4804 $
 * $Date: 2011-04-19 14:07:26 -0400 (Tue, 19 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#ifndef TDM_OBJECTIVE_CONDITION_H
#define TDM_OBJECTIVE_CONDITION_H

#include "../idlib/precompiled.h"

class CMissionData;

class ObjectiveCondition
{
private:
	// Possible effect types 
	enum Type
	{
		CHANGE_STATE,		// changes state of target objective
		CHANGE_VISIBILITY,	// changes visibility of target objective
		CHANGE_MANDATORY,	// changes mandatory flag of target objetive
		INVALID_TYPE,		// not a valid type
	};

	Type _type;

	int _value;
	int _srcMission;
	int _srcState;
	int _srcObj;	
	int _targetObj;

public:
	// Default constructor
	ObjectiveCondition();

	// Construct from a given dictionary
	ObjectiveCondition(const idDict& dict, int index);

	// Returns TRUE if the condition has enough valid parameters to be functional
	bool IsValid() const;

	// Applies this conditional action to the given objectives
	// Returns TRUE if the condition was applicable, FALSE otherwise
	bool Apply(CMissionData& missionData);

private:
	void ParseFromSpawnargs(const idDict& dict, int index);
};

#endif 
