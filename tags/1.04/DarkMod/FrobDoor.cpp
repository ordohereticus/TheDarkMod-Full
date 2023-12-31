/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 4387 $
 * $Date: 2010-12-26 05:44:56 -0500 (Sun, 26 Dec 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

// Copyright (C) 2004 Gerhard W. Gruber <sparhawk@gmx.at>
//

#include "../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: FrobDoor.cpp 4387 2010-12-26 10:44:56Z greebo $", init_version);

#include "../game/game_local.h"
#include "DarkModGlobals.h"
#include "BinaryFrobMover.h"
#include "Inventory/Item.h"
#include "Inventory/Category.h"
#include "FrobDoor.h"
#include "FrobDoorHandle.h"
#include "../tools/compilers/aas/aasfile.h"
#include "sndProp.h"
#include "StimResponse/StimResponseTimer.h"
#include "StimResponse/StimResponse.h"

#include "../game/ai/aas.h"

//===============================================================================
//CFrobDoor
//===============================================================================

const idEventDef EV_TDM_Door_OpenDoor( "OpenDoor", "f" );
const idEventDef EV_TDM_Door_GetDoorhandle( "GetDoorhandle", NULL, 'e' );

const idEventDef EV_TDM_Door_ClearPlayerImmobilization("EV_TDM_Door_ClearPlayerImmobilization", "e"); // allows player to handle weapons again

CLASS_DECLARATION( CBinaryFrobMover, CFrobDoor )
	EVENT( EV_TDM_Door_OpenDoor,			CFrobDoor::Event_OpenDoor)
	EVENT( EV_TDM_FrobMover_HandleLockRequest,	CFrobDoor::Event_HandleLockRequest ) // overrides binaryfrobmover's request
	EVENT( EV_TDM_Door_GetDoorhandle,		CFrobDoor::Event_GetDoorhandle)

	// Needed for PickableLock: Update Handle position on lockpick status update
	EVENT( EV_TDM_Lock_StatusUpdate,	CFrobDoor::Event_Lock_StatusUpdate)
	EVENT( EV_TDM_Lock_OnLockPicked,	CFrobDoor::Event_Lock_OnLockPicked)
	EVENT( EV_TDM_Door_ClearPlayerImmobilization,	CFrobDoor::Event_ClearPlayerImmobilization )
END_CLASS

CFrobDoor::CFrobDoor()
{
	DM_LOG(LC_FUNCTION, LT_DEBUG)LOGSTRING("this: %08lX [%s]\r", this, __FUNCTION__);
	m_FrobActionScript = "frob_door";
	m_DoubleDoor = NULL;
	m_LastHandleUpdateTime = -1;
}

CFrobDoor::~CFrobDoor()
{
	ClearDoorTravelFlag();
}

void CFrobDoor::Save(idSaveGame *savefile) const
{
	savefile->WriteInt(m_OpenPeers.Num());
	for (int i = 0; i < m_OpenPeers.Num(); i++)
	{
		savefile->WriteString(m_OpenPeers[i]);
	}

	savefile->WriteInt(m_LockPeers.Num());
	for (int i = 0; i < m_LockPeers.Num(); i++)
	{
		savefile->WriteString(m_LockPeers[i]);
	}

	m_DoubleDoor.Save(savefile);

	savefile->WriteInt(m_Doorhandles.Num());
	for (int i = 0; i < m_Doorhandles.Num(); i++)
	{
		m_Doorhandles[i].Save(savefile);
	}

	savefile->WriteInt(m_LastHandleUpdateTime);
}

void CFrobDoor::Restore( idRestoreGame *savefile )
{
	int num;
	savefile->ReadInt(num);
	m_OpenPeers.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		savefile->ReadString(m_OpenPeers[i]);
	}

	savefile->ReadInt(num);
	m_LockPeers.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		savefile->ReadString(m_LockPeers[i]);
	}

	m_DoubleDoor.Restore(savefile);
	
	savefile->ReadInt(num);
	m_Doorhandles.SetNum(num);
	for (int i = 0; i < num; i++)
	{
		m_Doorhandles[i].Restore(savefile);
	}

	savefile->ReadInt(m_LastHandleUpdateTime);

	SetDoorTravelFlag();
}

void CFrobDoor::Spawn()
{}

