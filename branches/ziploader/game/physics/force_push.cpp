/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2990 $
 * $Date: 2008-11-07 10:56:38 -0500 (Fri, 07 Nov 2008) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Id: force_push.cpp 2990 2008-11-07 15:56:38Z greebo $", init_version);

#include "force_push.h"
#include "../game_local.h"

CLASS_DECLARATION( idForce, CForcePush )
END_CLASS

CForcePush::CForcePush() :
	pushEnt(NULL),
	id(0),
	impactVelocity(vec3_zero),
	startPushTime(-1),
	owner(NULL)
{
	lastPushEnt = NULL;
	memset(&contactInfo, 0, sizeof(contactInfo));
}

void CForcePush::SetOwner(idEntity* ownerEnt)
{
	owner = ownerEnt;
}

void CForcePush::SetPushEntity(idEntity* pushEnt, int id)
{
	idEntity* lastPushEntity = lastPushEnt.GetEntity();

	if (pushEnt != lastPushEntity)
	{
		// entity has changed, reset the timer
		startPushTime = gameLocal.time;
	}

	// Check if we are pushing anything
	if (pushEnt == NULL)
	{
		// No, update the owning actor's push state
		if (owner != NULL)
		{
			SetOwnerIsPushing(false);
		}

		// Did we push anything the frame before?
		if (lastPushEntity != NULL && lastPushEntity->IsType(idMoveable::Type))
		{
			// Let the pushed entity know that it is not being pushed anymore
			static_cast<idMoveable*>(lastPushEntity)->SetIsPushed(false, vec3_zero);
		}
	}

	this->pushEnt = pushEnt;
	this->id = id;
}

void CForcePush::SetContactInfo(const trace_t& contactInfo, const idVec3& impactVelocity)
{
	this->contactInfo = contactInfo;
	this->impactVelocity = impactVelocity;
}

