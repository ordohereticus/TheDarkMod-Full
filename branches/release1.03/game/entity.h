// vim:ts=4:sw=4:cindent
/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $HeadURL$
 * $Revision: 4261 $
 * $Date: 2010-10-20 17:20:42 -0400 (Wed, 20 Oct 2010) $
 * $Author: grayman $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_ENTITY_H__
#define __GAME_ENTITY_H__

#include "../DarkMod/StimResponse/StimType.h"
#include "../DarkMod/overlaySys.h"
#include "../DarkMod/UserManager.h"
#include "../DarkMod/ModelGenerator.h"

class CStimResponseCollection;
class CStim;
typedef boost::shared_ptr<CStim> CStimPtr;
class CResponse;
typedef boost::shared_ptr<CResponse> CResponsePtr;
class CAbsenceMarker;
class CObjectiveLocation;

class CInventory;
typedef boost::shared_ptr<CInventory> CInventoryPtr;
class CInventoryItem;
typedef boost::shared_ptr<CInventoryItem> CInventoryItemPtr;
class CInventoryCursor;
typedef boost::shared_ptr<CInventoryCursor> CInventoryCursorPtr;

/**
* SDK_SIGNALS are used to signal either from a script to the SDK code or
* inside the SDK code itself. It can not signal from SDK to a script. In order
* to do this, you can use the regular signal mechanism from id.
*/
#define SDK_SIGNAL			int
typedef enum {
	SIG_REMOVE,
	SIG_CONTINUE,
	SIG_COUNT
} E_SDK_SIGNAL_STATE;

typedef struct {
	SDK_SIGNAL	m_Id;
	bool		m_Signaled;
	idEntity	*m_Object;
	void		*m_Data;
	E_SDK_SIGNAL_STATE	(*m_Fkt)(idEntity *oObject, void *pData);
} SDKSignalInfo;

/**
* This struct defines one entity with an optional offset, count and
* probability, to spawn it upon the death of another entity.
*/
struct FlinderSpawn {
	idStr		m_Entity;		//!< class of the entity to spawn
	idVec3		m_Offset;		//!< optional offset
	int			m_Count;		//!< count (default: 1)
	float		m_Probability;	//!< probability (0 .. 1.0) that this entity spawns
};

/*
===============================================================================

	Game entity base class.

===============================================================================
*/

static const int DELAY_DORMANT_TIME = 3000;
/**
* greebo: This is the value that has to be exceeded by the lightingQuotient
*		  returned by the LAS routines (range 0.0 ... 1.0).
*		  0.0 means no threshold - target entities are always visible
*		  This is used by the idEntity::canSeeEntity() member method.
*/
static const float VISIBILTIY_LIGHTING_THRESHOLD = 0.2f; 

extern const idEventDef EV_PostSpawn;
extern const idEventDef EV_FindTargets;
extern const idEventDef EV_RemoveTarget;
extern const idEventDef EV_AddTarget;
extern const idEventDef EV_Touch;
extern const idEventDef EV_Use;
extern const idEventDef EV_Activate;
extern const idEventDef EV_ActivateTargets;
extern const idEventDef EV_Hide;
extern const idEventDef EV_Show;
extern const idEventDef EV_GetShaderParm;
extern const idEventDef EV_SetShaderParm;
extern const idEventDef EV_SetOwner;
extern const idEventDef EV_GetAngles;
extern const idEventDef EV_SetAngles;
extern const idEventDef EV_SetLinearVelocity;
extern const idEventDef EV_SetAngularVelocity;
extern const idEventDef EV_SetSkin;
extern const idEventDef EV_StartSoundShader;
extern const idEventDef EV_StopSound;
extern const idEventDef EV_CacheSoundShader;
extern const idEventDef EV_ExtinguishLights;
extern const idEventDef EV_TeleportTo;
extern const idEventDef EV_IsDroppable;
extern const idEventDef EV_SetDroppable;

extern const idEventDef EV_IsType;

#ifdef MOD_WATERPHYSICS

extern const idEventDef EV_GetMass;				// MOD_WATERPHYSICS

extern const idEventDef EV_IsInLiquid;			// MOD_WATERPHYSICS

#endif		// MOD_WATERPHYSICS

extern const idEventDef EV_CopyBind;
extern const idEventDef EV_IsFrobable;
extern const idEventDef EV_SetFrobable;
extern const idEventDef EV_FrobHilight;
extern const idEventDef EV_InPVS;
extern const idEventDef EV_CanSeeEntity;
extern const idEventDef EV_CanBeUsedBy;

// greebo: Script event definition for dealing damage
extern const idEventDef EV_Damage;
extern const idEventDef EV_Heal;

extern const idEventDef EV_ResponseAllow;


// Think flags
enum {
	TH_ALL					= -1,
	TH_THINK				= 1,		// run think function each frame
	TH_PHYSICS				= 2,		// run physics each frame
	TH_ANIMATE				= 4,		// update animation each frame
	TH_UPDATEVISUALS		= 8,		// update renderEntity
	TH_UPDATEPARTICLES		= 16
};

// The impulse states a button can have
enum EImpulseState {
	EPressed,			// just pressed
	ERepeat,			// held down
	EReleased,			// just released
	ENumImpulseStates,
};

//
// Signals
// make sure to change script/doom_defs.script if you add any, or change their order
//
typedef enum {
	SIG_TOUCH,				// object was touched
	SIG_USE,				// object was used
	SIG_TRIGGER,			// object was activated
	SIG_REMOVED,			// object was removed from the game
	SIG_DAMAGE,				// object was damaged
	SIG_BLOCKED,			// object was blocked

	SIG_MOVER_POS1,			// mover at position 1 (door closed)
	SIG_MOVER_POS2,			// mover at position 2 (door open)
	SIG_MOVER_1TO2,			// mover changing from position 1 to 2
	SIG_MOVER_2TO1,			// mover changing from position 2 to 1

	NUM_SIGNALS
} signalNum_t;

// FIXME: At some point we may want to just limit it to one thread per signal, but
// for now, I'm allowing multiple threads.  We should reevaluate this later in the project
#define MAX_SIGNAL_THREADS 16		// probably overkill, but idList uses a granularity of 16

struct signal_t {
	int					threadnum;
	const function_t	*function;
};

class signalList_t {
public:
	idList<signal_t> signal[ NUM_SIGNALS ];
};

// Inventory-related flags.
enum {
	EINV_GROUP	= 1, // Jump to the next/previous group.
	EINV_PREV	= 2, // Iterate backwards.
	EINV_FAST	= 4, // Don't use group histories.
};

/**
* Attachment system info
**/
class CAttachInfo 
{
public:
	CAttachInfo() :
		channel(0),
		savedContents(-1)
	{}

	void Save( idSaveGame *savefile ) const;
	void Restore( idRestoreGame *savefile );

	idEntityPtr<idEntity>	ent;
	// Anim channel this attachment is on (if on an animated entity)
	int						channel;
	// unique name by which the entity refers to this attachment
	idStr					name; 

	// An additional int to save contents (used to set attachments nonsolid temporarily)
	int						savedContents;
};

/**
* Info structure for attaching things to named positions
**/
typedef struct SAttachPosition_s
{
	idStr			name;			//!< name of this position
	jointHandle_t	joint;		 	//!< joint it is relative to
	idAngles		angleOffset;	//!< rotational offset relative to joint orientation
	idVec3			originOffset;	//!< origin offset relative to joint origin

	void			Save( idSaveGame *savefile ) const;
	void			Restore( idRestoreGame *savefile );
} SAttachPosition;


class idEntity : public idClass {
public:
	static const int		MAX_PVS_AREAS = 4;

	int						entityNumber;			// index into the entity list
	int						entityDefNumber;		// index into the entity def list

	idLinkList<idEntity>	spawnNode;				// for being linked into spawnedEntities list
	idLinkList<idEntity>	activeNode;				// for being linked into activeEntities list

	idLinkList<idEntity>	snapshotNode;			// for being linked into snapshotEntities list
	int						snapshotSequence;		// last snapshot this entity was in
	int						snapshotBits;			// number of bits this entity occupied in the last snapshot

	idStr					name;					// name of entity
	idDict					spawnArgs;				// key/value pairs used to spawn and initialize entity
	idScriptObject			scriptObject;			// contains all script defined data for this entity

	int						thinkFlags;				// TH_? flags
	int						dormantStart;			// time that the entity was first closed off from player
	bool					cinematic;				// during cinematics, entity will only think if cinematic is set

	renderView_t *			renderView;				// for camera views from this entity
	idEntity *				cameraTarget;			// any remoteRenderMap shaders will use this

	idList< idEntityPtr<idEntity> >	targets;		// when this entity is activated these entities entity are activated