void CFrobDoor::PostSpawn()
{
	// Let the base class do its stuff first
	CBinaryFrobMover::PostSpawn();

	// Locate the double door entity before closing our portal
	FindDoubleDoor();

	// Wait until here for the first update of sound loss, in case a double door is open
	UpdateSoundLoss();

	if (!m_Open)
	{
		// Door starts _completely_ closed, try to shut the portal
		ClosePortal();
	}

	// Open the portal if either of the doors is open
	CFrobDoor* doubleDoor = m_DoubleDoor.GetEntity();
	if (m_Open || (doubleDoor != NULL && doubleDoor->IsOpen()))
	{
		OpenPortal();
	}

	// Search for all spawnargs matching "open_peer_N" and add them to our open peer list
	for (const idKeyValue* kv = spawnArgs.MatchPrefix("open_peer"); kv != NULL; kv = spawnArgs.MatchPrefix("open_peer", kv))
	{
		// Add the peer
		AddOpenPeer(kv->GetValue());
	}

	// Search for all spawnargs matching "lock_peer_N" and add them to our lock peer list
	for (const idKeyValue* kv = spawnArgs.MatchPrefix("lock_peer"); kv != NULL; kv = spawnArgs.MatchPrefix("lock_peer", kv))
	{
		// Add the peer
		AddLockPeer(kv->GetValue());
	}

	idStr doorHandleName = spawnArgs.GetString("door_handle", "");
	if (!doorHandleName.IsEmpty())
	{
		idEntity* handleEnt = gameLocal.FindEntity(doorHandleName);

		if (handleEnt != NULL && handleEnt->IsType(CFrobDoorHandle::Type))
		{
			// Convert to frobdoorhandle pointer and call the helper function
			CFrobDoorHandle* handle = static_cast<CFrobDoorHandle*>(handleEnt);
			AddDoorhandle(handle);

			// Check if we should bind the named handle to ourselves
			if (spawnArgs.GetBool("door_handle_bind_flag", "1"))
			{
				handle->Bind(this, true);
			}
		}
		else
		{
			DM_LOG(LC_LOCKPICK, LT_ERROR)LOGSTRING("Doorhandle entity not spawned or of wrong type: %s\r", doorHandleName.c_str());
		}
	}

	// greebo: Should we auto-setup the doorhandles?
	if (spawnArgs.GetBool("auto_setup_door_handles", "1"))
	{
		AutoSetupDoorHandles();
	}

	// greebo: Should we auto-setup the double door behaviour?
	if (spawnArgs.GetBool("auto_setup_double_door", "1"))
	{
		AutoSetupDoubleDoor();
	}

	SetDoorTravelFlag();
}

void CFrobDoor::SetDoorTravelFlag()
{
	// Flag the AAS areas the door is located in with door travel flag
	for (int i = 0; i < gameLocal.NumAAS(); i++)
	{
		idAAS*	aas = gameLocal.GetAAS(i);
		if (aas == NULL)
		{
			continue;
		}
		
		int areaNum = GetAASArea(aas);
		aas->SetAreaTravelFlag(areaNum, TFL_DOOR);
		aas->ReferenceDoor(this, areaNum);
	}
}

void CFrobDoor::ClearDoorTravelFlag()
{
	// Flag the AAS areas the door is located in with door travel flag
	for (int i = 0; i < gameLocal.NumAAS(); i++)
	{
		idAAS*	aas = gameLocal.GetAAS(i);
		if (aas == NULL)
		{
			continue;
		}
		
		int areaNum = GetAASArea(aas);
		if (areaNum > 0)
		{
			aas->RemoveAreaTravelFlag(areaNum, TFL_DOOR);
			aas->DeReferenceDoor(this, areaNum);
		}
		else
		{
			gameLocal.Warning("Door %s is not within a valid AAS area", name.c_str());
		}
	}
}

void CFrobDoor::Lock(bool bMaster)
{
	// Pass the call to the base class, the OnLock() event will be fired 
	// if the locking process is allowed
	CBinaryFrobMover::Lock(bMaster);
}

void CFrobDoor::OnLock(bool bMaster)
{
	// Call the base class first
	CBinaryFrobMover::OnLock(bMaster);

	if (bMaster)
	{
		LockPeers();
	}
}