void CForcePush::Evaluate( int time )
{
	if (owner == NULL) return;

	if (pushEnt == NULL) 
	{
		// nothing to do, but update the owning actor's push state
		SetOwnerIsPushing(false);
		return;
	}

	// Do not push static entity or non-pushable ones
	if (pushEnt->IsType(idStaticEntity::Type) || pushEnt->spawnArgs.GetBool("notPushable", "0"))
	{
		return;
	}

	idPhysics* physics = pushEnt->GetPhysics();
	//gameRenderWorld->DebugBox(colorRed, idBox(physics->GetBounds(), physics->GetOrigin(), physics->GetAxis()), 16);

	if (physics == NULL) return; // fix crash for entites not having physics

	float mass = physics->GetMass();
	float ownerMass = owner->GetPhysics()->GetMass();

	// This is the maximum mass an object can have to be kickable
	float massThresholdHeavy = ownerMass * cv_pm_push_heavy_threshold.GetFloat();

	bool isPushing = false;

	if (mass < massThresholdHeavy)
	{
		// Get the owner's velocity and project it on the impact normal
		float normalVelocity = -contactInfo.c.normal * impactVelocity;

		// Clamp to positive values
		if (normalVelocity < 0)
		{
			normalVelocity = 0;
		}

		float maxPlayerSpeed = pm_walkspeed.GetFloat() * cv_pm_runmod.GetFloat();
		float speedFraction = idMath::ClampFloat(0, 1, normalVelocity / maxPlayerSpeed);

		// Maximum resulting velocity in m/s
		const float vmax = 1.0f;

		// Calculate the resulting velocity of the kicked object, linearly scaled and clamped to [0..vmax] m/s
		float resultingVelocity = vmax * (1.02f - physics->GetMass()/49);
		resultingVelocity = METERS_TO_DOOM * idMath::ClampFloat(0, vmax, resultingVelocity);
		
		// This is the scale of our impulse, based on the maximum allowed kick velocity
		float scale = physics->GetMass() * resultingVelocity * speedFraction;

		// The pushed entity is not a heavy one, kick it
		//float scale = (-contactInfo.c.normal * impactVelocity) * ownerMass * cv_pm_pushmod.GetFloat();

		// Clamp the value to the maximum impulse we're allowed to give
		scale = idMath::ClampFloat(0, cv_pm_push_maximpulse.GetFloat(), scale);
		
		// greebo: Nullify the z-component of the impact impulse, we're always kicking horizontally
		idVec3 pushDirection(impactVelocity.x, impactVelocity.y, 0);
		pushDirection.NormalizeFast();

		// Check if the moveable has already a large impulse in that direction
		float currentDirectionalImpulse = (physics->GetLinearVelocity()*pushDirection) * physics->GetMass();

		if (currentDirectionalImpulse < cv_pm_push_maximpulse.GetFloat())
		{
			idVec3 pushImpulse = pushDirection * scale;

			//gameRenderWorld->DrawText( idStr(pushImpulse.LengthFast()), physics->GetAbsBounds().GetCenter(), 0.1f, colorWhite, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec*10 );
			//gameRenderWorld->DebugArrow( colorWhite, physics->GetAbsBounds().GetCenter(), physics->GetAbsBounds().GetCenter() - contactInfo.c.normal*100, 1, gameLocal.msec*10 );

			DM_LOG(LC_MOVEMENT, LT_INFO)LOGSTRING("Kicking impulse = %f,%f,%f, scale was %f\r", pushImpulse.x, pushImpulse.y, pushImpulse.z, scale);

			DM_LOG(LC_MOVEMENT, LT_INFO)LOGSTRING("Kicking obstacle %s, velocity BEFORE is %f,%f,%f\r", pushEnt->name.c_str(), 
				physics->GetLinearVelocity().x, physics->GetLinearVelocity().y, physics->GetLinearVelocity().z);
			DM_LOG(LC_MOVEMENT, LT_INFO)LOGSTRING("Kicking obstacle %s, angular velocity BEFORE is %f,%f,%f\r", pushEnt->name.c_str(), 
				physics->GetAngularVelocity().x, physics->GetAngularVelocity().y, physics->GetAngularVelocity().z);

			physics->PropagateImpulse(id, contactInfo.c.point, pushImpulse);	

			DM_LOG(LC_MOVEMENT, LT_INFO)LOGSTRING("Kicking obstacle %s, velocity AFTER is %f,%f,%f\r", pushEnt->name.c_str(), 
				physics->GetLinearVelocity().x, physics->GetLinearVelocity().y, physics->GetLinearVelocity().z);
			DM_LOG(LC_MOVEMENT, LT_INFO)LOGSTRING("Kicking obstacle %s, angular velocity AFTER is %f,%f,%f\r", pushEnt->name.c_str(), 
				physics->GetAngularVelocity().x, physics->GetAngularVelocity().y, physics->GetAngularVelocity().z);
		}
	}
	// The pushed entity is considered heavy
	else if (pushEnt == lastPushEnt.GetEntity())
	{
		int pushTime = gameLocal.time - startPushTime;
		//gameRenderWorld->DrawText( idStr(pushTime), physics->GetAbsBounds().GetCenter(), 0.1f, colorWhite, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec );

		int pushStartDelay = cv_pm_push_start_delay.GetInteger();

		// If we've been pushing long enough, start moving the obstacle
		if (pushTime > pushStartDelay)
		{
			// greebo: Scale the velocity during the acceleration phase
			float accelScale = idMath::ClampFloat(0, 1, (pushTime - pushStartDelay)/cv_pm_push_accel_time.GetFloat());

			// Let the entity itself modify its pushing behaviour
			float entityScale = pushEnt->spawnArgs.GetFloat("push_mod", "1");

			// Scale the movement velocity according to the object's mass
			// At maxPushMass, the velocity is zero, at the minimum push mass threshold below it's about 0.75
			float maxPushableMass = entityScale*cv_pm_push_max_mass.GetFloat();
			float massScale = idMath::ClampFloat(0.0f, 1.0f, 1.0f - (mass / maxPushableMass));

			// Project the impactVelocity onto the contact normal
			idVec3 pushVelocity = (impactVelocity * -contactInfo.c.normal) * (-contactInfo.c.normal);

			//gameRenderWorld->DebugArrow( colorRed, physics->GetAbsBounds().GetCenter(), physics->GetAbsBounds().GetCenter() + pushVelocity, 1, gameLocal.msec );

			// Finally, apply a maximum cap, based on the player's normal walkspeed
			float velocity = idMath::ClampFloat(0, pm_walkspeed.GetFloat()*0.8f, pushVelocity.NormalizeFast());

			//gameRenderWorld->DrawText( idStr(velocity * accelScale * massScale), physics->GetAbsBounds().GetCenter(), 0.1f, colorWhite, gameLocal.GetLocalPlayer()->viewAngles.ToMat3(), 1, gameLocal.msec );

			// Apply the mass scale and the acceleration scale to the capped velocity
			physics->SetLinearVelocity(pushVelocity * velocity * accelScale * massScale * entityScale);

			DM_LOG(LC_MOVEMENT, LT_INFO)LOGSTRING("Pushing obstacle %s\r", pushEnt->name.c_str());

			isPushing = true;

			// Update the pushed status if this entity is a moveable
			if (pushEnt->IsType(idMoveable::Type))
			{
				// Pass the pushDirection to the moveable
				static_cast<idMoveable*>(pushEnt)->SetIsPushed(true, impactVelocity);
			}
		}
	}

	// Update the owning actor's push state if it has changed
	SetOwnerIsPushing(isPushing);

	// Remember the last push entity
	lastPushEnt = pushEnt;

	// Clear the push entity again
	pushEnt = NULL;
}

void CForcePush::SetOwnerIsPushing(bool isPushing)
{
	// Update the owning actor's push state if it has changed
	if (owner->IsType(idActor::Type))
	{
		idActor* owningActor = static_cast<idActor*>(owner);

		if (owningActor->IsPushing() != isPushing) 
		{
			owningActor->SetIsPushing(isPushing);
		}
	}
}

void CForcePush::Save( idSaveGame *savefile ) const
{
	// Store the entity pointer behind the physics object
	savefile->WriteObject(pushEnt);
	lastPushEnt.Save(savefile);
	savefile->WriteInt(id);
	savefile->WriteTrace(contactInfo);
	savefile->WriteVec3(impactVelocity);
	savefile->WriteInt(startPushTime);
	savefile->WriteObject(owner);
}

void CForcePush::Restore( idRestoreGame *savefile )
{
	savefile->ReadObject(reinterpret_cast<idClass*&>(pushEnt));
	lastPushEnt.Restore(savefile);
	savefile->ReadInt(id);
	savefile->ReadTrace(contactInfo);
	savefile->ReadVec3(impactVelocity);
	savefile->ReadInt(startPushTime);
	savefile->ReadObject(reinterpret_cast<idClass*&>(owner));
}
