/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4737 $
 * $Date: 2011-03-30 01:50:23 -0400 (Wed, 30 Mar 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: Memory.cpp 4737 2011-03-30 05:50:23Z greebo $", init_version);

#include "Memory.h"
#include "../../game/ai/ai.h"

namespace ai
{

Memory::Memory(idAI* owningAI) :
	owner(owningAI),
	alertState(ERelaxed),
	lastAlertRiseTime(-1),
	deadTimeAfterAlertRise(300),
	lastPatrolChatTime(-1),
	lastTimeFriendlyAISeen(-1000),
	lastTimeEnemySeen(-1),
	lastTimeVisualStimBark(-1),
	countEvidenceOfIntruders(0),
	nextHeadTurnCheckTime(0),
	currentlyHeadTurning(false),
	headTurnEndTime(0),
	idlePosition(idMath::INFINITY, idMath::INFINITY, idMath::INFINITY),
	idleYaw(0),
	playIdleAnimations(true),
	enemiesHaveBeenSeen(false),
	hasBeenAttackedByEnemy(false),
	itemsHaveBeenStolen(false),
	itemsHaveBeenBroken(false),
	unconsciousPeopleHaveBeenFound(false),
	deadPeopleHaveBeenFound(false),
	alertPos(0,0,0),
	alertClass(EAlertClassCount),
	alertType(EAlertTypeCount),
	alertRadius(-1),
	lastAudioAlertTime(-1),
	stimulusLocationItselfShouldBeSearched(false),
	investigateStimulusLocationClosely(false),
	alertedDueToCommunication(false),
	lastAlertPosSearched(0,0,0),
	alertSearchCenter(idMath::INFINITY, idMath::INFINITY, idMath::INFINITY),
	alertSearchVolume(0,0,0),
	alertSearchExclusionVolume(0,0,0),
	lastEnemyPos(0,0,0),
	canHitEnemy(false),
	willBeAbleToHitEnemy(false),
	canBeHitByEnemy(false),
	currentSearchSpot(0,0,0),
	hidingSpotTestStarted(false),
	hidingSpotSearchDone(true),
	noMoreHidingSpots(false),
	restartSearchForHidingSpots(false),
	hidingSpotThinkFrameCount(0),
	firstChosenHidingSpotIndex(0),
	currentChosenHidingSpotIndex(0),
	chosenHidingSpot(0,0,0),
	hidingSpotInvestigationInProgress(false),
	fleeingDone(true),
	positionBeforeTakingCover(0,0,0),
	resolvingMovementBlock(false)
{}

// Save/Restore routines
void Memory::Save(idSaveGame* savefile) const
{
	savefile->WriteObject(owner);
	savefile->WriteInt(static_cast<int>(alertState));
	currentPath.Save(savefile);
	nextPath.Save(savefile);
	lastPath.Save(savefile);
	savefile->WriteInt(lastAlertRiseTime);
	savefile->WriteInt(lastPatrolChatTime);
	savefile->WriteInt(lastTimeFriendlyAISeen);
	savefile->WriteInt(lastTimeEnemySeen);
	savefile->WriteInt(lastTimeVisualStimBark);
	savefile->WriteInt(countEvidenceOfIntruders);
	savefile->WriteInt(nextHeadTurnCheckTime);
	savefile->WriteBool(currentlyHeadTurning);
	savefile->WriteInt(headTurnEndTime);
	savefile->WriteVec3(idlePosition);
	savefile->WriteFloat(idleYaw);
	savefile->WriteBool(playIdleAnimations);
	savefile->WriteBool(enemiesHaveBeenSeen);
	savefile->WriteBool(hasBeenAttackedByEnemy);
	savefile->WriteBool(itemsHaveBeenStolen);
	savefile->WriteBool(itemsHaveBeenBroken);
	savefile->WriteBool(unconsciousPeopleHaveBeenFound);
	savefile->WriteBool(deadPeopleHaveBeenFound);
	savefile->WriteVec3(alertPos);
	savefile->WriteInt(static_cast<int>(alertClass));
	savefile->WriteInt(static_cast<int>(alertType));
	savefile->WriteFloat(alertRadius);
	savefile->WriteBool(stimulusLocationItselfShouldBeSearched);
	savefile->WriteBool(investigateStimulusLocationClosely);
	savefile->WriteBool(alertedDueToCommunication);
	savefile->WriteVec3(lastAlertPosSearched);
	savefile->WriteVec3(alertSearchCenter);
	savefile->WriteVec3(alertSearchVolume);
	savefile->WriteVec3(alertSearchExclusionVolume);
	savefile->WriteVec3(lastEnemyPos);
	savefile->WriteBool(canHitEnemy);
	savefile->WriteBool(willBeAbleToHitEnemy);
	savefile->WriteBool(canBeHitByEnemy);
	savefile->WriteVec3(currentSearchSpot);
	savefile->WriteBool(hidingSpotTestStarted);
	savefile->WriteBool(hidingSpotSearchDone);
	savefile->WriteBool(noMoreHidingSpots);
	savefile->WriteBool(restartSearchForHidingSpots);
	savefile->WriteInt(hidingSpotThinkFrameCount);
	savefile->WriteInt(firstChosenHidingSpotIndex);
	savefile->WriteInt(currentChosenHidingSpotIndex);
	savefile->WriteVec3(chosenHidingSpot);
	savefile->WriteBool(hidingSpotInvestigationInProgress);
	savefile->WriteBool(fleeingDone);
	savefile->WriteVec3(positionBeforeTakingCover);
	savefile->WriteBool(resolvingMovementBlock);
	lastDoorHandled.Save(savefile); // grayman #2712

	doorRelated.currentDoor.Save(savefile);

	savefile->WriteInt(doorRelated.doorInfo.size());
	for (DoorInfoMap::const_iterator i = doorRelated.doorInfo.begin();
		 i != doorRelated.doorInfo.end(); ++i)
	{
		savefile->WriteObject(i->first);
		i->second->Save(savefile);
	}

	// greebo: Don't save the AAS areaNum => DoorInfo mapping (can be re-calculated at restore time)

	savefile->WriteInt(static_cast<int>(greetingInfo.size()));

	for (ActorGreetingInfoMap::const_iterator i = greetingInfo.begin();
		 i != greetingInfo.end(); ++i)
	{
		savefile->WriteObject(i->first);
		savefile->WriteInt(i->second.lastGreetingTime);
		savefile->WriteInt(i->second.lastConsiderTime);
	}
}

void Memory::Restore(idRestoreGame* savefile)
{
	savefile->ReadObject(reinterpret_cast<idClass*&>(owner));

	int temp;
	savefile->ReadInt(temp);
	alertState = static_cast<EAlertState>(temp);

	currentPath.Restore(savefile);
	nextPath.Restore(savefile);
	lastPath.Restore(savefile);
	savefile->ReadInt(lastAlertRiseTime);
	savefile->ReadInt(lastPatrolChatTime);
	savefile->ReadInt(lastTimeFriendlyAISeen);
	savefile->ReadInt(lastTimeEnemySeen);
	savefile->ReadInt(lastTimeVisualStimBark);
	savefile->ReadInt(countEvidenceOfIntruders);
	savefile->ReadInt(nextHeadTurnCheckTime);
	savefile->ReadBool(currentlyHeadTurning);
	savefile->ReadInt(headTurnEndTime);
	savefile->ReadVec3(idlePosition);
	savefile->ReadFloat(idleYaw);
	savefile->ReadBool(playIdleAnimations);
	savefile->ReadBool(enemiesHaveBeenSeen);
	savefile->ReadBool(hasBeenAttackedByEnemy);
	savefile->ReadBool(itemsHaveBeenStolen);
	savefile->ReadBool(itemsHaveBeenBroken);
	savefile->ReadBool(unconsciousPeopleHaveBeenFound);
	savefile->ReadBool(deadPeopleHaveBeenFound);
	savefile->ReadVec3(alertPos);

	savefile->ReadInt(temp);
	alertClass = static_cast<EAlertClass>(temp);

	savefile->ReadInt(temp);
	alertType = static_cast<EAlertType>(temp);

	savefile->ReadFloat(alertRadius);
	savefile->ReadBool(stimulusLocationItselfShouldBeSearched);
	savefile->ReadBool(investigateStimulusLocationClosely);
	savefile->ReadBool(alertedDueToCommunication);
	savefile->ReadVec3(lastAlertPosSearched);
	savefile->ReadVec3(alertSearchCenter);
	savefile->ReadVec3(alertSearchVolume);
	savefile->ReadVec3(alertSearchExclusionVolume);
	savefile->ReadVec3(lastEnemyPos);
	savefile->ReadBool(canHitEnemy);
	savefile->ReadBool(willBeAbleToHitEnemy);
	savefile->ReadBool(canBeHitByEnemy);
	savefile->ReadVec3(currentSearchSpot);
	savefile->ReadBool(hidingSpotTestStarted);
	savefile->ReadBool(hidingSpotSearchDone);
	savefile->ReadBool(noMoreHidingSpots);
	savefile->ReadBool(restartSearchForHidingSpots);
	savefile->ReadInt(hidingSpotThinkFrameCount);
	savefile->ReadInt(firstChosenHidingSpotIndex);
	savefile->ReadInt(currentChosenHidingSpotIndex);
	savefile->ReadVec3(chosenHidingSpot);
	savefile->ReadBool(hidingSpotInvestigationInProgress);
	savefile->ReadBool(fleeingDone);
	savefile->ReadVec3(positionBeforeTakingCover);
	savefile->ReadBool(resolvingMovementBlock);
	lastDoorHandled.Restore(savefile); // grayman #2712

	doorRelated.currentDoor.Restore(savefile);
	// Clear the containers before restoring them
	doorRelated.doorInfo.clear();
	doorRelated.areaDoorInfoMap.clear();

	int num;
	savefile->ReadInt(num);
	for (int i = 0; i < num; i++)
	{
		CFrobDoor* door;
		savefile->ReadObject(reinterpret_cast<idClass*&>(door));

		// Allocate a new doorinfo structure and insert it into the map
		DoorInfoPtr info(new DoorInfo);

		std::pair<DoorInfoMap::iterator, bool> result = 
			doorRelated.doorInfo.insert(DoorInfoMap::value_type(door, info));

		// The insertion must succeed (unique pointers in the map!), otherwise we have inconsistent data
		assert(result.second == true);

		info->Restore(savefile);
	}

	// greebo: Reconstruct the AAS areaNum => DoorInfo mapping
	for (DoorInfoMap::iterator i = doorRelated.doorInfo.begin();
		 i != doorRelated.doorInfo.end(); ++i)
	{
		// Use the areanumber as index and insert the pointer into the map
		doorRelated.areaDoorInfoMap.insert(
			AreaToDoorInfoMap::value_type(i->second->areaNum, i->second)
		);
	}

	greetingInfo.clear();

	savefile->ReadInt(temp);
	for (int i = 0; i < temp; ++i)
	{
		idAI* ai;
		savefile->ReadObject(reinterpret_cast<idClass*&>(ai));
		
		std::pair<ActorGreetingInfoMap::iterator, bool> result = greetingInfo.insert(
			ActorGreetingInfoMap::value_type(ai, GreetingInfo()));
		
		savefile->ReadInt(result.first->second.lastGreetingTime);
		savefile->ReadInt(result.first->second.lastConsiderTime);
	}
}

DoorInfo& Memory::GetDoorInfo(CFrobDoor* door)
{
	DoorInfoMap::iterator i = doorRelated.doorInfo.find(door);

	if (i != doorRelated.doorInfo.end())
	{
		return *(i->second);
	}
	else
	{
		DoorInfoPtr info(new DoorInfo);
		// Set the area number
		info->areaNum = door->GetAASArea(owner->GetAAS());

		// Insert into the map
		std::pair<DoorInfoMap::iterator, bool> result =
			doorRelated.doorInfo.insert(DoorInfoMap::value_type(door, info));

		// Add the areaNum => info mapping for faster lookup using area numbers
		doorRelated.areaDoorInfoMap.insert(
			AreaToDoorInfoMap::value_type(info->areaNum, info)
		);

		return *(result.first->second);
	}
}

DoorInfoPtr Memory::GetDoorInfo(int areaNum)
{
	AreaToDoorInfoMap::iterator i = doorRelated.areaDoorInfoMap.find(areaNum);
	return (i != doorRelated.areaDoorInfoMap.end()) ? i->second : DoorInfoPtr();
}

Memory::GreetingInfo& Memory::GetGreetingInfo(idActor* actor)
{
	// Insert structure if not existing
	ActorGreetingInfoMap::iterator i = greetingInfo.find(actor);

	if (i == greetingInfo.end())
	{
		i = greetingInfo.insert(ActorGreetingInfoMap::value_type(actor, GreetingInfo())).first;
	}

	return i->second;
}

} // namespace ai