void CFrobDoor::Unlock(bool bMaster)
{
	// Pass the call to the base class, the OnUnlock() event will be fired 
	// if the locking process is allowed
	CBinaryFrobMover::Unlock(bMaster);
}

void CFrobDoor::OnUnlock(bool bMaster)
{
	// Call the base class first
	CBinaryFrobMover::OnUnlock(bMaster);

	if (bMaster) 
	{
		UnlockPeers();
	}
}

void CFrobDoor::Open(bool bMaster)
{
	// If we have a doorhandle we want to tap it before the door starts to open if the door wasn't
	// already interrupted
	if (m_Doorhandles.Num() > 0 && !m_bInterrupted)
	{
		// Relay the call to the handles, the OpenDoor() call will come back to us
		for (int i = 0; i < m_Doorhandles.Num(); i++)
		{
			CFrobDoorHandle* handle = m_Doorhandles[i].GetEntity();
			if (handle == NULL) continue;

			handle->Tap();
		}

		if (bMaster)
		{
			TapPeers();
		}
	}
	else
	{
		// No handle present or interrupted, let's just proceed with our own open routine
		OpenDoor(bMaster);
	}
}

void CFrobDoor::OpenDoor(bool bMaster)
{
	DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("FrobDoor: Opening\r" );

	// Now pass the call to the base class, which will invoke PreOpen() and the other events
	CBinaryFrobMover::Open(bMaster);
}

void CFrobDoor::OnStartOpen(bool wasClosed, bool bMaster)
{
	// Call the base class first
	CBinaryFrobMover::OnStartOpen(wasClosed, bMaster);

	// We are actually opening, open the visportal too
	OpenPortal();

	// Update soundprop
	UpdateSoundLoss();

	if (bMaster)
	{
		OpenPeers();
	}
}

void CFrobDoor::Close(bool bMaster)
{
	DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("FrobDoor: Closing\r" );

	// Invoke the close method in any case
	CBinaryFrobMover::Close(bMaster);
}

void CFrobDoor::OnStartClose(bool wasOpen, bool bMaster)
{
	CBinaryFrobMover::OnStartClose(wasOpen, bMaster);

	if (bMaster)
	{
		ClosePeers();
	}
}

void CFrobDoor::OnClosedPositionReached() 
{
	// Call the base class
	CBinaryFrobMover::OnClosedPositionReached();

	// Try to close the visportal
	ClosePortal();

	// Update the sound propagation values
	UpdateSoundLoss();
}

bool CFrobDoor::CanBeUsedBy(const CInventoryItemPtr& item, const bool isFrobUse) 
{
	// First, check if the frob master can be used
	// If this doesn't succeed, perform additional checks
	idEntity* master = GetFrobMaster();
	if( master != NULL && master->CanBeUsedBy(item, isFrobUse) )
	{
		return true;
	}

	if (item == NULL) return false;

	assert(item->Category() != NULL);

	// FIXME: Move this to idEntity to some sort of "usable_by_inv_category" list?
	const idStr& itemName = item->Category()->GetName();
	if (itemName == "Keys")
	{
		// Keys can always be used on doors
		// Exception: for "frob use" this only applies when the door is locked
		return (isFrobUse) ? IsLocked() : true;
	}
	else if (itemName == "Lockpicks")
	{
		if (!m_Lock->IsPickable())
		{
			// Lock is not pickable
			DM_LOG(LC_LOCKPICK, LT_DEBUG)LOGSTRING("FrobDoor %s is not pickable\r", name.c_str());
			return false;
		}

		// Lockpicks behave similar to keys
		return (isFrobUse) ? IsLocked() : true;
	}

	return false;
}

