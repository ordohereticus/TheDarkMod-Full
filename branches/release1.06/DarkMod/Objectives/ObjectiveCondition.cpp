/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4822 $
 * $Date: 2011-04-28 12:58:36 -0400 (Thu, 28 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "MissionData.h"
#include "ObjectiveCondition.h"
#include "CampaignStatistics.h"

static bool init_version = FileVersionList("$Id: ObjectiveCondition.cpp 4822 2011-04-28 16:58:36Z greebo $", init_version);

ObjectiveCondition::ObjectiveCondition() :
	_type(INVALID_TYPE),
	_value(-1),
	_srcMission(-1),
	_srcState(STATE_INVALID),
	_srcObj(-1),
	_targetObj(-1)
{}

ObjectiveCondition::ObjectiveCondition(const idDict& dict, int index)
{
	ParseFromSpawnargs(dict, index);
}

bool ObjectiveCondition::IsValid() const
{
	return _type != INVALID_TYPE && _value > -1 && _srcMission > -1 && _srcObj > -1 && _targetObj > -1;
}

bool ObjectiveCondition::Apply(CMissionData& missionData)
{
	assert(IsValid()); // enforce validity in debug builds

	// Get the state of the source objective in the source mission to see whether we can apply this
	
	if (_srcMission < gameLocal.m_CampaignStats->Num())
	{
		const MissionStatistics& stats = (*gameLocal.m_CampaignStats)[_srcMission];

		EObjCompletionState state = stats.GetObjectiveState(_srcObj);

		if (state != _srcState)
		{
			DM_LOG(LC_OBJECTIVES, LT_DEBUG)LOGSTRING("Objective state in mission %d is not matching the required one %d, cannot apply.\r", state, _srcObj);
			return false; // not applicable
		}
	}
	else
	{
		DM_LOG(LC_OBJECTIVES, LT_ERROR)LOGSTRING("No mission data available for the given source mission %d.\r", _srcMission);
		return false; // can't apply
	}

	// Source objective is matching, try to find target objective
	if (_targetObj >= missionData.GetNumObjectives())
	{
		DM_LOG(LC_OBJECTIVES, LT_ERROR)LOGSTRING("Target objective not found in this mission: %d.\r", _targetObj);
		return false; // can't apply
	}

	switch (_type)
	{
	case CHANGE_STATE:
		// Attempt to set the completion state
		DM_LOG(LC_OBJECTIVES, LT_DEBUG)LOGSTRING("Objective condition will set the state of objective %d to %d.\r", _targetObj, _value);

		// Set completion state and suppress event firing, we might not have a player around to receive GUI events
		missionData.SetCompletionState(_targetObj, _value, false);
		break;

	case CHANGE_VISIBILITY:
		DM_LOG(LC_OBJECTIVES, LT_DEBUG)LOGSTRING("Objective condition will set the visiblity of objective %d to %d\r", _targetObj, _value);
		missionData.SetObjectiveVisibility(_targetObj, _value != 0, false); // don't fire events
		break;

	case CHANGE_MANDATORY:
		DM_LOG(LC_OBJECTIVES, LT_DEBUG)LOGSTRING("Objective condition will set the mandatory flag of objective %d to %d\r", _targetObj, _value);
		missionData.SetObjectiveMandatory(_targetObj, _value != 0);
		break;

	case INVALID_TYPE:
		DM_LOG(LC_OBJECTIVES, LT_ERROR)LOGSTRING("Attempting to apply invalid objective condition.\r");
		break;
	};

	return true;
}

void ObjectiveCondition::ParseFromSpawnargs(const idDict& dict, int index)
{
	/** 
		Schema:

		obj_condition_1_src_mission		<missionNumber> (0-based)
		obj_condition_1_src_obj			<objectiveNumber> (0-based)
		obj_condition_1_src_state		<objectiveState> 0|1|2
		obj_condition_1_target_obj		<objectiveNumber> (0-based)
		obj_condition_1_type			changestate|changevisibility|changemandatory
		obj_condition_1_value			changestate: 0|1|2
										changevisibility: 0|1
										changemandatory: 0|1
	*/

	idStr prefix = va("obj_condition_%d_", index);

	const char* type = dict.GetString(prefix + "type");

	if (idStr::Cmp(type, "changestate") == 0)
	{
		_type = CHANGE_STATE;
	}
	else if (idStr::Cmp(type, "changevisibility") == 0)
	{
		_type = CHANGE_VISIBILITY;
	}
	else if (idStr::Cmp(type, "changemandatory") == 0)
	{
		_type = CHANGE_MANDATORY;
	}
	else
	{
		_type = INVALID_TYPE;
	}

	// Parse the rest of the integer-based members
	_value =		dict.GetInt(prefix + "value", "-1");
	_srcMission =	dict.GetInt(prefix + "src_mission", "-1");

	int srcState =	dict.GetInt(prefix + "src_state", "-1");

	if (srcState >= STATE_INCOMPLETE && srcState <= STATE_FAILED)
	{
		_srcState = static_cast<EObjCompletionState>(srcState);
	}
	else
	{
		_srcState = STATE_INVALID;
	}

	_srcObj =		dict.GetInt(prefix + "src_obj", "-1");
	_targetObj =	dict.GetInt(prefix + "target_obj", "-1");
}
