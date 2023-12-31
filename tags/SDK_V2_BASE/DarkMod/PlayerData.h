/******************************************************************************/
/*                                                                            */
/*         DarkModGlobals (C) by Gerhard W. Gruber in Germany 2004            */
/*                          All rights reserved                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************
 *
 * PROJECT: DarkMod
 * $Source$
 * $Revision: 43 $
 * $Date: 2004-11-24 16:59:06 -0500 (Wed, 24 Nov 2004) $
 * $Author: sparhawk $
 * $Name$
 *
 * $Log$
 * Revision 1.3  2004/11/24 21:59:06  sparhawk
 * *) Multifrob implemented
 * *) Usage of items against other items implemented.
 * *) Basic Inventory system added.
 *
 * Revision 1.2  2004/11/03 21:47:17  sparhawk
 * Changed debug LogString for better performance and group settings
 *
 * Revision 1.1  2004/10/31 20:03:36  sparhawk
 * CDarkMod created to contain relevant player data seperate from id code.
 *
 *
 * DESCRIPTION: This file data which is relevant to the player. Examples
 * are: pointer to the inventory, currently highlighted entity and others.
 *
 *****************************************************************************/

#ifndef PLAYERDATA_H
#define PLAYERDATA_H

/**
 * CInventoryItem is a metaclass for storing entities in the inventory.
 * There are several properties for various items which are not directly
 * stored in the item itself, because they are independent from the
 * entity and only related to the inventory.
 */
class CInventoryItem {
public:
	CInventoryItem(void);

public:
	/**
	 * The entity that is associated with this inventory entry.
	 */
	idEntity	*m_Entity;

	/**
	 * Id is a key that allows an entity to be identified as the
	 * same object. For example: All broadhead arrows should have
	 * the same id, all fire arrows should have the same diffrent id
	 * and so on. If keys have the same id, it means that they are 
	 * treated as being the same key.
	 */
	idStr		m_Id;

	/**
	 * Value contains the vlaue of this entity in case it is a loot
	 * item.
	 */
	long		m_Value;

	/**
	 * Number of items if this is an item that can be stored
	 * repeatedly. Usually this will be weapons but can be others as well.
	 */
	long		m_Count;
};

/**
 * CDarkModPlayer is a class that maintains player data. The purpose of this
 * this class is mainly to be indenependent from idPlayer and seperate the code
 * from id's code.
 * Player data will store all additional data that is required like inventory,
 * special player states, currently highlighted entity and others.
 */
class CDarkModPlayer {
public:
	CDarkModPlayer(void);
	~CDarkModPlayer(void);

	/**
	 * FrobEntity is NULL when no entity is highlighted. Otherwise it will point 
	 * to the entity which is currently highlighted.
	 */
	idEntity	*m_FrobEntity;

	/**
	 * Adds an entity to the inventory. If this entity is a single one it is simply
	 * added. If it can be stored multiple times, the appropiate slot is searched and
	 * the counter is increased.
	 */
	void					AddEntity(idEntity *);

	/**
	 * Removes an entry from the inventory. If it is a single entity the item is
	 * removed from the list, otherwise the counter is decreased until the counter
	 * reaches zero which causes the item to be removed from the list.
	 */
	void					RemoveEntity(idEntity *);
	void					RemoveEntity(long);

	/**
	 * Find an entity in the list and return the index.
	 */
	long					GetEntity(idEntity *);

	/**
	 * Return the entity at the given index.
	 */
	idEntity				*GetEntity(long);

	/**
	 * Select the next/previous entry in the inventory. The selected entity
	 * is the one that is currently displayed and is also used in case the USE
	 * key is pressed.
	 */
	void					SelectNext(void);
	void					SelectPrev(void);

public:
	/**
	 * Selection points to the entity in the list that is currently selected
	 * from the inventory.
	 * If no item is selected it is 0. This means that the first entry of
	 * the list will be a dummy that can be selected but does nothing.
	 */
	long						m_Selection;
	idList<CInventoryItem>	m_Inventory;
};

#endif