	int						health;					// FIXME: do all objects really need health?
	int						maxHealth;				// greebo: Moved this from idInventory to here

	struct entityFlags_s {
		bool				notarget			:1;	// if true never attack or target this entity
		bool				noknockback			:1;	// if true no knockback from hits
		bool				takedamage			:1;	// if true this entity can be damaged
		bool				hidden				:1;	// if true this entity is not visible
		bool				bindOrientated		:1;	// if true both the master orientation is used for binding
		bool				solidForTeam		:1;	// if true this entity is considered solid when a physics team mate pushes entities
		bool				forcePhysicsUpdate	:1;	// if true always update from the physics whether the object moved or not
		bool				selected			:1;	// if true the entity is selected for editing
		bool				neverDormant		:1;	// if true the entity never goes dormant
		bool				isDormant			:1;	// if true the entity is dormant
		bool				hasAwakened			:1;	// before a monster has been awakened the first time, use full PVS for dormant instead of area-connected
		bool				networkSync			:1; // if true the entity is synchronized over the network
	} fl;

	/**
	* When an entity is hidden, these store the following information just before the hide:
	* These store the clipmodel contents, clipmask and whether the clipmodel was enabled
	* These variables are reset to this data when the entity is un-hidden
	**/
	int					m_preHideContents;
	int					m_preHideClipMask;
	/**
	* Entity contents may be overwritten by a custom contents spawnarg
	* Store it here to keep track of it, because derived classes can
	* set the contents in their spawn method, and need to know to overwrite it
	**/
	int					m_CustomContents;

	/**
	 * UsedBy ist the list of entity names that this entity can be used by.
	 * i.e. A door can have a list of keys that can unlock it. A fountain can
	 * be used by a water arrow to make it holy, etc.
	 * The list is initialized by the key "used"_by" in the entity and contains
	 * the names of the entities seperated by ';'. If the first character of
	 * the name is a '*' the name donates a classname instead of an entity. In
	 * this case all entities of that class can be used by this entity.
	 * Example: If you have several holy fountains you would have to list all
	 * fountain names. Alternatively you can create a "holy_fountain" class and
	 * list it like this:
	 * "used_by" "*holy_fountain;*holy_bottle;holy_cross"
	 * In this example the entity can be used by any holy_fountain, any holy_bottle
	 * and addtionaly by the entity named holy_cross.
	 */
	idList<idStr>			m_UsedByName;
	/**
	* Same, but checks against inv_names
	**/
	idList<idStr>			m_UsedByInvName;
	/**
	* Same, but checks against inv_categories
	**/
	idList<idStr>			m_UsedByCategory;
	/**
	* Same, but checks against the entityDef name (e.g. atdm:playertools_lockpick*)
	* This may be redundant sometimes, because classnames are often a part of the 
	* entity name.  But not always, mappers can rename entities whatever they want
	**/
	idList<idStr>			m_UsedByClassname;

	/**
	* Set to true if objective locations should update the objectives system when
	* this entity the objective area.
	* May also determine inventory callbacks to objective system.
	**/
	bool					m_bIsObjective;

	/**
	 * greebo: The list of objective locations whose bounds this entity is currently within.
	 * This is needed to raise a safe mission event when this entity is destroyed, otherwise
	 * the objective location entity is unable to catch entity destructions.
	 */
	idList<idEntityPtr<CObjectiveLocation> > m_objLocations;

	/**
	* Set to true if the entity is frobable.  May be dynamically changed.
	**/
	bool					m_bFrobable;

	/**
	* Set to true if the entity is a "simple" frobable like a door, lever, button
	* These items can still be frobbed while doing things like shouldering bodies
	**/
	bool					m_bFrobSimple;

	/**
	* Frobdistance determines the distance in renderunits. If set to 0
	* the entity is not frobable.
	**/
	int						m_FrobDistance;

	/**
	* Frob bias: Multiplies the angle deviation cosine test in idPlayer::FrobCheck
	* Effectively biases the frob selection toward this object.  Used for small objects
	* to make them easier to frob when placed next to large objects.
	**/
	float					m_FrobBias;

	/**
	* Controls whether this entity, when attached to an AI, will become solid when the AI
	* is alerted and nonsolid when the AI is not alerted.
	* Often applied in conjunction with attach_set_nonsolid spawnarg
	**/
	bool					m_bAttachedAlertControlsSolidity;

	/**
	* Set to true if this entity is a climbable rope.  This could be set on either
	* static ropes or AF entities, so it makes sense to let all entities have this var
	**/
	bool					m_bIsClimbableRope;

	/**
	* TDM: The default playback rate of each animation, indexed by the animation's
	* index number. Only used by idActor; but the animation classes deal with idEntity
	* directly, and it's more efficient to declare this on all entities than to do
	* RTTI on every animation frame.
	**/
	idList<float>			m_animRates;

	/**
	* Actor who set this item in motion.  Cleared when it comes to rest.
	* Always NULL for non-physical items.
	**/
	idEntityPtr<idActor>	m_SetInMotionByActor;
	/**
	* Actor who last moved this item.  Never cleared.
	* Always NULL for non-physical items.
	**/
	idEntityPtr<idActor>	m_MovedByActor;

	// The light quotient for this entity, calculated by the LAS
	float					m_LightQuotient;
	// The last time the above value has been calculated
	int						m_LightQuotientLastEvalTime;

	/**
	* Tels: Contains (sharable, constant) LOD data if non-NULL.
	*/
	lod_data_t*				m_LOD;

	/**
	* Tels: Info for LOD, per entity. used if m_LOD != NULL:
	* Timestamp for next LOD think.
	**/
	int						m_DistCheckTimeStamp;

	/**
	* Current LOD (0 - normal, 1,2,3,4,5 LOD, 6 hidden)
	**/
	int						m_LODLevel;

	/* Store the current model and skin to avoid flicker by not
	*  switching from one model/skin to the same model/skin when
	*  changing the LOD.
	*/
	int						m_ModelLODCur;
	int						m_SkinLODCur;

public:
	ABSTRACT_PROTOTYPE( idEntity );

	idEntity();
	virtual ~idEntity();

	void					Spawn( void );

	/**
	* Load and check the visual and collision model, as well as any model for
	* the broken state.
	*/
	virtual void			LoadModels( void );

	/**
	* Parse the LOD spawnargs and fill the m_LOD struct  if nec.
	* Returns true if this entity (or the passed in entity def) has LOD.
	*/
	bool					ParseLODSpawnargs( const idDict* dict, const float fRandom);

	/**
	 * Tels: Stop LOD changes. If doTeam is true, also disables it on teammembers.
	 */
	void					StopLOD( const bool doTeam);

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	/**
	 * Tels: Save/Restore optional LOD data.
	 */
	void					SaveLOD( idSaveGame *savefile ) const;
	void					RestoreLOD( idRestoreGame *savefile );

	const char *			GetEntityDefName( void ) const;
	void					SetName( const char *name );
	const char *			GetName( void ) const;

	virtual void			UpdateChangeableSpawnArgs( const idDict *source );

							// clients generate views based on all the player specific options,
							// cameras have custom code, and everything else just uses the axis orientation
	virtual renderView_t *	GetRenderView();

	// greebo: Call this to trigger this entity
	virtual void			Activate(idEntity* activator);

	// thinking
	virtual void			Think( void );

	// Tels: If LOD is enabled on this entity, compute new LOD level and new alpha value.
	// We pass in a pointer to the data (so the LODE can use shared data) as well as the distance,
	// so the lode can pre-compute the distance.
	virtual float			ThinkAboutLOD( const lod_data_t* lod_data, const float deltaSq );

	// Tels: If LOD is enabled on this entity, call ThinkAboutLOD, computing new LOD level and new
	// alpha value, then do the right things like Hide/Show, SetAlpha, switch models/skin etc.
	// We pass in a pointer to the data (so the LODE can use shared data) as well as the distance,
	// so the lode can pre-compute the distance.
	virtual	bool			SwitchLOD( const lod_data_t *m_LOD, const float deltaSq );

	bool					CheckDormant( void );	//!< dormant == on the active list, but out of PVS
	virtual	void			DormantBegin( void );	//!< called when entity becomes dormant
	virtual	void			DormantEnd( void );		//!< called when entity wakes from being dormant
	bool					IsActive( void ) const;
	void					BecomeActive( int flags );
	void					BecomeInactive( int flags );
	void					BecomeBroken( idEntity *activator );	//!< Entity breaks up
	void					UpdatePVSAreas( const idVec3 &pos );