bool CFrobDoor::UseBy(EImpulseState impulseState, const CInventoryItemPtr& item)
{
	if (item == NULL) return false;

	// Pass the call on to the master, if we have one
	if (GetFrobMaster() != NULL) 
	{
		return GetFrobMaster()->UseBy(impulseState, item);
	}

	assert(item->Category() != NULL);

	// Retrieve the entity behind that item and reject NULL entities
	idEntity* itemEntity = item->GetItemEntity();
	if (itemEntity == NULL) return false;

	// Get the name of this inventory category
	const idStr& itemName = item->Category()->GetName();
	
	if (itemName == "Keys" && impulseState == EPressed) 
	{
		// Keys can be used on button PRESS event, let's see if the key matches
		if (m_UsedByName.FindIndex(itemEntity->name) != -1)
		{
			// If we're locked or closed, just toggle the lock. 
			if (IsLocked() || IsAtClosedPosition())
			{
				ToggleLock();
			}
			// If we're open, set a lock request and start closing.
			else
			{
				// Close the door and set the lock request to true
				CloseAndLock();
			}

			return true;
		}
		else
		{
			FrobMoverStartSound("snd_wrong_key");
			return false;
		}
	}
	else if (itemName == "Lockpicks")
	{
		if (!m_Lock->IsPickable())
		{
			// Lock is not pickable
			DM_LOG(LC_LOCKPICK, LT_DEBUG)LOGSTRING("FrobDoor %s is not pickable\r", name.c_str());
			return false;
		}

		// Lockpicks are different, we need to look at the button state
		// First we check if this item is a lockpick. It has to be of the toolclass lockpick
		// and the type must be set.
		idStr str = itemEntity->spawnArgs.GetString("lockpick_type", "");

		if (str.Length() == 1)
		{
			// greebo: Check if the item owner is a player, and if yes, 
			// update the immobilization flags.
			idEntity* itemOwner = item->GetOwner();

			if (itemOwner->IsType(idPlayer::Type))
			{
				idPlayer* playerOwner = static_cast<idPlayer*>(itemOwner);
				playerOwner->SetImmobilization("Lockpicking", EIM_ATTACK);

				// Schedule an event 1/3 sec. from now, to enable weapons again after this time
				CancelEvents(&EV_TDM_Door_ClearPlayerImmobilization);
				PostEventMS(&EV_TDM_Door_ClearPlayerImmobilization, 300, playerOwner);
			}

			// Pass the call to the lockpick routine
			return m_Lock->ProcessLockpickImpulse(impulseState, static_cast<int>(str[0]));
		}
		else
		{
			gameLocal.Warning("Wrong 'type' spawnarg for lockpicking on item %s, must be a single character.", itemEntity->name.c_str());
			return false;
		}
	}

	return false;
}

void CFrobDoor::UpdateSoundLoss()
{
	if (!areaPortal) return; // not a portal door

	CFrobDoor* doubleDoor = m_DoubleDoor.GetEntity();

	// If we have no double door, assume the bool to be "open"
	bool doubleDoorIsOpen = (doubleDoor != NULL) ? doubleDoor->IsOpen() : true;
	bool thisDoorIsOpen = IsOpen();

	// TODO: check the spawnarg: sound_char, and return the 
	// appropriate loss for that door, open or closed

	float lossDB = 0.0f;

	if (thisDoorIsOpen && doubleDoorIsOpen)
	{
		lossDB = spawnArgs.GetFloat("loss_open", "1.0");
	}
	else if (thisDoorIsOpen && !doubleDoorIsOpen)
	{
		lossDB = spawnArgs.GetFloat("loss_double_open", "1.0");
	}
	else
	{
		lossDB = spawnArgs.GetFloat("loss_closed", "15.0");
	}
	
	gameLocal.m_sndProp->SetPortalLoss(areaPortal, lossDB);
}

void CFrobDoor::FindDoubleDoor()
{
	idClipModel* clipModelList[MAX_GENTITIES];

	idBounds clipBounds = physicsObj.GetAbsBounds();
	clipBounds.ExpandSelf( 10.0f );

	int numListedClipModels = gameLocal.clip.ClipModelsTouchingBounds( clipBounds, CONTENTS_SOLID, clipModelList, MAX_GENTITIES );

	for (int i = 0; i < numListedClipModels; i++) 
	{
		idClipModel* clipModel = clipModelList[i];
		idEntity* obEnt = clipModel->GetEntity();

		// Ignore self
		if (obEnt == this) continue;

		if (obEnt->IsType(CFrobDoor::Type))
		{
			// check the visportal inside the other door, if it's the same as this one => double door
			int testPortal = gameRenderWorld->FindPortal(obEnt->GetPhysics()->GetAbsBounds());

			if (testPortal == areaPortal && testPortal != 0)
			{
				DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("FrobDoor %s found double door %s\r", name.c_str(), obEnt->name.c_str());
				m_DoubleDoor = static_cast<CFrobDoor*>(obEnt);
			}
		}
	}
}

