/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2848 $
 * $Date: 2008-09-15 02:44:08 -0400 (Mon, 15 Sep 2008) $
 * $Author: ishtvan $
 *
 ***************************************************************************/

#ifndef __MELEEWEAPON_H__
#define __MELEEWEAPON_H__

#include "../game/entity.h"

class idPlayer;

/**
* Class for melee weapons (and shields) that are usually attached to an AI.
* Handles activating and deactivating the clipmodels for attacks and parries.
* Handles setting correct contents on the clipmodels.
* Handles collisions between weapons and actors, between weapons and parries, 
* between weapons and world, and between weapons and the item that
* the player is currently holding in the Grabber.
**/

typedef enum
{
	MELEETYPE_OVERHEAD = BIT(0),
	MELEETYPE_SLASH_LR = BIT(1), // slashing (attacker's) left to right
	MELEETYPE_SLASH_RL = BIT(2), // slashing (attacker's) right to left
	MELEETYPE_THRUST = BIT(3),
	MELEETYPE_UNBLOCKABLE = BIT(4) // e.g., animal claws
} EMeleeType;

class CMeleeWeapon : public idMoveable 
{
public:
	CLASS_PROTOTYPE( CMeleeWeapon );

							CMeleeWeapon( void );
							virtual ~CMeleeWeapon( void );

	/**
	* Think loop calls idMoveable::Think, then checks collisions for attacks with bound weapons
	**/
	virtual void			Think( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	/**
	* Get, set or clear the actor that owns this weapon
	**/
	idActor *GetOwner( void ) { return m_Owner.GetEntity(); };
	void SetOwner( idActor *actor );
	void ClearOwner( void );

	/**
	* Activate an attack and test it for collisions from this point on
	* ActOwner is the actor that owns this weapon, AttName the string name of the attack,
	**/
	void ActivateAttack( idActor *ActOwner, const char *AttName );
	/**
	* Disables the attack (stops testing for collisions and clears any custom clipmodel)
	**/
	void DeactivateAttack( void );

	/**
	* Set up the clipmodel, clipmask and stims for a parry
	* ActOwner is the actor that owns this weapon, ParryName the string name of the parry
	**/
	void ActivateParry( idActor *ActOwner, const char *ParryName );

	/**
	* Disables the parry clipmodel and clears vars
	**/
	void DeactivateParry( void );

	/**
	* Called when weapon has just been deflected by a parry or the world
	**/
	void Deflected( idEntity *other );

	/**
	* Called when we hit a damagable and have to damage it
	**/
	void DealDamage( /* going to need a lot of args here */ );

protected:
	/**
	* Clears the extra melee clipmodel if it exists,
	* resets member vars
	**/
	void ClearClipModel( void );

	/**
	* Test to see if a melee attack in progress collides with
	* anything that should stop it.
	**/
	void CheckAttack( idVec3 OldOrigin, idMat3 OldAxis ); 

	/**
	* Parry success test, called when a melee attack hits a melee parry
	* Also handles deactivating/bouncing the attack if parry succeeded
	* 
	* POTENTIAL PROBLEM: What happens for fake cases like thrust parry on slash attack, how does
	* it let an attack thru if it fails, without always returning itself in the trace?
	* One solution would be to temporarily disable the parry for a moment, but what if the thrust
	* attack was not going to hit distance-wise, and another slash attack came in a little later?
	* Would modeling just the very tip of a thrust attack help?
	*
	* Maybe a better solution is to modify the attack in progress so that
	* it no longer collides with parries once it "gets past" an invalid parry
	**/
	void TestParry( CMeleeWeapon *other, idVec3 dir, trace_t *trace );

	/**
	* Called to handle physics, particle FX, sound, damage
	* When a melee attack hits something
	* Location is the JOINT HANDLE that was hit
	* If we directly hit an AF, not necessarily equal to  CLIPMODEL_ID_FOR_JOINT_HANDLE(tr.c.id)
	**/
	void MeleeCollision( idEntity *other, idVec3 dir, trace_t *trace, int location );

	/**
	* Set up the clipmodel for the current parry/attack action
	* Expects m_ActionName, m_bAttacking and m_bParrying to be already set.
	**/
	void SetupClipModel( void );

	/**
	* Return the type of attack or parry.  Used to test whether a parry will
	* work against a given attack.
	**/
	EMeleeType GetMeleeType( void ) { return m_MeleeType; };

protected:
	/**
	* Actor that owns this melee weapon.  Most likely also the bindmaster.
	**/
	idEntityPtr<idActor>	m_Owner;

	/**
	* Modified clipmodel for the weapon
	* Can optionally be used when either parrying or attacking
	* If we use the actual weapon clipmodel, this is set to NULL
	**/
	idClipModel				*m_WeapClip;

	/**
	* Whether we are actively parrying or attacking
	**/
	bool					m_bAttacking;
	bool					m_bParrying;

	/**
	* Name of the attack or parry (e.g., "slash_lr")
	**/
	idStr					m_ActionName;

	/**
	* Whether the modified clipmodel is axis-aligned or moves with joint
	**/
	bool					m_bClipAxAlign;

	/**
	* Whether this attack is set to collide with world/solids or not
	* (Typically player attacks collide with solids, AI's only collide with melee parries)
	**/
	bool					m_bWorldCollide;

	/**
	* Whether we're using a modified clipmodel or our own entity clipmodel
	**/
	bool					m_bModCM;

	/**
	* Type of melee action for parry tests
	* Slash, overhand, thrust, or unblockable
	**/
	EMeleeType				m_MeleeType;

	/**
	* Origin and axis from the previous frame
	**/
	idVec3					m_OldOrigin;
	idMat3					m_OldAxis;

	/**
	* Whether the parry stops being effective after a successful parry
	* (Applies to player weapon parries)
	**/
	bool					m_bParryStopOnSuccess;

// Cosmetic stuff:

	/**
	* Keep swinging when we hit moveables at or below this mass
	**/
	float					m_StopMass;

	/**
	* Number of particles we've generated in this attack
	**/
	int						m_ParticlesMade;

};

#endif /* !__MELEEWEAPON_H__ */