	// visuals
	virtual void			Present( void );
	virtual renderEntity_t *GetRenderEntity( void );
	virtual int				GetModelDefHandle( void );
	virtual void			SetModel( const char *modelname );
	void					SetSkin( const idDeclSkin *skin );
	const idDeclSkin *		GetSkin( void ) const;
	void					SetShaderParm( int parmnum, float value );
	virtual void			SetColor( const float red, const float green, const float blue );
	virtual void			SetColor( const idVec3 &color );
	virtual void			GetColor( idVec3 &out ) const;
	virtual void			SetColor( const idVec4 &color );
	virtual void			GetColor( idVec4 &out ) const;
	virtual void			FreeModelDef( void );
	virtual void			FreeLightDef( void );
	virtual void			Hide( void );
	virtual void			Show( void );
	bool					IsHidden( void ) const;

	/**
	 * Tels: Only set the alpha channel to fade in/out
	 */
	virtual void			SetAlpha( const float alpha );
	/**
	 * Tels: Same as SetAlpha(); but set bound children, too, if doTeam is true
	 */
	virtual void			SetAlpha( const float alpha, const bool doTeam );

	/**
	 * greebo: Returns the light quotient for this entity, a value determined by the 
	 * number of lights hitting this entity's bounding box, determined by the LAS.
	 *
	 * AI are using this method to determine an object's visibility. The result 
	 * is cached, to save it from being recalculated for multiple AI each frame.
	 */
	float					GetLightQuotient();

	// TDM: SZ: January 9, 2006 Made virtual to handle unique behavior in descendents
	virtual void			UpdateVisuals( void );

	void					UpdateModel( void );
	void					UpdateModelTransform( void );
	virtual void			ProjectOverlay( const idVec3 &origin, const idVec3 &dir, float size, const char *material );
	int						GetNumPVSAreas( void );
	const int *				GetPVSAreas( void );
	void					ClearPVSAreas( void );
	bool					PhysicsTeamInPVS( pvsHandle_t pvsHandle );