CFrobDoor* CFrobDoor::GetDoubleDoor()
{
	return m_DoubleDoor.GetEntity();
}

void CFrobDoor::ClosePortal()
{
	CFrobDoor* doubleDoor = m_DoubleDoor.GetEntity();

	if (doubleDoor == NULL || !doubleDoor->IsOpen())
	{
		// No double door or double door is closed too
		if (areaPortal) 
		{
			SetPortalState(false);
		}
	}
}

void CFrobDoor::SetFrobbed(bool val)
{
	DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("door_body [%s] %08lX is frobbed\r", name.c_str(), this);

	// First invoke the base class, then check for a doorhandle
	idEntity::SetFrobbed(val);

	for (int i = 0; i < m_Doorhandles.Num(); i++)
	{
		CFrobDoorHandle* handle = m_Doorhandles[i].GetEntity();
		if (handle == NULL) continue;

		handle->SetFrobbed(val);
	}

	m_Lock->OnFrobbedStatusChange(val);
}

bool CFrobDoor::IsFrobbed()
{
	// If the door has a handle and it is frobbed, then we are also considered 
	// to be frobbed.
	for (int i = 0; i < m_Doorhandles.Num(); i++)
	{
		CFrobDoorHandle* handle = m_Doorhandles[i].GetEntity();
		if (handle == NULL) continue;

		if (handle->IsFrobbed())
		{
			return true;
		}
	}

	// None of the handles are frobbed
	return idEntity::IsFrobbed();
}

void CFrobDoor::UpdateHandlePosition()
{
	// greebo: Don't issue an handle update position call each frame,
	// this might cause movers to freeze in place, as the extrapolation class
	// let's them rest for the first frame
	if (gameLocal.time <= m_LastHandleUpdateTime + gameLocal.msec) return;

	m_LastHandleUpdateTime = gameLocal.time;

	// Calculate the fraction based on the current pin/sample state
	float fraction = m_Lock->CalculateHandleMoveFraction();

	if (cv_lp_debug_hud.GetBool())
	{
		idPlayer* player = gameLocal.GetLocalPlayer();
		player->SetGuiString(player->lockpickHUD, "StatusText3", idStr(fraction));
	}

	// Tell the doorhandles to update their position
	for (int i = 0; i < m_Doorhandles.Num(); ++i)
	{
		CFrobDoorHandle* handle = m_Doorhandles[i].GetEntity();

		if (handle == NULL) continue;

		handle->SetFractionalPosition(fraction);
	}
}

void CFrobDoor::Event_Lock_StatusUpdate()
{
	UpdateHandlePosition();
}

void CFrobDoor::Event_Lock_OnLockPicked()
{
	// "Lock is picked" signal, unlock in master mode
	Unlock(true);
}

void CFrobDoor::AttackAction(idPlayer* player)
{
	idEntity* master = GetFrobMaster();

	if (master != NULL) 
	{
		master->AttackAction(player);
		return;
	}

	// No master
	m_Lock->AttackAction(player);
}

void CFrobDoor::OpenPeers()
{
	OpenClosePeers(true);
}

void CFrobDoor::ClosePeers()
{
	OpenClosePeers(false);
}

void CFrobDoor::OpenClosePeers(bool open)
{
	// Cycle through our "open peers" list and issue the call to them
	for (int i = 0; i < m_OpenPeers.Num(); i++)
	{
		const idStr& openPeer = m_OpenPeers[i];

		DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("Trying linked entity [%s]\r", openPeer.c_str());

		idEntity* ent = gameLocal.FindEntity(openPeer);

		if (ent != NULL && ent->IsType(CFrobDoor::Type))
		{
			DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("Calling linked entity [%s] for OpenClose\r", openPeer.c_str());

			CFrobDoor* door = static_cast<CFrobDoor*>(ent);

			if (open)
			{
				if (door->IsAtClosedPosition())
				{
					// Other door is at closed position, let the handle tap
					door->Open(false);
				}
				else
				{
					// The other door is open or in an intermediate state, just call open
					door->OpenDoor(false);
				}
			}
			else
			{
				door->Close(false);
			}
		}
		else
		{
			DM_LOG(LC_FROBBING, LT_ERROR)LOGSTRING("[%s] Linked entity [%s] not spawned or not of class CFrobDoor\r", name.c_str(), openPeer.c_str());
		}
	}
}

