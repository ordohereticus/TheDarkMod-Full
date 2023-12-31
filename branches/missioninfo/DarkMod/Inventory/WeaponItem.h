/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 3126 $
 * $Date: 2009-01-08 00:22:17 -0500 (Thu, 08 Jan 2009) $
 * $Author: greebo $
 *
 ***************************************************************************/
#ifndef __DARKMOD_INVENTORYWEAPONITEM_H__
#define __DARKMOD_INVENTORYWEAPONITEM_H__

#include "Item.h"

/**
 * WeaponInventoryItem is an item that belongs to a group. This item represents
 * a weapon entityDef and provides some methods to manage the weapon's ammo.
 */
class CInventoryWeaponItem :
	public CInventoryItem
{
protected:
	// The name of the weapon entityDef
	idStr	m_WeaponDefName;

	// The unique name for this weapon, e.g. "broadhead" or "blackjack"
	idStr	m_WeaponName;

	// The name of the projectile def name for this weapon (can change during runtime). 
	// Can be empty for melee weapons.
	idStr	m_ProjectileDefName;

	// The maximum amount of ammo for this weapon
	int		m_MaxAmmo;

	// The current amount of ammonition (set to getStartAmmo() in constructor)
	int		m_Ammo;

	// The index of this weapon [0..INF)
	int		m_WeaponIndex;

	// TRUE, if this weapon doesn't need ammo (like shortsword, blackjack)
	bool	m_AllowedEmpty;

	// TRUE if toggling this weapon is allowed (i.e. selecting it when it is already selected)
	bool	m_IsToggleable;

	// TRUE if this weapon can be selected.
	bool	m_Enabled;

public:
	// Default constructor, should only be used during restoring from savegames
	CInventoryWeaponItem();

	CInventoryWeaponItem(const idStr& weaponDefName, idEntity* owner);

	virtual void	Save( idSaveGame *savefile ) const;
	virtual void	Restore(idRestoreGame *savefile);

	// TRUE if this weapon is enabled
	bool IsEnabled() const;
	void SetEnabled(bool enabled);

	// Retrieves the maximum amount of ammo this weapon can hold
	int GetMaxAmmo();
	// Retrives the amount of ammo at player spawn time
	int GetStartAmmo();

	// Returns TRUE if this weapon doesn't need ammo and therefore can be selected 
	bool IsAllowedEmpty() const;

	// Returns TRUE if this weapon is toggleable
	bool IsToggleable() const;

	// Returns the currently available ammonition
	int GetAmmo() const;
	// Sets the new ammonition value (is automatically clamped to [0,maxAmmo])
	void SetAmmo(int newAmount);

	/**
 	 * This is used to check whether a weapon can "fire". This is always "1" for 
	 * weapons without ammo (sword, blackjack). For all other weapons, the ammo amount
	 * is returned.
	 */
	int HasAmmo();

	/**
	 * "Uses" a certain <amount> of ammo. This decreases the current ammo counter
	 * by the given value. Only affects the ammo count of weapons that actually need ammo.
	 */
	void UseAmmo(int amount);

	// Sets/Returns the weapon index (corresponds to the keyboard number keys used to access the weapons)
	void SetWeaponIndex(int index);
	int  GetWeaponIndex() const;

	/**
	 * greebo: Returns the name of the weapon, as derived from the weaponDef name.
	 *         entityDef "weapon_broadhead" => weapon name: "broadhead"
	 */
	const idStr& GetWeaponName() const;

	/**
	 * greebo: Returns the name of the projectile entityDef, if this weapon fires projectiles in the first place.
	 * Melee weapons will return an empty string.
	 */
	const idStr& GetProjectileDefName() const;
	void SetProjectileDefName(const idStr& weaponDefName);

	// Restores the projectile def name as originally defined in the weapon def
	void ResetProjectileDefName();

	// Returns the name of the weapon entityDef
	const idStr& GetWeaponDefName() const;
};
typedef boost::shared_ptr<CInventoryWeaponItem> CInventoryWeaponItemPtr;

#endif /* __DARKMOD_INVENTORYWEAPONITEM_H__ */