	// animation
	virtual bool			UpdateAnimationControllers( void );
	bool					UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView );
	static bool				ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView );
	virtual idAnimator *	GetAnimator( void );	// returns animator object used by this entity

	// sound
	virtual bool			CanPlayChatterSounds( void ) const;

	// angua: propVolMod only modifies the volume of the propagated sound
	bool					StartSound( const char *soundName, const s_channelType channel, int soundShaderFlags, bool broadcast, int *length, float propVolMod = 0);
	bool					StartSoundShader( const idSoundShader *shader, const s_channelType channel, int soundShaderFlags, bool broadcast, int *length);
	void					StopSound( const s_channelType channel, bool broadcast );	// pass SND_CHANNEL_ANY to stop all sounds
	void					SetSoundVolume( float volume );
	void					UpdateSound( void );
	int						GetListenerId( void ) const;
	idSoundEmitter *		GetSoundEmitter( void ) const;
	void					FreeSoundEmitter( bool immediate );

	// entity binding
	virtual void			PreBind( void );
	virtual void			PostBind( void );
	virtual void			PreUnbind( void );
	virtual void			PostUnbind( void );
	void					JoinTeam( idEntity *teammember );
	
	/** 
	 * greebo: Returns the first team entity matching the given type. If the second
	 * argument is specified, it returns the next entity after that one.
	 *
	 * @returns: NULL if nothing found, the entity pointer otherwise. The entity pointer can be
	 * safely static_cast<> onto the given type, as the type check has already been performed in this function.
	 */
	idEntity*				FindMatchingTeamEntity(const idTypeInfo& type, idEntity* lastMatch = NULL);

	void					Bind( idEntity *master, bool orientated );
	void					BindToJoint( idEntity *master, const char *jointname, bool orientated );
	void					BindToJoint( idEntity *master, jointHandle_t jointnum, bool orientated );
	void					BindToBody( idEntity *master, int bodyId, bool orientated );
	void					Unbind( void );
	bool					IsBound( void ) const;
	bool					IsBroken( void ) const;					//!< return true if model was broken
	bool					IsBoundTo( idEntity *master ) const;
	idEntity *				GetBindMaster( void ) const;
	jointHandle_t			GetBindJoint( void ) const;
	int						GetBindBody( void ) const;
	idEntity *				GetTeamMaster( void ) const;
	idEntity *				GetNextTeamEntity( void ) const;
	/**
	* TDM: Get a list of bound team members lower down on the chain than this one
	* Populates the list in the argument argument with entity pointers
	**/
	void					GetTeamChildren( idList<idEntity *> *list );
	void					ConvertLocalToWorldTransform( idVec3 &offset, idMat3 &axis );
	idVec3					GetLocalVector( const idVec3 &vec ) const;
	idVec3					GetLocalCoordinates( const idVec3 &vec ) const;
	idVec3					GetWorldVector( const idVec3 &vec ) const;
	idVec3					GetWorldCoordinates( const idVec3 &vec ) const;
	bool					GetMasterPosition( idVec3 &masterOrigin, idMat3 &masterAxis ) const;
	void					GetWorldVelocities( idVec3 &linearVelocity, idVec3 &angularVelocity ) const;
	int						GetModelDefHandle(void) const  { return modelDefHandle; }; 
	// physics
							// set a new physics object to be used by this entity
	void					SetPhysics( idPhysics *phys );
							// get the physics object used by this entity
	idPhysics *				GetPhysics( void ) const;
							// restore physics pointer for save games
	void					RestorePhysics( idPhysics *phys );
							// run the physics for this entity
	bool					RunPhysics( void );
							// set the origin of the physics object (relative to bindMaster if not NULL)
	void					SetOrigin( const idVec3 &org );
							// set the axis of the physics object (relative to bindMaster if not NULL)
	void					SetAxis( const idMat3 &axis );
							// use angles to set the axis of the physics object (relative to bindMaster if not NULL)
	void					SetAngles( const idAngles &ang );
							// get the floor position underneath the physics object
	bool					GetFloorPos( float max_dist, idVec3 &floorpos ) const;
							// retrieves the transformation going from the physics origin/axis to the visual origin/axis
	virtual bool			GetPhysicsToVisualTransform( idVec3 &origin, idMat3 &axis );
							// retrieves the transformation going from the physics origin/axis to the sound origin/axis
	virtual bool			GetPhysicsToSoundTransform( idVec3 &origin, idMat3 &axis );
							// called from the physics object when colliding, should return true if the physics simulation should stop
	virtual bool			Collide( const trace_t &collision, const idVec3 &velocity );
	/**
	* TDM: Process collision stims when collisions occur
	* Body is the AF body that was struck (defaults to -1)
	**/
	virtual void			ProcCollisionStims( idEntity *other, int body = -1 );
							// retrieves impact information, 'ent' is the entity retrieving the info
	virtual void			GetImpactInfo( idEntity *ent, int id, const idVec3 &point, impactInfo_t *info );
							// apply an impulse to the physics object, 'ent' is the entity applying the impulse
	virtual void			ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse );
							// add a force to the physics object, 'ent' is the entity adding the force
	virtual void			AddForce( idEntity *ent, int id, const idVec3 &point, const idVec3 &force );
							// activate the physics object, 'ent' is the entity activating this entity (and ignored in the code in entity.cpp...)
	virtual void			ActivatePhysics( idEntity *ent );
							// returns true if the physics object is at rest
	virtual bool			IsAtRest( void ) const;
							// returns the time the physics object came to rest
	virtual int				GetRestStartTime( void ) const;
							// add a contact entity
	virtual void			AddContactEntity( idEntity *ent );
							// remove a touching entity
	virtual void			RemoveContactEntity( idEntity *ent );

	// damage
							// returns true if this entity can be damaged from the given origin
	virtual bool			CanDamage( const idVec3 &origin, idVec3 &damagePoint ) const;
							// applies damage to this entity
	virtual	void			Damage( idEntity *inflictor, idEntity *attacker, 
									const idVec3 &dir, const char *damageDefName, 
									const float damageScale, const int location, trace_t *tr = NULL );
	// greebo: Heals this entity using the information found in the entitDef named healDefName
	// returns 1 if the entity was healed, or 0 if the entity was already at full health
	virtual int			heal(const char* healDefName, float healScale);
							// adds a damage effect like overlays, blood, sparks, debris etc.
	virtual void			AddDamageEffect( const trace_t &collision, const idVec3 &velocity, const char *damageDefName );
							// callback function for when another entity recieved damage from this entity.  damage can be adjusted and returned to the caller.
	virtual void			DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage );
							// notifies this entity that it is in pain
	virtual bool			Pain( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );
							// notifies this entity that is has been killed
	virtual void			Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );

	// scripting
	virtual bool			ShouldConstructScriptObjectAtSpawn( void ) const;
	virtual idThread *		ConstructScriptObject( void );
	virtual void			DeconstructScriptObject( void );
	void					SetSignal( signalNum_t signalnum, idThread *thread, const function_t *function );
	void					ClearSignal( idThread *thread, signalNum_t signalnum );
	void					ClearSignalThread( signalNum_t signalnum, idThread *thread );
	bool					HasSignal( signalNum_t signalnum ) const;
	void					Signal( signalNum_t signalnum );
	void					SignalEvent( idThread *thread, signalNum_t signalnum );

	// gui
	void					TriggerGuis( void );
	bool					HandleGuiCommands( idEntity *entityGui, const char *cmds );
	virtual bool			HandleSingleGuiCommand( idEntity *entityGui, idLexer *src );

	// targets
	void					FindTargets( void );
	void					RemoveNullTargets( void );
	void					ActivateTargets( idEntity *activator ) const;
	// greebo: Removes the given target ent from this entity's targets.
	virtual void			RemoveTarget(idEntity* target);
	virtual void			AddTarget(idEntity* target);


	// misc
	virtual void			Teleport( const idVec3 &origin, const idAngles &angles, idEntity *destination );
	bool					TouchTriggers( void ) const;
	idCurve_Spline<idVec3> *GetSpline( void ) const;
	virtual void			ShowEditingDialog( void );

	enum {
		EVENT_STARTSOUNDSHADER,
		EVENT_STOPSOUNDSHADER,
		EVENT_MAXEVENTS
	};

	virtual void			ClientPredictionThink( void );
	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );
	virtual bool			ServerReceiveEvent( int event, int time, const idBitMsg &msg );
	virtual bool			ClientReceiveEvent( int event, int time, const idBitMsg &msg );

	void					WriteBindToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadBindFromSnapshot( const idBitMsgDelta &msg );
	void					WriteColorToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadColorFromSnapshot( const idBitMsgDelta &msg );
	void					WriteGUIToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadGUIFromSnapshot( const idBitMsgDelta &msg );

	void					ServerSendEvent( int eventId, const idBitMsg *msg, bool saveEvent, int excludeClient ) const;
	void					ClientSendEvent( int eventId, const idBitMsg *msg ) const;

	// ---===<* Darkmod functions *>===---

	/**
	 * LoadTDMSettings will initialize the settings required for 
	 * darkmod to operate. It should be called from idEntity::Spawn(), 
	 * which will be valid for all entities. If a class needs
	 * to load additional settings, which are only of interest to this
	 * particular class it should override this virtual function but
	 * don't forget to call it for the base class.
	 */
	virtual void			LoadTDMSettings(void);

	/**
	 * Will evaluate one def_broken flinder and find out how many of this piece
	 * to spawn, then spawn them. Called from Flinderize().
	 * @return: Count of entities that where spawned or -1 for error.
	 *
	*/
	virtual int				SpawnFlinder(const FlinderSpawn& bs, idEntity *activator);

	/**
	 * Evaluate def_flinder spawnargs and call SpawnFlinder() for each found.
	*/
	virtual void			Flinderize (idEntity *activator);

	/**
	* Parses spawnarg list of attachments and puts them into the list.
	**/
	void ParseAttachmentSpawnargs( idList<idDict> *argsList, idDict *from );

	/**
	 * Frobaction will determine what a particular item should do when an entity is highlighted.
	 * The actual action depends on the type of the entity.
	 * Loot is being picked up and counted to the loot.
	 * Special items are transfered to the inventory. If the item is also loot it will count 
	 *    to that as well.
	 * Doors are tested for their state (locked/unlocked) and opened if unlocked. if the 
	 *    lockpicks or an appropriate key is equipped the door will either unlock or the
	 *    lockpick HUD should appear.
	 * Windows are just smaller doors so they don't need special treatment.
	 * AI is tested for its state. If it is an ally some defined script should start. If
	 *    it is unconscious or dead it is picked up and the player can carry it around.
	 * If it is a movable item the item is picked up and the player can carry it around.
	 * Switches are flipped and/or de-/activated and appropriate scripts are triggered.
	 *
	 * @frobMaster indicates whether the entity is allowed to call its master or not.
	 *
	 * @isFrobPeerAction: this is TRUE if this frob action was propagated from a peer.
	 */
	virtual void FrobAction(bool frobMaster, bool isFrobPeerAction = false);

	/**
	* Function that is called if the player holds down the frob button on this object
	* By default, does nothing
	**/
	virtual void FrobHeld(bool frobMaster, bool isFrobPeerAction = false, int holdTime = 0) {};

	/**
	* Function that is called if the player holds down the frob button on this object
	* By default, does nothing
	**/
	virtual void FrobReleased(bool frobMaster, bool isFrobPeerAction = false, int holdTime = 0) {};

	/**
	 * greebo: A frobbed entity might receive this signal if the player is hitting
	 * the attack button in this exact frame. (Used for lockpicking.)
	 *
	 * The player responsible for this action is passed as argument.
	 */
	virtual void AttackAction(idPlayer* player);

	/**
	 * greebo: Returns TRUE if the given inventory item matches this entity, i.e.
	 * if the entity can be used by the given inventory item, FALSE otherwise.
	 * Note: This just routes the call to the overloaded CanBeUsedBy(idEntity*);
	 *
	 * @isFrobUse: This is true if the Use action originated from a frob button hit.
	 */
	virtual bool CanBeUsedBy(const CInventoryItemPtr& item, const bool isFrobUse);

	/**
	 * greebo: Returns TRUE if the given entity matches this entity, i.e.
	 * if the entity can be used by the given entity, FALSE otherwise.
	 * If a frob_master was set, the call is redirected to that one.
	 *
	 * @isFrobUse: This is true if the Use action originated from a frob button hit.
	 *
	 * The standard entity returns FALSE, this method needs to be overridden 
	 * by the subclasses.
	 */
	virtual bool CanBeUsedBy(idEntity* entity, const bool isFrobUse);

	/**
	 * greebo: Uses this entity by the given inventory item. The button state
	 * is needed to handle the exact user interaction, e.g. while lockpicking.
	 * If a frob_master was set, the call is redirected to that one.
	 *
	 * @returns: TRUE if the item could be used, FALSE otherwise.
	 */
	virtual bool UseBy(EImpulseState nState, const CInventoryItemPtr& item);

	/**
	* Toggle whether the entity has been frobbed.  Should ONLY be called by idPlayer::PerformFrobCheck
	**/
	virtual void SetFrobbed( bool val );

	/**
	* Set whether this entity is frobable or not
	**/
	virtual void SetFrobable( bool val );

	/**
	* Return whether the entity is currently frobbed.
	* Should be false at the beginning of the frame
	**/
	virtual bool IsFrobbed( void );

	/**
	* DarkMod sound prop functions (called by StartSound and StopSound)
	**/

	/**
	* The following two functions propagate either a suspicious or
	* an environmental sound.  They are called by idEntity::StartSound.
	*
	* The parameters are the local sound name (ie, the name in the entity
	* def file, INCLUDING the "snd_" at the beginning), and the global sound
	* name (name in the soundprop def file, NOT including the prefix)
	*
	* (For now sounds can be suspicious or environmental, but not both.
	*  defining a sound as suspicious overrides any environmental def
	*  with the same sound name. )
	**/

	/**
	* Propagate a suspicious sound
	**/
	void PropSoundS( const char *localName, const char *globalName, float VolModIn = 0.0f );

	/**
	* Propagate an environmental sound (called by PropSound)
	**/
	void PropSoundE( const char *localName, const char *globalName, float VolModIn = 0.0f );

	/**
	* Propagate a sound directly, outside of StartSound
	* Direct calling should be done in cases where the gamecode calls
	* StartSoundShader directly, without going thru StartSound.
	* 
	* PropSoundDirect first looks for a local definition of the sound
	* on the entity, to find volume/duration modifier, extra flags
	* and the "global" sound definition (in the sound def file)
	*
	* If the local definition is not found, it calls sound prop
	* with the unmodified global definition.
	*
	* VolModIn is a modifier in dB added to the volume, in addition to 
	* any modifier that might be present in the local sound def.
	**/
	void PropSoundDirect( const char *sndName, bool bForceLocal = false, 
						  bool bAssumeEnv = false, float VolModIn = 0.0f );

	CStimResponseCollection *GetStimResponseCollection(void) { return m_StimResponseColl; };

	/**
	 * greebo: This allows an entity to "relay" the response to another entity.
	 * The default implementation of this virtual function returns the <this> pointer,
	 * but subclasses may return any other entity (but not NULL!).
	 * 
	 * Example: idAFAttachments return their "body" entity as response entity.
	 */
	virtual idEntity* GetResponseEntity() { return this; };

	// Returns (and creates if necessary) this entity's inventory.
	const CInventoryPtr&		Inventory();
	// Returns (and creates if necessary) this entity's inventory cursor.
	const CInventoryCursorPtr&	InventoryCursor();

	/**
	 * greebo: This is called when an objective location adds this entity
	 * to its list and starts tracking it.
	 */
	void OnAddToLocationEntity(CObjectiveLocation* locationEnt);

	/**
	 * greebo: This is called when an objective location removes this entity
	 * from its internal list and stops tracking it.
	 */
	void OnRemoveFromLocationEntity(CObjectiveLocation* locationEnt);

	/**
	 * greebo: This event gets fired right after spawn time. It checks the spawnargs
	 *         and adds itself to someone else's inventory on demand. If the target entity
	 *         is not found, the event postpones itself by a few hundred msec.
	 *         The optional count argument takes care that this doesn't happen too often.
	 */
	void Event_InitInventory(int callCount = 0);

	/**
	 * CreateOverlay will create an overaly for the given entity. This is a hud element
	 * that displays some objects on the screen. The default is for the playerscreen
	 * to show the inventory items and other stats. However, each entity can have his own
	 * hud which allows him to add a custom display for inventory items.
	 */
	void					Event_CreateOverlay( const char *guiFile, int layer );
	int						CreateOverlay( const char *guiFile, int layer );

	/**
	* greebo: DestroyOverlay removes the overlay from this entity. Pass the handle
	*		  as argument, it's the <int> returned by CreateOverlay.
	*/
	void					Event_DestroyOverlay(int handle);
	void					DestroyOverlay(int handle);

	// Returns the GUI with the given handle or NULL if not found
	idUserInterface*		GetOverlay(int handle);

	/**
	 * Generic function for calling a scriptfunction with arbitrary arguments.
	 * The the thread is returned or NULL.
	 */
	idThread *CallScriptFunctionArgs(const char *ScriptFunction, bool ClearStack, int Delay, const char *Format, ...);

	/**
	* Attach another entity to this entity.  The base version assumes single-clipmodel
	* rigid body physics and simply calls idEntity::Bind.  
	* Will be overloaded in derived classes with joints to call BindToJoint.
	* The position argument is an optional named attachment position
	* Attachment name is the string name for future lookups, not actually used by idEntity
	* but used in derived classes.
	**/
	virtual void Attach( idEntity *ent, const char *PosName = NULL, const char *AttName = NULL );

	/**
	* Detach the attachment of that name.
	* Note that to preserve the name->index mapping, the entry for
	* the detached object stays in the m_Attachments list, but the entity
	* field gets set to NULL so that it's skipped over in various operations
	* NOTE: This could lead to memory hogging if something gets detached/attached
	*		many times.
	**/
	virtual void Detach( const char *AttName );

	/**
	* Detach by index in the m_Attachments array.  Otherwise same as above
	**/
	virtual void DetachInd( int ind );

	/**
	* Reattach an existing attachment to the given offset and angles relative
	* to the origin and orientatin of the entity
	*
	* A different version taking a joint argument exists on idAnimatedEntity
	* for things with MD5 structures
	**/
	virtual void ReAttachToCoords( const char *AttName, const idVec3 offset, const idAngles angles );

	/**
	* Reattach to a predefined attach position, otherwise same as above
	**/
	virtual void ReAttachToPos( const char *AttName, const char *PosName  );

	/**
	* Show or hide an attachment, by name or by attachment array index.
	**/
	void ShowAttachment( const char *AttName, bool bShow );
	void ShowAttachmentInd( int ind, bool bShow );

	/**
	* Returns an entity pointer for a given attachment name.
	* Returns NULL if no such named attachment exists on this entity.
	**/
	virtual idEntity *GetAttachment( const char *AttName );

	/**
	* Returns an entity pointer for a given index of the attachment array.
	* Returns NULL if no such named attachment exists directly on this entity.
	**/
	virtual idEntity *GetAttachment( int ind );

	/**
	* Helper function that looks up the attachment index from the name->index map
	* Returns -1 if the index is not present in the map or if it's out of bounds
	* of the m_Attachments array.
	**/
	virtual int	GetAttachmentIndex( const char *AttName );

	/**
	* Returns an entity pointer for a given attachment name.
	* Returns NULL if no such named attachment exists on this entity,
	* or on any entity attached to this entity.
	**/
	virtual idEntity *GetAttachmentFromTeam( const char *AttName );


	/**
	* Returns a pointer to the attachment info structure for a given attachment.
	* Returns NULL if no such named attachment exists on this entity.
	**/
	virtual CAttachInfo *GetAttachInfo( const char *AttName );

	/**
	* Returns a pointer to the attachment position with this name. 
	* Returns NULL if no attachment position exists with this name.
	**/
	virtual SAttachPosition *GetAttachPosition( const char *AttachName );

	/**
	* Store the attachment info in the argument references given.
	* Returns false if the attachment index was invalid.
	**/
	bool PrintAttachInfo( int ind, idStr &joint, idVec3 &offset, idAngles &angles );
	
	/**
	* Called when the given entity is about to attach (bind) to this entity.
	* Does not actually bind it.
	**/
	virtual void BindNotify( idEntity *ent );
	
	/**
	* Called when the given entity is about to detach (unbind) from this entity.
	* Does not actually unbind it.
	**/
	virtual void UnbindNotify( idEntity *ent );

	/**
	 * GetSignalId assignes a unique Id to be used in a signal function. To differentiate these
	 * from the script signals I named them SDKSignal. These signal mechanism can be used for both.
	 * So you can have a SDK function signaling, or a script. You can not signal a script, though, only
	 * from a signal to SDK. To signal a script use the regular signal functions.
	 */
	SDK_SIGNAL			GetSDKSignalId(void);

	/**
	 * Adds a new signal funktion that can be triggered. The object pointer must always be set. 
	 * The datapointer can be NULL and would contain a pointer to data that the signal function
	 * may need. If the SIGNAL is 0 an error occured and the signal was not added to the list.
	 */
	SDK_SIGNAL		AddSDKSignal(E_SDK_SIGNAL_STATE (*oFkt)(idEntity *oObject, void *pData), void *pData);
	void			CheckSDKSignal(void);

	/**
	 * SDKSignal will trigger a signal that a script has finished.
	 */
	void SDKSignal(SDK_SIGNAL SDKSignalId, int bState);

	/**
	 * Return nonzero if this entity could potentially attack the given (target) entity at range,
	 * or entities in general if target is NULL.
	 */
	virtual float			RangedThreatTo(idEntity* target);

	/**
	 * AddToInventory will add an entity to the inventory. The item is only
	 * added if the appropriate spawnargs are set, otherwise it will be rejected
	 * and NULL is returned.
	 */
	virtual CInventoryItemPtr AddToInventory(idEntity *ent);

	virtual void NextInventoryItem();	// Cycles to the next item in the inventory.
	virtual void PrevInventoryItem();	// Cycles to the previous item in the inventory.
	virtual void NextInventoryGroup();	// Cycles to the next group in the inventory.
	virtual void PrevInventoryGroup();	// Cycles to the previous group in the inventory.

	/**
	 * greebo: This cycles through a specific inventory group (=category). 
	 * If the cursor is already pointing to an item in the given group, it is advanced to the next item. 
	 * If the cursor has reached the "end" of the group, it is set back to the first item. 
	 * If the cursor is currently not pointing to an item of the given group, it is set to the
	 * first item in that group.
	 */
	virtual void CycleInventoryGroup(const idStr& groupName);

	// Gets called when the inventory cursor has changed its selection focus
	virtual void OnInventorySelectionChanged(const CInventoryItemPtr& prevItem = CInventoryItemPtr());

	/**
	 * greebo: Gets called when anything within the inventory has been altered (item count, item icon)
	 * This does NOT get called when the selection changes, only the inventory's contents.
	 */
	virtual void OnInventoryItemChanged();

	/**
	 * Changes the inventory count of the given item <name> in <category> by <amount>
	 */
	void ChangeInventoryItemCount(const char* invName, const char* invCategory, int amount);

	/**
	 * Changes the amount of the given loot type in the inventory of this entity.
	 * @returns: the new amount of the affected loot type.
	 */
	int ChangeLootAmount(int lootType, int amount);

	/**
	 * greebo: This replaces the inventory item (represented by oldItem) with the
	 * given <newItem> entity. <newItem> needs to be a valid inventory item entity or NULL.
	 *
	 * <oldItem> is removed from the inventory and <newItem> will take its position, provided
	 * both items share the same category name. If the categories are different, the position
	 * can not be guaranteed to be the same after the operation.
	 *
	 * If <newItem> is NULL, <oldItem> will just be removed and no replacement takes place.
	 *
	 * @returns TRUE on success, FALSE otherwise.
	 */
	bool ReplaceInventoryItem(idEntity* oldItem, idEntity* newItem);

	/**
	 * Script event: Changes the lightgem modifier value of the given item <name> in <category> to <icon>
	 */
	void ChangeInventoryIcon(const char* invName, const char* invCategory, const char* icon);

	/**
	 * Script event: Changes the lightgem modifier value of the given item <name> in <category> to <value>
	 */
	void ChangeInventoryLightgemModifier(const char* invName, const char* invCategory, int value);

	/**
	 * Return true if this entity can be mantled, false otherwise.
	 */
	virtual bool			IsMantleable();

	// Accessors for the frob peer list
	virtual void			AddFrobPeer(const idStr& frobPeerName);
	virtual void			AddFrobPeer(idEntity* peer);
	virtual void			RemoveFrobPeer(const idStr& frobPeerName);
	virtual void			RemoveFrobPeer(idEntity* peer);

	// Returns the frob master for this entity or NULL if not set (or not existing anymore).
	idEntity*				GetFrobMaster();

	inline UserManager&		GetUserManager()
	{
		return m_userManager;
	}

	// Stim/Response methods
	CStimPtr				AddStim(StimType type, float radius = 0.0f, bool removable = true, bool isDefault = false);
	CResponsePtr			AddResponse(StimType type, bool removable = true, bool isDefault = false);

	/**
	 * RemoveStim/Response removes the given stim. If the entity has no 
	 * stims/responses left, it is also removed from the global list in gameLocal.
	 */
	void					RemoveStim(StimType type);
	void					RemoveResponse(StimType type);

	void					IgnoreResponse(StimType type, idEntity* fromEntity);
	void					AllowResponse(StimType type, idEntity* fromEntity);

	void					EnableResponse(StimType type) { SetResponseEnabled(type, true); }
	void					DisableResponse(StimType type) { SetResponseEnabled(type, false); }

	// Enables / disables the given response
	void					SetResponseEnabled(StimType type, bool enabled);

	void					EnableStim(StimType type) { SetStimEnabled(type, true); }
	void					DisableStim(StimType type) { SetStimEnabled(type, false); }

	void					SetStimEnabled(StimType type, bool enabled);

	void					ClearStimIgnoreList(StimType type);

	/**
	 * This triggers a stand-alone response (without an actual Stim) on this entity.
	 *
	 * @source: The entity that caused the response.
	 * @stimType: the according stim index (e.g. ST_FROB)
	 */
	void					TriggerResponse(idEntity* source, StimType type);

	// greebo: Callback invoked by the response on incoming stims to trigger custom code
	// To be overridden by subclasses, default implementation is empty
	virtual void			OnStim(const CStimPtr& stim, idEntity* stimSource) 
	{ }

	/**
	 * greebo: Script events to get/set the team of this entity.
	 */
	void					Event_GetTeam();
	void					Event_SetTeam(int newTeam);

	void					Event_IsEnemy( idEntity *ent );
	void					Event_IsFriend( idEntity *ent );
	void					Event_IsNeutral( idEntity *ent );

	void					Event_SetEntityRelation (idEntity* entity, int relation);
	void					Event_ChangeEntityRelation(idEntity* entity, int relationChange);


	int						team;

	// angua: entities can have personal relationships to other entities that are used instead of the team relations.
	typedef std::map<const idEntity*, int> EntityRelationsMap;
	EntityRelationsMap m_EntityRelations;

	// angua: set a relation to another entity
	// this can be friendly (>0), neutral(0) or hostile (<0)
	void SetEntityRelation(idEntity* entity, int relation);

	// angua: this changes the current relation to an entity by adding the new amount
	void ChangeEntityRelation(idEntity* entity, int relationChange);

	/**
	* Checks with the global Relationship Manager and the personal relations to see if the
	* other entity is an enemy of this entity.
	**/
	bool IsEnemy(const idEntity *other);
	// As above, but checks for Friend
	bool IsFriend(const idEntity *other);
	// As above, but checks for Neutral
	bool IsNeutral(const idEntity *other);


	float					GetAbsenceNoticeability();

	// angua: this checks whether an entity has been removed or returned to the original position
	// and spawns or destroys an absence marker
	void					Event_CheckAbsence();
	bool					SpawnAbsenceMarker();
	bool					DestroyAbsenceMarker();

	// Saves the CONTENTS values of all attachments
	void SaveAttachmentContents();

	// Sets all attachment contents to the given value
	void SetAttachmentContents(int newContents);

	// Restores all CONTENTS values, previously saved with SaveAttachmentContents()
	void RestoreAttachmentContents();