void CFrobDoor::LockPeers()
{
	LockUnlockPeers(true);
}

void CFrobDoor::UnlockPeers()
{
	LockUnlockPeers(false);
}

void CFrobDoor::LockUnlockPeers(bool lock)
{
	// Go through the list and issue the call
	for (int i = 0; i < m_LockPeers.Num(); i++)
	{
		const idStr& lockPeer = m_LockPeers[i];

		DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("Trying linked entity [%s]\r", lockPeer.c_str());

		idEntity* ent = gameLocal.FindEntity(lockPeer);

		if (ent != NULL && ent->IsType(CFrobDoor::Type))
		{
			DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("Calling linked entity [%s] for lock\r", lockPeer.c_str());

			CFrobDoor* door = static_cast<CFrobDoor*>(ent);
			
			if (lock)
			{
				door->Lock(false);
			}
			else
			{
				door->Unlock(false);
			}
		}
		else
		{
			DM_LOG(LC_FROBBING, LT_ERROR)LOGSTRING("Linked entity [%s] not found or of wrong type\r", lockPeer.c_str());
		}
	}
}

void CFrobDoor::TapPeers()
{
	// In master mode, tap the handles of the master_open chain too
	// Cycle through our "open peers" list and issue the call to them
	for (int i = 0; i < m_OpenPeers.Num(); i++)
	{
		const idStr& openPeer = m_OpenPeers[i];

		DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("Trying linked entity [%s]\r", openPeer.c_str());

		idEntity* ent = gameLocal.FindEntity(openPeer);

		if (ent != NULL && ent->IsType(CFrobDoor::Type))
		{
			DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("Calling linked entity [%s] for tapping\r", openPeer.c_str());
			CFrobDoor* other = static_cast<CFrobDoor*>(ent);

			if (other->GetDoorhandle() != NULL)
			{
				other->GetDoorhandle()->Tap();
			}
		}
		else
		{
			DM_LOG(LC_FROBBING, LT_ERROR)LOGSTRING("[%s] Linked entity [%s] not spawned or not of class CFrobDoor\r", name.c_str(), openPeer.c_str());
		}
	}
}

void CFrobDoor::AddOpenPeer(const idStr& peerName)
{
	// Find the entity and check if it's actually a door
	idEntity* entity = gameLocal.FindEntity(peerName);

	if (entity != NULL && entity->IsType(CFrobDoor::Type))
	{
		m_OpenPeers.AddUnique(peerName);
		DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("%s: open_peer %s added to local list (%u)\r", name.c_str(), peerName.c_str(), m_OpenPeers.Num());
	}
	else
	{
		DM_LOG(LC_ENTITY, LT_ERROR)LOGSTRING("open_peer [%s] not spawned or of wrong type.\r", peerName.c_str());
	}
}

void CFrobDoor::RemoveOpenPeer(const idStr& peerName)
{
	m_OpenPeers.Remove(peerName);
}

void CFrobDoor::AddLockPeer(const idStr& peerName)
{
	// Find the entity and check if it's actually a door
	idEntity* entity = gameLocal.FindEntity(peerName);

	if (entity != NULL && entity->IsType(CFrobDoor::Type))
	{
		m_LockPeers.AddUnique(peerName);
		DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("%s: lock_peer %s added to local list (%u)\r", name.c_str(), peerName.c_str(), m_LockPeers.Num());
	}
	else
	{
		DM_LOG(LC_ENTITY, LT_ERROR)LOGSTRING("lock_peer [%s] not spawned or of wrong type.\r", peerName.c_str());
	}
}

void CFrobDoor::RemoveLockPeer(const idStr& peerName)
{
	m_LockPeers.Remove(peerName);
}

CFrobDoorHandle* CFrobDoor::GetDoorhandle()
{
	return (m_Doorhandles.Num() > 0) ? m_Doorhandles[0].GetEntity() : NULL;
}

