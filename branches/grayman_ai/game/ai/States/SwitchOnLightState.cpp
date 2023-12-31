/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5185 $ (Revision of last commit) 
 $Date: 2012-01-08 00:59:48 -0500 (Sun, 08 Jan 2012) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#include "precompiled_game.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id: SwitchOnLightState.cpp 5185 2012-01-08 05:59:48Z greebo $");

#include "SwitchOnLightState.h"
#include "../Memory.h"
#include "../Tasks/MoveToPositionTask.h"
#include "../Tasks/SingleBarkTask.h"
#include "../../StimResponse/StimResponse.h"
#include "../Tasks/RandomHeadturnTask.h"
#include "../Tasks/RandomTurningTask.h"

namespace ai
{

// grayman #2603 - heights for determining whether a light or switch is high/med/low off the floor

#define RELIGHT_HEIGHT_HIGH 65
#define RELIGHT_HEIGHT_LOW  30
#define RELIGHT_MAX_HEIGHT 100 // grayman #2603 - AI can't reach a light or switch higher than this

SwitchOnLightState::SwitchOnLightState()
{}

SwitchOnLightState::SwitchOnLightState(idLight* light)
{
	_light = light;
}

// Get the name of this state
const idStr& SwitchOnLightState::GetName() const
{
	static idStr _name(STATE_SWITCH_ON_LIGHT);
	return _name;
}

// grayman #2603 - Wrap up and end state

void SwitchOnLightState::Wrapup(idAI* owner, idLight* light, bool ignore)
{
	if (ignore)
	{
		light->IgnoreResponse(ST_VISUAL,owner); // ignore until the light changes state again
	}
	else
	{
		light->AllowResponse(ST_VISUAL,owner); // respond to the next stim
	}

	light->SetBeingRelit(false);
	owner->m_RelightingLight = false;
	owner->GetMind()->EndState();
}

// grayman #2603 - determine max forward reach for relighting method

float SwitchOnLightState::GetMaxReach(idAI* owner, idEntity* torch, idStr lightType)
{
	float forward;

	float reach = owner->GetArmReachLength();
	if (lightType == AIUSE_LIGHTTYPE_ELECTRIC)
	{
		forward = reach;
	}
	else if (torch)
	{
		forward = reach + 10;
	}
	else // tinderbox
	{
		forward = reach/2;
	}
	return forward;
}

// grayman #2603 - a copy of HandleElevatorTask::MoveToButton() modified for our purposes

bool SwitchOnLightState::GetSwitchGoal(idAI* owner, CBinaryFrobMover* mySwitch, idVec3 &goal)
{
	idBounds bounds = owner->GetPhysics()->GetBounds();
	float size = idMath::Fabs(bounds[0][1]);
	_standOff = 2*size; // offset larger than the owner's size

	// This switch could be a button that translates horizontally or vertically,
	// or a lever that rotates.

	// Start with the assumption that it's a horizontally-translating button.
	// That should cover the majority of cases.

	idVec3 trans = mySwitch->spawnArgs.GetVector("translate", "0 0 0");
	if (trans.z != 0) // vertical movement?
	{
		idVec3 switchSize = mySwitch->GetPhysics()->GetBounds().GetSize();
		if (switchSize.y > switchSize.x)
		{
			trans = idVec3(1,0,0);
		}
		else
		{
			trans = idVec3(0,1,0);
		}
	}
	else if (trans.LengthFast() == 0)
	{
		// rotating switch

		idVec3 switchSize = mySwitch->GetPhysics()->GetBounds().GetSize();
		if (switchSize.x > switchSize.y)
		{
			trans = idVec3(1,0,0);
		}
		else
		{
			trans = idVec3(0,1,0);
		}
	}
	trans.NormalizeFast();

	const idVec3& switchOrigin = mySwitch->GetPhysics()->GetOrigin();

	goal = switchOrigin - _standOff * trans;

	// Is there a path to the target?

	idVec3 ownerOrigin = owner->GetPhysics()->GetOrigin();
	int areaNum = owner->PointReachableAreaNum(ownerOrigin, 1.0f);
	int goalAreaNum = owner->PointReachableAreaNum(goal, 1.0f);
	aasPath_t path;

	if (!owner->PathToGoal(path, areaNum, ownerOrigin, goalAreaNum, goal, owner)) // first attempt
	{
		// not reachable, try alternate target positions at the sides and behind the switch

		trans *= -1;
		goal = switchOrigin - _standOff * trans;
		goalAreaNum = owner->PointReachableAreaNum(goal, 1.0f);

		if (!owner->PathToGoal(path, areaNum, ownerOrigin, goalAreaNum, goal, owner)) // second attemp
		{
			const idVec3& gravity = owner->GetPhysics()->GetGravityNormal();
			trans = trans.Cross(gravity);
			goal = switchOrigin - _standOff * trans;
			goalAreaNum = owner->PointReachableAreaNum(goal, 1.0f);

			if (!owner->PathToGoal(path, areaNum, ownerOrigin, goalAreaNum, goal, owner)) // third attempt
			{
				trans *= -1;
				goal = switchOrigin - _standOff * trans;
				goalAreaNum = owner->PointReachableAreaNum(goal, 1.0f);

				if (!owner->PathToGoal(path, areaNum, ownerOrigin, goalAreaNum, goal, owner)) // fourth attempt
				{
					return false;
				}
			}
		}
	}

	// Find the floor at this point

	idVec3 bottomPoint = goal;
	bottomPoint.z -= 256;
	trace_t result;
	gameLocal.clip.TracePoint(result, goal, bottomPoint, MASK_OPAQUE, NULL); // trace down
	float height = switchOrigin.z - result.endpos.z;

	// adjust _standOff based on height of switch off the floor

	if (height < RELIGHT_HEIGHT_LOW) // low
	{
		// Adjust standoff and set new goal. Assume path to goal is still good.

		_standOff += 16; // farther from goal
		goal = switchOrigin - _standOff * trans;
	}

	goal.z = result.endpos.z; // where we hit
	goal.z++; // move up slightly
	return true;
}

// grayman #2603 - check for relight positions

bool SwitchOnLightState::CheckRelightPosition(idLight* light, idAI* owner, idVec3& pos)
{
	// Does this light have a moveable holder? If so, we can't rely on relight_positions,
	// since the light might have been moved.

	idEntity* bindMaster = light->GetBindMaster();
	while (bindMaster != NULL)
	{
		if (bindMaster->IsType(idMoveable::Type)) // is any of the bindMasters an idMoveable? (looking for candles)
		{
			return false; // can't rely on any relight_positions
		}
		bindMaster = bindMaster->GetBindMaster(); // go up the hierarchy
	}

	// Look through the target list for relight_position entities.

	idEntity* check4Position = light;
	bool posFound = false;
	idList<idVec3> positions;
	positions.Clear();
	while ((check4Position != NULL) && !posFound)
	{
		for (int i = 0 ; i < check4Position->targets.Num() ; i++)
		{
			idEntity* ent = check4Position->targets[i].GetEntity();

			if (ent == NULL)
			{
				continue;
			}

			const char *classname;
			ent->spawnArgs.GetString("classname", NULL, &classname);
			if (idStr::Cmp(classname, "atdm:relight_position") == 0)
			{
				pos = ent->GetPhysics()->GetOrigin();
				positions.Append(pos);
				posFound = true;
			}
		}
		check4Position = check4Position->GetBindMaster(); // go up the hierarchy
	}

	if (posFound)
	{
		// Pick the relight position that's closest to the AI.

		// grayman - This is a crazy way of sorting, but I couldn't
		// get the compiler to like a sort on a list of idVec3 distances
		// from the AI to the positions. Feel free to make this better.

		int num = positions.Num();
		idVec3 org = owner->GetPhysics()->GetOrigin();

		idList<int> distances1,distances2;
		distances1.SetNum(num);
		distances2.SetNum(num);
		for (int i = 0 ; i < num ; i++)
		{
			distances1[i] = distances2[i] = (int)(positions[i] - org).LengthSqr(); // integer squared distance from AI to relight position
		}
		distances1.Sort();
		int areaNum = owner->PointReachableAreaNum(org, 1.0f);
		for (int i = 0 ; i < num ; i++) // sorted index
		{
			for (int j = 0 ; j < num ; j++) // unsorted index
			{
				if (distances2[j] == distances1[i]) // is element j of the unsorted list equal to element i of the sorted list?
				{
					pos = positions[j]; // if so, use the unsorted index to retrieve the matching relight position
					int targetAreaNum = owner->PointReachableAreaNum(pos, 1.0f);
					aasPath_t path;
					if (owner->PathToGoal(path, areaNum, org, targetAreaNum, pos, owner))
					{
						return true;
					}
					break; // try the next sorted element
				}
			}
		}
		posFound = false; // Couldn't find a path to any of the relight positions
	}

	return posFound;
}

void SwitchOnLightState::Init(idAI* owner)
{
	// grayman #2603 - a number of changes were made in this method

	// Shortcut reference
	Memory& memory = owner->GetMemory();

	// don't initiate a relight if something more important has happened
	if (memory.stopRelight)
	{
		return;
	}

	State::Init(owner);

	DM_LOG(LC_AI, LT_INFO)LOGSTRING("SwitchOnLightState initialised.\r");
	assert(owner);

	idLight* light = _light.GetEntity();
	assert(light);

	// Make sure light is still off

	bool lightOn = ((light->GetLightLevel() > 0) && !light->IsSmoking());
	bool ignoreLight;

	if (lightOn)
	{
		ignoreLight = true;
		Wrapup(owner,light,ignoreLight);
		return;
	}

	_waitEndTime = 0;
	bool pathToGoal = true; // whether there's a path to the switch or light
	bool reachable = true;  // whether a light is too high to reach

	idStr lightType = light->spawnArgs.GetString(AIUSE_LIGHTTYPE_KEY);
	idVec3 goalOrigin;
	idVec3 goalDirection;
	idVec3 finalTargetPoint;

	// Determine if this light is controlled by a switch. After all entities were spawned
	// at map start, each AIUSE_LIGHTTYPE_ELECTRIC light stored all the switches targetting it.
	// Checking for switches on torches is harmless, since they won't have any.
	//
	// GetSwitch() returns the switch closest to the AI. You can have multiple switches for a light,
	// i.e. one at each end of a hall, and the AI will go to the one nearest him at the moment he
	// decides to relight the light.
	//
	// TODO: GetSwitch() doesn't consider reachability. If the closest switch is unreachable at the
	// moment, GetSwitch() should fall back to the next-closest switch, seeking one that's reachable.
	
	idEntity* mySwitch = light->GetSwitch(owner); // Are there switches? If so, use the closest one.
	if (mySwitch && mySwitch->IsType(CBinaryFrobMover::Type))
	{
		// There's a switch, so the AI should walk to the switch and frob it. That should
		// be all that's needed to turn the light back on. The AI should do this even if
		// he doesn't have a direct LOS to the switch.

		_goalEnt = mySwitch;
		goalOrigin = _goalEnt->GetPhysics()->GetOrigin();

		// Find a spot to stand while activating it.

		CBinaryFrobMover* switchMover = static_cast<CBinaryFrobMover*>(mySwitch);
		pathToGoal = GetSwitchGoal(owner,switchMover,finalTargetPoint);
	}
	else if (CheckRelightPosition(light,owner,finalTargetPoint)) // No switch. Does the light have a relight_position?
	{
		// Relight position found and there's a path to get to it.

		_goalEnt = light;
		goalDirection = finalTargetPoint - _goalEnt->GetPhysics()->GetOrigin();
		goalDirection.z = 0;
		_standOff = goalDirection.LengthFast();
	}
	else
	{
		_goalEnt = light;
		goalOrigin = _goalEnt->GetPhysics()->GetOrigin();
		goalDirection = owner->GetPhysics()->GetOrigin() - goalOrigin;
	
		idVec3 size(16, 16, 82);
		idAAS* aas = owner->GetAAS();
		if (aas)
		{
			size = aas->GetSettings()->boundingBoxes[0][1];
		}

		idVec2 projection = goalDirection.ToVec2();

		// Am I carrying a torch? Useful when setting the standoff distance

		idEntity* torch = owner->GetTorch();

		// Move a bit from the goal origin towards the AI 
		// and perform a trace down to detect the ground.

		goalDirection.NormalizeFast();

		// If the goal entity has a bounding box (i.e. a model with
		// a built-in light, or a switch), use an average of its x/y sizes to add to
		// how far away the AI should stand.

		idEntity* bindMaster = _goalEnt->GetBindMaster();
		_standOff = 0;
		bool isMoveable = false;
		while (bindMaster != NULL)
		{
			isMoveable |= bindMaster->IsType(idMoveable::Type); // is any of the bindMasters an idMoveable? (looking for candles) 
			idVec3 goalSize = bindMaster->GetPhysics()->GetBounds().GetSize();
			float goalDist = (goalSize.x + goalSize.y)/4;
			if (goalDist > _standOff)
			{
				_standOff = goalDist;
			}
			bindMaster = bindMaster->GetBindMaster(); // go up the hierarchy
		}

		_standOff += 2*size.x;
		float armReach = GetMaxReach(owner,torch,lightType); // grayman #2603
		float standOffTemp = _standOff; // use this to establish reachability
		if (standOffTemp < armReach) // can't try to get close to candles on tables
		{
			standOffTemp = armReach;
		}

		// Use standOffTemp to find the floor near the goal. In case this is a candle sitting
		// on a table, you have to move out a reasonable distance to clear the table.

		goalDirection.z = 0; // grayman #2603 - ignore vertical component
		idVec3 startPoint = goalOrigin + goalDirection * standOffTemp;
		idVec3 bottomPoint = startPoint;
		bottomPoint.z -= RELIGHT_MAX_HEIGHT;

		idVec3 targetPoint = startPoint;
		trace_t result;
		if (gameLocal.clip.TracePoint(result, startPoint, bottomPoint, MASK_OPAQUE, NULL))
		{
			// Found the floor.

			targetPoint.z = result.endpos.z + 1; // move the target point to the floor

			//gameRenderWorld->DebugArrow(colorRed, startPoint, targetPoint, 2, 5000);

			// Is there a path to the target point?

			int areaNum = owner->PointReachableAreaNum(owner->GetPhysics()->GetOrigin(), 1.0f);
			int targetAreaNum = owner->PointReachableAreaNum(targetPoint, 1.0f);
			aasPath_t path;
			if (owner->PathToGoal(path, areaNum, owner->GetPhysics()->GetOrigin(), targetAreaNum, targetPoint, owner))
			{
				// A path has been found. Now we adjust where we're going to stand based on
				// the relight method.

				// Abandon standOffTemp and use the original _standOff.

				float ht = goalOrigin.z - targetPoint.z; // height of goal off the floor
				if (lightType == AIUSE_LIGHTTYPE_TORCH)
				{
					_standOff -= 16; // start with this (move closer)
					if (ht > RELIGHT_HEIGHT_HIGH) // high
					{
						if (torch)
						{
							_standOff += 8; // move away
						}
					}
					else if (ht < RELIGHT_HEIGHT_LOW) // low
					{
						if (isMoveable) // try to stay away from low moveables, which tend to get kicked
						{
							_standOff += 16; // move away
						}
						if (!torch)  // tinderbox
						{
							_standOff += 16; // move away
						}
					}
					else // medium
					{
						if (torch)
						{
							_standOff += 8; // move away
						}
					}
				}
				else // electric light, no switch
				{
					if (ht < RELIGHT_HEIGHT_LOW) // low
					{
						_standOff += 8; // move away
					}
				}
				finalTargetPoint = goalOrigin + goalDirection * _standOff; // use adjusted standoff
				finalTargetPoint.z = targetPoint.z;
			}
			else
			{
				pathToGoal = false;
			}
		}
		else
		{
			reachable = false;
		}
	}

	if (pathToGoal && reachable)
	{
		owner->actionSubsystem->ClearTasks();
		owner->movementSubsystem->ClearTasks();
		_relightSpot = finalTargetPoint;

		// Don't allow barks if the Alert Level is 1 or higher.

		if (owner->AI_AlertLevel < owner->thresh_1)
		{
			memory.nextTimeLightStimBark = gameLocal.time + REBARK_DELAY;
			memory.lastTimeVisualStimBark = gameLocal.time;
			idStr bark;
			if (gameLocal.random.RandomFloat() < 0.5)
			{
				bark = "snd_yesRelightTorch";
			}
			else
			{
				bark = (lightType == AIUSE_LIGHTTYPE_TORCH) ? "snd_foundTorchOut" : "snd_foundLightsOff";
			}
			CommMessagePtr message; // no message, but the argument is needed so the start delay can be included
			owner->GetSubsystem(SubsysCommunication)->PushTask(TaskPtr(new SingleBarkTask(bark,message,2000)));
		}

		light->IgnoreResponse(ST_VISUAL, owner);

		// grayman #2603 - if AI is sitting, he has to stand before sending him on his way

		owner->movementSubsystem->PushTask(TaskPtr(new MoveToPositionTask(_relightSpot,idMath::INFINITY,5)));
		if (owner->GetMoveType() == MOVETYPE_SIT)
		{
			_relightState = EStateSitting;
		}
		else
		{
			_relightState = EStateStarting;
		}

		_waitEndTime = gameLocal.time + 1000; // allow time for move to begin
		return;
	}

	// The goal is unreachable, or there's no path to get to it.

	// Re: No path to goal. Success depends on the angle of approach. I.e. a candle sitting on a table
	// might only allow the AI to stand in certain places to light it. Try again later, when perhaps
	// the angle of approach provides a more favorable outcome.

	// Re: Unreachable. Probably can't reach the light; too far above ground. This could also be from the trace
	// hitting a table a candle is sitting on. Try again later. Also, a wall torch halfway up stairs
	// might allow for relighting when the AI is walking down the stairs, but not when he's walking up.

	ignoreLight = false;

	// As a light receives negative barks ("light out" and "won't relight light"), the odds of emitting
	// this type of bark go down. When the light is relit, the odds are reset to 100%. This should reduce
	// the number of such barks, which can get tiresome.

	if (light->NegativeBark(owner))
	{
		idStr bark;
		if (gameLocal.random.RandomFloat() < 0.5)
		{
			bark = "snd_noRelightTorch";
		}
		else
		{
			bark = (lightType == AIUSE_LIGHTTYPE_TORCH) ? "snd_foundTorchOut" : "snd_foundLightsOff";
		}
		CommMessagePtr message; // no message, but the argument is needed so the start delay can be included
		owner->GetSubsystem(SubsysCommunication)->PushTask(TaskPtr(new SingleBarkTask(bark,message,2000)));
	}
	
	Wrapup(owner,light,ignoreLight);
}

// Gets called each time the mind is thinking
void SwitchOnLightState::Think(idAI* owner)
{
	// grayman #2603 - a number of changes were made in this method

	// It's possible that during the animation of touching a torch to an unlit flame, that
	// the torch's fire stim will light the unlit flame. You have to account for that.

	Memory& memory = owner->GetMemory();

	idLight* light = _light.GetEntity();
	if (light == NULL)
	{
		owner->m_RelightingLight = false;
		owner->GetMind()->EndState();
		return;
	}

	bool lightOn = ((light->GetLightLevel() > 0) && !light->IsSmoking());
	bool ignoreLight;

	// check if something happened to abort the relight (i.e. dropped torch, higher alert)
	if (owner->GetMemory().stopRelight)
	{
		ignoreLight = lightOn;
		Wrapup(owner,light,ignoreLight);
		return;
	}

	owner->PerformVisualScan();	// Let the AI check its senses
	if (owner->AI_AlertLevel >= owner->thresh_5) // finished if alert level is too high
	{
		ignoreLight = false;
		Wrapup(owner,light,ignoreLight);
		return;
	}

	if (lightOn) // If the light comes on before you relight it, act appropriately
	{
		switch (_relightState)
		{
		case EStateSitting:
		case EStateStarting:
		case EStateApproaching:
		case EStateTurningToward:
			ignoreLight = true;
			Wrapup(owner,light,ignoreLight);
			return;
		case EStateRelight:
		case EStatePause:
		case EStateFinal:
		default:
			break;
		}
	}

	if ((owner->m_HandlingDoor) || (owner->m_HandlingElevator))
	{
		return; // we're handling a door or elevator, so delay processing the rest of the relight
	}

	switch (_relightState)
	{
		case EStateSitting:
			if (gameLocal.time >= _waitEndTime)
			{
				if (owner->AI_MOVE_DONE && (owner->GetMoveType() != MOVETYPE_GET_UP)) // standing yet?
				{
					owner->movementSubsystem->PushTask(TaskPtr(new MoveToPositionTask(_relightSpot,idMath::INFINITY,5)));
					_relightState = EStateStarting;
					_waitEndTime = gameLocal.time + 1000; // allow time for move to begin
				}
			}
			break;
		case EStateStarting:
			if (owner->AI_FORWARD || (gameLocal.time >= _waitEndTime))
			{
				_relightState = EStateApproaching;
			}
			break;
		case EStateApproaching:
			{
				// Still walking toward the goal (switch or flame)

				idVec3 size(16, 16, 82);
				if (idAAS* aas = owner->GetAAS())
				{
					size = aas->GetSettings()->boundingBoxes[0][1];
				}

				idVec3 goalOrigin = _goalEnt->GetPhysics()->GetOrigin();

				idVec3 goalDirection = owner->GetPhysics()->GetOrigin() - goalOrigin;
				goalDirection.z = 0;
				float delta = goalDirection.LengthFast();

				if (delta <= _standOff)
				{
					owner->StopMove(MOVE_STATUS_DONE);
				}

				if (owner->AI_MOVE_DONE)
				{
					float lightHeight = goalOrigin.z - owner->GetPhysics()->GetOrigin().z;
					if ((delta <= 2 * owner->GetArmReachLength()) && (lightHeight <= RELIGHT_MAX_HEIGHT))
					{
						owner->TurnToward(goalOrigin);
						_relightState = EStateTurningToward;
						_waitEndTime = gameLocal.time + 750; // allow time for turn to complete
					}
					else // too far away
					{
						// Bark, but not too often

						if (light->NegativeBark(owner))
						{
							memory.nextTimeLightStimBark = gameLocal.time + REBARK_DELAY;
							owner->GetSubsystem(SubsysCommunication)->PushTask(TaskPtr(new SingleBarkTask("snd_noRelightTorch")));
						}

						// TODO: Try moving closer?
						ignoreLight = false;
						Wrapup(owner,light,ignoreLight);
						return;
					}
				}
				break;
			}
		case EStateTurningToward:
			if (gameLocal.time >= _waitEndTime)
			{
				// A candle or torch could still be smoking at this point from having
				// just been put out. When the smoke clears, it's okay to perform our relight.

				if (light->IsSmoking())
				{
					break; // stay in this state until the smoke stops
				}
				StartSwitchOn(owner,light); // starts the relight animation
				owner->m_performRelight = false; // animation sets this to TRUE at the relight frame
				_relightState = EStateRelight;
				_waitEndTime = gameLocal.time + 10000; // failsafe in case something aborts the animation
			}
			break;
		case EStateRelight:
			if (owner->m_performRelight)
			{
				// Time to relight the light.
				// If you're dealing with a torch or candle, frob the light directly. If it's an electric
				// light, either _goalEnt is a switch, or it's the light itself. If a switch, activate it.
				// If it's not a switch, frob any bindMaster. If there's no bindMaster, frob the light itself.
				// Frobbing an existing bindMaster makes sure that all activities related to frobbing this
				// light are dealt with.

				idStr lightType = light->spawnArgs.GetString(AIUSE_LIGHTTYPE_KEY);
				if (lightType == AIUSE_LIGHTTYPE_TORCH)
				{
					light->CallScriptFunctionArgs("frob_ignite", true, 0, "e", light);
				}
				else // handle electric lights
				{
					if (_goalEnt == light)
					{
						idEntity* bindMaster = light->GetBindMaster();
						if (bindMaster) // light holder
						{
							bindMaster->CallScriptFunctionArgs("LightsOn", true, 0, "e", bindMaster);
						}
						else // light
						{
							light->CallScriptFunctionArgs("LightsOn", true, 0, "e", light);
						}
					}
					else // switch
					{
						_goalEnt->Activate(owner);
					}
				}
				_relightState = EStatePause;
				owner->m_performRelight = false;
				_waitEndTime = gameLocal.time + 10000; // failsafe in case something aborts the animation
			}
			else if (gameLocal.time >= _waitEndTime) // animation problem - abort
			{
				owner->m_performRelight = false;
				_relightState = EStatePause;
			}
			break;
		case EStatePause:
			if ((owner->AnimDone(ANIMCHANNEL_TORSO,4)) || (gameLocal.time >= _waitEndTime))
			{
				_waitEndTime = gameLocal.time + 1000; // pause before walking away
				_relightState = EStateFinal;
			}
			break;
		case EStateFinal:
			if (gameLocal.time >= _waitEndTime)
			{
				// Set up search if latched

				if (owner->m_LatchedSearch)
				{
					owner->m_LatchedSearch = false;
					if (owner->AI_AlertLevel < owner->thresh_4)
					{
						memory.alertPos = light->GetPhysics()->GetOrigin();
						memory.alertClass = EAlertVisual_2;
						memory.alertType = EAlertTypeLightSource;
						
						// Prepare search as if there is an enemy that has escaped
						memory.alertRadius = LOST_ENEMY_ALERT_RADIUS;
						memory.alertSearchVolume = LOST_ENEMY_SEARCH_VOLUME; 
						memory.alertSearchExclusionVolume.Zero();
							
						owner->AI_VISALERT = false;

						// Do new reaction to stimulus after relighting
						memory.stimulusLocationItselfShouldBeSearched = true;
						memory.alertedDueToCommunication = false;
					}
				}

				ignoreLight = false;
				light->SetChanceNegativeBark(1.0); // reset
				Wrapup(owner,light,ignoreLight);
				return;
			}
			break;
		default:
			break;
	}
}

void SwitchOnLightState::StartSwitchOn(idAI* owner, idLight* light)
{
	// grayman #2603 - a number of changes were made in this method

	owner->movementSubsystem->ClearTasks();
	owner->StopMove(MOVE_STATUS_DONE);
	idStr torsoAnimation = "";
	float lightHeight = _goalEnt->GetPhysics()->GetOrigin().z - owner->GetPhysics()->GetOrigin().z;
	idStr lightType = light->spawnArgs.GetString(AIUSE_LIGHTTYPE_KEY);

	if (lightType == AIUSE_LIGHTTYPE_TORCH)
	{
		// If I have a torch, I'll use that. If not, I'll use the tinderbox. Since
		// that animation looks iffy when I'm carrying a melee weapon, I'll put it
		// away first. Dealing with the weapon is done in the animation that
		// uses the tinderbox animation. That's why we can't use the torch's
		// replacement animation ability for these animations.

		if (owner->GetTorch() != NULL)
		{
			torsoAnimation = "Torso_Relight_Torch";
		}
		else // use tinderbox (sheathes any drawn weapon, relights, then redraws weapon)
		{
			torsoAnimation = "Torso_Relight_Tinderbox";
		}
	}
	else // electric
	{
		torsoAnimation = "Torso_Relight_Electric"; // reach up toward the switch or light
	}
	
	if (lightHeight > RELIGHT_HEIGHT_HIGH) // high?
	{
		torsoAnimation.Append("_High"); // reach up toward the switch or light
	}
	else if (lightHeight < RELIGHT_HEIGHT_LOW) // low?
	{
		torsoAnimation.Append("_Low"); // reach down toward the switch or light
	}
	else // medium
	{
		torsoAnimation.Append("_Med"); // reach out toward the switch or light
	}

	owner->SetAnimState(ANIMCHANNEL_TORSO, torsoAnimation.c_str(), 4); // this plays the legs anim also
}

void SwitchOnLightState::Save(idSaveGame* savefile) const
{
	State::Save(savefile);
	_light.Save(savefile);

	savefile->WriteInt(_waitEndTime);
	savefile->WriteObject(_goalEnt);	// grayman #2603
	savefile->WriteFloat(_standOff);	// grayman #2603
	savefile->WriteInt(static_cast<int>(_relightState)); // grayman #2603
	savefile->WriteVec3(_relightSpot);	// grayman #2603
}

void SwitchOnLightState::Restore(idRestoreGame* savefile)
{
	State::Restore(savefile);
	_light.Restore(savefile);

	savefile->ReadInt(_waitEndTime);
	savefile->ReadObject(reinterpret_cast<idClass*&>(_goalEnt)); // grayman #2603
	savefile->ReadFloat(_standOff);		// grayman #2603
	int temp;
	savefile->ReadInt(temp);
	_relightState = static_cast<ERelightState>(temp); // grayman #2603
	savefile->ReadVec3(_relightSpot);	// grayman #2603
}

StatePtr SwitchOnLightState::CreateInstance()
{
	return StatePtr(new SwitchOnLightState);
}

// Register this state with the StateLibrary
StateLibrary::Registrar switchOnLightStateRegistrar(
	STATE_SWITCH_ON_LIGHT, // Task Name
	StateLibrary::CreateInstanceFunc(&SwitchOnLightState::CreateInstance) // Instance creation callback
);

} // namespace ai