protected:
	/**
	* Update frob highlight state if frobbed.
	* Clears highlight state if it's no longer frobbed.
	* Called in idEntity::Present
	**/
	void UpdateFrobState();

	/**
	* This controls the shader parms for the frob highlighting,
	* according to the frob highlight state. Includes fading algorithms.
	* Called in idEntity::Present().
	**/
	void UpdateFrobDisplay();

	/**
	* Activate/deactivate frob highlighting on an entity
	* Also calls this on any of the entity's peers
	**/
	void SetFrobHighlightState( bool bVal );

	/**
	* Call ParseAttachmentSpawnargs, spawns the attachements and binds them on to the entity.
	**/
	virtual void ParseAttachments( void );

	/**
	* Parse attachment positions from the spawnargs
	**/
	virtual void ParseAttachPositions( void );

	/**
	* Bind to the same object that the "other" argument is bound to
	**/
	void					Event_CopyBind( idEntity *other );

	/**
	* Returns the bindmaster
	**/
	void					Event_GetBindMaster( void );

	/**
	* Return the number of bind team members lower in the chain than this one
	**/
	void					Event_NumBindChildren( void );
	/**
	* Return the ind_th bind team member
	**/
	void					Event_GetBindChild( int ind );

protected:
	renderEntity_t			renderEntity;				//!< used to present a model to the renderer
	int						modelDefHandle;				//!< handle to static renderer model
	refSound_t				refSound;					//!< used to present sound to the audio engine
	idStr					brokenModel;				//!< model set when health drops down to or below zero

	/**
	* List storing attachment data for each attachment
	**/
	idList<CAttachInfo>		m_Attachments;

	/**
	* Maps string name of an attachment to an index in m_Attachments
	**/
	typedef std::map<std::string, int>	AttNameMap;
	AttNameMap							m_AttNameMap;

	/**
	* List of predefined attachment positions for this entity
	* If the entity doesn't have joints, positions are relative to origin
	**/
	idList<SAttachPosition>	m_AttachPositions;

	/**
	* Used to keep track of the GUIs used by this entity.
	**/
	COverlaySys					m_overlays;

	/**
	* This is set by idPlayer if the player is looking at the entity in a way
	* that will frob it.
	**/
	bool						m_bFrobbed;

	/**
	* This is separate from m_bFrobbed due to peer frob highlighting,
	* to let an entity display the highlight when not frobbed.
	**/
	bool						m_bFrobHighlightState;

	/**
	* Timestamp indicating when the frob highlight last changed
	* Used for continuous fade in and fade out.
	**/
	int							m_FrobChangeTime;

	/**
	 * FrobActionScript will contain the name of the script that is to be
	 * exected whenever a frobaction occurs. The default should be set by
	 * the constructor of the respective derived class but can be overriden
	 * by the property "frob_action_script" in the entity definition file.
	 */
	idStr						m_FrobActionScript;

	/**
	 * FrobPeers lists the names of other entites that will also be
	 * frobbed when this entity is frobbed.
	 */
	idStrList					m_FrobPeers;

	/**
	 * This contains the name of the "master" entity. If this entity receives
	 * an incoming FrobAction or inventory item usage, the action is redirected
	 * to the frob master.
	 */
	idStr						m_MasterFrob;

	// Is set to TRUE once the frob action function is entered on this entity. Prevents stack overflows.
	bool						m_FrobActionLock;

	CStimResponseCollection		*m_StimResponseColl;

	SDK_SIGNAL					m_Signal;
	idList<SDKSignalInfo *>		m_SignalList;

	float						m_AbsenceNoticeability;
	idBounds					m_StartBounds;
	bool						m_AbsenceStatus;
	idEntityPtr<CAbsenceMarker>	m_AbsenceMarker;

	bool						m_bIsMantleable;

	/**
	* Set to true when entity becomes broken (via damage)
	**/
	bool						m_bIsBroken;

	/** Used to implement waitForRender()...
	 *	This merely contains a bounding box and a callback.
	 */
	renderEntity_t			m_renderTrigger;
	/// The render world handle for m_renderTrigger.
	int						m_renderTriggerHandle;
	///	The thread that's waiting for this entity to render. (via waitForRender())
	int						m_renderWaitingThread;

	/**	Called when m_renderTrigger is rendered.
	 *	It will resume the m_renderWaitingThread.
	 */
	static bool				WaitForRenderTriggered( renderEntity_s* renderEntity, const renderView_s* view );
	/**	Called to update m_renderTrigger after the render entity is modified.
	 *	Only updates the render trigger if a thread is waiting for it.
	 */
	void					PresentRenderTrigger();

	/**
	* Set and get whether the entity is frobable
	* Note: IsFrobable is only used by scripting, since we can check public var m_bFrobable
	**/
	void					Event_IsFrobable( void );
	void					Event_SetFrobable( bool bVal );
	void					Event_IsHilighted( void );

	// Frobs this entity.
	void					Event_Frob();
	// Frobhighlights this entity or turns it off
	void					Event_FrobHilight( bool bVal );

	// angua: List of actors that currently use this entity
	UserManager m_userManager;