void CFrobDoor::AddDoorhandle(CFrobDoorHandle* handle)
{
	// Store the pointer and the original position
	idEntityPtr<CFrobDoorHandle> handlePtr;
	handlePtr = handle;

	if (m_Doorhandles.FindIndex(handlePtr) != -1)
	{
		return; // handle is already known
	}

	m_Doorhandles.Append(handlePtr);

	// Let the handle know about us
	handle->SetDoor(this);

	// Set up the frob peer relationship between the door and the handle
	m_FrobPeers.AddUnique(handle->name);
	handle->AddFrobPeer(name);
	handle->SetFrobable(m_bFrobable);
}

void CFrobDoor::AutoSetupDoorHandles()
{
	// Find a suitable teamchain member
	idEntity* part = FindMatchingTeamEntity(CFrobDoorHandle::Type);

	while (part != NULL)
	{
		// Found the handle, set it up
		AddDoorhandle(static_cast<CFrobDoorHandle*>(part));

		DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("%s: Auto-added door handle %s to local list.\r", name.c_str(), part->name.c_str());

		// Get the next handle
		part = FindMatchingTeamEntity(CFrobDoorHandle::Type, part);
	}

	for (int i = 0; i < m_Doorhandles.Num(); i++)
	{
		CFrobDoorHandle* handle = m_Doorhandles[i].GetEntity();
		if (handle == NULL) continue;

		// The first handle is the master, all others get their master flag set to FALSE
		handle->SetMasterHandle(i == 0);
	}
}

void CFrobDoor::AutoSetupDoubleDoor()
{
	CFrobDoor* doubleDoor = m_DoubleDoor.GetEntity();

	if (doubleDoor != NULL)
	{
		DM_LOG(LC_ENTITY, LT_INFO)LOGSTRING("%s: Auto-setting up %s as double door.\r", name.c_str(), doubleDoor->name.c_str());

		if (spawnArgs.GetBool("auto_setup_double_door_frob_peer", "0"))
		{
			// Add the door to our frob_peer set
			m_FrobPeers.AddUnique(doubleDoor->name);

			// Add ourselves to the double door as frob peer
			doubleDoor->AddFrobPeer(name);
			doubleDoor->SetFrobable(m_bFrobable);
		}

		if (spawnArgs.GetBool("auto_setup_double_door_open_peer", "0"))
		{
			// Add "self" to the peers of the other door
			doubleDoor->AddOpenPeer(name);
			// Now add the name of the other door to our own peer list
			AddOpenPeer(doubleDoor->name);
		}

		if (spawnArgs.GetBool("auto_setup_double_door_lock_peer", "0"))
		{
			// Add "self" to the peers of the other door
			doubleDoor->AddLockPeer(name);
			// Now add the name of the other door to our own peer list
			AddLockPeer(doubleDoor->name);
		}
	}
}

bool CFrobDoor::PreLock(bool bMaster)
{
	if (!CBinaryFrobMover::PreLock(bMaster))
	{
		return false;
	}

	// Allow closing if all lock peers are at their closed position
	return AllLockPeersAtClosedPosition();
}

bool CFrobDoor::AllLockPeersAtClosedPosition()
{
	// Go through the list and check whether the lock peers are closed
	for (int i = 0; i < m_LockPeers.Num(); i++)
	{
		const idStr& lockPeer = m_LockPeers[i];

		DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("Trying linked entity [%s]\r", lockPeer.c_str());

		idEntity* ent = gameLocal.FindEntity(lockPeer);

		if (ent != NULL && ent->IsType(CFrobDoor::Type))
		{
			DM_LOG(LC_FROBBING, LT_DEBUG)LOGSTRING("Calling linked entity [%s] for lock\r", lockPeer.c_str());

			CFrobDoor* door = static_cast<CFrobDoor*>(ent);
			
			if (!door->IsAtClosedPosition())
			{
				return false;
			}
		}
		else
		{
			DM_LOG(LC_FROBBING, LT_ERROR)LOGSTRING("Linked entity [%s] not found or of wrong type\r", lockPeer.c_str());
		}
	}

	return true;
}

