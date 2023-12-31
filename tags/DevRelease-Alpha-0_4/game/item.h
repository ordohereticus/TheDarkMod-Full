/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Source$
 * $Revision: 744 $
 * $Date: 2007-01-20 00:19:57 -0500 (Sat, 20 Jan 2007) $
 * $Author: sophisticatedzombie $
 *
 * $Log$
 * Revision 1.6  2007/01/20 05:19:57  sophisticatedzombie
 * Spawns an idAbsenceMarkerEntity when moved or put in an inventory.
 *
 * Revision 1.5  2006/08/14 01:07:02  ishtvan
 * fixed hide/show in idMoveableItem to disable the clipmodel
 *
 * Revision 1.4  2004/11/05 18:58:09  sparhawk
 * Moved frobcode to idEntity to make it available for all entities.
 *
 * Revision 1.3  2004/11/03 00:06:08  sparhawk
 * Frob highlight finished and working.
 *
 * Revision 1.2  2004/10/30 17:19:40  sparhawk
 * Frob highlight added.
 *
 * Revision 1.1.1.1  2004/10/30 15:52:31  sparhawk
 * Initial release
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_ITEM_H__
#define __GAME_ITEM_H__


/*
===============================================================================

  Items the player can pick up or use.

===============================================================================
*/

class idItem : public idEntity {
public:
	CLASS_PROTOTYPE( idItem );

							idItem();
	virtual					~idItem();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Spawn( void );
	void					GetAttributes( idDict &attributes );
	virtual bool			GiveToPlayer( idPlayer *player );
	virtual bool			Pickup( idPlayer *player );
	virtual void			Think( void );
	virtual void			Present();


	enum {
		EVENT_PICKUP = idEntity::EVENT_MAXEVENTS,
		EVENT_RESPAWN,
		EVENT_RESPAWNFX,
		EVENT_MAXEVENTS
	};

	virtual void			ClientPredictionThink( void );
	virtual bool			ClientReceiveEvent( int event, int time, const idBitMsg &msg );

	// networking
	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );

	// TDM: SZ: UpdateVisuals is overridden to check if the object moved in case
	// we have to spawn an absence entity
	virtual void			UpdateVisuals( void );

	// TDM: SZ: Spawns an absence marker entity that advertises, at the objects orgOrigin, 
	// that the object is missing.
	virtual bool spawnAbsenceMarkerEntity();

	// TDM: SZ: Destroys the absence marker entity tied to this object. If there is none,
	// nothing is done
	virtual void destroyAbsenceMarkerEntity();

	// This indicates which team owns the item (or thinks it does :P )
	int ownerTeam;

private:
	idVec3					orgOrigin;
	bool					spin;
	bool					pulse;
	bool					canPickUp;

	// This should scale from 0.0 (none) to 1.0 (hard to miss)
	float					noticeabilityIfAbsent;


	// Has the original origin been set?
	bool					b_orgOriginSet;
	idEntityPtr<idEntity>	absenceEntityPtr;

	// for item pulse effect
	int						itemShellHandle;
	const idMaterial *		shellMaterial;

	// used to update the item pulse effect
	mutable bool			inView;
	mutable int				inViewTime;
	mutable int				lastCycle;
	mutable int				lastRenderViewTime;

	bool					UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView );
	static bool				ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView );

	void					Event_DropToFloor( void );
	void					Event_Touch( idEntity *other, trace_t *trace );
	void					Event_Trigger( idEntity *activator );
	void					Event_Respawn( void );
	void					Event_RespawnFx( void );
};

class idItemPowerup : public idItem {
public:
	CLASS_PROTOTYPE( idItemPowerup );

							idItemPowerup();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Spawn();
	virtual bool			GiveToPlayer( idPlayer *player );

private:
	int						time;
	int						type;
};

class idObjective : public idItem {
public:
	CLASS_PROTOTYPE( idObjective );

							idObjective();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Spawn();

private:
	idVec3					playerPos;

	void					Event_Trigger( idEntity *activator );
	void					Event_HideObjective( idEntity *e );
	void					Event_GetPlayerPos();
	void					Event_CamShot();
};

class idVideoCDItem : public idItem {
public:
	CLASS_PROTOTYPE( idVideoCDItem );

	void					Spawn();
	virtual bool			GiveToPlayer( idPlayer *player );
};

class idPDAItem : public idItem {
public:
	CLASS_PROTOTYPE( idPDAItem );

	virtual bool			GiveToPlayer( idPlayer *player );
};

class idMoveableItem : public idItem {
public:
	CLASS_PROTOTYPE( idMoveableItem );

							idMoveableItem();
	virtual					~idMoveableItem();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Spawn( void );
	virtual void			Think( void );
	virtual bool			Pickup( idPlayer *player );

	static void				DropItems( idAnimatedEntity *ent, const char *type, idList<idEntity *> *list );
	static idEntity	*		DropItem( const char *classname, const idVec3 &origin, const idMat3 &axis, const idVec3 &velocity, int activateDelay, int removeDelay );

	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );

protected:
	void					Hide();
	void					Show();

private:
	idPhysics_RigidBody		physicsObj;
	idClipModel *			trigger;
	const idDeclParticle *	smoke;
	int						smokeTime;

	void					Gib( const idVec3 &dir, const char *damageDefName );

	void					Event_DropToFloor( void );
	void					Event_Gib( const char *damageDefName );
};

class idMoveablePDAItem : public idMoveableItem {
public:
	CLASS_PROTOTYPE( idMoveablePDAItem );

	virtual bool			GiveToPlayer( idPlayer *player );
};

/*
===============================================================================

  Item removers.

===============================================================================
*/

class idItemRemover : public idEntity {
public:
	CLASS_PROTOTYPE( idItemRemover );

	void					Spawn();
	void					RemoveItem( idPlayer *player );

private:
	void					Event_Trigger( idEntity *activator );
};

class idObjectiveComplete : public idItemRemover {
public:
	CLASS_PROTOTYPE( idObjectiveComplete );

							idObjectiveComplete();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Spawn();

private:
	idVec3					playerPos;

	void					Event_Trigger( idEntity *activator );
	void					Event_HideObjective( idEntity *e );
	void					Event_GetPlayerPos();
};

#endif /* !__GAME_ITEM_H__ */