private:
	idPhysics_Static	defaultPhysicsObj;		// default physics object
	idPhysics *			physics;				// physics used for this entity
	/**
	* TDM: Clipmodel for frobbing only (used to make frobbing easier without effecting physics)
	**/
	idClipModel *		m_FrobBox;
	idEntity *			bindMaster;				// entity bound to if unequal NULL
	jointHandle_t		bindJoint;				// joint bound to if unequal INVALID_JOINT
	int					bindBody;				// body bound to if unequal -1
	idEntity *			teamMaster;				// master of the physics team
	idEntity *			teamChain;				// next entity in physics team

	int					numPVSAreas;			// number of renderer areas the entity covers
	int					PVSAreas[MAX_PVS_AREAS];// numbers of the renderer areas the entity covers

	signalList_t *		signals;

	int					mpGUIState;				// local cache to avoid systematic SetStateInt

	// A pointer to our inventory.
	CInventoryPtr		m_Inventory;

	// A pointer to our cursor - the cursor is for arbitrary use, and may not point to our own inventory.
	CInventoryCursorPtr	m_InventoryCursor;

	// Information about the most recent voice and body sound requests. grayman - #2341
	idSoundShader *		previousVoiceShader;	// shader for the most recent voice request (safe because shader pointers are constant, being decls)
	int					previousVoiceIndex;		// index of most recent voice sound requested (1->N, where there are N sounds in the shader)
	idSoundShader *		previousBodyShader;		// shader for the most recent body request (safe because shader pointers are constant, being decls)
	int					previousBodyIndex;		// index of most recent body sound requested (1->N, where there are N sounds in the shader)