CFrobDoorHandle* CFrobDoor::GetNearestHandle(const idVec3& pos)
{
	// Skip calculation if only one doorhandle present in the first place
	if (m_Doorhandles.Num() == 1) 
	{
		return m_Doorhandles[0].GetEntity();
	}

	float bestDistanceSqr = idMath::INFINITY;
	CFrobDoorHandle* returnValue = NULL;

	for (int i = 0; i < m_Doorhandles.Num(); ++i)
	{
		CFrobDoorHandle* candidate = m_Doorhandles[i].GetEntity();

		if (candidate == NULL) continue;

		float candidateDistanceSqr = (candidate->GetPhysics()->GetOrigin() - pos).LengthSqr();

		if (candidateDistanceSqr < bestDistanceSqr)
		{
			// Found a new nearest handle
			returnValue = candidate;
			bestDistanceSqr = candidateDistanceSqr;
		}
	}

	return returnValue;
}

bool CFrobDoor::GetPhysicsToSoundTransform(idVec3 &origin, idMat3 &axis)
{
	// This will kick in for doors without any handles, these are playing their
	// sounds from the nearest point to the player's eyes, mid-bounding-box.
	const idBounds& bounds = GetPhysics()->GetAbsBounds();
	idVec3 eyePos = gameLocal.GetLocalPlayer()->GetEyePosition();

	// greebo: Choose the corner which is nearest to the player's eyeposition
	origin.x = (idMath::Fabs(bounds[0].x - eyePos.x) < idMath::Fabs(bounds[1].x - eyePos.x)) ? bounds[0].x : bounds[1].x;
	origin.y = (idMath::Fabs(bounds[0].y - eyePos.y) < idMath::Fabs(bounds[1].y - eyePos.y)) ? bounds[0].y : bounds[1].y;
	origin.z = (bounds[0].z + bounds[1].z) * 0.5f;

	axis.Identity();

	// The called expects the origin in local space
	origin -= GetPhysics()->GetOrigin();

	//gameRenderWorld->DebugArrow(colorWhite, GetPhysics()->GetOrigin() + origin, eyePos, 0, 5000);

	return true;
}

int CFrobDoor::FrobMoverStartSound(const char* soundName)
{
	if (m_Doorhandles.Num() > 0)
	{
		// greebo: Find the handle nearest to the player, as one of the doorhandles could be 
		// behind a closed visportal.
		CFrobDoorHandle* handle = GetNearestHandle(gameLocal.GetLocalPlayer()->GetEyePosition());

		if (handle != NULL)
		{
			// Let the sound play from the handle, but use the soundshader
			// as defined on this entity.
			idStr sound = spawnArgs.GetString(soundName, "");

			if (sound.IsEmpty())
			{
				gameLocal.Warning("Cannot find sound %s on door %s\r", soundName, name.c_str());
				return 0;
			}

			const idSoundShader* shader = declManager->FindSound(sound);

			int length = 0;
			handle->StartSoundShader(shader, SND_CHANNEL_ANY, 0, false, &length);

			//gameRenderWorld->DebugArrow(colorWhite, handle->GetPhysics()->GetOrigin(), gameLocal.GetLocalPlayer()->GetEyePosition(), 1, 5000);

			return length;
		}
	}

	// No handle or NULL handle, just call the base class
	return CBinaryFrobMover::FrobMoverStartSound(soundName);
}

void CFrobDoor::Event_GetDoorhandle()
{
	idThread::ReturnEntity(m_Doorhandles.Num() > 0 ? m_Doorhandles[0].GetEntity() : NULL);
}

void CFrobDoor::Event_OpenDoor(float master)
{
	OpenDoor(master != 0.0f ? true : false);
}

void CFrobDoor::Event_HandleLockRequest()
{
	// Check if all the peers are at their "closed" position, if yes: lock the door(s), if no: postpone the event
	if (AllLockPeersAtClosedPosition())
	{
		// Yes, all lock peers are at their "closed" position, lock ourselves and all peers
		Lock(true);
	}
	else
	{
		// One or more peers are not at their closed position (yet), postpone the event
		PostEventMS(&EV_TDM_FrobMover_HandleLockRequest, LOCK_REQUEST_DELAY);
	}
}

void CFrobDoor::Event_ClearPlayerImmobilization(idEntity* player)
{
	if (!player->IsType(idPlayer::Type)) return;

	// Release the immobilization imposed on the player by Lockpicking
	static_cast<idPlayer*>(player)->SetImmobilization("Lockpicking", 0);
}