private:
	void				FixupLocalizedStrings();

	bool				DoDormantTests( void );	// dormant == on the active list, but out of PVS

	// physics
	// initialize the default physics
	void					InitDefaultPhysics( const idVec3 &origin, const idMat3 &axis );
	// update visual position from the physics
	void					UpdateFromPhysics( bool moveBack );

	// entity binding
	bool					InitBind( idEntity *master );	// initialize an entity binding
	void					FinishBind( void );				// finish an entity binding
	void					RemoveBinds( void );			// deletes any entities bound to this object
	void					QuitTeam( void );				// leave the current team

	void					UpdatePVSAreas( void );

public:			// Events should be public, so they can be used from other places as well.
	// events
	void					Event_GetName( void );
	void					Event_SetName( const char *name );
	void					Event_IsType ( const char *pstr_typeName );
	void					Event_FindTargets( void );
	void					Event_ActivateTargets( idEntity *activator );
	void					Event_AddTarget(idEntity* target);
	void					Event_RemoveTarget(idEntity* target);
	void					Event_NumTargets( void );
	void					Event_GetTarget( float index );
	void					Event_RandomTarget( const char *ignore );
	void					Event_Bind( idEntity *master );
	void					Event_BindPosition( idEntity *master );
	void					Event_BindToJoint( idEntity *master, const char *jointname, float orientated );
	void					Event_BindToBody( idEntity *master, int bodyId, bool orientated );
	void					Event_Unbind( void );
	void					Event_RemoveBinds( void );
	void					Event_SpawnBind( void );
	void					Event_SetOwner( idEntity *owner );
	void					Event_SetModel( const char *modelname );
	void					Event_SetSkin( const char *skinname );
	void					Event_GetShaderParm( int parmnum );
	void					Event_SetShaderParm( int parmnum, float value );
	void					Event_SetShaderParms( float parm0, float parm1, float parm2, float parm3 );
	void					Event_SetColor( float red, float green, float blue );
	void					Event_GetColor( void );
	void					Event_IsHidden( void );
	void					Event_Hide( void );
	void					Event_Show( void );
	void					Event_CacheSoundShader( const char *soundName );
	void					Event_StartSoundShader( const char *soundName, int channel );
	void					Event_StopSound( int channel, int netSync );
	void					Event_StartSound( const char *soundName, int channel, int netSync );
	void					Event_FadeSound( int channel, float to, float over );
	void					Event_SetSoundVolume( float to );
	void					Event_GetWorldOrigin( void );
	void					Event_SetWorldOrigin( idVec3 const &org );
	void					Event_GetOrigin( void );
	void					Event_SetOrigin( const idVec3 &org );
	void					Event_GetAngles( void );
	void					Event_SetAngles( const idAngles &ang );
	void					Event_SetLinearVelocity( const idVec3 &velocity );
	void					Event_GetLinearVelocity( void );
	void					Event_SetAngularVelocity( const idVec3 &velocity );
	void					Event_GetAngularVelocity( void );

	void					Event_SetContents(const int contents);
	void					Event_GetContents();
	void					Event_SetClipMask(const int clipMask);
	void					Event_GetClipMask();

	void					Event_SetSize( const idVec3 &mins, const idVec3 &maxs );
	void					Event_GetSize( void );
	void					Event_GetMins( void );
	void					Event_GetMaxs( void );
	void					Event_Touches( idEntity *ent );
	void					Event_GetNextKey( const char *prefix, const char *lastMatch );
	void					Event_SetKey( const char *key, const char *value );
	void					Event_GetKey( const char *key );
	void					Event_GetIntKey( const char *key );
	void					Event_GetFloatKey( const char *key );
	void					Event_GetVectorKey( const char *key );
	void					Event_GetEntityKey( const char *key );
	void					Event_RemoveKey( const char *key );
	void					Event_RestorePosition( void );
	void					Event_UpdateCameraTarget( void );
	void					Event_DistanceTo( idEntity *ent );
	void					Event_DistanceToPoint( const idVec3 &point );
	void					Event_StartFx( const char *fx );
	void					Event_WaitFrame( void );
	void					Event_Wait( float time );
	void					Event_HasFunction( const char *name );
	void					Event_CallFunction( const char *name );
    void					Event_CallGlobalFunction( const char *funcname, idEntity *ent );
	void					Event_SetNeverDormant( int enable );

	/**
	 * greebo: Extinguishes all lights in the teamchain, including self.
	 */
	void					Event_ExtinguishLights();

	void					Event_InPVS( void );
	void					Event_WaitForRender( void );

	/**
	* greebo: This is the method that can be invoked from scripts. It basically
	* passes the call to the member method Damage().
	*
	* This Deals damage to this entity (gets translated into the idEntity::Damage() method within the SDK).
	*
	* @inflictor: This is the entity causing the damage (maybe a projectile)
	* @attacker: This is the "parent" entity of the inflictor, the one that is responsible for the inflictor (can be the same)
	* @dir: The direction the attack is coming from.
	* @damageDefName: The name of the damage entityDef to know what damage is being dealt to <self> (e.g. "damage_lava")
	* @damageScale: The scale of the damage (pass 1.0 as default, this should be ok).
	*/
	void					Event_Damage( idEntity *inflictor, idEntity *attacker, 
										  const idVec3 &dir, const char *damageDefName, 
										  const float damageScale );
	/**
	* greebo: This method heals this entity using the values defined in the
	* the entityDef specified by <healDefName>.
	*/
	void					Event_Heal( const char *healDefName, const float healScale );
	void					Event_SetGui( int handle, const char *guiFile );
	void					Event_GetGui( int handle );
	void					Event_SetGuiString( int handle, const char *key, const char *val );
	void					Event_GetGuiString( int handle, const char *key );
	void					Event_SetGuiFloat( int handle, const char *key, float f );
	void					Event_GetGuiFloat( int handle, const char *key );
	void					Event_SetGuiInt( int handle, const char *key, int n );
	void					Event_GetGuiInt( int handle, const char *key );
	void					Event_SetGuiStringFromKey( int handle, const char *key, idEntity *src, const char *spawnArg );
	void					Event_CallGui( int handle, const char *namedEvent );

	/**
	* Tels: Teleport the entity to the given entity's origin and orientation.
	*/
	void					Event_TeleportTo(idEntity *target);
	/**
	* Get/set droppable on this entity and its inventory item if it's already in the inventory
	**/
	void					Event_IsDroppable();
	void					Event_SetDroppable( bool Droppable );
	/**
	* Tels: Return the sum of all lights in the entities PVS.
	*/
	void 					Event_GetLightInPVS( const float lightFalloff, const float lightDistScale);

	/**
	* Tels: Toggle the noShadow flag on this entity.
	*/
	void 					Event_noShadows( bool noShadow );


	void					Event_LoadExternalData( const char *xdFile, const char* prefix );

	void					Event_GetInventory();

	void					Event_GetLootAmount(int lootType);
	void					Event_ChangeLootAmount(int lootType, int amount);
	void					Event_AddInvItem(idEntity* ent);
	// Tels: The reverse of AddInvItem()
	void					Event_AddItemToInv(idEntity* ent);
	void					Event_ReplaceInvItem(idEntity* oldItem, idEntity* newItem);
	void					Event_GetNextInvItem();
	void					Event_GetPrevInvItem();
	void					Event_SetCurInvCategory(const char* categoryName);
	void					Event_SetCurInvItem(const char* itemName);
	void					Event_GetCurInvCategory();
	void					Event_GetCurInvItemEntity();
	void					Event_GetCurInvItemName();
	void					Event_GetCurInvItemId();
	void					Event_GetCurInvIcon();

	// tels: remove all bound entities that have "unbindonalertlevel" higher or equal than alertlevel
	void					RemoveBindsOnAlert( const int alertIndex );
	void					DetachOnAlert( const int alertIndex );

	// Stim/Response Script Event Interface
	void					Event_StimAdd(int stimType, float radius);
	void					Event_StimRemove(int stimType);
	void					Event_StimEnable(int stimType, int state);
	void					Event_StimClearIgnoreList(int stimType);
	void					Event_ResponseAdd(int stimType);
	void					Event_ResponseRemove(int stimType);
	void					Event_ResponseEnable(int stimType, int State);
	void					Event_ResponseTrigger(idEntity* source, int stimType);
	void					Event_ResponseIgnore(int stimType, idEntity*);
	void					Event_ResponseAllow(int stimType, idEntity*);
	void					Event_ResponseSetAction(int stimType, const char* action);
	void					Event_GetResponseEntity();

	void					Event_TimerCreate(int StimType, int Hour, int Minute, int Seconds, int Milisecond);
	void					Event_TimerStop(int StimType);
	void					Event_TimerStart(int StimType);
	void					Event_TimerRestart(int StimType);
	void					Event_TimerReset(int StimType);
	void					Event_TimerSetState(int StimType, int State);

	/**
	* Used to propagate a sound directly via scripting, without playing the audible sound
	* VolModIn is a volume modifier added to the sound's volume.
	**/
	void					Event_PropSoundMod( const char *sndName, float VolModIn = 0.0 );

	void					Event_PropSound( const char *sndName );

#ifdef MOD_WATERPHYSICS

	void					Event_GetMass( int body );	// MOD_WATERPHYSICS

	void					Event_IsInLiquid( void );	// MOD_WATERPHYSICS

#endif		// MOD_WATERPHYSICS
	
	void					Event_RangedThreatTo(idEntity*);

	/**
	* greebo: Returns true if the target entity can be seen 
	*
	* @useLighting: If set to TRUE, takes lighting into account.
	*/
	bool					canSeeEntity(idEntity* target, int useLighting);

	/**
	* greebo: script event wrapper for the above canSeeEntity() method.
	*/
	void					Event_CanSeeEntity(idEntity* target, int useLighting);

	/**
	* ishtvan: Let script acces CanBeUsedBy on this entity
	**/
	void					Event_CanBeUsedBy(idEntity *itemEnt);

public:			// events that need to have an accessible counterpart
	void					SetGuiString(int handle, const char *key, const char *val);
	const char				*GetGuiString(int handle, const char *key);
	void					SetGuiFloat(int handle, const char *key, float f);
	float					GetGuiFloat(int handle, const char *key);
	void					SetGuiInt(int handle, const char *key, int n);
	int						GetGuiInt(int handle, const char *key);
	void					CallGui(int handle, const char *namedEvent);
};

/*
===============================================================================

	Animated entity base class.
	
===============================================================================
*/

typedef struct damageEffect_s {
	jointHandle_t			jointNum;
	idVec3					localOrigin;
	idVec3					localNormal;
	int						time;
	const idDeclParticle*	type;
	struct damageEffect_s *	next;
} damageEffect_t;

class idAnimatedEntity : public idEntity 
{
public:
	CLASS_PROTOTYPE( idAnimatedEntity );

							idAnimatedEntity();
							~idAnimatedEntity();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Spawn( void ); //TDM: added so that we can cache anim rates

	virtual void			ClientPredictionThink( void );
	virtual void			Think( void );

	void					UpdateAnimation( void );

	virtual idAnimator *	GetAnimator( void );
	virtual void			SetModel( const char *modelname );

	bool					GetJointWorldTransform( jointHandle_t jointHandle, int currentTime, idVec3 &offset, idMat3 &axis );
	bool					GetJointTransformForAnim( jointHandle_t jointHandle, int animNum, int currentTime, idVec3 &offset, idMat3 &axis ) const;

	virtual int				GetDefaultSurfaceType( void ) const;
	virtual void			AddDamageEffect( const trace_t &collision, const idVec3 &velocity, const char *damageDefName );
	void					AddLocalDamageEffect( jointHandle_t jointNum, const idVec3 &localPoint, const idVec3 &localNormal, const idVec3 &localDir, const idDeclEntityDef *def, const idMaterial *collisionMaterial );
	void					UpdateDamageEffects( void );

	virtual bool			ClientReceiveEvent( int event, int time, const idBitMsg &msg );

	/**
	* Overloads idEntity::Attach to bind to a joint
	* AttName is the optional name of the attachment for indexing purposes (e.g., "melee_weapon")
	* Ent is the entity being attached
	* PosName is the optional position name to attach to.
	**/
	virtual void			Attach( idEntity *ent, const char *PosName = NULL, const char *AttName = NULL );

	/**
	* Reattach an existing attachment
	* 
	* The next arguments are the name of the joint to attach to, the translation
	* offset from that joint, and a (pitch, yaw, roll) angle vector that defines the 
	* rotation of the attachment relative to the joint's orientation.
	**/
	virtual void			ReAttachToCoords( const char *AttName, idStr jointName, idVec3 offset, idAngles angles );

	/**
	* Cache the animation rates from spawnargs
	* Always called at spawn, sometimes called later if spawnargs changed and rates need recaching
	**/
	virtual void			CacheAnimRates( void );


	/**
	* Tels: Return the position of the joint (by name) in world coordinates
	**/
	idVec3 				GetJointWorldPos( const char *jointname );

	/**
	* Tels: Return the entity closest to the given joint, excluding itself. AIUseType is something
	*	like "AIUSE_FOOD" and only entites that have the spawnarg "AIUse" "AIUSE_FOOD" will be
	*	considered. From all these entities, the one closest to the joint is returned, or NULL
	*	if no such entity could be found. Called from GetEntityClosestToJoint().
	**/
	idEntity* GetEntityFromClassClosestToJoint( const idVec3 joint_origin, const char* AIUseType, const float max_dist_sqr );

	/**
	* Tels: Return the entity closest to the given joint, excluding itself. entitySelector is either
	*	a direct entity name or something like "AIUSE_FOOD". First we look at all spawnargs with
	*	prefix "prefix", and if these all fail to produce an entity, fall back to entitySelector.
	*	Returns NULL if no suitable entity could be found.
	**/
	idEntity* GetEntityClosestToJoint( const char* jointname, const char* entitySelector, const char* prefix, const float max_dist );

	enum {
		EVENT_ADD_DAMAGE_EFFECT = idEntity::EVENT_MAXEVENTS,
		EVENT_MAXEVENTS
	};

protected:
	/**
	* Used internally by the Attach methods.
	* Offset and axis are filled with the correct offset and axis
	* for attaching to a particular joint.
	* Calls GetJointWorldTransform on idAnimated entities.
	**/
	virtual void GetAttachingTransform( jointHandle_t jointHandle, idVec3 &offset, idMat3 &axis );

protected:
	idAnimator				animator;
	damageEffect_t *		damageEffects;

private:
	void					Event_GetJointHandle( const char *jointname );
	void 					Event_ClearAllJoints( void );
	void 					Event_ClearJoint( jointHandle_t jointnum );
	void 					Event_SetJointPos( jointHandle_t jointnum, jointModTransform_t transform_type, const idVec3 &pos );
	void 					Event_SetJointAngle( jointHandle_t jointnum, jointModTransform_t transform_type, const idAngles &angles );
	void 					Event_GetJointPos( jointHandle_t jointnum );
	void 					Event_GetJointAngle( jointHandle_t jointnum );
};

#endif /* !__GAME_ENTITY_H__ */
